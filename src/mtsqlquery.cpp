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

#include "mtsqlquery.h"

#include <QDebug>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QVariant>
#include <QSqlRecord>
#include <QSqlError>

MTSqlQuery::MTSqlQuery(QSqlResult *result):
    QSqlQuery(result)
{}

MTSqlQuery::MTSqlQuery(const QString &query, QSqlDatabase db, bool forward):
    QSqlQuery(db)
{
    setForwardOnly(forward);

    if (!query.isEmpty())
        exec(query);
}

MTSqlQuery::MTSqlQuery(QSqlDatabase db, bool forward):
    QSqlQuery(db)
{
    setForwardOnly(forward);
}

MTSqlQuery::MTSqlQuery(const QSqlQuery &other):
    QSqlQuery(other)
{}

MTSqlQuery::MTSqlQuery(const MTSqlQuery &other):
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
#ifdef QT_DEBUG
        qDebug() << lastQuery();
        qDebug() << error.text();
#else
        QDir desktop(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));

        QFile file(desktop.absoluteFilePath("Leaklog-errors.log"));
        file.open(QIODevice::Append | QIODevice::Text);
        QTextStream output(&file);
        output.setCodec("UTF-8");
        output << lastQuery() << endl;
        output << error.text() << endl;
        file.close();
#endif
    }
}

QVariant MTSqlQuery::value(int index) const
{
    // If null, return a null string, because QVariant(QVariant::Double).toString().isNull() == false
    return isNull(index) ? QVariant(QVariant::String) : QSqlQuery::value(index);
}

QVariant MTSqlQuery::value(const QString &field) const
{
    return isNull(field) ? QVariant(QVariant::String) : QSqlQuery::value(field);
}

bool MTSqlQuery::boolValue(const QString &field) const
{
    return QSqlQuery::value(field).toInt();
}

int MTSqlQuery::intValue(const QString &field) const
{
    return QSqlQuery::value(field).toInt();
}

qlonglong MTSqlQuery::longLongValue(const QString &field) const
{
    return QSqlQuery::value(field).toLongLong();
}

double MTSqlQuery::doubleValue(const QString &field) const
{
    return QSqlQuery::value(field).toDouble();
}

QString MTSqlQuery::stringValue(const QString &field) const
{
    return isNull(field) ? QString() : QSqlQuery::value(field).toString();
}

QVariant MTSqlQuery::nextValue(int index)
{
    return next() ? value(index) : QVariant(QVariant::String);
}
