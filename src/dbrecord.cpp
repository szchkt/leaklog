/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2015 Matus & Michal Tomlein

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

#include "dbrecord.h"

Column::Column(const QString &name, const QString &type):
    _name(name),
    _type(type)
{
}

QString Column::name() const
{
    return _name;
}

QString Column::type() const
{
    return _type;
}

QString Column::toString() const
{
    return QString("%1 %2").arg(_name).arg(_type);
}

QString ColumnList::toString() const
{
    QString columns;
    bool first = true;
    foreach (const Column &column, *this) {
        if (first) {
            first = false;
        } else {
            columns.append(", ");
        }
        columns.append(column.toString());
    }
    return columns;
}

QStringList ColumnList::toStringList(std::function<QString(const Column &)> f) const
{
    QStringList result;
    foreach (const Column &column, *this) {
        result << f(column);
    }
    return result;
}

QStringList ColumnList::columnNameList() const
{
    QStringList names;
    foreach (const Column &column, *this) {
        names << column.name();
    }
    return names;
}

QSet<QString> ColumnList::columnNameSet() const
{
    QSet<QString> names;
    foreach (const Column &column, *this) {
        names << column.name();
    }
    return names;
}

DBRecord::DBRecord():
    QObject(),
    MTRecord()
{}

DBRecord::DBRecord(const QString &type, const QString &id_field, const QString &id, const QVariantMap &savedValues):
    QObject(),
    MTRecord(type, id_field, id, savedValues)
{}

QString DBRecord::dateUpdated()
{
    return stringValue("date_updated");
}

QString DBRecord::updatedBy()
{
    return stringValue("updated_by");
}
