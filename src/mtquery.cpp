/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2016 Matus & Michal Tomlein

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

MTQuery::MTQuery(const QString &table, const MTDictionary &parents):
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
    QString select = "SELECT " + fields + " FROM " + r_table;
    if (!r_joins.isEmpty())
        select.append(" " + r_joins.join(" "));
    if (r_parents.count() || r_filter.count() || !r_predicate.isEmpty()) { select.append(" WHERE "); }
    int i;
    for (i = 0; i < r_parents.count(); ++i) {
        if (i) { select.append(" AND "); }
        select.append(r_parents.key(i) + " = :" + r_parents.key(i));
    }
    for (i = 0; i < r_filter.count(); ++i) {
        if (r_parents.count() || i) { select.append(" AND "); }
        if (r_filter.key(i).contains('?'))
            select.append(r_filter.key(i).replace('?', QString(":_filter%1").arg(i)));
        else
            select.append(QString(is_remote ? "%1::text LIKE :_filter%2" : "%1 LIKE :_filter%2").arg(r_filter.key(i)).arg(i));
    }
    if (!r_predicate.isEmpty()) {
        if (r_parents.count() || i) { select.append(" AND "); }
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
    for (int i = 0; i < r_parents.count(); ++i) {
        query.bindValue(":" + r_parents.key(i), r_parents.value(i));
    }
    for (int i = 0; i < r_filter.count(); ++i) {
        query.bindValue(QString(":_filter%1").arg(i), r_filter.value(i));
    }
    return query;
}

QVariantMap MTQuery::list(const QString &fields, const QString &order_by) const
{
    QVariantMap list;
    MTSqlQuery query = select(fields, order_by);
    query.setForwardOnly(true);
    query.exec();
    if (!query.next()) { return list; }
    for (int i = 0; i < query.record().count(); ++i) {
        list.insert(query.record().fieldName(i), query.value(i));
    }
    return list;
}

ListOfVariantMaps MTQuery::listAll(const QString &fields, const QString &order_by) const
{
    ListOfVariantMaps list;
    MTSqlQuery query = order_by.isEmpty() ? select(fields) : select(fields, order_by);
    query.setForwardOnly(true);
    query.exec();
    while (query.next()) {
        QVariantMap map;
        for (int i = 0; i < query.record().count(); ++i) {
            map.insert(query.record().fieldName(i), query.value(i));
        }
        list << map;
    }

    return list;
}

QVariantMap MTQuery::sumAll(const QString &fields) const
{
    QVariantMap list;
    MTSqlQuery query = select(fields);
    query.setForwardOnly(true);
    query.exec();
    while (query.next()) {
        for (int i = 0; i < query.record().count(); ++i) {
            list[query.record().fieldName(i)] = list[query.record().fieldName(i)].toDouble() + query.value(i).toDouble();
        }
    }
    return list;
}

MultiMapOfVariantMaps MTQuery::mapAll(const QString &map_to, const QString &fields) const
{
    MultiMapOfVariantMaps map;
    QStringList list_map_to = map_to.split("::");
    MTSqlQuery query = select(fields == "*" ? fields : (fields + ", " + list_map_to.join(", ")));
    query.setForwardOnly(true);
    query.exec();
    QList<int> indices;
    for (int i = 0; i < list_map_to.count(); ++i) {
        indices << query.record().indexOf(list_map_to.at(i));
        if (indices.last() < 0) { return map; }
    }
    while (query.next()) {
        QVariantMap row_map; QStringList list_key;
        for (int i = 0; i < query.record().count(); ++i) {
            row_map.insert(query.record().fieldName(i), query.value(i));
        }
        for (int i = 0; i < indices.count(); ++i) {
            list_key << query.value(indices.at(i)).toString();
        }
        map.insert(list_key.join("::"), row_map);
    }
    return map;
}
