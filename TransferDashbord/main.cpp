/**************************************************************************
**   UpCloudEcosphere
**   Copyright (c) UpCloud C/C++ development team.
**   All rights 2016 reserved.
**   Author : Lynzabo
**************************************************************************/

#include "mainwindow.h"
#include <QApplication>
#include "transfer.h"
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <iostream>
#include "string.h"
#include "time.h"
#include "queue.h"
#include "transferfactory.h"
#include "queuemgr.h"

static void runEngines(bool init = true);
static QString argsToArg(int argc,char** argv);
static void showHelp();

MainWindow* g_wndMain = 0;
//应用配置文件路径
static QString m_strSettingsPath;
static QueueMgr* g_qmgr = 0;
//带图形界面打开时候,初始是否隐藏界面
static bool m_bStartHidden = false;
//图形界面加载还是控制台加载应用程序
static bool m_bStartGUI = true;
//连接传输类 Transfer定义全局变量
extern QVector<EngineEntry> g_enginesDownload;
extern QVector<EngineEntry> g_enginesUpload;

int main(int argc, char *argv[])
{
    QCoreApplication* app = 0;
    int rval;
    QString arg;
    qsrand(time(0));

    QCoreApplication::setOrganizationName("Dolezel");
    QCoreApplication::setOrganizationDomain("dolezel.info");
    QCoreApplication::setApplicationName("fatrat");

    arg = argsToArg(argc, argv);

    //是否打开图形界面核心Application
    if (m_bStartGUI)
        app = new QApplication(argc, argv);
    else
        app = new QCoreApplication(argc, argv);
    //初始化传输功能类信息
    initTransferClasses();
    //启动传输引擎
    runEngines();
//    qRegisterMetaType<QString*>("QString*");
//	qRegisterMetaType<QByteArray*>("QByteArray*");
//	qRegisterMetaType<Transfer*>("Transfer*");
//	qRegisterMetaType<Transfer::TransferList>("Transfer::TransferList");
    //加载所有传输队列
    Queue::loadQueues();
    // force singleton creation
    TransferFactory::instance();
    g_qmgr = new QueueMgr;
    if(m_bStartGUI)
        g_wndMain = new MainWindow(m_bStartHidden);
    else
        qDebug() << "FatRat is up and running now";
    //暂不添加启动应用程序时候,通过参数就添加传输
//    if(!arg.isEmpty() && m_bStartGUI)
//        g_wndMain->addTransfer(arg);
    if(m_bStartGUI)
        QApplication::setQuitOnLastWindowClosed(false);
    rval = app->exec();
    delete g_wndMain;
    g_qmgr->exit();
    //初始化所有队列
    Queue::stopQueues();
    Queue::saveQueues();
    Queue::unloadQueues();
    //停止引擎
    runEngines(false);
    delete g_qmgr;
    delete app;

    return rval;
}
static void runEngines(bool init)
{
    for(int i=0;i<g_enginesDownload.size();i++)
    {
        if(init)
        {
            if(g_enginesDownload[i].lpfnInit)
                g_enginesDownload[i].lpfnInit();
        }
        else
        {
            if(g_enginesDownload[i].lpfnExit)
                g_enginesDownload[i].lpfnExit();
        }
    }

    for(int i=0;i<g_enginesUpload.size();i++)
    {
        if(init)
        {
            if(g_enginesUpload[i].lpfnInit)
                g_enginesUpload[i].lpfnInit();
        }
        else
        {
            if(g_enginesUpload[i].lpfnExit)
                g_enginesUpload[i].lpfnExit();
        }
    }
}
QString argsToArg(int argc,char** argv)
{
    QString arg, prg(argv[0]);
    if (prg == "fatrat-nogui" || prg.endsWith("/fatrat-nogui"))
        m_bStartGUI = false;

    for(int i=1;i<argc;i++)
    {
        arg = argv[i];
        //忽略大小写比较字符串
#ifdef Q_OS_WIN
        if(!_stricmp(argv[i], "--hidden") || !_stricmp(argv[i], "-i"))
#else
        if(!strcmpIgnoreCase(argv[i], "--hidden") || !strcmpIgnoreCase(argv[i], "-i"))
#endif
            m_bStartHidden = true;
#ifdef Q_OS_WIN
        else if(!_stricmp(argv[i], "--nogui") || !_stricmp(argv[i], "-n"))
#else
        if(!strcmpIgnoreCase(argv[i], "--nogui") || !strcmpIgnoreCase(argv[i], "-n"))
#endif
            m_bStartGUI = false;
#ifdef Q_OS_WIN
        else if(!_stricmp(argv[i], "--help") || !_stricmp(argv[i], "-h"))
#else
        else if(!strcmpIgnoreCase(argv[i], "--help") || !strcmpIgnoreCase(argv[i], "-h"))
#endif
            showHelp();
    }

    return arg;
}
void showHelp()
{
    std::cout << "FastTransfer (2.0.0)\n\n"
            "Copyright (C) 20016-2099 Lynzabo\n"
            "Licensed under the terms of the GNU GPL version 3 as published by the Free Software Foundation\n\n"
            "-i, --hidden     \tHide the GUI at startup (only if the tray icon exists)\n"
            "-n, --nogui      \tStart with no GUI at all\n"
            "-h, --help       \tShow this help\n\n"
            "If started in the GUI mode, you may pass transfers as arguments and they will be presented to the user\n";
    exit(0);
}
