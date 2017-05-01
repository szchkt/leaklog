/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2017 Matus & Michal Tomlein

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

class JournalEntry : public MTRecord
{
public:
    enum Operation {
        Insertion = 1,
        Update = 2,
        Deletion = 3
    };

    JournalEntry(const QString &id = QString());

    inline QString sourceUUID() { return stringValue("source_uuid"); }
    inline void setSourceUUID(const QString &value) { setValue("source_uuid", value); }
    inline int entryID() { return intValue("entry_id"); }
    inline void setEntryID(int value) { setValue("entry_id", value); }
    inline Operation operation() { return (Operation)intValue("operation_id"); }
    inline void setOperation(Operation value) { setValue("operation_id", value); }
    inline int tableID() { return intValue("table_id"); }
    inline void setTableID(int value) { setValue("table_id", value); }
    inline QString recordUUID() { return stringValue("row_uuid"); }
    inline void setRecordUUID(const QString &value) { setValue("row_uuid", value); }
    inline int columnID() { return intValue("column_id"); }
    inline void setColumnID(int value) { setValue("column_id", value); }
    inline QString dateCreated() { return stringValue("date_created"); }

    static QString tableName();
    static const ColumnList &columns();

    static int tableIDForName(const QString &name);
    static QString tableNameForID(int id);

    static int columnIDForName(const QString &name);
    static QString columnNameForID(int id);

protected:
    bool isJournaled() const;
};

#endif // JOURNALENTRY_H
