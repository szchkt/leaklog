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

#include "inspectioncompressor.h"

InspectionsCompressor::InspectionsCompressor(const QString &id, const MTDictionary &dict):
    MTRecord(tableName(), "id", id, dict)
{
    setSerialId(true);
}

QString InspectionsCompressor::tableName()
{
    return "inspections_compressors";
}

class InspectionsCompressorColumns
{
public:
    InspectionsCompressorColumns() {
        columns << Column("id", "SERIAL NOT NULL");
        columns << Column("customer_id", "INTEGER");
        columns << Column("circuit_id", "INTEGER");
        columns << Column("date", "TEXT");
        columns << Column("compressor_id", "BIGINT");
        columns << Column("date_updated", "TEXT");
        columns << Column("updated_by", "TEXT");
    }

    ColumnList columns;
};

const ColumnList &InspectionsCompressor::columns()
{
    static InspectionsCompressorColumns columns;
    return columns.columns;
}
