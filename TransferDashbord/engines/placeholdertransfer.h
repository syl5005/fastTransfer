/**************************************************************************
**   UpCloudEcosphere
**   Copyright (c) UpCloud C/C++ development team.
**   All rights 2016 reserved.
**   Author : Lynzabo
**************************************************************************/

#ifndef PLACEHOLDERTRANSFER_H
#define PLACEHOLDERTRANSFER_H
#include "transfer.h"
#include <QList>

class PlaceholderTransfer : public Transfer
{
public:
    PlaceholderTransfer(QString strClass);
    virtual void changeActive(bool bActive);
    virtual void speeds(int& down, int& up) const;
    virtual qulonglong total() const;
    virtual qulonglong done() const;
    virtual QString name() const;
    virtual QString myClass() const;
    virtual QString message() const;

    virtual void init(QString, QString) {}

    virtual void load(const QDomNode& map);
    virtual void save(QDomDocument& doc, QDomNode& map) const;

    void setSpeedLimits(int,int) {}
    virtual QString uri() const { return QString(); }
    virtual QString object() const { return QString(); }
    virtual void setObject(QString) { }

    virtual QString dataPath(bool bDirect = true) const;
private:
    QString m_strClass;
    QDomNode m_root;
    QDomDocument m_doc;
};

#endif // PLACEHOLDERTRANSFER_H
