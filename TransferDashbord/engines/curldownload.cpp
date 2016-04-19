/**************************************************************************
**   UpCloudEcosphere
**   Copyright (c) UpCloud C/C++ development team.
**   All rights 2016 reserved.
**   Author : Lynzabo
**************************************************************************/

#include "curldownload.h"
#include "curlpoller.h"

CurlDownload::CurlDownload()
{

}

void CurlDownload::globalInit()
{
    new CurlPoller;
    CurlPoller::setTransferTimeout(20);
}

void CurlDownload::globalExit()
{
    delete CurlPoller::instance();
}

int CurlDownload::acceptable(QString uri, bool)
{
    QUrl url = uri;
    QString scheme = url.scheme();

    if(scheme != "http" && scheme != "ftp" && scheme != "ftps" && scheme != "https" && scheme != "sftp")
        return 0;
    else
        return 2;
}

