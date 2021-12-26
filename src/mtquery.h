/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2021 Matus & Michal Tomlein

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

#ifndef MTQUERY_H
#define MTQUERY_H

#include "defs.h"
#include "mtdictionary.h"
#include "mtsqlquery.h"

#include <QSqlRecord>

#include <functional>

class MTQuery
{
public:
    MTQuery() {}
    MTQuery(const QString &table, const QVariantMap &parents = QVariantMap());
    MTQuery(const MTQuery &other);
    virtual ~MTQuery() {}
    MTQuery &operator=(const MTQuery &other);
    void addFilter(const QString &column, const QString &filter);
    inline QString table() const { return r_table; }
    void setTable(const QString &table) { r_table = table; }
    void addJoin(const QString &join) { r_joins << join; }
    inline QVariantMap &parents() { return r_parents; }
    inline const QVariantMap &parents() const { return r_parents; }
    inline QVariant parent(const QString &field) const { return r_parents.value(field); }
    bool exists() const;
    MTSqlQuery select(const QString &fields = "*", const QString &order_by = QString(), int limit = 0) const;
    QVariantMap list(const QString &fields = "*", const QString &order_by = QString()) const;
    ListOfVariantMaps listAll(const QString &fields = "*", const QString &order_by = QString()) const;
    QVariantMap sumAll(const QString &fields) const;
    MultiMapOfVariantMaps mapAll(const QString &map_to, const QString &fields = "*") const;
    void setPredicate(const QString &predicate) { r_predicate = predicate; }
    qlonglong max(const QString &attr) {
        return list(QString("MAX(%1) AS max").arg(attr)).value("max").toLongLong();
    }

private:
    QString r_table;
    QStringList r_joins;
    QVariantMap r_parents;
    QString r_predicate;
    MTDictionary r_filter;
};

template <class T>
class MTRecordQuery : public MTQuery
{
public:
    MTRecordQuery(): MTQuery() {}
    MTRecordQuery(const QString &table, const QVariantMap &parents = QVariantMap()): MTQuery(table, parents) {}
    MTRecordQuery(const MTQuery &other): MTQuery(other) {}

    void each(std::function<void(T &)> f) const {
        each(QString(), f);
    }

    void each(const QString &order_by, std::function<void(T &)> f) const {
        MTSqlQuery query = select("*", order_by);
        query.exec();
        QSqlRecord query_record = query.record();
        while (query.next()) {
            QVariantMap values;
            for (int i = 0; i < query_record.count(); ++i) {
                values.insert(query_record.fieldName(i), query.value(i));
            }
            T record(values.value("uuid").toString(), values);
            f(record);
        }
    }

    T first(const QString &order_by = QString()) const {
        MTSqlQuery query = select("*", order_by, 1);
        query.exec();
        QSqlRecord query_record = query.record();
        if (query.next()) {
            QVariantMap values;
            for (int i = 0; i < query_record.count(); ++i) {
                values.insert(query_record.fieldName(i), query.value(i));
            }
            return T(values.value("uuid").toString(), values);
        }
        T record;
        for (auto i = parents().constBegin(); i != parents().constEnd(); ++i) {
            record.setValue(i.key(), i.value());
        }
        return record;
    }

    QList<T> all(const QString &order_by = QString()) const {
        QList<T> records;
        MTSqlQuery query = select("*", order_by);
        query.exec();
        QSqlRecord record = query.record();
        while (query.next()) {
            QVariantMap values;
            for (int i = 0; i < record.count(); ++i) {
                values.insert(record.fieldName(i), query.value(i));
            }
            records << T(values.value("uuid").toString(), values);
        }
        return records;
    }

    QMap<QString, T> map(const QString &key) const {
        QMap<QString, T> result;
        MTSqlQuery query = select();
        query.exec();
        QSqlRecord record = query.record();
        while (query.next()) {
            QVariantMap values;
            for (int i = 0; i < record.count(); ++i) {
                values.insert(record.fieldName(i), query.value(i));
            }
            result.insert(values.value(key).toString(), T(values.value("uuid").toString(), values));
        }
        return result;
    }

    QList<T> where(const QString &predicate, const QString &order_by = QString()) {
        setPredicate(predicate);
        return all(order_by);
    }

    void removeAll() const {
        auto records = all();
        foreach (auto record, records)
            record.remove();
    }
};

#endif // MTQUERY_H
