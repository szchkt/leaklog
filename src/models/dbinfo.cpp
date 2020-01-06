/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2020 Matus & Michal Tomlein

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

#include "dbinfo.h"

#include "mtsqlquery.h"
#include "global.h"

#include <QDataStream>
#include <QDate>
#include <QFileInfo>
#include <QSqlError>

using namespace Global;

QString DBInfo::valueForKey(const QString &key, const QString &default_value, const QSqlDatabase &database)
{
    MTSqlQuery query(QString("SELECT value FROM db_info WHERE id = '%1'").arg(key), database);
    if (!query.next())
        return default_value;
    return query.value(0).toString();
}

QSqlError DBInfo::setValueForKey(const QString &key, const QString &value, const QSqlDatabase &database)
{
    MTSqlQuery query(QString("SELECT value FROM db_info WHERE id = '%1'").arg(key), database);
    if (query.next())
        return MTSqlQuery(QString("UPDATE db_info SET value = '%1' WHERE id = '%2'").arg(value).arg(key), database).lastError();
    return MTSqlQuery(QString("INSERT INTO db_info (id, value) VALUES ('%1', '%2')").arg(key).arg(value), database).lastError();
}

QString DBInfo::databaseUUID(const QSqlDatabase &database)
{
    QString database_uuid = valueForKey("database_uuid", QString(), database);
    if (database_uuid.isEmpty()) {
        database_uuid = createUUID();
        setValueForKey("database_uuid", database_uuid, database);
    }
    return database_uuid;
}

QString DBInfo::databaseName(const QSqlDatabase &database)
{
    QString database_name = valueForKey("database_name", QString(), database);
    if (database_name.isEmpty()) {
        database_name = QFileInfo(database.databaseName()).completeBaseName();
    }
    return database_name;
}

void DBInfo::setDatabaseName(const QString &database_name, const QSqlDatabase &database)
{
    setValueForKey("database_name", database_name, database);
}

QString DBInfo::autosaveMode()
{
    return valueForKey("autosave", "immediate");
}

void DBInfo::setAutosaveMode(const QString &autosave_mode)
{
    setValueForKey("autosave", autosave_mode);
}

bool DBInfo::isCurrentUserAdmin()
{
    QString current_user = currentUser();
    return valueForKey("admin", current_user) == current_user;
}

int DBInfo::isDatabaseLocked()
{
    QString locked = valueForKey("locked");
    if (locked == "true")
        return 1;
    if (locked == "auto")
        return 2;
    return 0;
}

bool DBInfo::isRecordLocked(const QString &date)
{
    if (isDatabaseLocked())
        return date < lockDate();
    return false;
}

QString DBInfo::lockDate()
{
    return valueForKey("locked") == "auto" ?
            QDate::currentDate().addDays(-valueForKey("autolock_days").toInt()).toString(DATE_FORMAT) :
            valueForKey("lock_date");
}

int DBInfo::isOperationPermitted(const QString &operation, const QString &record_owner)
{
    if (!isDatabaseLocked())
        return 4;
    if (isDatabaseRemote() && isCurrentUserAdmin())
        return 3;
    QString permission = valueForKey(operation + "_permitted", "true");
    if (permission == "true")
        return 1;
    if (permission == "owner")
        return record_owner == currentUser() ? 2 : -2;
    return -1;
}

QList<QVariantMap> DBInfo::refrigerants()
{
    QList<QVariantMap> refrigerants;

    QString refrigerants_string = valueForKey("refrigerants");
    if (!refrigerants_string.isEmpty()) {
        QByteArray refrigerants_bytes = QByteArray::fromBase64(refrigerants_string.toLatin1());
        QDataStream stream(refrigerants_bytes);
        stream.setVersion(QDataStream::Qt_4_6);
        stream >> refrigerants;
    }

    return refrigerants;
}

void DBInfo::setRefrigerants(const QList<QVariantMap> &refrigerants)
{
    QByteArray refrigerants_bytes;
    QDataStream stream(&refrigerants_bytes, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_4_6);
    stream << refrigerants;

    setValueForKey("refrigerants", QString::fromLatin1(refrigerants_bytes.toBase64()));
}

QString DBInfo::tableName()
{
    return "db_info";
}

class DBInfoColumns
{
public:
    DBInfoColumns() {
        columns << Column("id", "TEXT");
        columns << Column("value", "TEXT");
    }

    ColumnList columns;
};

const ColumnList &DBInfo::columns()
{
    static DBInfoColumns columns;
    return columns.columns;
}
