/**************************************************************************
**   UpCloudEcosphere
**   Copyright (c) UpCloud C/C++ development team.
**   All rights 2016 reserved.
**   Author : Lynzabo
**************************************************************************/

#ifndef CURLUPLOAD_H
#define CURLUPLOAD_H
#include "transfer.h"

class CurlUpload : public Transfer
{
public:
    CurlUpload();
    static Transfer* createInstance() { return new CurlUpload; }
    static int acceptable(QString url, bool bDrop);
};

#endif // CURLUPLOAD_H
