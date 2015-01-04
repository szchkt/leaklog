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

#include "mtrecord.h"
#include "global.h"

#include <QSqlRecord>
#include <QDateTime>
#include <QDebug>
#include <QSqlError>

using namespace Global;

MTRecord::MTRecord(const QString &table, const QString &id_field, const QString &id, const MTDictionary &parents):
    r_table(table),
    r_id_field(id_field),
    r_id(id),
    r_parents(parents),
    r_order(true),
    r_serial_id(false)
{}

MTRecord::MTRecord(const MTRecord &other)
{
    r_table = other.r_table;
    r_id_field = other.r_id_field;
    r_id = other.r_id;
    r_joins = other.r_joins;
    r_parents = other.r_parents;
    r_custom_where = other.r_custom_where;
    r_filter = other.r_filter;
    r_values = other.r_values;
    r_order = other.r_order;
    r_serial_id = other.r_serial_id;
}

MTRecord &MTRecord::operator=(const MTRecord &other)
{
    r_table = other.r_table;
    r_id_field = other.r_id_field;
    r_id = other.r_id;
    r_joins = other.r_joins;
    r_parents = other.r_parents;
    r_custom_where = other.r_custom_where;
    r_filter = other.r_filter;
    r_values = other.r_values;
    r_order = other.r_order;
    r_serial_id = other.r_serial_id;
    return *this;
}

void MTRecord::addFilter(const QString &column, const QString &filter)
{
    r_filter.insert(column, filter);
}

bool MTRecord::exists()
{
    if (r_id.isEmpty() && r_parents.isEmpty()) { return false; }
    MTSqlQuery find_record = select(r_id_field);
    find_record.exec();
    return find_record.next();
}

MTSqlQuery MTRecord::select(const QString &fields, Qt::SortOrder order)
{
    return select(fields, QString("%1 %2").arg(r_id_field).arg(order == Qt::DescendingOrder ? "DESC" : "ASC"));
}

MTSqlQuery MTRecord::select(const QString &fields, const QString &order_by)
{
    bool is_remote = isDatabaseRemote();
    bool has_id = !r_id.isEmpty();
    int i;
    QString select = "SELECT " + fields + " FROM " + r_table;
    if (!r_joins.isEmpty())
        select.append(" " + r_joins.join(" "));
    if (has_id || r_parents.count() || r_filter.count() || !r_custom_where.isEmpty()) { select.append(" WHERE "); }
    if (has_id) { select.append(r_id_field + " = :_id"); }
    for (i = 0; i < r_parents.count(); ++i) {
        if (has_id || i) { select.append(" AND "); }
        select.append(r_parents.key(i) + " = :" + r_parents.key(i));
    }
    for (i = 0; i < r_filter.count(); ++i) {
        if (has_id || r_parents.count() || i) { select.append(" AND "); }
        if (r_filter.key(i).contains('?'))
            select.append(r_filter.key(i).replace('?', QString(":_filter%1").arg(i)));
        else
            select.append(QString(is_remote ? "%1::text LIKE :_filter%2" : "%1 LIKE :_filter%2").arg(r_filter.key(i)).arg(i));
    }
    if (!r_custom_where.isEmpty()) {
        if (has_id || r_parents.count() || i) { select.append(" AND "); }
        select.append(r_custom_where);
    }
    if (!r_id_field.isEmpty() && !order_by.isEmpty() && r_order)
        select.append(QString(" ORDER BY %1").arg(order_by));
    MTSqlQuery query;
    if (!query.prepare(select)) {
        qDebug() << query.lastError();
    }
    if (has_id) { query.bindValue(":_id", r_id); }
    for (int i = 0; i < r_parents.count(); ++i) {
        query.bindValue(":" + r_parents.key(i), r_parents.value(i));
    }
    for (int i = 0; i < r_filter.count(); ++i) {
        query.bindValue(QString(":_filter%1").arg(i), r_filter.value(i));
    }
    return query;
}

QVariantMap MTRecord::list(const QString &fields, bool refresh)
{
    return list(fields, QString("%1 ASC").arg(r_id_field), refresh);
}

QVariantMap MTRecord::list(const QString &fields, const char *order_by, bool refresh)
{
    return list(fields, QString(order_by), refresh);
}

