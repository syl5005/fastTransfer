/**************************************************************************
**   UpCloudEcosphere
**   Copyright (c) UpCloud C/C++ development team.
**   All rights 2016 reserved.
**   Author : Lynzabo
**************************************************************************/

#ifndef TRANSFERFACTORY_H
#define TRANSFERFACTORY_H
#include <QObject>
#include "transfer.h"
#include "runtimeexception.h"

class TransferFactory : public QObject
{
Q_OBJECT
public:
    static TransferFactory* instance();

    // Create a Transfer instance in the correct thread
    Transfer* createInstance(const char* clsName);
    Transfer* createInstance(QString clsName);

    void setState(Transfer* t, Transfer::State state);

    // Init a Transfer in the correct thread
    void init(Transfer* t, QString source, QString target);
private:
    TransferFactory();
    TransferFactory(const TransferFactory &) {}
    void operator=(const TransferFactory&) {}
    ~TransferFactory() {}
private slots:
    void createInstance(QString clsName, Transfer** t);
    void init(Transfer* t, QString source, QString target, RuntimeException* e, bool* eThrown);
    void setStateSlot(Transfer* t, Transfer::State state);
private:
    static TransferFactory* m_instance;
};

#endif // TRANSFERFACTORY_H
