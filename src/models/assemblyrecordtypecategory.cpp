/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2018 Matus & Michal Tomlein

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

#include "assemblyrecordtypecategory.h"

#include "inputwidgets.h"
#include "editdialoguewidgets.h"

#include <QApplication>

AssemblyRecordTypeCategory::AssemblyRecordTypeCategory(const QString &uuid, const QVariantMap &savedValues):
    MTRecord(tableName(), uuid, savedValues)
{}

QString AssemblyRecordTypeCategory::tableName()
{
    return "assembly_record_type_categories";
}

class AssemblyRecordTypeCategoryColumns
{
public:
    AssemblyRecordTypeCategoryColumns() {
        columns << Column("uuid", "UUID PRIMARY KEY");
        columns << Column("ar_type_uuid", "UUID");
        columns << Column("ar_item_category_uuid", "UUID");
        columns << Column("position", "INTEGER");
        columns << Column("date_updated", "TEXT");
        columns << Column("updated_by", "TEXT");
    }

    ColumnList columns;
};

const ColumnList &AssemblyRecordTypeCategory::columns()
{
    static AssemblyRecordTypeCategoryColumns columns;
    return columns.columns;
}

class AssemblyRecordTypeCategoryAttributes
{
public:
    AssemblyRecordTypeCategoryAttributes() {
        dict.insert("ar_type_uuid", QApplication::translate("AssemblyRecordTypeCategory", "Assembly record type ID"));
        dict.insert("ar_item_category_uuid", QApplication::translate("AssemblyRecordTypeCategory", "Assembly record category type ID"));
    }

    MTDictionary dict;
};

const MTDictionary &AssemblyRecordTypeCategory::attributes()
{
    static AssemblyRecordTypeCategoryAttributes dict;
    return dict.dict;
}
