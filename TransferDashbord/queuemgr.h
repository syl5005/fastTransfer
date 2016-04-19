/**************************************************************************
**   UpCloudEcosphere
**   Copyright (c) UpCloud C/C++ development team.
**   All rights 2016 reserved.
**   Author : Lynzabo
**************************************************************************/

#ifndef QUEUEMGR_H
#define QUEUEMGR_H
#include <QThread>
#include <QTimer>
#include "queue.h"
#include <QSettings>
#include <QMap>
#include "runtimeexception.h"
class QueueMgr : public QObject
{
Q_OBJECT
public:
    QueueMgr();
    void exit();
    static QueueMgr* instance() { return m_instance; }
private:
    void doMove(Queue* q, Transfer* t);
public slots:
    void doWork();
private:
    static QueueMgr* m_instance;
    QTimer* m_timer;
    int m_down, m_up;
    int m_nCycle;
};

#endif // QUEUEMGR_H
