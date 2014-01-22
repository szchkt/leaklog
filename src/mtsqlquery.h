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

#ifndef MTSQLQUERY_H
#define MTSQLQUERY_H

#include <QSqlQuery>

class MTSqlQuery : public QSqlQuery
{
public:
    MTSqlQuery(QSqlResult *result);
    MTSqlQuery(const QString &query = QString(), QSqlDatabase db = QSqlDatabase());
    MTSqlQuery(QSqlDatabase db);
    MTSqlQuery(const QSqlQuery &other);

    bool exec(const QString &query);
    bool exec();

    QVariant value(int index) const;
    QVariant value(const QString &field) const;
    bool boolValue(const QString &field) const;
    int intValue(const QString &field) const;
    qlonglong longLongValue(const QString &field) const;
    double doubleValue(const QString &field) const;
    QString stringValue(const QString &field) const;

protected:
    void printLastError() const;
};

#endif // MTSQLQUERY_H
