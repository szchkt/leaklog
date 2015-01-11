/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2015 Matus & Michal Tomlein

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

#include "compressor.h"

#include <QApplication>

Compressor::Compressor(const QString &id, const MTDictionary &dict):
    MTRecord(tableName(), "id", id, dict)
{
    setSerialId(true);
}

QString Compressor::tableName()
{
    return "compressors";
}

class CompressorColumns
{
public:
    CompressorColumns() {
        columns << Column("id", "BIGINT NOT NULL");
        columns << Column("customer_id", "INTEGER");
        columns << Column("circuit_id", "INTEGER");
        columns << Column("name", "TEXT");
        columns << Column("manufacturer", "TEXT");
        columns << Column("type", "TEXT");
        columns << Column("sn", "TEXT");
        columns << Column("date_updated", "TEXT");
        columns << Column("updated_by", "TEXT");
    }

    ColumnList columns;
};

const ColumnList &Compressor::columns()
{
    static CompressorColumns columns;
    return columns.columns;
}

class CompressorAttributes
{
public:
    CompressorAttributes() {
        dict.insert("id", QApplication::translate("Compressor", "ID"));
        dict.insert("name", QApplication::translate("Compressor", "Compressor name"));
        dict.insert("manufacturer", QApplication::translate("Compressor", "Manufacturer"));
        dict.insert("type", QApplication::translate("Compressor", "Type"));
        dict.insert("sn", QApplication::translate("Compressor", "Serial number"));
    }

    MTDictionary dict;
};

const MTDictionary &Compressor::attributes()
{
    static CompressorAttributes dict;
    return dict.dict;
}
