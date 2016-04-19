/**************************************************************************
**   UpCloudEcosphere
**   Copyright (c) UpCloud C/C++ development team.
**   All rights 2016 reserved.
**   Author : Lynzabo
**************************************************************************/

#ifndef LOGGER_H
#define LOGGER_H
#include <QtCore>
#include <QObject>

class Logger : public QObject
{
    Q_OBJECT
public:
    Logger();
    ~Logger();

    Q_INVOKABLE QString logContents() const;
    Q_PROPERTY(QString logContents READ logContents)
    static Logger* global() { return &m_global; }

    void toggleSysLog(bool on);
public slots:
    void enterLogMessage(QString msg);
    void enterLogMessage(QString sender, QString msg);
signals:
    void logMessage(QString msg);
private:
    QString m_strLog;
    bool m_bSysLog;
    mutable QReadWriteLock m_lock;
    static Logger m_global;
};

#endif // LOGGER_H
