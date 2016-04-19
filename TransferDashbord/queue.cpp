/**************************************************************************
**   UpCloudEcosphere
**   Copyright (c) UpCloud C/C++ development team.
**   All rights 2016 reserved.
**   Author : Lynzabo
**************************************************************************/

#include "queue.h"
#include "engines/placeholdertransfer.h"


//队列列表
QList<Queue*> g_queues;
//队列表列读写锁
QReadWriteLock g_queuesLock(QReadWriteLock::Recursive);
bool Queue::m_bLoaded = false;

Queue::Queue(QObject *parent) : QObject(parent),m_lock(QReadWriteLock::Recursive)
  ,m_nDownLimit(0),m_nUpLimit(0),m_nDownTransferLimit(1),m_nUpTransferLimit(1)
  ,m_bUpAsDown(false),m_nDownAuto(0), m_nUpAuto(0)
{
    m_uuid = QUuid::createUuid();
    m_strDefaultDirectory = QDir::homePath();
}

Queue::~Queue()
{
    QWriteLocker l(&m_lock);
    qDebug() << "Queue::~Queue()";
    qDeleteAll(m_transfers);
}

void Queue::stopQueues()
{
    QReadLocker l(&g_queuesLock);
    for(int i=0;i<g_queues.size();i++)
    {
        Queue* q = g_queues[i];

        q->lock();
        for(int j=0;j<q->size();j++)
        {
            Transfer* t = q->at(j);
            if(t->isActive())
                t->changeActive(false);
        }
        q->unlock();
    }
}

void Queue::loadQueues()
{
    QDomDocument doc;
    QFile file;
    QDir dir = QDir::home();

    dir.mkpath(".local/share/fastTransfer");
    if(!dir.cd(".local/share/fastTransfer"))
        return;
    //C:/Users/Lynzabo/.local/share/fastTransfer/queues.xml
    file.setFileName(dir.absoluteFilePath("queues.xml"));

    QString errmsg;
    if(!file.open(QIODevice::ReadOnly) || !doc.setContent(&file, false, &errmsg))
    {
        qDebug() << "Failed to open " << file.fileName();
        if(!errmsg.isEmpty())
            qDebug() << "PARSE ERROR!" << errmsg;

        // default queue for new users
        Queue* q = new Queue;
        q->setName(QObject::tr("Main queue"));
        g_queues << q;
    }
    else
    {
        g_queuesLock.lockForWrite();
        qDeleteAll(g_queues);

        qDebug() << "Loading queues";

        QDomElement n = doc.documentElement().firstChildElement("queue");
        while(!n.isNull())
        {
            if(!n.hasAttribute("name"))
                continue;
            else
            {
                Queue* pQueue = new Queue;

                pQueue->m_strName = n.attribute("name");
                pQueue->m_nDownLimit = n.attribute("downlimit").toInt();
                pQueue->m_nUpLimit = n.attribute("uplimit").toInt();
                pQueue->m_nDownTransferLimit = n.attribute("dtranslimit").toInt();
                pQueue->m_nUpTransferLimit = n.attribute("utranslimit").toInt();
                pQueue->m_bUpAsDown = n.attribute("upasdown").toInt() != 0;
                pQueue->m_uuid = QUuid( n.attribute("uuid", pQueue->m_uuid.toString()) );
                pQueue->m_strDefaultDirectory = n.attribute("defaultdir", pQueue->m_strDefaultDirectory);
                pQueue->m_strMoveDirectory = n.attribute("movedir");

                pQueue->loadQueue(n);
                g_queues << pQueue;
            }
            n = n.nextSiblingElement("queue");
        }

        g_queuesLock.unlock();
    }

    m_bLoaded = true;
}

void Queue::unloadQueues()
{
    qDebug() << "Queue::unloadQueues()";
    qDeleteAll(g_queues);
}

void Queue::saveQueuesAsync()
{
    BackgroundSaver* t = new BackgroundSaver;
    connect(t, SIGNAL(finished()), t, SLOT(deleteLater()));
    t->start();
}

