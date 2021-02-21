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

#ifndef MTSQLQUERYRESULT_H
#define MTSQLQUERYRESULT_H

#include "mtrecord.h"
#include "mtsqlquery.h"

#include <QSqlRecord>

class MTSqlQueryResult;

template<class Key>
class MTSqlQueryResultBase
{
public:
    MTSqlQueryResultBase(const QString &q, QSqlDatabase db = QSqlDatabase()) {
        _query = new MTSqlQuery(db.isValid() ? db : QSqlDatabase::database());
        _query->exec(q);
        _pos = -1;
    }

    MTSqlQueryResultBase(QSqlDatabase db = QSqlDatabase()) {
        _query = new MTSqlQuery(db.isValid() ? db : QSqlDatabase::database());
        _pos = -1;
    }

    virtual ~MTSqlQueryResultBase() {
        if (_query) { delete _query; }
    }

    void bindValue(const QString &placeholder, const QVariant &value, QSql::ParamType type = QSql::In) {
        _query->bindValue(placeholder, value, type);
    }

    QVariant boundValue(const QString &placeholder) const {
        return _query->boundValue(placeholder);
    }

    bool exec(const QString &q) {
        bool ok = _query->exec(q);
        if (ok) { saveResult(); }
        return ok;
    }

    bool exec() {
        bool ok = _query->exec();
        if (ok) { saveResult(); }
        return ok;
    }

    bool next() {
        _pos++;
        if (_pos >= _result.count()) { _pos = -1; return false; }
        return true;
    }

    bool prepare(const QString &q) {
        return _query->prepare(q);
    }

    MTSqlQuery *query() {
        return _query;
    }

    QSqlRecord record() const {
        return _query->record();
    }

    QVariant value(const Key &key) const {
        if (_pos < 0)
            return QVariant();
        return _result.at(_pos).value(key);
    }

    int count() const {
        return _result.count();
    }

protected:
    int *pos() {
        return &_pos;
    }

    QList<QMap<Key, QVariant> > *result() {
        return &_result;
    }

    virtual void saveResult() = 0;

private:
    MTSqlQuery *_query;
    QList<QMap<Key, QVariant> > _result;
    int _pos;

    friend class MTSqlQueryResult;
};

class MTSqlQueryResult : public MTSqlQueryResultBase<int>
{
protected:
    void saveResult();
};

#endif // MTSQLQUERYRESULT_H
