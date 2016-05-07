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
#include <QSqlError>

using namespace Global;

MTRecord::MTRecord(const QString &table, const QString &id_field, const QString &id, const QVariantMap &savedValues):
    r_table(table),
    r_id_field(id_field),
    r_id(id),
    r_saved_values(savedValues)
{}

MTRecord::MTRecord(const MTRecord &other)
{
    r_table = other.r_table;
    r_id_field = other.r_id_field;
    r_id = other.r_id;
    r_saved_values = other.r_saved_values;
    r_current_values = other.r_current_values;
}

MTRecord &MTRecord::operator=(const MTRecord &other)
{
    r_table = other.r_table;
    r_id_field = other.r_id_field;
    r_id = other.r_id;
    r_saved_values = other.r_saved_values;
    r_current_values = other.r_current_values;
    return *this;
}

bool MTRecord::exists() const
{
    if (r_id.isEmpty()) { return false; }
    MTSqlQuery find_record = select(r_id_field);
    find_record.exec();
    return find_record.next();
}

MTSqlQuery MTRecord::select(const QString &fields, Qt::SortOrder order) const
{
    return select(fields, QString("%1.%2 %3").arg(r_table.split(' ').first()).arg(r_id_field).arg(order == Qt::DescendingOrder ? "DESC" : "ASC"));
}

MTSqlQuery MTRecord::select(const QString &fields, const QString &order_by) const
{
    bool has_id = !r_id.isEmpty();
    QString select = "SELECT " + fields + " FROM " + r_table;
    if (has_id) {
        select.append(" WHERE ");
        select.append(r_id_field + " = :_id");
    }
    if (!r_id_field.isEmpty() && !order_by.isEmpty())
        select.append(QString(" ORDER BY %1").arg(order_by));
    MTSqlQuery query;
    if (!query.prepare(select)) {
        qDebug() << query.lastError();
    }
    if (has_id) { query.bindValue(":_id", r_id); }
    return query;
}

QVariantMap MTRecord::list(const QString &fields) const
{
    return list(fields, QString("%1.%2 ASC").arg(r_table.split(' ').first()).arg(r_id_field));
}

QVariantMap MTRecord::list(const QString &fields, const char *order_by) const
{
    return list(fields, QString(order_by));
}

QVariantMap MTRecord::list(const QString &fields, const QString &order_by) const
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

void MTRecord::refresh(bool reset)
{
    r_saved_values = list();
    if (reset)
        r_current_values.clear();
}

void MTRecord::reset()
{
    r_current_values.clear();
}

void MTRecord::setValue(const QString &field, const QVariant &value)
{
    if (!r_saved_values.contains(field) || r_saved_values.value(field) != value) {
        r_current_values.insert(field, value);
    } else {
        r_current_values.remove(field);
    }
}

bool MTRecord::update(const QString &field, const QVariant &value, bool add_columns)
{
    r_current_values.insert(field, value);
    return save(add_columns);
}

bool MTRecord::update(const QVariantMap &values, bool add_columns)
{
    QVariantMapIterator i(values);
    while (i.hasNext()) { i.next();
        r_current_values.insert(i.key(), i.value());
    }

    return save(add_columns);
}

bool MTRecord::save(bool add_columns)
{
    if (r_current_values.isEmpty())
        return true;

    bool has_id = !r_id.isEmpty();
    QString update;

    if (!has_id && r_id_field == "uuid" && !r_current_values.contains("uuid"))
        r_current_values.insert("uuid", createUUID());
    if (!r_current_values.contains("date_updated"))
        r_current_values.insert("date_updated", QDateTime::currentDateTime().toString(DATE_TIME_FORMAT));
    if (!r_current_values.contains("updated_by"))
        r_current_values.insert("updated_by", currentUser());

    QVariantMapIterator i(r_current_values);
    if (add_columns) {
        QSqlDatabase db = QSqlDatabase::database();
        MTDictionary field_names = getTableFieldNames(r_table, db);
        while (i.hasNext()) { i.next();
            if (!field_names.contains(i.key())) {
                addColumn(i.key(), r_table, db);
            }
        }
        i.toFront();
    }

    if (has_id && !exists()) {
        has_id = false;
    }

    if (has_id) {
        update = "UPDATE " + r_table + " SET ";
        while (i.hasNext()) { i.next();
            update.append(i.key() + " = :" + i.key());
            if (i.hasNext()) update.append(", ");
        }
        if (has_id)
            update.append(" WHERE " + r_id_field + " = :_id");
    } else {
        bool append_comma = false;
        update = "INSERT INTO " + r_table + " (";
        while (i.hasNext()) { i.next();
            update.append(i.key());
            if (i.hasNext()) { update.append(", "); }
            else { append_comma = true; }
        }
        if (!r_id.isEmpty() && !r_id_field.isEmpty() && !r_current_values.contains(r_id_field)) {
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
        if (!r_id.isEmpty() && !r_id_field.isEmpty() && !r_current_values.contains(r_id_field)) {
            if (append_comma) { update.append(", "); }
            update.append(":_id");
        }
        update.append(")");
    }

    MTSqlQuery query;
    if (!query.prepare(update)) {
        qDebug() << query.lastError();
    }

    if (has_id || (!r_id.isEmpty() && !r_id_field.isEmpty() && !r_current_values.contains(r_id_field)))
        query.bindValue(":_id", r_id);

    i.toFront();
    while (i.hasNext()) { i.next();
        query.bindValue(":" + i.key(), i.value());
    }

    bool result = query.exec();
    if (!r_id.isEmpty()) {
        r_id = r_current_values.value(r_id_field, r_id).toString();
    } else {
        r_id = r_current_values.value(r_id_field, query.lastInsertId()).toString();
    }

    i.toFront();
    while (i.hasNext()) { i.next();
        r_saved_values.insert(i.key(), i.value());
    }
    r_current_values.clear();

    return result;
}

bool MTRecord::remove() const
{
    if (r_id.isEmpty())
        return false;
    QString remove = QString("DELETE FROM %1 WHERE %2 = :_id").arg(r_table).arg(r_id_field);
    MTSqlQuery query;
    if (!query.prepare(remove)) {
        qDebug() << query.lastError();
    }
    query.bindValue(":_id", r_id);
    bool result = query.exec();
    return result;
}
