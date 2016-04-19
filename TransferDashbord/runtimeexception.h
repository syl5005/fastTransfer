/**************************************************************************
**   UpCloudEcosphere
**   Copyright (c) UpCloud C/C++ development team.
**   All rights 2016 reserved.
**   Author : Lynzabo
**************************************************************************/

#ifndef RUNTIMEEXCEPTION
#define RUNTIMEEXCEPTION
#include <QString>

class RuntimeException
{
public:
    RuntimeException(QString str = QString())
    : m_error(str)
    {
    }
    QString what() const
    {
        return m_error;
    }
private:
    QString m_error;
};
#endif // RUNTIMEEXCEPTION

