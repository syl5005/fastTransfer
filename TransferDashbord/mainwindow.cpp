/**************************************************************************
**   UpCloudEcosphere
**   Copyright (c) UpCloud C/C++ development team.
**   All rights 2016 reserved.
**   Author : Lynzabo
**************************************************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(bool bStartHidden, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //restoreWindowState(bStartHidden/* && m_trayIcon.isVisible()*/);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initWindow()
{
    connActions();
}

void MainWindow::connActions()
{
    connect(ui->actionSettings, SIGNAL(triggered()), this, SLOT(showSettings()));
//    connect(ui->actionNewTransfer, SIGNAL(triggered()), this, SLOT(addTransfer()));
}

void MainWindow::showSettings()
{
}

//void MainWindow::addTransfer(QString uri, Transfer::Mode mode, QString className, int queue)
//{

//}
