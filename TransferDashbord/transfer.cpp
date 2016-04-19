/**************************************************************************
**   UpCloudEcosphere
**   Copyright (c) UpCloud C/C++ development team.
**   All rights 2016 reserved.
**   Author : Lynzabo
**************************************************************************/

#include "transfer.h"
#include "engines/curldownload.h"
#include "engines/curlupload.h"
//多线程安全单例TransferNotifier
Q_GLOBAL_STATIC(TransferNotifier, thisTransferNotifier)

//定义传输类
QVector<EngineEntry> g_enginesDownload;
QVector<EngineEntry> g_enginesUpload;


Transfer::Transfer(bool local):m_bLocal(local),m_state(Paused),m_nTimeRunning(0),m_nDownLimit(0), m_nUpLimit(0)
  ,m_nDownLimitInt(0), m_nUpLimitInt(0),m_bWorking(false),m_mode(Download)
{
    m_uuid = QUuid::createUuid();
}

Transfer *Transfer::createInstance(QString className)
{
    qDebug() << "Transfer::createInstance():" << className;
    for(int i=0;i<g_enginesDownload.size();i++)
    {
        if(className == g_enginesDownload[i].shortName)
            return g_enginesDownload[i].lpfnCreate2(&g_enginesDownload[i]);
    }
    for(int i=0;i<g_enginesUpload.size();i++)
    {
        if(className == g_enginesUpload[i].shortName)
            return g_enginesUpload[i].lpfnCreate2(&g_enginesUpload[i]);
    }

    return 0;
}

Transfer *Transfer::createInstance(Transfer::Mode mode, int classID)
{
    const EngineEntry* entries = engines(mode);

    return entries[classID].lpfnCreate2(&entries[classID]);
}

const EngineEntry *Transfer::engines(Transfer::Mode type)
{
    return (type == Download) ? g_enginesDownload.constData() : g_enginesUpload.constData();
}

void Transfer::load(const QDomNode &map)
{
    int down = 0, up = 0;

    setState(string2state(getXMLProperty(map, "state")));
    down = getXMLProperty(map, "downlimit").toInt();
    up = getXMLProperty(map, "uplimit").toInt();
    m_strComment = getXMLProperty(map, "comment");
    m_nTimeRunning = getXMLProperty(map, "timerunning").toLongLong();
    m_uuid = getXMLProperty(map, "uuid");

    if(m_uuid.isNull())
        m_uuid = QUuid::createUuid();

    QDomElement n = map.firstChildElement("action");
    while(!n.isNull())
    {
        if(n.attribute("state") == "Completed")
            m_strCommandCompleted = n.firstChild().toText().data();
        n = n.nextSiblingElement("action");
    }

    setUserSpeedLimits(down, up);
}

void Transfer::save(QDomDocument &doc, QDomNode &node) const
{
    setXMLProperty(doc, node, "state", state2string(m_state));
    setXMLProperty(doc, node, "downlimit", QString::number(m_nDownLimit));
    setXMLProperty(doc, node, "uplimit", QString::number(m_nUpLimit));
    setXMLProperty(doc, node, "comment", m_strComment);
    setXMLProperty(doc, node, "timerunning", QString::number(timeRunning()));
    setXMLProperty(doc, node, "uuid", m_uuid.toString());

    QDomElement elem = doc.createElement("action");
    QDomText text = doc.createTextNode(m_strCommandCompleted);
    elem.setAttribute("state", "Completed");
    elem.appendChild(text);

    node.appendChild(elem);
}

QString Transfer::getXMLProperty(const QDomNode &node, QString name)
{
    QDomNode n = node.firstChildElement(name);
    if(n.isNull())
        return QString();
    else
        return n.firstChild().toText().data();
}

void Transfer::setXMLProperty(QDomDocument &doc, QDomNode &node, QString name, QString value)
{
    QDomElement sub = doc.createElement(name);
    QDomText text = doc.createTextNode(value);
    sub.appendChild(text);
    node.appendChild(sub);
}

Transfer::State Transfer::string2state(QString s)
{
#define IFCASE(x) if(s == #x) return (x)
    IFCASE(Waiting);
    else IFCASE(Active);
    else IFCASE(ForcedActive);
    else IFCASE(Paused);
    else IFCASE(Failed);
    else IFCASE(Completed);
    else return Paused;
#undef IFCASE
}

QString Transfer::state2string(Transfer::State s)
{
#define DOCASE(s) case (s): return #s
    switch(s)
    {
    DOCASE(Waiting);
    DOCASE(Active);
    DOCASE(ForcedActive);
    DOCASE(Paused);
    DOCASE(Failed);
    DOCASE(Completed);
    default: return "Paused";
    }
#undef DOCASE
}

Transfer::State Transfer::state() const
{
    return m_state;
}

