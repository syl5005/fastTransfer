/**************************************************************************
**   UpCloudEcosphere
**   Copyright (c) UpCloud C/C++ development team.
**   All rights 2016 reserved.
**   Author : Lynzabo
**************************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(bool bStartHidden,QWidget *parent = 0);
    ~MainWindow();
    void initWindow();
    void connActions();
public slots:
    /**
     * @brief showSettings  打开设置
     */
    void showSettings();
    //void addTransfer(QString uri = QString(), Transfer::Mode mode = Transfer::ModeInvalid, QString className = QString(), int queue = -1);
private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
