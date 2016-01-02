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

#ifndef MTRECORD_H
#define MTRECORD_H

#include "defs.h"
#include "mtdictionary.h"
#include "mtsqlquery.h"

#include <QVariant>
#include <QSqlRecord>

class MTRecord
{
public:
    MTRecord() {}
    MTRecord(const QString &table, const QString &id_field, const QString &id, const MTDictionary &parents = MTDictionary());
    MTRecord(const MTRecord &other);
    virtual ~MTRecord() {}
    MTRecord &operator=(const MTRecord &other);
    void addFilter(const QString &column, const QString &filter);
    inline QString table() const { return r_table; }
    void setTable(const QString &table) { r_table = table; }
    void addJoin(const QString &join) { r_joins << join; }
    void setUnordered(bool unordered) { r_order = !unordered; }
    inline QString idField() const { return r_id_field; }
    inline QString id() const { return r_id; }
    inline QString &id() { return r_id; }
    void setId(const QString &id) { r_id = id; }
    inline MTDictionary &parents() { return r_parents; }
    inline QString parent(const QString &field) const { return r_parents.value(field); }
    bool exists() const;
    MTSqlQuery select(const QString &fields = "*", Qt::SortOrder order = Qt::AscendingOrder) const;
    MTSqlQuery select(const QString &fields, const QString &order_by) const;
    QVariantMap list(const QString &fields = "*") const;
    QVariantMap list(const QString &fields, const char *order_by) const;
    QVariantMap list(const QString &fields, const QString &order_by) const;
    void refresh(bool reset = true);
    void reset();
    void setValue(const QString &field, const QVariant &value);
    inline QVariantMap savedValues() {
        if (r_saved_values.isEmpty())
            refresh(false);
        return r_saved_values;
    }
    inline QVariant savedValue(const QString &field, const QVariant &default_value = QVariant()) {
        if (r_saved_values.isEmpty())
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
    template<class T>
    QList<T> all(const QString &order_by = QString()) const {
        QList<T> records;
        MTSqlQuery query = order_by.isEmpty() ? select() : select("*", order_by);
        query.setForwardOnly(true);
        query.exec();
        while (query.next()) {
            QVariantMap values;
            for (int i = 0; i < query.record().count(); ++i) {
                values.insert(query.record().fieldName(i), query.value(i));
            }
            T record;
            record.r_table = r_table;
            record.r_id_field = r_id_field;
            record.r_id = values.value(r_id_field).toString();
            record.r_saved_values = values;
            records << record;
        }
        return records;
    }
    template<class T>
    QList<T> where(const QString &predicate, const QString &order_by = QString()) {
        setPredicate(predicate);
        return all<T>(order_by);
    }
    ListOfVariantMaps listAll(const QString &fields = "*", const QString &order_by = QString()) const;
    QVariantMap sumAll(const QString &fields) const;
    MultiMapOfVariantMaps mapAll(const QString &map_to, const QString &fields = "*") const;
    bool update(const QString &field, const QVariant &value, bool add_columns = false);
    bool update(const QVariantMap &values = QVariantMap(), bool add_columns = false);
    bool save(bool add_columns = false);
    virtual bool remove();
    void setPredicate(const QString &predicate) { r_predicate = predicate; }
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
    QString r_predicate;
    MTDictionary r_filter;
    QVariantMap r_saved_values;
    QVariantMap r_current_values;
    bool r_order;
};

#endif // MTRECORD_H
