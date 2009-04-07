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

MTSqlQueryResult::MTSqlQueryResult(const QString & q, QSqlDatabase db)
{
    _query = new QSqlQuery(db.isValid() ? db : QSqlDatabase::database());
    _query->exec(q);
    _pos = -1;
}

MTSqlQueryResult::MTSqlQueryResult(QSqlDatabase db)
{
    _query = new QSqlQuery(db.isValid() ? db : QSqlDatabase::database());
    _pos = -1;
}

MTSqlQueryResult::~MTSqlQueryResult()
{
    if (_query) { delete _query; }
}

void MTSqlQueryResult::bindValue(const QString & placeholder, const QVariant & value, QSql::ParamType type)
{
    _query->bindValue(placeholder, value, type);
}

QVariant MTSqlQueryResult::boundValue(const QString & placeholder) const
{
    return _query->boundValue(placeholder);
}

bool MTSqlQueryResult::exec(const QString & q)
{
    bool ok = _query->exec(q);
    if (ok) { saveResult(); }
    return ok;
}

bool MTSqlQueryResult::exec()
{
    bool ok = _query->exec();
    if (ok) { saveResult(); }
    return ok;
}

void MTSqlQueryResult::saveResult()
{
    int n = _query->record().count();
    _pos = -1;
    _result.clear();
    StringVariantMap row;
    while (_query->next()) {
        row.clear();
        for (int i = 0; i < n; ++i) {
            row.insert(toString(i), _query->value(i));
        }
        _result << row;
    }
}

bool MTSqlQueryResult::next()
{
    _pos++;
    if (_pos >= _result.count()) { _pos = -1; return false; }
    return true;
}

int * MTSqlQueryResult::pos()
{
    return &_pos;
}

bool MTSqlQueryResult::prepare(const QString & q)
{
    return _query->prepare(q);
}

QSqlQuery * MTSqlQueryResult::query()
{
    return _query;
}

QSqlRecord MTSqlQueryResult::record() const
{
    return _query->record();
}

ListOfStringVariantMaps * MTSqlQueryResult::result()
{
    return &_result;
}

QVariant MTSqlQueryResult::value(int i) const
{
    return value(toString(i));
}

QVariant MTSqlQueryResult::value(const QString & s) const
{
    if (_pos < 0) { return QVariant(); }
    return _result.at(_pos).value(s);
}

int MTSqlQueryResult::count() const
{
    return _result.count();
}
