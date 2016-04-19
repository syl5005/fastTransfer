/**************************************************************************
**   UpCloudEcosphere
**   Copyright (c) UpCloud C/C++ development team.
**   All rights 2016 reserved.
**   Author : Lynzabo
**************************************************************************/
#include "placeholdertransfer.h"
#include <cassert>
#include <QtDebug>

PlaceholderTransfer::PlaceholderTransfer(QString strClass)
    : m_strClass(strClass)
{
}

void PlaceholderTransfer::changeActive(bool bActive)
{
    if (bActive)
        setState(Failed);
}

void PlaceholderTransfer::speeds(int& down, int& up) const
{
    down = up = 0;
}
qulonglong PlaceholderTransfer::total() const
{
    return 0;
}
qulonglong PlaceholderTransfer::done() const
{
    return 0;
}

QString PlaceholderTransfer::name() const
{
    return tr("Error: transfer class %1 not found, this is a placeholder").arg(m_strClass);
}
QString PlaceholderTransfer::myClass() const
{
    return m_strClass;
}

QString PlaceholderTransfer::message() const
{
    return tr("This is a placeholder");
}

void PlaceholderTransfer::load(const QDomNode& map)
{
    m_root = m_doc.importNode(map, true);
    m_doc.appendChild(m_root);

    Transfer::load(map);
}

void PlaceholderTransfer::save(QDomDocument& doc, QDomNode& map) const
{
    //Transfer::save(doc, map);

    QDomNode x = doc.importNode(m_root, true);

    while(x.childNodes().count())
    {
        map.appendChild(x.childNodes().item(0));
    }
}

QString PlaceholderTransfer::dataPath(bool bDirect) const
{
    return QString();
}
