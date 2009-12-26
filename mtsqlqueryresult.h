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

#ifndef MTSQLQUERYRESULT_H
#define MTSQLQUERYRESULT_H

#include "mtrecord.h"

#include <QSqlQuery>

class MTSqlQueryResult : public QObject
{
    Q_OBJECT

public:
    MTSqlQueryResult(const QString &, QSqlDatabase = QSqlDatabase());
    MTSqlQueryResult(QSqlDatabase = QSqlDatabase());
    ~MTSqlQueryResult();

    void bindValue(const QString &, const QVariant &, QSql::ParamType = QSql::In);
    QVariant boundValue(const QString &) const;
    bool exec(const QString &);
    bool exec();
    bool next();
    bool prepare(const QString &);
    QSqlQuery * query();
    QSqlRecord record() const;
    QVariant value(int) const;
    QVariant value(const QString &) const;
    int count() const;

protected:
    int * pos();
    ListOfStringVariantMaps * result();
    virtual void saveResult();

private:
    QSqlQuery * _query;
    ListOfStringVariantMaps _result;
    int _pos;
};

#endif // MTSQLQUERYRESULT_H
