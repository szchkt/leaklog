/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2009 Matus & Michal Tomlein

 Leaklog is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public Licence
 as published by the Free Software Foundation; either version 2
 of the Licence, or (at your option) any later version.

 Leaklog is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public Licence for more details.

 You should have received a copy of the GNU General Public Licence
 along with Leaklog; if not, write to the Free Software Foundation,
 Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
********************************************************************/

#ifndef MTRECORD_H
#define MTRECORD_H

#include "defs.h"
#include "mtdictionary.h"

#include <QVariant>

class QSqlQuery;

class MTRecord : public QObject
{
    Q_OBJECT

public:
    MTRecord(): QObject() {}
    MTRecord(const QString &, const QString &, const QString &, const MTDictionary &);
    MTRecord(const MTRecord &);
    MTRecord & operator=(const MTRecord &);
    void addFilter(const QString &, const QString &);
    inline QString table() const { return r_table; }
    inline QString idField() const { return r_id_field; }
    inline QString id() const { return r_id; }
    inline MTDictionary & parents() { return r_parents; }
    inline QString parent(const QString & field) const { return r_parents.value(field); }
    bool exists();
    QSqlQuery select(const QString & = "*");
    StringVariantMap list(const QString & = "*");
    inline QVariant value(const QString & field, const QVariant & default_value = QVariant()) {
        return list(field).value(field, default_value);
    }
    inline QString stringValue(const QString & field, const QString & default_value = QString()) {
        return list(field).value(field, default_value).toString();
    }
    ListOfStringVariantMaps listAll(const QString & = "*");
    MultiMapOfStringVariantMaps mapAll(const QString &, const QString & = "*");
    bool update(const StringVariantMap &, bool = false);
    bool remove();

private:
    QString r_table;
    QString r_id_field;
    QString r_id;
    MTDictionary r_parents;
    MTDictionary r_filter;
};

#endif // MTRECORD_H
