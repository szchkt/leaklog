/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2018 Matus & Michal Tomlein

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

MTRecord::MTRecord(const QString &table, const QString &uuid, const QVariantMap &savedValues):
    r_table(table),
    r_uuid(uuid),
    r_saved_values(savedValues)
{}

MTRecord::MTRecord(const MTRecord &other)
{
    r_table = other.r_table;
    r_uuid = other.r_uuid;
    r_saved_values = other.r_saved_values;
    r_current_values = other.r_current_values;
}

MTRecord &MTRecord::operator=(const MTRecord &other)
{
    r_table = other.r_table;
    r_uuid = other.r_uuid;
    r_saved_values = other.r_saved_values;
    r_current_values = other.r_current_values;
    return *this;
}

bool MTRecord::exists() const
{
    if (r_uuid.isEmpty()) { return false; }
    MTSqlQuery find_record = select("uuid");
    find_record.exec();
    return find_record.next();
}

MTSqlQuery MTRecord::select(const QString &fields, Qt::SortOrder order) const
{
    return select(fields, QString("%1.uuid %3").arg(r_table.split(' ').first()).arg(order == Qt::DescendingOrder ? "DESC" : "ASC"));
}

MTSqlQuery MTRecord::select(const QString &fields, const QString &order_by) const
{
    bool has_uuid = !r_uuid.isEmpty();
    QString select = "SELECT " + fields + " FROM " + r_table;
    if (has_uuid) {
        select.append(" WHERE uuid = :_uuid");
    }
    if (!order_by.isEmpty())
        select.append(QString(" ORDER BY %1").arg(order_by));
    MTSqlQuery query;
    if (!query.prepare(select)) {
        qDebug() << query.lastError();
    }
    if (has_uuid) { query.bindValue(":_uuid", r_uuid); }
    return query;
}

QVariantMap MTRecord::list(const QString &fields) const
{
    return list(fields, QString("%1.uuid ASC").arg(r_table.split(' ').first()));
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
    if (r_saved_values.isEmpty() && !r_uuid.isEmpty())
        refresh(false);
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

    bool has_uuid = !r_uuid.isEmpty();
    QString journal_uuid = r_uuid;

    if (!has_uuid && !r_current_values.contains("uuid")) {
        journal_uuid = createUUID();
        r_current_values.insert("uuid", journal_uuid);
    }

    if (!r_current_values.contains("date_updated"))
        r_current_values.insert("date_updated", QDateTime::currentDateTime().toString(DATE_TIME_FORMAT));
    if (!r_current_values.contains("updated_by"))
        r_current_values.insert("updated_by", currentUser());

    if (has_uuid && !exists()) {
        has_uuid = false;
    }

    if (isJournaled()) {
        if (has_uuid) {
            QStringList columns; {
                QVariantMapIterator i(r_current_values);
                while (i.hasNext()) { i.next();
                    if (!r_saved_values.contains(i.key())) {
                        columns << i.key();
                    }
                }
            }

            if (columns.count()) {
                MTSqlQuery query = select(columns.join(", "), QString());
                query.setForwardOnly(true);
                query.exec();

                if (query.next()) {
                    for (int i = 0; i < query.record().count(); ++i) {
                        QString column = query.record().fieldName(i);
                        QVariant value = query.value(i);
                        r_saved_values.insert(column, value);

                        if (r_current_values.contains(column) && r_current_values.value(column) == value) {
                            r_current_values.remove(column);
                        }
                    }
                }
            }

            QVariantMapIterator i(r_current_values);
            while (i.hasNext()) { i.next();
                if (!journalUpdate(r_table, r_uuid, i.key()))
                    return false;
            }
        } else {
            if (!journalInsertion(r_table, journal_uuid))
                return false;
        }
    }

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

    QString update;
    if (has_uuid) {
        update = "UPDATE " + r_table + " SET ";
        while (i.hasNext()) { i.next();
            update.append(i.key() + " = :" + i.key());
            if (i.hasNext()) update.append(", ");
        }
        if (has_uuid)
            update.append(" WHERE uuid = :_uuid");
    } else {
        bool append_comma = false;
        update = "INSERT INTO " + r_table + " (";
        while (i.hasNext()) { i.next();
            update.append(i.key());
            if (i.hasNext()) { update.append(", "); }
            else { append_comma = true; }
        }
        if (!r_uuid.isEmpty() && !r_current_values.contains("uuid")) {
            if (append_comma) { update.append(", "); }
            update.append("uuid");
        }
        update.append(") VALUES (");
        append_comma = false;
        i.toFront();
        while (i.hasNext()) { i.next();
            update.append(":" + i.key());
            if (i.hasNext()) { update.append(", "); }
            else { append_comma = true; }
        }
        if (!r_uuid.isEmpty() && !r_current_values.contains("uuid")) {
            if (append_comma) { update.append(", "); }
            update.append(":_uuid");
        }
        update.append(")");
    }

    MTSqlQuery query;
    if (!query.prepare(update)) {
        qDebug() << query.lastError();
    }

    if (has_uuid || (!r_uuid.isEmpty() && !r_current_values.contains("uuid")))
        query.bindValue(":_uuid", r_uuid);

    i.toFront();
    while (i.hasNext()) { i.next();
        query.bindValue(":" + i.key(), i.value());
    }

    bool result = query.exec();
    if (!r_uuid.isEmpty()) {
        r_uuid = r_current_values.value("uuid", r_uuid).toString();
    } else {
        r_uuid = r_current_values.value("uuid", query.lastInsertId()).toString();
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
    if (r_uuid.isEmpty())
        return false;
    QString remove = QString("DELETE FROM %1 WHERE uuid = :_uuid").arg(r_table);
    MTSqlQuery query;
    if (!query.prepare(remove)) {
        qDebug() << query.lastError();
    }
    query.bindValue(":_uuid", r_uuid);
    bool result = query.exec();
    if (result && isJournaled()) {
        journalDeletion(r_table, r_uuid);
    }
    return result;
}

bool MTRecord::isJournaled() const
{
    return true;
}