QVariantMap MTRecord::list(const QString &fields, const QString &order_by, bool refresh)
{
    if (!refresh && !r_values.isEmpty())
        return r_values;
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

void MTRecord::readValues(const QString &fields)
{
    r_values = list(fields, true);
}

ListOfVariantMaps MTRecord::listAll(const QString &fields, const QString &order_by)
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

QVariantMap MTRecord::sumAll(const QString &fields)
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

MultiMapOfVariantMaps MTRecord::mapAll(const QString &map_to, const QString &fields)
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

bool MTRecord::update(const QString &field, const QVariant &value, bool add_columns, bool force_update)
{
    QVariantMap set;
    set.insert(field, value);
    return update(set, add_columns, force_update);
}

bool MTRecord::update(const QVariantMap &values, bool add_columns, bool force_update)
{
    bool has_id = !r_id.isEmpty();
    QString update;
    QSqlDatabase db = QSqlDatabase::database();
    QVariantMap set(values);
    if (!set.contains("date_updated"))
        set.insert("date_updated", QDateTime::currentDateTime().toString(DATE_TIME_FORMAT));
    if (!set.contains("updated_by"))
        set.insert("updated_by", currentUser());
    QMapIterator<QString, QVariant> i(set);
    if (add_columns) {
        MTDictionary field_names = getTableFieldNames(r_table, db);
        while (i.hasNext()) { i.next();
            if (!field_names.contains(i.key())) {
                addColumn(i.key(), r_table, db);
            }
        }
        i.toFront();
    }
    if (has_id && !exists()) { has_id = false; }
    if (has_id || force_update) {
        update = "UPDATE " + r_table + " SET ";
        while (i.hasNext()) { i.next();
            update.append(i.key() + " = :" + i.key());
            if (i.hasNext()) update.append(", ");
        }
        if (has_id)
            update.append(" WHERE " + r_id_field + " = :_id");
        else if (r_parents.count())
            update.append(" WHERE ");
        for (int p = 0; p < r_parents.count(); ++p) {
            if (has_id || p) { update.append(" AND "); }
            update.append(r_parents.key(p) + " = :_" + r_parents.key(p));
        }
    } else {
        bool append_comma = false;
        update = "INSERT INTO " + r_table + " (";
        while (i.hasNext()) { i.next();
            update.append(i.key());
            if (i.hasNext()) { update.append(", "); }
            else { append_comma = true; }
        }
        for (int p = 0; p < r_parents.count(); ++p) {
            if (set.contains(r_parents.key(p))) continue;
            if (append_comma) { update.append(", "); }
            update.append(r_parents.key(p));
            append_comma = true;
        }
        if (!set.contains(r_id_field) && !r_id_field.isEmpty() && !r_serial_id) {
            if (append_comma) { update.append(", "); }
            update.append(r_id_field);
        }
        update.append(") VALUES (");
        append_comma = false;
        i.toFront();
        while (i.hasNext()) { i.next();
            update.append(":" + i.key());
            if (i.hasNext()) { update.append(", "); }
            else { append_comma = true; }
        }
        for (int p = 0; p < r_parents.count(); ++p) {
            if (set.contains(r_parents.key(p))) continue;
            if (append_comma) { update.append(", "); }
            update.append(":_" + r_parents.key(p));
            append_comma = true;
        }
        if (!set.contains(r_id_field) && !r_id_field.isEmpty() && !r_serial_id) {
            if (append_comma) { update.append(", "); }
            update.append(":_id");
        }
        update.append(")");
    }
    MTSqlQuery query;
    if (!query.prepare(update)) {
        qDebug() << query.lastError();
    }
    if ((has_id || (!set.contains(r_id_field) && !r_serial_id && !force_update)) && !r_id_field.isEmpty())
        query.bindValue(":_id", r_id);
    for (int p = 0; p < r_parents.count(); ++p) {
        if (!has_id && !force_update && set.contains(r_parents.key(p)))
            continue;
        query.bindValue(":_" + r_parents.key(p), r_parents.value(p));
    }
    i.toFront();
    while (i.hasNext()) { i.next();
        query.bindValue(":" + i.key(), i.value());
        if (r_parents.contains(i.key())) { r_parents.setValue(i.key(), i.value().toString()); }
    }
    bool result = query.exec();
    if (!r_id.isEmpty()) {
        r_id = set.value(r_id_field, r_id).toString();
    } else {
        r_id = set.value(r_id_field, query.lastInsertId()).toString();
    }
    return result;
}

bool MTRecord::remove()
{
    if (r_id.isEmpty() && r_parents.isEmpty()) { return false; }
    bool has_id = !r_id.isEmpty();
    QString remove = "DELETE FROM " + r_table + " WHERE ";
    if (has_id) { remove.append(r_id_field + " = :_id"); }
    for (int i = 0; i < r_parents.count(); ++i) {
        if (has_id || i) { remove.append(" AND "); }
        remove.append(r_parents.key(i) + " = :" + r_parents.key(i));
    }
    MTSqlQuery query;
    if (!query.prepare(remove)) {
        qDebug() << query.lastError();
    }
    if (has_id) { query.bindValue(":_id", r_id); }
    for (int i = 0; i < r_parents.count(); ++i) {
        query.bindValue(":" + r_parents.key(i), r_parents.value(i));
    }
    bool result = query.exec();
    if (result) { r_id.clear(); }
    return result;
}
