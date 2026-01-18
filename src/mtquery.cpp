/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2026 Matus & Michal Tomlein

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

#include "mtquery.h"
#include "global.h"

#include <QSqlError>

using namespace Global;

MTQuery::MTQuery(const QString &table, const QVariantMap &parents):
    r_table(table),
    r_parents(parents)
{}

MTQuery::MTQuery(const MTQuery &other)
{
    r_table = other.r_table;
    r_joins = other.r_joins;
    r_parents = other.r_parents;
    r_predicate = other.r_predicate;
    r_filter = other.r_filter;
}

MTQuery &MTQuery::operator=(const MTQuery &other)
{
    r_table = other.r_table;
    r_joins = other.r_joins;
    r_parents = other.r_parents;
    r_predicate = other.r_predicate;
    r_filter = other.r_filter;
    return *this;
}

void MTQuery::addFilter(const QString &column, const QString &filter)
{
    r_filter.insert(column, filter);
}

bool MTQuery::exists() const
{
    if (r_parents.isEmpty()) { return false; }
    MTSqlQuery find_record = select("*", QString(), 1);
    find_record.exec();
    return find_record.next();
}

MTSqlQuery MTQuery::select(const QString &fields, const QString &order_by, int limit) const
{
    bool is_remote = isDatabaseRemote();
    QString select = "SELECT " + fields;
    foreach (auto join, r_joins) {
        if (!join.second.isEmpty()) {
            select.append(", " + join.second);
        }
    }
    select.append(" FROM " + r_table);
    foreach (auto join, r_joins) {
        if (!join.first.isEmpty()) {
            select.append(" " + join.first);
        }
    }
    if (r_parents.count() || r_filter.count() || !r_predicate.isEmpty()) { select.append(" WHERE "); }
    for (auto i = r_parents.constBegin(); i != r_parents.constEnd(); ++i) {
        if (i != r_parents.constBegin()) { select.append(" AND "); }
        select.append(i.key() + " = :" + i.key());
    }
    for (int i = 0; i < r_filter.count(); ++i) {
        if (r_parents.count() || i) { select.append(" AND "); }
        if (r_filter.key(i).contains('?'))
            select.append(r_filter.key(i).replace('?', QString(":_filter%1").arg(i)));
        else
            select.append(QString(is_remote ? "%1::text LIKE :_filter%2" : "%1 LIKE :_filter%2").arg(r_filter.key(i)).arg(i));
    }
    if (!r_predicate.isEmpty()) {
        if (r_parents.count() || r_filter.count()) { select.append(" AND "); }
        select.append(r_predicate);
    }
    if (!order_by.isEmpty())
        select.append(QString(" ORDER BY %1").arg(order_by));
    if (limit)
        select.append(QString(" LIMIT %1").arg(limit));
    MTSqlQuery query;
    if (!query.prepare(select)) {
        qDebug() << query.lastError();
    }
    for (auto i = r_parents.constBegin(); i != r_parents.constEnd(); ++i) {
        query.bindValue(":" + i.key(), i.value());
    }
    for (int i = 0; i < r_filter.count(); ++i) {
        query.bindValue(QString(":_filter%1").arg(i), r_filter.value(i));
    }
    return query;
}

QVariantMap MTQuery::list(const QString &fields, const QString &order_by) const
{
    QVariantMap list;
    MTSqlQuery query = select(fields, order_by, 1);
    query.exec();
    QSqlRecord record = query.record();
    if (!query.next()) { return list; }
    for (int i = 0; i < record.count(); ++i) {
        list.insert(record.fieldName(i), query.value(i));
    }
    return list;
}

ListOfVariantMaps MTQuery::listAll(const QString &fields, const QString &order_by) const
{
    ListOfVariantMaps list;
    MTSqlQuery query = order_by.isEmpty() ? select(fields) : select(fields, order_by);
    query.exec();
    QSqlRecord record = query.record();
    while (query.next()) {
        QVariantMap map;
        for (int i = 0; i < record.count(); ++i) {
            map.insert(record.fieldName(i), query.value(i));
        }
        list << map;
    }

    return list;
}

QVariantMap MTQuery::sumAll(const QString &fields) const
{
    QVariantMap list;
    MTSqlQuery query = select(fields);
    query.exec();
    QSqlRecord record = query.record();
    while (query.next()) {
        for (int i = 0; i < record.count(); ++i) {
            list[record.fieldName(i)] = list[record.fieldName(i)].toDouble() + query.value(i).toDouble();
        }
    }
    return list;
}

MultiMapOfVariantMaps MTQuery::mapAll(const QString &map_to, const QString &fields) const
{
    MultiMapOfVariantMaps map;
    MTSqlQuery query = select(fields == "*" ? fields : (fields + ", " + map_to));
    query.exec();
    QSqlRecord record = query.record();

    int index = map_to.lastIndexOf('.');
    if (index < 0) {
        index = record.indexOf(map_to);
    } else {
        index = record.indexOf(map_to.mid(index + 1));
    }

    if (index < 0)
        return map;

    while (query.next()) {
        QVariantMap row_map;
        for (int i = 0; i < record.count(); ++i) {
            row_map.insert(record.fieldName(i), query.value(i));
        }
        map.insert(query.value(index).toString(), row_map);
    }

    return map;
}
