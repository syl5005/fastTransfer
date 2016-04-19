/**************************************************************************
**   UpCloudEcosphere
**   Copyright (c) UpCloud C/C++ development team.
**   All rights 2016 reserved.
**   Author : Lynzabo
**************************************************************************/

#include "logger.h"
#include <QDate>
#include <QTime>
//调用Linux读写日志
//#include <syslog.h>

Logger Logger::m_global;

Logger::Logger()
    : m_bSysLog(false)
{
}

Logger::~Logger()
{
//    if (m_bSysLog)
//        closelog();
}

void Logger::toggleSysLog(bool on)
{
    if (on != m_bSysLog)
    {
//        if (on)
//            openlog("fatrat", LOG_PID, LOG_USER);
//        else
//            closelog();
        m_bSysLog = on;
    }
}

void Logger::enterLogMessage(QString msg)
{
    QWriteLocker l(&m_lock);

    QString text = QString("%1 %2 - %3")
        .arg( QDate::currentDate().toString(Qt::ISODate) )
        .arg( QTime::currentTime().toString(Qt::ISODate) )
        .arg(msg);

    if(!m_strLog.isEmpty())
        m_strLog += '\n';

    m_strLog += text;

    if (m_strLog.size() > 2*1024*1024)
        m_strLog = m_strLog.mid(1024*1024);

    l.unlock();
    emit logMessage(text);

    if (m_bSysLog)
    {
        std::string str = text.toStdString();
        qDebug() << str.c_str();
//        syslog(LOG_INFO, "%s", str.c_str());
    }
}

void Logger::enterLogMessage(QString sender, QString msg)
{
    enterLogMessage(QString("[%1]: %2").arg(sender).arg(msg));
}

QString Logger::logContents() const
{
    QReadLocker l(&m_lock); // TODO: Is this sufficient?
    return m_strLog;
}
