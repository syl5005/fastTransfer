#-------------------------------------------------
#
# Project created by QtCreator 2016-04-18T15:28:04
#
#-------------------------------------------------

QT       += core gui dbus xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TransferDashbord
TEMPLATE = app

DEFINES += WITH_CURL \
#        WITH_BITTORRENT

RESOURCES += \
    common/resources.qrc


SOURCES += main.cpp\
        mainwindow.cpp \
    logger.cpp \
    transfer.cpp \
    engines/curldownload.cpp \
    engines/curlpoller.cpp \
    test.cpp \
    engines/curlupload.cpp \
    queue.cpp \
    engines/placeholdertransfer.cpp \
    transferfactory.cpp \
    queuemgr.cpp

HEADERS  += mainwindow.h \
    logger.h \
    transfer.h \
    engines/statictransfermessage.h \
    engines/curldownload.h \
    engines/curlpoller.h \
    engines/curlupload.h \
    queue.h \
    engines/placeholdertransfer.h \
    transferfactory.h \
    runtimeexception.h \
    queuemgr.h

FORMS    += mainwindow.ui
