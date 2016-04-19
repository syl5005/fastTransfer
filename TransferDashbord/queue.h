/**************************************************************************
**   UpCloudEcosphere
**   Copyright (c) UpCloud C/C++ development team.
**   All rights 2016 reserved.
**   Author : Lynzabo
**************************************************************************/

#ifndef QUEUE_H
#define QUEUE_H

#include <QtCore>
#include <QDomNode>
#include <QDomDocument>
#include "transfer.h"

class Queue;
//队列列表  在queue.cpp中定义
extern QList<Queue*> g_queues;
//队列表列读写锁   在queue.cpp中定义
extern QReadWriteLock g_queuesLock;

/*!
 * \brief The Queue class   传输队列
 */
class Queue : public QObject
{
    Q_OBJECT
public:
    explicit Queue(QObject *parent = 0);
    ~Queue();
    /**
     * @brief stopQueues    停止所有队列
     */
    static void stopQueues();
    /**
     * @brief loadQueues    加载所有传输队列
     */
    static void loadQueues();
    /**
     * @brief unloadQueues  清空所有传输队列
     */
    static void unloadQueues();
    /**
     * @brief saveQueuesAsync   异步保存所有队列
     */
    static void saveQueuesAsync();
    /**
     * @brief saveQueues    保存所有队列
     */
    static void saveQueues();

    //定义name属性
    Q_PROPERTY(QString name READ name WRITE setName)
    Q_INVOKABLE void setName(QString name);
    Q_INVOKABLE QString name() const;

    //transferLimits
    Q_INVOKABLE void setTransferLimits(int down = -1,int up = -1) { m_nDownTransferLimit=down; m_nUpTransferLimit=up; }
    void transferLimits(int& down,int& up) const { down=m_nDownTransferLimit; up=m_nUpTransferLimit; }
    //speedLimits
    Q_INVOKABLE void setSpeedLimits(int down,int up) { m_nDownLimit=down; m_nUpLimit=up; }
    void speedLimits(int& down, int& up) const { down=m_nDownLimit; up=m_nUpLimit; }
    //moveDirectory
    Q_INVOKABLE void setMoveDirectory(QString path);
    Q_INVOKABLE QString moveDirectory() const;
    Q_PROPERTY(QString moveDirectory READ moveDirectory WRITE setMoveDirectory)

    Q_INVOKABLE int size();
    Q_PROPERTY(int size READ size)

    void lock() { m_lock.lockForRead(); }
    void unlock() { m_lock.unlock(); }

    Q_INVOKABLE Transfer* at(int r);
    QQueue<QPair<int,int> > speedData() const { return m_qSpeedData; }
    //删除当前Transfer
    Q_INVOKABLE void remove(int n, bool nolock = false);
    //获取当前Transfer
    Transfer* take(int n, bool nolock = false);

    void autoLimits(int& down, int& up) const { down=m_nDownAuto; up=m_nUpAuto; }
    void setAutoLimits(int down, int up);
public:
    // statistics
    struct Stats
    {
        int active_d, waiting_d, active_u, waiting_u;
        int down, up;
    } m_stats;
private:
    void loadQueue(const QDomNode& node);
    void saveQueue(QDomNode& node,QDomDocument& doc);
private:
    mutable QReadWriteLock m_lock;
    QString m_strName;
    int m_nDownLimit;
    int m_nUpLimit;
    int m_nDownTransferLimit;
    int m_nUpTransferLimit;
    bool m_bUpAsDown;
    QUuid m_uuid;
    QString m_strDefaultDirectory;
    QString m_strMoveDirectory;
    int m_nDownAuto, m_nUpAuto;


    friend class QueueMgr;

protected:
    void updateGraph();

    static bool m_bLoaded;
    QList<Transfer*> m_transfers;
    QQueue<QPair<int,int> > m_qSpeedData;
    class BackgroundSaver : public QThread
    {
    public:
        virtual void run();
    };
};

#endif // QUEUE_H
