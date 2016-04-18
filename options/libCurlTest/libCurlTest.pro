#-------------------------------------------------
#
# Project created by QtCreator 2016-04-18T13:53:06
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = libCurlTest
TEMPLATE = app


SOURCES += main.cpp\
        widget.cpp

HEADERS  += widget.h

FORMS    += widget.ui


win32{
win32:CONFIG(release, debug|release): LIBS += -L$$PWD/libcurl/lib/ -llibcurl
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/libcurl/lib/ -llibcurld

INCLUDEPATH += $$PWD/libcurl/include
DEPENDPATH += $$PWD/libcurl/include
#copy libcurl.dll to compile directory

EXTRA_BINFILES +=  $$PWD/libcurl/lib/libcurl.dll
EXTRA_BINFILES_WIN = $${EXTRA_BINFILES}
EXTRA_BINFILES_WIN ~= s,/,\\,g
CONFIG(release, debug|release): DESTDIR_WIN = $$OUT_PWD/release
else:CONFIG(debug, debug|release): DESTDIR_WIN = $$OUT_PWD/debug

DESTDIR_WIN ~= s,/,\\,g
for(FILE,EXTRA_BINFILES_WIN){
            QMAKE_POST_LINK +=$$quote(cmd /c copy /y $${FILE} $${DESTDIR_WIN}$$escape_expand(\n\t))
}
}
