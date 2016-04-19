/**************************************************************************
**   UpCloudEcosphere
**   Copyright (c) UpCloud C/C++ development team.
**   All rights 2016 reserved.
**   Author : Lynzabo
**************************************************************************/

#ifndef STATICTRANSFERMESSAGE
#define STATICTRANSFERMESSAGE
#include <QtCore>

template <typename T> class StaticTransferMessage : public T
{
public:
    void setMessage(QString text) { m_strMessage = text; }
    QString message() const { return m_strMessage; }
protected:
    QString m_strMessage;
};
#endif // STATICTRANSFERMESSAGE

