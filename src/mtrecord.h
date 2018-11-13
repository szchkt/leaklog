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

#ifndef MTRECORD_H
#define MTRECORD_H

#include "defs.h"
#include "mtsqlquery.h"

#include <QVariant>
#include <QSqlRecord>

class MTRecord
{
public:
    MTRecord() {}
    MTRecord(const QString &table, const QString &uuid, const QVariantMap &savedValues = QVariantMap());
    MTRecord(const MTRecord &other);
    virtual ~MTRecord() {}
    MTRecord &operator=(const MTRecord &other);
    inline QString table() const { return r_table; }
    inline QString uuid() const { return r_uuid; }
    bool exists() const;
    MTSqlQuery select(const QString &fields = "*", Qt::SortOrder order = Qt::AscendingOrder) const;
    MTSqlQuery select(const QString &fields, const QString &order_by) const;
    QVariantMap list(const QString &fields = "*") const;
    QVariantMap list(const QString &fields, const char *order_by) const;
    QVariantMap list(const QString &fields, const QString &order_by) const;
    void refresh(bool reset = true);
    void reset();
    void resetValue(const QString &field);
    void duplicate();
    void setValue(const QString &field, const QVariant &value);
    inline QVariantMap values() {
        return savedValues().unite(currentValues());
    }
    inline QVariantMap currentValues() const {
        return r_current_values;
    }
    inline QVariantMap savedValues() {
        if (r_saved_values.isEmpty() && !r_uuid.isEmpty())
            refresh(false);
        return r_saved_values;
    }
    inline QVariant savedValue(const QString &field, const QVariant &default_value = QVariant()) {
        if (r_saved_values.isEmpty() && !r_uuid.isEmpty())
            refresh(false);
        return r_saved_values.value(field, default_value);
    }
    inline QVariant value(const QString &field, const QVariant &default_value = QVariant()) {
        if (r_current_values.contains(field))
            return r_current_values.value(field);
        return savedValue(field, default_value);
    }
    inline bool boolValue(const QString &field, bool default_value = false) {
        return value(field, default_value).toInt();
    }
    inline int intValue(const QString &field, int default_value = 0) {
        return value(field, default_value).toInt();
    }
    inline qlonglong longLongValue(const QString &field, qlonglong default_value = 0L) {
        return value(field, default_value).toLongLong();
    }
    inline double doubleValue(const QString &field, double default_value = 0.0) {
        return value(field, default_value).toDouble();
    }
    inline QString stringValue(const QString &field, const QString &default_value = QString()) {
        return value(field, default_value).toString();
    }
    bool update(const QString &field, const QVariant &value, bool add_columns = false);
    bool update(const QVariantMap &values = QVariantMap(), bool add_columns = false);
    bool save(bool add_columns = false);
    virtual bool remove() const;

protected:
    virtual bool isJournaled() const;

private:
    QString r_table;
    QString r_uuid;
    QVariantMap r_saved_values;
    QVariantMap r_current_values;
};

#endif // MTRECORD_H
