/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008 Matus & Michal Tomlein

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

QString Global::toString(const QVariant & v) { return v.toString(); }

QString Global::upArrow() { return QApplication::translate("Global", "\342\206\221", 0, QApplication::UnicodeUTF8); }

QString Global::downArrow() { return QApplication::translate("Global", "\342\206\223", 0, QApplication::UnicodeUTF8); }

void Global::copyTable(const QString & table, QSqlDatabase * from, QSqlDatabase * to, const QString & filter)
{
    QSqlQuery select("SELECT * FROM " + table + QString(filter.isEmpty() ? "" : (" WHERE " + filter)), *from);
    if (select.next() && select.record().count()) {
        QString copy("INSERT INTO " + table + " (");
        QStringList field_names = getTableFieldNames(table, to);
        QString field_name;
        for (int i = 0; i < select.record().count(); ++i) {
            field_name = select.record().fieldName(i);
            copy.append(i == 0 ? "" : ", ");
            copy.append(field_name);
            if (!field_names.contains(field_name)) {
                QSqlQuery add_column("ALTER TABLE " + table + " ADD COLUMN " + field_name + " VARCHAR", *to);
            }
        }
        copy.append(") VALUES (");
        for (int i = 0; i < select.record().count(); ++i) {
            copy.append(i == 0 ? ":" : ", :");
            copy.append(select.record().fieldName(i));
        }
        copy.append(")");
        do {
            QSqlQuery insert(*to);
            insert.prepare(copy);
            for (int i = 0; i < select.record().count(); ++i) {
                insert.bindValue(":" + select.record().fieldName(i), select.value(i));
            }
            insert.exec();
        } while (select.next());
    }
}

QStringList Global::getTableFieldNames(const QString & table, QSqlDatabase * database)
{
    QStringList field_names;
    QSqlQuery query("SELECT * FROM " + table, *database);
    for (int i = 0; i < query.record().count(); ++i) {
        field_names << query.record().fieldName(i);
    }
    return field_names;
}

using namespace Global;

MTRecord::MTRecord(const QString & type, const QString & id, const MTDictionary & parents)
{
    r_type = type;
    r_id = id;
    r_parents = parents;
}

MTRecord::MTRecord(const MTRecord & other):
QObject()
{
    r_type = other.r_type;
    r_id = other.r_id;
    r_parents = other.r_parents;
}

MTRecord & MTRecord::operator=(const MTRecord & other)
{
    r_type = other.r_type;
    r_id = other.r_id;
    r_parents = other.r_parents;
    return *this;
}

QString MTRecord::tableForRecordType(const QString & type)
{
    /*if (type == "customer") {
        return "customers";
    } else if (type == "circuit") {
        return "circuits";
    } else if (type == "inspection") {
        return "inspections";
    } else if (type == "variable") {
        return "variables";
    } else if (type == "subvariable") {
        return "subvariables";
    } else if (type == "table") {
        return "tables";
    } else if (type == "warning") {
        return "warnings";
    } else {*/
        return type + "s";
    //}
}

QSqlQuery MTRecord::select(const QString & fields)
{
    bool has_id = !r_id.isEmpty();
    QString id_field = r_type == "inspection" ? "date" : "id";
    QString select = "SELECT " + fields + " FROM " + tableForRecordType(r_type) + " WHERE ";
    if (has_id) { select.append(id_field + " = :_id"); }
    for (int i = 0; i < r_parents.count(); ++i) {
        if (has_id || i != 0) { select.append(" AND "); }
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

QMap<QString, QVariant> MTRecord::list(const QString & fields)
{
    QMap<QString, QVariant> list;
    QSqlQuery query = select(fields);
    query.setForwardOnly(true);
    query.exec();
    if (!query.next()) { return list; }
    for (int i = 0; i < query.record().count(); ++i) {
        list.insert(query.record().fieldName(i), query.value(i));
    }
    return list;
}

QList<QMap<QString, QVariant> > MTRecord::listAll(const QString & fields)
{
    QList<QMap<QString, QVariant> > list;
    QSqlQuery query = select(fields);
    query.setForwardOnly(true);
    query.exec();
    while (query.next()) {
        QMap<QString, QVariant> map;
        for (int i = 0; i < query.record().count(); ++i) {
            map.insert(query.record().fieldName(i), query.value(i));
        }
        list << map;
    }
    return list;
}

bool MTRecord::update(const QMap<QString, QVariant> & set, bool add_columns)
{
    bool has_id = !r_id.isEmpty();
    QString id_field = r_type == "inspection" ? "date" : "id";
    QString update;
    QMapIterator<QString, QVariant> i(set);
    if (add_columns) {
        QSqlDatabase db = QSqlDatabase::database();
        QStringList field_names = getTableFieldNames(tableForRecordType(r_type), &db);
        while (i.hasNext()) { i.next();
            if (!field_names.contains(i.key())) {
                QSqlQuery add_column("ALTER TABLE " + tableForRecordType(r_type) + " ADD COLUMN " + i.key() + " VARCHAR");
            }
        }
        i.toFront();
    }
    if (has_id) {
        QSqlQuery find_record = select(id_field);
        find_record.exec();
        if (!find_record.next()) { has_id = false; }
    }
    if (has_id) {
        update = "UPDATE " + tableForRecordType(r_type) + " SET ";
        while (i.hasNext()) { i.next();
            update.append(i.key() + " = :" + i.key());
            if (i.hasNext()) { update.append(", "); }
        }
        update.append(" WHERE " + id_field + " = :_id");
        for (int p = 0; p < r_parents.count(); ++p) {
            update.append(" AND " + r_parents.key(p) + " = :_" + r_parents.key(p));
        }
    } else {
        update = "INSERT INTO " + tableForRecordType(r_type) + " (";
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
    QString id_field = r_type == "inspection" ? "date" : "id";
    QString remove = "DELETE FROM " + tableForRecordType(r_type) + " WHERE ";
    if (has_id) { remove.append(id_field + " = :_id"); }
    for (int i = 0; i < r_parents.count(); ++i) {
        if (has_id || i != 0) { remove.append(" AND "); }
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
