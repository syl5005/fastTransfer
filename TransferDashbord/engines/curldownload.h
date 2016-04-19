/**************************************************************************
**   UpCloudEcosphere
**   Copyright (c) UpCloud C/C++ development team.
**   All rights 2016 reserved.
**   Author : Lynzabo
**************************************************************************/

#ifndef CURLDOWNLOAD_H
#define CURLDOWNLOAD_H
#include "statictransfermessage.h"
#include "transfer.h"

class CurlDownload : public StaticTransferMessage<Transfer>
{
    Q_OBJECT
public:
    CurlDownload();
    static void globalInit();
    static void globalExit();
    static Transfer* createInstance() { return new CurlDownload; }
    static int acceptable(QString uri, bool);
};

#endif // CURLDOWNLOAD_H
