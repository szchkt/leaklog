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

#include "mtdictionary.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QSqlField>
#include <QMultiMap>

#define StringVariantMap QMap<QString, QVariant>

#define ListOfStringVariantMaps QList<StringVariantMap>

#define MapOfStringVariantMaps QMap<QString, StringVariantMap>

#define MultiMapOfStringVariantMaps QMultiMap<QString, StringVariantMap>

class MTRecord : public QObject
{
    Q_OBJECT

public:
    MTRecord() {}
    MTRecord(const QString &, const QString &, const MTDictionary &);
    MTRecord(const MTRecord &);
    MTRecord & operator=(const MTRecord &);
    void addFilter(const QString &, const QString &);
    inline QString table() const { return r_table; }
    inline QString id() const { return r_id; }
    inline MTDictionary * parents() { return &r_parents; }
    bool exists();
    QSqlQuery select(const QString & = "*");
    StringVariantMap list(const QString & = "*");
    ListOfStringVariantMaps listAll(const QString & = "*");
    MultiMapOfStringVariantMaps mapAll(const QString &, const QString & = "*");
    bool update(const StringVariantMap &, bool = false);
    bool remove();

    QString idFieldName() const;

private:
    QString r_table;
    QString r_id;
    MTDictionary r_parents;
    MTDictionary r_filter;
};

#endif // MTRECORD_H
