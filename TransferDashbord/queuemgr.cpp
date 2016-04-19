/**************************************************************************
**   UpCloudEcosphere
**   Copyright (c) UpCloud C/C++ development team.
**   All rights 2016 reserved.
**   Author : Lynzabo
**************************************************************************/

#include "queuemgr.h"

extern QReadWriteLock g_queuesLock;
QueueMgr* QueueMgr::m_instance = 0;

QueueMgr::QueueMgr():m_down(0), m_up(0),m_nCycle(0)
{
    m_instance = this;

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(doWork()), Qt::DirectConnection);
}

void QueueMgr::exit()
{
    delete m_timer;

    QReadLocker l(&g_queuesLock);
    foreach(Queue* q,g_queues)
    {
        q->lock();
        foreach(Transfer* d,q->m_transfers)
        {
            if(d->isActive())
                d->changeActive(false);
        }
        q->unlock();
    }
}

void QueueMgr::doMove(Queue *q, Transfer *t)
{
    QString whereTo = q->moveDirectory();
    if(whereTo.isEmpty() || t->primaryMode() != Transfer::Download)
        return;

    try
    {
        t->setObject(whereTo);
    }
    catch(const RuntimeException& e)
    {
        t->enterLogMessage("QueueMgr", tr("Failed to move the transfer's data: %1").arg(e.what()));
    }
}

void QueueMgr::doWork()
{
    int total[2] = { 0, 0 };
    g_queuesLock.lockForRead();

    const bool autoremove = false;

    foreach(Queue* q,g_queues)
    {
        int lim_down,lim_up;
        int down,up, active = 0;

        Queue::Stats stats;

        memset(&stats, 0, sizeof stats);

        q->transferLimits(lim_down,lim_up);
        q->speedLimits(down,up);
        q->updateGraph();

        q->lock();

        QList<int> stopList, resumeList;

        for(int i=0;i<q->m_transfers.size();i++)
        {
            Transfer* d = q->m_transfers[i];
            int downs,ups;
            Transfer::State state = d->state();
            Transfer::Mode mode = d->mode();

            d->updateGraph();
            d->speeds(downs,ups);

            if(downs >= 1024 && mode == Transfer::Download)
                d->m_bWorking = true;
            else if(ups >= 1024 && mode == Transfer::Upload)
                d->m_bWorking = true;

            stats.down += downs;
            stats.up += ups;

            if(state == Transfer::Waiting || state == Transfer::Active)
            {
                int* lim;

                if(mode == Transfer::Download || q->m_bUpAsDown)
                    lim = &lim_down;
                else
                    lim = &lim_up;

                if(*lim != 0)
                {
                    (*lim)--;
                    resumeList << i;
                }
                else
                    stopList << i;
            }
            else if(state == Transfer::Completed && autoremove)
            {
                doMove(q, d);
                q->remove(i--, true);
            }

            if(d->isActive())
            {
                ( (mode == Transfer::Download) ? stats.active_d : stats.active_u) ++;
                active++;
            }
            else if(d->state() == Transfer::Waiting)
                ( (mode == Transfer::Download) ? stats.waiting_d : stats.waiting_u) ++;
        }

        foreach(int x, stopList)
            q->m_transfers[x]->setState(Transfer::Waiting);
        foreach(int x, resumeList)
            q->m_transfers[x]->setState(Transfer::Active);

        total[0] += stats.down;
        total[1] += stats.up;

        int size = q->size();

        if(size && active)
        {
            float avgd, avgu, supd, supu;
            int curd, curu;

            q->autoLimits(curd, curu);

            if(!curd)
                curd = down/active;
            else if(down)
            {
                avgd = float(stats.down) / active;
                supd = float(down) / active;
                curd += (supd-avgd)/active;
            }
            else
                curd = 0;

            if(!curu)
                curu = up/active;
            else if(up)
            {
                avgu = float(stats.up) / active;
                supu = float(up) / active;
                //qDebug() << "avgu:" << avgu << "supu:" << supu << "->" << (supu-avgu)/active;
                curu += (supu-avgu)/active;
            }
            else
                curu = 0;

            if(curd)
            {
                if(curd < 1024)
                    curd = 1024;
                else if(curd < down/active)
                    curd = down/active;
            }
            if(curd > down)
                curd = down;
            if(curu)
            {
                if(curu < 1024)
                    curu = 1024;
                else if(curu < up/active)
                {
                    //qDebug() << "Normalizing to" << (up/active) << "active being" << active;
                    curu = up/active;
                }
            }
            if(curu > up)
                curu = up;

            //qDebug() << "Setting" << curu;
            q->setAutoLimits(curd, curu);
        }

        q->m_stats = stats;

        q->unlock();
    }

    g_queuesLock.unlock();

    m_down = total[0];
    m_up = total[1];

    if(++m_nCycle > 60)
    {
        m_nCycle = 0;
        Queue::saveQueuesAsync();
//        g_settings->sync();
    }
}

