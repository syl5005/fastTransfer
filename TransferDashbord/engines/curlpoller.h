/**************************************************************************
**   UpCloudEcosphere
**   Copyright (c) UpCloud C/C++ development team.
**   All rights 2016 reserved.
**   Author : Lynzabo
**************************************************************************/

#ifndef CURLPOLLER_H
#define CURLPOLLER_H
#include <QtCore>

class CurlPoller : public QThread
{
public:
    CurlPoller();
    ~CurlPoller();
    static void setTransferTimeout(int timeout);
    static int getTransferTimeout() { return m_nTransferTimeout; }
    static CurlPoller* instance() { return m_instance; }
protected:
    static CurlPoller* m_instance;
    static int m_nTransferTimeout;
};

#endif // CURLPOLLER_H
