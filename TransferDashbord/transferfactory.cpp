/**************************************************************************
**   UpCloudEcosphere
**   Copyright (c) UpCloud C/C++ development team.
**   All rights 2016 reserved.
**   Author : Lynzabo
**************************************************************************/
#include "transferfactory.h"
#include <QThread>
#include <QMetaType>
#include <QtDebug>
#include <cassert>

TransferFactory* TransferFactory::m_instance = 0;

Transfer* TransferFactory::createInstance(const char* clsName)
{
    if (QThread::currentThread() != thread())
    {
        Transfer* t;
        // We need to use Transfer** as Qt doesn't support return values over queued connections
        QMetaObject::invokeMethod(this, "createInstance", Qt::BlockingQueuedConnection, Q_ARG(QString, clsName), Q_ARG(Transfer**, &t));
        return t;
    }
    else
    {
        return Transfer::createInstance(clsName);
    }
}

void TransferFactory::init(Transfer* t, QString source, QString target)
{
    qDebug() << "Source: "<< source;
    qDebug() << "Target: "<< target;
    if (QThread::currentThread() != thread())
    {
        RuntimeException e;
        bool thrown;

        QMetaObject::invokeMethod(this, "init", Qt::BlockingQueuedConnection, Q_ARG(Transfer*, t), Q_ARG(QString, source), Q_ARG(QString, target), Q_ARG(RuntimeException*, &e), Q_ARG(bool*, &thrown));

        if (thrown)
            throw e;
    }
    else
    {
        return t->init(source, target);
    }
}

void TransferFactory::setState(Transfer* t, Transfer::State state)
{
    if (QThread::currentThread() != thread())
        QMetaObject::invokeMethod(this, "setStateSlot", Qt::QueuedConnection, Q_ARG(Transfer*, t), Q_ARG(Transfer::State, state));
    else
        t->setState(state);

}

void TransferFactory::setStateSlot(Transfer* t, Transfer::State state)
{
    t->setState(state);
}

void TransferFactory::createInstance(QString clsName, Transfer** t)
{
    *t = Transfer::createInstance(clsName);
}

void TransferFactory::init(Transfer* t, QString source, QString target, RuntimeException* pe, bool* eThrown)
{
    try
    {
        qDebug() << "Calling real init";
        t->init(source, target);
        *eThrown = false;
    }
    catch (const RuntimeException& e)
    {
        *pe = e;
        *eThrown = true;
    }
}

TransferFactory::TransferFactory()
{
    qRegisterMetaType<bool*>("bool*");
    qRegisterMetaType<RuntimeException*>("RuntimeException*");
    qRegisterMetaType<Transfer*>("Transfer*");
    qRegisterMetaType<Transfer**>("Transfer**");
    m_instance = this;
}

TransferFactory* TransferFactory::instance()
{
    static TransferFactory singleton;
    return &singleton;
}

Transfer* TransferFactory::createInstance(QString clsName)
{
    QByteArray ar = clsName.toLatin1();
    return createInstance(ar.data());
}
