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

#include "warningfilter.h"

WarningFilter::WarningFilter(const QString &uuid, const QVariantMap &savedValues):
    MTRecord(tableName(), uuid, savedValues)
{}

QString WarningFilter::tableName()
{
    return "warnings_filters";
}

class WarningFilterColumns
{
public:
    WarningFilterColumns() {
        columns << Column("uuid", "UUID PRIMARY KEY");
        columns << Column("warning_uuid", "UUID");
        columns << Column("circuit_attribute", "TEXT");
        columns << Column("function", "TEXT");
        columns << Column("value", "TEXT");
        columns << Column("date_updated", "TEXT");
        columns << Column("updated_by", "TEXT");
    }

    ColumnList columns;
};

const ColumnList &WarningFilter::columns()
{
    static WarningFilterColumns columns;
    return columns.columns;
}
