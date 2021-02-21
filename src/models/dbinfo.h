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

#ifndef DBINFO_H
#define DBINFO_H

#include "dbrecord.h"

class DBInfo
{
private:
    DBInfo() {}

public:
    static QString valueForKey(const QString &, const QString & = QString(), const QSqlDatabase & = QSqlDatabase::database());
    static QSqlError setValueForKey(const QString &, const QString &, const QSqlDatabase & = QSqlDatabase::database());

    static QString databaseUUID(const QSqlDatabase & = QSqlDatabase::database());
    static QString databaseName(const QSqlDatabase & = QSqlDatabase::database());
    static void setDatabaseName(const QString &database_name, const QSqlDatabase & = QSqlDatabase::database());
    static QString autosaveMode();
    static void setAutosaveMode(const QString &autosave_mode);
    static bool isCurrentUserAdmin();
    static int isDatabaseLocked();
    static bool isRecordLocked(const QString &);
    static QString lockDate();
    static int isOperationPermitted(const QString &, const QString & = QString());
    static QList<QVariantMap> refrigerants();
    static void setRefrigerants(const QList<QVariantMap> &refrigerants);

    static QString tableName();
    static const ColumnList &columns();
};

#endif // DBINFO_H
