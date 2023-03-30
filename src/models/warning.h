/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2023 Matus & Michal Tomlein

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

#ifndef WARNING_H
#define WARNING_H

#include "dbrecord.h"

class WarningFilter;
class WarningCondition;

class WarningRecord : public DBRecord
{
    Q_OBJECT

public:
    WarningRecord(const QString &uuid = QString());

    void initEditDialogue(EditDialogueWidgets *);

    inline int scope() { return intValue("scope"); }
    inline void setScope(int value) { setValue("scope", value); }
    inline bool enabled() { return intValue("enabled"); }
    inline void setEnabled(bool value) { setValue("enabled", (int)value); }
    inline QString name() { return stringValue("name"); }
    inline void setName(const QString &value) { setValue("name", value); }
    inline QString description() { return stringValue("description"); }
    inline void setDescription(const QString &value) { setValue("description", value); }
    inline int delay() { return intValue("delay"); }
    inline void setDelay(int value) { setValue("delay", value); }

    MTRecordQuery<WarningFilter> filters() const;
    MTRecordQuery<WarningCondition> conditions() const;

    static QString tableName();
    static inline MTRecordQuery<WarningRecord> query(const QVariantMap &parents = QVariantMap()) { return MTRecordQuery<WarningRecord>(tableName(), parents); }
    static const ColumnList &columns();
    bool remove() const;
};

#endif // WARNING_H
