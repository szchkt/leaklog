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

#include "inspectionfile.h"

#include "inspection.h"
#include "file.h"

InspectionFile::InspectionFile(const QString &uuid, const QVariantMap &savedValues):
    MTRecord(tableName(), uuid, savedValues)
{}

Inspection InspectionFile::inspection()
{
    return inspectionUUID();
}

File InspectionFile::file()
{
    return fileUUID();
}

QString InspectionFile::tableName()
{
    return "inspections_files";
}

class InspectionFileColumns
{
public:
    InspectionFileColumns() {
        columns << Column("uuid", "UUID PRIMARY KEY");
        columns << Column("inspection_uuid", "UUID");
        columns << Column("file_uuid", "UUID");
        columns << Column("description", "TEXT");
        columns << Column("date_updated", "TEXT");
        columns << Column("updated_by", "TEXT");
    }

    ColumnList columns;
};

const ColumnList &InspectionFile::columns()
{
    static InspectionFileColumns columns;
    return columns.columns;
}
