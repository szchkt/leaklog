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

#include "assemblyrecorditem.h"

#include "inputwidgets.h"
#include "editdialoguewidgets.h"

#include <QApplication>

AssemblyRecordItem::AssemblyRecordItem(const QString &uuid):
    MTRecord(tableName(), "uuid", uuid)
{}

AssemblyRecordItem::AssemblyRecordItem(const MTDictionary &parents):
    MTRecord(tableName(), "uuid", QString(), parents)
{}

AssemblyRecordItem::AssemblyRecordItem(const QString &table, const QString &id_field, const QString &id, const MTDictionary &parents):
    MTRecord(table, id_field, id, parents)
{}

QString AssemblyRecordItem::tableName()
{
    return "assembly_record_items";
}

class AssemblyRecordItemColumns
{
public:
    AssemblyRecordItemColumns() {
        columns << Column("uuid", "UUID PRIMARY KEY");
        columns << Column("ar_item_type_uuid", "UUID");
        columns << Column("ar_item_category_uuid", "UUID");
        columns << Column("arno", "TEXT");
        columns << Column("value", "TEXT");
        columns << Column("acquisition_price", "NUMERIC");
        columns << Column("list_price", "NUMERIC");
        columns << Column("source", "SMALLINT NOT NULL DEFAULT 0");
        columns << Column("name", "TEXT");
        columns << Column("unit", "TEXT");
        columns << Column("discount", "NUMERIC");
        columns << Column("date_updated", "TEXT");
        columns << Column("updated_by", "TEXT");
    }

    ColumnList columns;
};

const ColumnList &AssemblyRecordItem::columns()
{
    static AssemblyRecordItemColumns columns;
    return columns.columns;
}

class AssemblyRecordItemAttributes
{
public:
    AssemblyRecordItemAttributes() {
        dict.insert("arno", QApplication::translate("AssemblyRecordItem", "Assembly record number"));
        dict.insert("ar_item_type_uuid", QApplication::translate("AssemblyRecordItem", "Record item type ID"));
        dict.insert("value", QApplication::translate("AssemblyRecordItem", "Value"));
        dict.insert("acquisition_price", QApplication::translate("AssemblyRecordItem", "Acquisition price"));
        dict.insert("list_price", QApplication::translate("AssemblyRecordItem", "List price"));
    }

    MTDictionary dict;
};

const MTDictionary &AssemblyRecordItem::attributes()
{
    static AssemblyRecordItemAttributes dict;
    return dict.dict;
}

AssemblyRecordItemByInspector::AssemblyRecordItemByInspector(const QString &inspector_uuid):
    AssemblyRecordItem("assembly_record_items LEFT JOIN inspections ON assembly_record_items.arno = inspections.arno"
                       " LEFT JOIN circuits ON inspections.circuit_uuid = circuits.uuid"
                       " LEFT JOIN customers ON inspections.customer_uuid = customers.uuid", "", "",
                       {{"ar_item_type_uuid", inspector_uuid}, {"source", QString::number(AssemblyRecordItem::Inspectors)}})
{}
