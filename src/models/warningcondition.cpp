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

#include "warningcondition.h"

WarningCondition::WarningCondition(const QString &uuid, const QVariantMap &savedValues):
    MTRecord(tableName(), uuid, savedValues)
{}

QString WarningCondition::tableName()
{
    return "warnings_conditions";
}

class WarningConditionColumns
{
public:
    WarningConditionColumns() {
        columns << Column("uuid", "UUID PRIMARY KEY");
        columns << Column("warning_uuid", "UUID");
        columns << Column("value_ins", "TEXT");
        columns << Column("function", "TEXT");
        columns << Column("value_nom", "TEXT");
        columns << Column("date_updated", "TEXT");
        columns << Column("updated_by", "TEXT");
    }

    ColumnList columns;
};

const ColumnList &WarningCondition::columns()
{
    static WarningConditionColumns columns;
    return columns.columns;
}
