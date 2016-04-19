/**************************************************************************
**   UpCloudEcosphere
**   Copyright (c) UpCloud C/C++ development team.
**   All rights 2016 reserved.
**   Author : Lynzabo
**************************************************************************/

#include "curlupload.h"

CurlUpload::CurlUpload()
{

}

int CurlUpload::acceptable(QString url, bool bDrop)
{
    if(bDrop)
        return (url.startsWith("file://") || url.startsWith("/")) ? 2 : 0;
    else
        return (url.startsWith("ftp://") || url.startsWith("sftp://")) ? 2 : 0;
}

