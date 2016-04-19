/**************************************************************************
**   UpCloudEcosphere
**   Copyright (c) UpCloud C/C++ development team.
**   All rights 2016 reserved.
**   Author : Lynzabo
**************************************************************************/

#include <QCoreApplication>
#include <QtCore>
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QFile file;
    QDir dir = QDir::home();

    dir.mkpath(".local/share/fatrat");
    if(!dir.cd(".local/share/fatrat"))
        return 0;
    qDebug() << dir.absoluteFilePath("queues.xml");
    return a.exec();
}

