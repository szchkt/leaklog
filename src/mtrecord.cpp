/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2010 Matus & Michal Tomlein

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

#include <QSqlQuery>
#include <QSqlRecord>
#include <QDateTime>
#include <QSqlError>
#include <QMessageBox>

using namespace Global;

MTRecord::MTRecord(const QString & table, const QString & id_field, const QString & id, const MTDictionary & parents):
QObject(), r_table(table), r_id_field(id_field), r_id(id), r_parents(parents)
{}

MTRecord::MTRecord(const MTRecord & other):
QObject()
{
    r_table = other.r_table;
    r_id_field = other.r_id_field;
    r_id = other.r_id;
    r_parents = other.r_parents;
    r_filter = other.r_filter;
}

MTRecord & MTRecord::operator=(const MTRecord & other)
{
    r_table = other.r_table;
    r_id_field = other.r_id_field;
    r_id = other.r_id;
    r_parents = other.r_parents;
    r_filter = other.r_filter;
    return *this;
}

void MTRecord::addFilter(const QString & column, const QString & filter)
{
    r_filter.insert(column, filter);
}

bool MTRecord::exists()
{
    if (r_id.isEmpty() && r_parents.isEmpty()) { return false; }
    QSqlQuery find_record = select(r_id_field);
    find_record.exec();
    return find_record.next();
}

QSqlQuery MTRecord::select(const QString & fields, Qt::SortOrder order)
{
    bool has_id = !r_id.isEmpty();
    QString select = "SELECT " + fields + " FROM " + r_table;
    if (has_id || r_parents.count() || r_filter.count()) { select.append(" WHERE "); }
    if (has_id) { select.append(r_id_field + " = :_id"); }
    for (int i = 0; i < r_parents.count(); ++i) {
        if (has_id || i) { select.append(" AND "); }
        select.append(r_parents.key(i) + " = :" + r_parents.key(i));
    }
    for (int i = 0; i < r_filter.count(); ++i) {
        if (has_id || r_parents.count() || i) { select.append(" AND "); }
        select.append(r_filter.key(i) + " LIKE :" + r_filter.key(i));
    }
    if (!r_id_field.isEmpty()) select.append(QString(" ORDER BY %1 %2").arg(r_id_field).arg(order == Qt::DescendingOrder ? "DESC" : "ASC"));
    QSqlQuery query;
    query.prepare(select);
    if (has_id) { query.bindValue(":_id", r_id); }
    for (int i = 0; i < r_parents.count(); ++i) {
        query.bindValue(":" + r_parents.key(i), r_parents.value(i));
    }
    for (int i = 0; i < r_filter.count(); ++i) {
        query.bindValue(":" + r_filter.key(i), r_filter.value(i));
    }
    return query;
}

QVariantMap MTRecord::list(const QString & fields, bool refresh)
{
    if (!refresh && !r_attributes.isEmpty())
        return r_attributes;
    QVariantMap list;
    QSqlQuery query = select(fields);
    query.setForwardOnly(true);
    query.exec();
    if (!query.next()) { return list; }
    for (int i = 0; i < query.record().count(); ++i) {
        list.insert(query.record().fieldName(i), query.value(i));
    }
    return list;
}

void MTRecord::readAttributes()
{
    r_attributes = list("*", true);
}

ListOfVariantMaps MTRecord::listAll(const QString & fields)
{
    ListOfVariantMaps list;
    QSqlQuery query = select(fields);
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

QVariantMap MTRecord::sumAll(const QString & fields)
{
    QVariantMap list;
    QSqlQuery query = select(fields);
    query.setForwardOnly(true);
    query.exec();
    while (query.next()) {
        for (int i = 0; i < query.record().count(); ++i) {
            list[query.record().fieldName(i)] = list[query.record().fieldName(i)].toDouble() + query.value(i).toDouble();
        }
    }
    return list;
}

MultiMapOfVariantMaps MTRecord::mapAll(const QString & map_to, const QString & fields)
{
    MultiMapOfVariantMaps map;
    QStringList list_map_to = map_to.split("::");
    QSqlQuery query = select(fields == "*" ? fields : (fields + ", " + list_map_to.join(", ")));
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

bool MTRecord::update(const QVariantMap & set, bool add_columns)
{
    bool has_id = !r_id.isEmpty();
    QString update;
    QMapIterator<QString, QVariant> i(set);
    if (add_columns) {
        QSqlDatabase db = QSqlDatabase::database();
        MTDictionary field_names = getTableFieldNames(r_table, &db);
        while (i.hasNext()) { i.next();
            if (!field_names.contains(i.key())) {
                addColumn(i.key(), r_table, &db);
            }
        }
        i.toFront();
    }
    if (has_id && !exists()) { has_id = false; }
    if (has_id) {
        update = "UPDATE " + r_table + " SET ";
        while (i.hasNext()) { i.next();
            update.append(i.key() + " = :" + i.key());
            /*if (i.hasNext())*/ update.append(", ");
        }
        update.append("date_updated = :date_updated");
        update.append(" WHERE " + r_id_field + " = :_id");
        for (int p = 0; p < r_parents.count(); ++p) {
            update.append(" AND " + r_parents.key(p) + " = :_" + r_parents.key(p));
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
        if (!set.contains(r_id_field) && !r_id_field.isEmpty()) {
            if (append_comma) { update.append(", "); }
            update.append(r_id_field);
        }
        update.append(", date_updated) VALUES (");
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
        if (!set.contains(r_id_field) && !r_id_field.isEmpty()) {
            if (append_comma) { update.append(", "); }
            update.append(":_id");
        }
        update.append(", :date_updated)");
    }
    QSqlQuery query;
    query.prepare(update);
    query.bindValue(":date_updated", QDateTime::currentDateTime().toString("yyyy.MM.dd-hh:mm"));
    if ((has_id || !set.contains(r_id_field)) && !r_id_field.isEmpty()) { query.bindValue(":_id", r_id); }
    for (int p = 0; p < r_parents.count(); ++p) {
        if (!has_id && set.contains(r_parents.key(p))) continue;
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
    QSqlQuery query;
    query.prepare(remove);
    if (has_id) { query.bindValue(":_id", r_id); }
    for (int i = 0; i < r_parents.count(); ++i) {
        query.bindValue(":" + r_parents.key(i), r_parents.value(i));
    }
    bool result = query.exec();
    if (result) { r_id.clear(); }
    return result;
}

void MTRecord::setId(const QString & r_id)
{
    this->r_id = r_id;
}