void Queue::saveQueues()
{
    if (!m_bLoaded)
    {
        qDebug() << "Not saving queues as they haven't been loaded yet.";
        return;
    }

    QDomDocument doc;
    QDomElement root;
    QFile file;
    QDir dir = QDir::home();

    if(!dir.cd(".local/share/fastTransfer"))
        return;
    file.setFileName(dir.filePath("queues.xml.new"));

    if(!file.open(QIODevice::WriteOnly))
        return;

    root = doc.createElement("fatrat");
    doc.appendChild(root);

    g_queuesLock.lockForRead();

    foreach(Queue* q, g_queues)
    {
        QDomElement elem = doc.createElement("queue");
        elem.setAttribute("name",q->m_strName);
        elem.setAttribute("downlimit",QString::number(q->m_nDownLimit));
        elem.setAttribute("uplimit",QString::number(q->m_nUpLimit));
        elem.setAttribute("dtranslimit",QString::number(q->m_nDownTransferLimit));
        elem.setAttribute("utranslimit",QString::number(q->m_nUpTransferLimit));
        elem.setAttribute("upasdown",QString::number(q->m_bUpAsDown));
        elem.setAttribute("uuid",q->m_uuid.toString());
        elem.setAttribute("defaultdir",q->m_strDefaultDirectory);
        elem.setAttribute("movedir",q->m_strMoveDirectory);

        q->saveQueue(elem,doc);
        root.appendChild(elem);
    }

    g_queuesLock.unlock();
    if (file.write(doc.toByteArray()) == -1 || !file.flush())
        Logger::global()->enterLogMessage(tr("Queue"), tr("Failed to write the queue file!"));
    else {
        file.close();

        QByteArray path = (dir.path() + "/queues.xml.new").toUtf8();
        QByteArray dpath = (dir.path() + "/queues.xml").toUtf8();

        qDebug() << "Saving queue to" << dpath;
        rename(path.data(), dpath.data());
    }
}

void Queue::setName(QString name)
{
    QWriteLocker l(&m_lock);
    m_strName = name;
}

QString Queue::name() const
{
    QReadLocker l(&m_lock);
    return m_strName;
}

void Queue::setMoveDirectory(QString path)
{
    QWriteLocker l(&m_lock);
    m_strMoveDirectory = path;
}

QString Queue::moveDirectory() const
{
    QReadLocker l(&m_lock);
    return m_strMoveDirectory;
}

int Queue::size()
{
    //cout << "Queue size: " << m_transfers.size() << endl;
    return m_transfers.size();
}

Transfer *Queue::at(int r)
{
    if(r < 0 || r >= m_transfers.size())
        return 0;
    else
        return m_transfers[r];
}

void Queue::remove(int n, bool nolock)
{
    Transfer* d = take(n, nolock);

    if(d->isActive())
        d->setState(Transfer::Paused);
    d->deleteLater();
}

Transfer *Queue::take(int n, bool nolock)
{
    Transfer* d = 0;

    if(!nolock)
        m_lock.lockForWrite();
    if(n < size() && n >= 0)
        d = m_transfers.takeAt(n);
    if(!nolock)
        m_lock.unlock();

    return d;
}

void Queue::setAutoLimits(int down, int up)
{
    m_nDownAuto = down;
    m_nUpAuto = up;

    foreach(Transfer* d, m_transfers)
    {
        if(!d->isActive())
            continue;
        d->setInternalSpeedLimits(down, up);
    }
}

void Queue::loadQueue(const QDomNode &node)
{
    m_lock.lockForWrite();

    qDeleteAll(m_transfers);

    QDomElement n = node.firstChildElement("download");
    while(!n.isNull())
    {
        QDomElement e = n.firstChildElement("param");
        QMap<QString,QString> map;
        Transfer* d;

        d = Transfer::createInstance(n.attribute("class"));

        if(d != 0)
        {
            /*while(!e.isNull())
            {
                if(e.hasAttribute("name"))
                    map[e.attribute("name")] = e.text();

                e = e.nextSiblingElement("param");
            }
            */
            d->load(n);
            m_transfers << d;
        }
        else
        {
            qDebug() << "***ERROR*** Unable to createInstance " << n.attribute("class");

            d = new PlaceholderTransfer(n.attribute("class"));
            d->load(n);
            m_transfers << d;
        }

        n = n.nextSiblingElement("download");
    }

    m_lock.unlock();
}

void Queue::saveQueue(QDomNode &node, QDomDocument &doc)
{
    lock();

    foreach(Transfer* d,m_transfers)
    {
        QDomElement elem = doc.createElement("download");

        d->save(doc, elem);
        elem.setAttribute("class",d->myClass());

        node.appendChild(elem);
    }

    unlock();
}

void Queue::updateGraph()
{
    int downq = 0, upq = 0;

    lock();

    for(int i=0;i<size();i++)
    {
        int up,down;
        at(i)->speeds(down,up);

        downq += down;
        upq += up;
    }

    unlock();

    if(m_qSpeedData.size() >= 5*60)
        m_qSpeedData.dequeue();
    m_qSpeedData.enqueue(QPair<int,int>(downq,upq));
}


void Queue::BackgroundSaver::run()
{
    Queue::saveQueues();
}
