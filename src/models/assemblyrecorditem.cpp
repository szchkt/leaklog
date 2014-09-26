/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2014 Matus & Michal Tomlein

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

AssemblyRecordItem::AssemblyRecordItem(const QString &record_id):
    MTRecord(tableName(), "", "", MTDictionary("arno", record_id))
{}

AssemblyRecordItem::AssemblyRecordItem(const QString &table, const QString &id_column, const QString &id, const MTDictionary &parents):
    MTRecord(table, id_column, id, parents)
{}

QString AssemblyRecordItem::tableName()
{
    return "assembly_record_items";
}

class AssemblyRecordItemColumns
{
public:
    AssemblyRecordItemColumns() {
        columns << Column("arno", "TEXT");
        columns << Column("item_type_id", "INTEGER");
        columns << Column("value", "TEXT");
        columns << Column("acquisition_price", "REAL");
        columns << Column("list_price", "REAL");
        columns << Column("source", "INTEGER");
        columns << Column("name", "TEXT");
        columns << Column("category_id", "INTEGER");
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
        dict.insert("item_type_id", QApplication::translate("AssemblyRecordItem", "Record item type ID"));
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

AssemblyRecordItemByInspector::AssemblyRecordItemByInspector(const QString &inspector_id):
    AssemblyRecordItem("assembly_record_items LEFT JOIN inspections ON assembly_record_items.arno = inspections.arno", "", "",
                       MTDictionary(QStringList() << "item_type_id" << "source",
                                    QStringList() << inspector_id << QString::number(AssemblyRecordItem::Inspectors)))
{}
