/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2014 Matus & Michal Tomlein

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

#include "defs.h"
#include "mtdictionary.h"
#include "mtsqlquery.h"

#include <QVariant>

class MTRecord
{
public:
    MTRecord() {}
    MTRecord(const QString &table, const QString &id_field, const QString &id, const MTDictionary &parents);
    MTRecord(const MTRecord &other);
    virtual ~MTRecord() {}
    MTRecord &operator=(const MTRecord &other);
    void addFilter(const QString &column, const QString &filter);
    inline QString table() const { return r_table; }
    void setTable(const QString &table) { r_table = table; }
    void addJoin(const QString &join) { r_joins << join; }
    void setUnordered(bool unordered) { r_order = !unordered; }
    void setSerialId(bool serial_id) { r_serial_id = serial_id; }
    inline QString idField() const { return r_id_field; }
    inline QString id() const { return r_id; }
    inline QString &id() { return r_id; }
    void setId(const QString &id) { r_id = id; }
    inline MTDictionary &parents() { return r_parents; }
    inline QString parent(const QString &field) const { return r_parents.value(field); }
    bool exists();
    MTSqlQuery select(const QString &fields = "*", Qt::SortOrder order = Qt::AscendingOrder);
    MTSqlQuery select(const QString &fields, const QString &order_by);
    QVariantMap list(const QString &fields = "*", bool refresh = false);
    QVariantMap list(const QString &fields, const char *order_by, bool refresh = false);
    QVariantMap list(const QString &fields, const QString &order_by, bool refresh = false);
    void readValues(const QString &fields = "*");
    inline QVariantMap &values() { return r_values; }
    inline QVariant value(const QString &field, const QVariant &default_value = QVariant()) {
        return r_values.contains(field) ? r_values.value(field) : list(field).value(field, default_value);
    }
    inline bool boolValue(const QString &field, bool default_value = false) {
        return r_values.contains(field) ? r_values.value(field).toInt() : list(field).value(field, default_value).toInt();
    }
    inline int intValue(const QString &field, int default_value = 0) {
        return r_values.contains(field) ? r_values.value(field).toInt() : list(field).value(field, default_value).toInt();
    }
    inline qlonglong longLongValue(const QString &field, qlonglong default_value = 0L) {
        return r_values.contains(field) ? r_values.value(field).toLongLong() : list(field).value(field, default_value).toLongLong();
    }
    inline double doubleValue(const QString &field, double default_value = 0.0) {
        return r_values.contains(field) ? r_values.value(field).toDouble() : list(field).value(field, default_value).toDouble();
    }
    inline QString stringValue(const QString &field, const QString &default_value = QString()) {
        return r_values.contains(field) ? r_values.value(field).toString() : list(field).value(field, default_value).toString();
    }
    ListOfVariantMaps listAll(const QString &fields = "*", const QString &order_by = QString());
    QVariantMap sumAll(const QString &fields);
    MultiMapOfVariantMaps mapAll(const QString &map_to, const QString &fields = "*");
    bool update(const QString &field, const QVariant &value, bool add_columns = false, bool force_update = false);
    bool update(const QVariantMap &values, bool add_columns = false, bool force_update = false);
    virtual bool remove();
    void setCustomWhere(const QString &where) { r_custom_where = where; }
    qlonglong max(const QString &attr) {
        setUnordered(true);
        return list(QString("MAX(%1) AS max").arg(attr)).value("max").toLongLong();
    }

private:
    QString r_table;
    QString r_id_field;
    QString r_id;
    QStringList r_joins;
    MTDictionary r_parents;
    QString r_custom_where;
    MTDictionary r_filter;
    QVariantMap r_values;
    bool r_order;
    bool r_serial_id;
};

#endif // MTRECORD_H
