/**************************************************************************
**   UpCloudEcosphere
**   Copyright (c) UpCloud C/C++ development team.
**   All rights 2016 reserved.
**   Author : Lynzabo
**************************************************************************/

#ifndef TRANSFER_H
#define TRANSFER_H
#include "logger.h"
#include <QtCore>
#include <QDialog>
#include <QDomNode>
#include <QDomDocument>
struct EngineEntry;

class Transfer : public Logger
{
public:
    Transfer(bool local = false);
    //传输状态
    enum State { Waiting, Active, ForcedActive, Paused, Failed, Completed };
    //传输模式
    enum Mode { ModeInvalid, Download, Upload };
    virtual void init(QString source, QString target) = 0;
    /**
     * @brief createInstance    创建当前传输实例
     * @param className
     * @return
     */
    static Transfer* createInstance(QString className);
    static Transfer* createInstance(Mode mode, int classID);
    static const EngineEntry* engines(Mode type);
    /**
     * @brief load  序列化
     * @param map
     */
    virtual void load(const QDomNode& map);
    /**
     * @brief save  保存到Dom中
     * @param doc
     * @param node
     */
    virtual void save(QDomDocument& doc, QDomNode& node) const;
    /**
     * @brief getXMLProperty    在DOM上获取值
     * @param node
     * @param name
     * @return
     */
    static QString getXMLProperty(const QDomNode& node, QString name);
    /**
     * @brief setXMLProperty    往DOM上赋值
     * @param doc
     * @param node
     * @param name
     * @param value
     */
    static void setXMLProperty(QDomDocument& doc, QDomNode& node, QString name, QString value);
    /**
     * @brief string2state  从字符串获取传输状态
     * @param s
     * @return
     */
    static State string2state(QString s);
    /**
     * @brief state2string  使用传输状态获取字符串
     * @param s
     * @return
     */
    static QString state2string(State s);
    //传输状态
    State state() const;
    virtual void setState(State newState);
    Q_INVOKABLE QString stateString() const;
    Q_INVOKABLE void setStateString(QString s);
    Q_PROPERTY(QString state READ stateString WRITE setStateString)

    //传输状态
    Q_INVOKABLE bool isActive() const;
    Q_PROPERTY(bool active READ isActive)
    //name  纯虚函数
    Q_INVOKABLE virtual QString name() const = 0;
    Q_PROPERTY(QString name READ name)
    //myClass 纯虚函数
    Q_INVOKABLE virtual QString myClass() const = 0;
    Q_PROPERTY(QString myClass READ myClass)
    //dataPath
    // If direct is true: returns the path of the file being downloaded
    // If direct is false: returns the directory where that file is located
    Q_INVOKABLE virtual QString dataPath(bool bDirect = true) const;
    Q_PROPERTY(QString dataPath READ dataPath)
    //object
    // For Upload-oriented classes: object = local file
    // For Download-oriented classes: object = local directory
    Q_INVOKABLE virtual void setObject(QString object) = 0;
    Q_INVOKABLE virtual QString object() const = 0;
    Q_PROPERTY(QString object WRITE setObject READ object)

    //primaryMode
    Q_INVOKABLE virtual Mode primaryMode() const { return Download; } // because the BitTorrent transfer may switch modes at run-time
    Q_PROPERTY(Transfer::Mode primaryMode READ primaryMode)

    //传输速度
    virtual void speeds(int& down, int& up) const = 0;
    Q_INVOKABLE void setUserSpeedLimits(int down,int up);
    void userSpeedLimits(int& down,int& up) const { down=m_nDownLimit; up=m_nUpLimit; }
    //mode
    Q_INVOKABLE Mode mode() const { return m_mode; }
    Q_PROPERTY(Transfer::Mode mode READ mode)
    //timeRunning
    qint64 timeRunning() const;
    Q_PROPERTY(qint64 timeRunning READ timeRunning)
signals:
    void stateChanged(Transfer::State prev, Transfer::State now);

protected:
    void fileCompleted();
    virtual void changeActive(bool nowActive) = 0;
    virtual void setSpeedLimits(int down,int up);
    void updateGraph();
    void setInternalSpeedLimits(int down,int up);
protected:
    bool m_bLocal;
    State m_state;
    State m_lastState;
    QString m_strCommandCompleted;
    QString m_strComment;
    QDateTime m_timeStart;
    qint64 m_nTimeRunning;
    QUuid m_uuid;
    int m_nDownLimit,m_nUpLimit;
    int m_nDownLimitInt,m_nUpLimitInt;
    bool m_bWorking;
    Mode m_mode;
    QQueue<QPair<int,int> > m_qSpeedData;
    friend class QueueMgr;
    friend class Queue;
};
class TransferNotifier : public QObject
{
Q_OBJECT
public:
    TransferNotifier();
    static TransferNotifier* instance();
signals:
    void stateChanged(Transfer* d, Transfer::State prev, Transfer::State now);
    void modeChanged(Transfer* d, Transfer::Mode prev, Transfer::Mode now);
    friend class Transfer;
};

struct EngineEntry
{
    const char* shortName;
    const char* longName;
    void (*lpfnInit)();
    void (*lpfnExit)();
    union
    {
        Transfer* (*lpfnCreate)();
        Transfer* (*lpfnCreate2)(const EngineEntry*);
    };
    union
    {
        // localSearch - only for Upload transfer classes
        // Normally a upload class would be called with a remote URL (where to upload)
        // If localsearch == true, then we're giving the function a local file (where from upload)
        // Used _only_ for drag&drop operation if a local file is dropped
        int (*lpfnAcceptable)(QString, bool /*localSearch*/);
        int (*lpfnAcceptable2)(QString, bool /*localSearch*/, const EngineEntry*);
    };
    // mass proprerties changing
    QDialog* (*lpfnMultiOptions)(QWidget* parent, QList<Transfer*>& transfers);
};

void initTransferClasses();
#endif // TRANSFER_H
