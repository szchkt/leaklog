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

#include "global.h"

using namespace Global;

MTRecord::MTRecord(const QString & table, const QString & id, const MTDictionary & parents)
{
    r_table = table;
    r_id = id;
    r_parents = parents;
}

MTRecord::MTRecord(const MTRecord & other):
QObject()
{
    r_table = other.r_table;
    r_id = other.r_id;
    r_parents = other.r_parents;
}

MTRecord & MTRecord::operator=(const MTRecord & other)
{
    r_table = other.r_table;
    r_id = other.r_id;
    r_parents = other.r_parents;
    return *this;
}

QString MTRecord::idFieldName() const
{
    if (r_table == "inspections" || r_table == "repairs" || r_table == "refrigerant_management") {
        return "date";
    }
    return "id";
}

bool MTRecord::exists()
{
    if (r_id.isEmpty() && r_parents.isEmpty()) { return false; }
    QSqlQuery find_record = select(idFieldName());
    find_record.exec();
    return find_record.next();
}

QSqlQuery MTRecord::select(const QString & fields)
{
    bool has_id = !r_id.isEmpty();
    QString id_field = idFieldName();
    QString select = "SELECT " + fields + " FROM " + r_table;
    if (has_id || r_parents.count()) { select.append(" WHERE "); }
    if (has_id) { select.append(id_field + " = :_id"); }
    for (int i = 0; i < r_parents.count(); ++i) {
        if (has_id || i) { select.append(" AND "); }
        select.append(r_parents.key(i) + " = :" + r_parents.key(i));
    }
    select.append(" ORDER BY " + id_field);
    QSqlQuery query;
    query.prepare(select);
    if (has_id) { query.bindValue(":_id", r_id); }
    for (int i = 0; i < r_parents.count(); ++i) {
        query.bindValue(":" + r_parents.key(i), r_parents.value(i));
    }
    return query;
}

StringVariantMap MTRecord::list(const QString & fields)
{
    StringVariantMap list;
    QSqlQuery query = select(fields);
    query.setForwardOnly(true);
    query.exec();
    if (!query.next()) { return list; }
    for (int i = 0; i < query.record().count(); ++i) {
        list.insert(query.record().fieldName(i), query.value(i));
    }
    return list;
}

ListOfStringVariantMaps MTRecord::listAll(const QString & fields)
{
    ListOfStringVariantMaps list;
    QSqlQuery query = select(fields);
    query.setForwardOnly(true);
    query.exec();
    while (query.next()) {
        StringVariantMap map;
        for (int i = 0; i < query.record().count(); ++i) {
            map.insert(query.record().fieldName(i), query.value(i));
        }
        list << map;
    }
    return list;
}

MultiMapOfStringVariantMaps MTRecord::mapAll(const QString & map_to, const QString & fields)
{
    MultiMapOfStringVariantMaps map;
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
        StringVariantMap row_map; QStringList list_key;
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

bool MTRecord::update(const StringVariantMap & set, bool add_columns)
{
    bool has_id = !r_id.isEmpty();
    QString id_field = idFieldName();
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
            if (i.hasNext()) { update.append(", "); }
        }
        update.append(" WHERE " + id_field + " = :_id");
        for (int p = 0; p < r_parents.count(); ++p) {
            update.append(" AND " + r_parents.key(p) + " = :_" + r_parents.key(p));
        }
    } else {
        update = "INSERT INTO " + r_table + " (";
        while (i.hasNext()) { i.next();
            update.append(i.key());
            if (i.hasNext() || r_parents.count()) { update.append(", "); }
        }
        for (int p = 0; p < r_parents.count(); ++p) {
            update.append(r_parents.key(p));
            if (p < r_parents.count() - 1) { update.append(", "); }
        }
        update.append(") VALUES (");
        i.toFront();
        while (i.hasNext()) { i.next();
            update.append(":" + i.key());
            if (i.hasNext() || r_parents.count()) { update.append(", "); }
        }
        for (int p = 0; p < r_parents.count(); ++p) {
            update.append(":_" + r_parents.key(p));
            if (p < r_parents.count() - 1) { update.append(", "); }
        }
        update.append(")");
    }
    QSqlQuery query;
    query.prepare(update);
    if (has_id) { query.bindValue(":_id", r_id); }
    for (int p = 0; p < r_parents.count(); ++p) {
        query.bindValue(":_" + r_parents.key(p), r_parents.value(p));
    }
    i.toFront();
    while (i.hasNext()) { i.next();
        query.bindValue(":" + i.key(), i.value());
        if (r_parents.contains(i.key())) { r_parents.setValue(i.key(), i.value().toString()); }
    }
    bool result = query.exec();
    if (has_id) {
        r_id = set.value(id_field, r_id).toString();
    } else {
        r_id = set.value(id_field, query.lastInsertId()).toString();
    }
    return result;
}

bool MTRecord::remove()
{
    if (r_id.isEmpty() && r_parents.isEmpty()) { return false; }
    bool has_id = !r_id.isEmpty();
    QString remove = "DELETE FROM " + r_table + " WHERE ";
    if (has_id) { remove.append(idFieldName() + " = :_id"); }
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