void Transfer::setState(Transfer::State newState)
{
    bool now,was = isActive();
    m_lastState = m_state;

    if(newState == m_lastState)
        return;

    enterLogMessage(tr("Changed state: %1 -> %2").arg(state2string(m_state)).arg(state2string(newState)));

    if(newState == Completed)
        fileCompleted();

    m_state = newState;
    now = isActive();

    if(now != was)
    {
        m_bWorking = false;
        changeActive(now);

        if(now)
            m_timeStart = QDateTime::currentDateTime();
        else
            m_nTimeRunning += m_timeStart.secsTo(QDateTime::currentDateTime());
    }
    //通知状态更改
    if(!m_bLocal)
        emit TransferNotifier::instance()->stateChanged(this, m_lastState, newState);
    emit stateChanged(m_state, newState);
}

bool Transfer::isActive() const
{
    return m_state == Active || m_state == ForcedActive;
}

QString Transfer::dataPath(bool bDirect) const
{
    QString obj = object();

    if(primaryMode() == Download)
    {
        if(bDirect)
        {
            if (name().isEmpty())
                return QString();
            return QDir(obj).filePath(name());
        }
        else
            return obj;
    }
    else
    {
        if(bDirect)
            return obj;
        else
        {
            QDir dir(obj);
            dir.cdUp();
            return dir.absolutePath();
        }
    }
}

void Transfer::setUserSpeedLimits(int down, int up)
{
    m_nDownLimitInt = m_nDownLimit = down;
    m_nUpLimitInt = m_nUpLimit = up;
    setSpeedLimits(down,up);
}

qint64 Transfer::timeRunning() const
{
    if(!isActive())
        return m_nTimeRunning;
    else
        return m_nTimeRunning + m_timeStart.secsTo(QDateTime::currentDateTime());
}

void Transfer::fileCompleted()
{
    if(m_strCommandCompleted.isEmpty())
        return;

    QString exec = m_strCommandCompleted;
    for(int i=0;i<exec.size() - 1;)
    {
        if(exec[i++] != '%')
            continue;
        QChar t = exec[i];
        QString text;

        if(t == QChar('N')) // transfer name
            text = name();
        else if(t == QChar('T')) // transfer type
            text = myClass();
        else if(t == QChar('D')) // destination directory
            text = dataPath(false);
        else if(t == QChar('P')) // data path
            text = dataPath(true);
        else
            continue;

        exec.replace(i-1, 2, text);
        i += text.size() - 1;
    }

    qDebug() << "Executing" << exec;

    QProcess::startDetached(exec);
}

void Transfer::setSpeedLimits(int down, int up)
{

}

void Transfer::updateGraph()
{
    int down, up;

    speeds(down,up);

    if(m_qSpeedData.size() >= 5*60)
        m_qSpeedData.dequeue();
    m_qSpeedData.enqueue(QPair<int,int>(down,up));
}

void Transfer::setInternalSpeedLimits(int down, int up)
{
    if((m_nDownLimit < down && m_nDownLimit) || !down)
        down = m_nDownLimit;
    if((m_nUpLimit < up && m_nUpLimit) || !up)
        up = m_nUpLimit;

    if(down != m_nDownLimitInt || up != m_nUpLimitInt)
    {
        m_nDownLimitInt = down;
        m_nUpLimitInt = up;

        setSpeedLimits(down,up);
    }
}


void initTransferClasses()
{
#ifdef ENABLE_FAKEDOWNLOAD
    { // couldn't look more lazy and lame
        EngineEntry e = { "FakeDownload", "Fake engine", 0, 0, { FakeDownload::createInstance }, { FakeDownload::acceptable }, 0, 0 };
        g_enginesDownload << e;
    }
#endif
#ifdef WITH_BITTORRENT
    {
        EngineEntry e = { "TorrentDownload", "BitTorrent download", TorrentDownload::globalInit, TorrentDownload::globalExit, { TorrentDownload::createInstance }, { TorrentDownload::acceptable }, 0 };
        g_enginesDownload << e;
    }
#endif
#ifdef WITH_CURL
    {
        EngineEntry e = { "GeneralDownload", "CURL HTTP(S)/FTP(S)/SFTP download", CurlDownload::globalInit, CurlDownload::globalExit, { CurlDownload::createInstance }, { CurlDownload::acceptable }, /*CurlDownload::createMultipleOptionsWidget*/0 };
        g_enginesDownload << e;
    }
    {
        EngineEntry e = { "FtpUpload", "CURL FTP(S)/SFTP upload", 0, 0, { CurlUpload::createInstance }, { CurlUpload::acceptable }, 0 };
        g_enginesUpload << e;
    }
//    {
//        EngineEntry e = { "MetalinkDownload", "Metalink file handler", MetalinkDownload::globalInit, 0, { MetalinkDownload::createInstance }, { MetalinkDownload::acceptable }, 0 };
//        g_enginesDownload << e;
//    }
#endif
}

TransferNotifier::TransferNotifier()
{
    qRegisterMetaType<Transfer::State>("Transfer::State");
    qRegisterMetaType<Transfer::Mode>("Transfer::Mode");
}

TransferNotifier *TransferNotifier::instance()
{
    return thisTransferNotifier();
}
