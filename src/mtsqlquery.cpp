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

#include "mtsqlquery.h"

#include <QDebug>
#include <QVariant>
#include <QSqlRecord>
#include <QSqlError>

MTSqlQuery::MTSqlQuery(QSqlResult *result):
    QSqlQuery(result)
{}

MTSqlQuery::MTSqlQuery(const QString &query, QSqlDatabase db):
    QSqlQuery(query, db)
{
    if (!query.isEmpty())
        printLastError();
}

MTSqlQuery::MTSqlQuery(QSqlDatabase db):
    QSqlQuery(db)
{}

MTSqlQuery::MTSqlQuery(const QSqlQuery &other):
    QSqlQuery(other)
{}

bool MTSqlQuery::exec(const QString &query)
{
    bool result = QSqlQuery::exec(query);
    printLastError();
    return result;
}

bool MTSqlQuery::exec()
{
    bool result = QSqlQuery::exec();
    printLastError();
    return result;
}

void MTSqlQuery::printLastError() const
{
    QSqlError error = lastError();
    if (error.type() != QSqlError::NoError) {
        qDebug() << lastQuery();
        qDebug() << error.text();
    }
}

QVariant MTSqlQuery::value(int index) const
{
    // If null, return a null string, because QVariant(QVariant::Double).toString().isNull() == false
    return isNull(index) ? QVariant(QVariant::String) : QSqlQuery::value(index);
}

QVariant MTSqlQuery::value(const QString &field) const
{
    return MTSqlQuery::value(record().indexOf(field));
}

bool MTSqlQuery::boolValue(const QString &field) const
{
    return QSqlQuery::value(record().indexOf(field)).toInt();
}

int MTSqlQuery::intValue(const QString &field) const
{
    return QSqlQuery::value(record().indexOf(field)).toInt();
}

qlonglong MTSqlQuery::longLongValue(const QString &field) const
{
    return QSqlQuery::value(record().indexOf(field)).toLongLong();
}

double MTSqlQuery::doubleValue(const QString &field) const
{
    return QSqlQuery::value(record().indexOf(field)).toDouble();
}

QString MTSqlQuery::stringValue(const QString &field) const
{
    return MTSqlQuery::value(record().indexOf(field)).toString();
}
