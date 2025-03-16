/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2025 Matus & Michal Tomlein

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

#ifndef JOURNALENTRY_H
#define JOURNALENTRY_H

#include "dbrecord.h"

class JournalEntry
{
private:
    JournalEntry() {}

public:
    enum {
        Version = 3,
    };

    enum Operation {
        Insertion = 1,
        Update = 2,
        Deletion = 3
    };

    static QString tableName();
    static const ColumnList &columns();

    static int tableIDForName(const QString &name);
    static QString tableNameForID(int id, const QString &default_value = QString());

    static int columnIDForName(const QString &name);
    static QString columnNameForID(int id, const QString &default_value = QString());
    static int versionForColumnID(int column_id);
    static bool shouldJournalUpdateOnInsertionForColumnID(int column_id, const QVariant &value);
};

#endif // JOURNALENTRY_H
