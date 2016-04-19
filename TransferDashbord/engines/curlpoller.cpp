/**************************************************************************
**   UpCloudEcosphere
**   Copyright (c) UpCloud C/C++ development team.
**   All rights 2016 reserved.
**   Author : Lynzabo
**************************************************************************/

#include "curlpoller.h"

CurlPoller* CurlPoller::m_instance = 0;
int CurlPoller::m_nTransferTimeout = 20;

CurlPoller::CurlPoller()
{
    if(!m_instance)
    {
        m_instance = this;
        start();
    }
}

CurlPoller::~CurlPoller()
{
    if(isRunning())
        wait();

    if (this == m_instance)
        m_instance = 0;
}

void CurlPoller::setTransferTimeout(int timeout)
{
    m_nTransferTimeout = timeout;
}

