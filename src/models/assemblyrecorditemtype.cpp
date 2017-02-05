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

#include "assemblyrecorditemtype.h"

#include "inputwidgets.h"
#include "editdialoguewidgets.h"
#include "global.h"

#include <QApplication>

using namespace Global;

AssemblyRecordItemType::AssemblyRecordItemType(const QString &id):
    DBRecord(tableName(), "id", id, MTDictionary())
{}

void AssemblyRecordItemType::initEditDialogue(EditDialogueWidgets *md)
{
    QString currency = DBInfo::valueForKey("currency", "EUR");

    md->setWindowTitle(tr("Assembly Record Item Type"));

    QVariantMap attributes;
    if (!id().isEmpty() || !values().isEmpty()) {
        attributes = list();
    }

    md->addInputWidget(new MDHiddenIdField("id", md->widget(), attributes.value("id")));
    md->addInputWidget(new MDLineEdit("name", tr("Name:"), md->widget(), attributes.value("name").toString()));
    md->addInputWidget(new MDLineEdit("unit", tr("Unit:"), md->widget(), attributes.value("unit").toString()));
    md->addInputWidget(new MDDoubleSpinBox("acquisition_price", tr("Acquisition price:"), md->widget(), 0.0, 999999999.9, attributes.value("acquisition_price").toDouble(), currency));
    md->addInputWidget(new MDDoubleSpinBox("list_price", tr("List price:"), md->widget(), 0.0, 999999999.9, attributes.value("list_price").toDouble(), currency));
    md->addInputWidget(new MDDoubleSpinBox("discount", tr("Discount:"), md->widget(), 0.0, 100.0, attributes.value("discount").toDouble(), "%"));
    md->addInputWidget(new MDSpinBox("ean", tr("EAN code:"), md->widget(), 0, 99999999, attributes.value("ean").toInt()));
    md->addInputWidget(new MDCheckBox("auto_show", tr("Automatically add to assembly record"), md->widget(), attributes.value("auto_show").toBool()));
    md->addInputWidget(new MDComboBox("category_id", tr("Category:"), md->widget(), attributes.value("category_id").toString(), listAssemblyRecordItemCategories(true)));
    md->addInputWidget(new MDComboBox("inspection_variable_id", tr("Get value from inspection:"), md->widget(), attributes.value("inspection_variable_id").toString(), listAllVariables()));
    md->addInputWidget(new MDComboBox("value_data_type", tr("Data type:"), md->widget(), attributes.value("value_data_type", Global::Numeric).toString(), listDataTypes()));

    QStringList used_ids; MTSqlQuery query_used_ids;
    query_used_ids.setForwardOnly(true);
    query_used_ids.prepare("SELECT id FROM assembly_record_item_types" + QString(id().isEmpty() ? "" : " WHERE id <> :id"));
    if (!id().isEmpty()) { query_used_ids.bindValue(":id", id()); }
    if (query_used_ids.exec()) {
        while (query_used_ids.next()) {
            used_ids << query_used_ids.value(0).toString();
        }
    }
    md->setUsedIds(used_ids);
}

QString AssemblyRecordItemType::tableName()
{
    return "assembly_record_item_types";
}

class AssemblyRecordItemTypeColumns
{
public:
    AssemblyRecordItemTypeColumns() {
        columns << Column("id", "INTEGER PRIMARY KEY");
        columns << Column("name", "TEXT");
        columns << Column("acquisition_price", "NUMERIC");
        columns << Column("list_price", "NUMERIC");
        columns << Column("ean", "INTEGER");
        columns << Column("unit", "TEXT");
        columns << Column("category_id", "INTEGER");
        columns << Column("inspection_variable_id", "TEXT");
        columns << Column("value_data_type", "INTEGER");
        columns << Column("discount", "NUMERIC");
        columns << Column("auto_show", "INTEGER");
        columns << Column("date_updated", "TEXT");
        columns << Column("updated_by", "TEXT");
    }

    ColumnList columns;
};

const ColumnList &AssemblyRecordItemType::columns()
{
    static AssemblyRecordItemTypeColumns columns;
    return columns.columns;
}

class AssemblyRecordItemTypeAttributes
{
public:
    AssemblyRecordItemTypeAttributes() {
        dict.insert("id", QApplication::translate("AssemblyRecordItemType", "ID"));
        dict.insert("name", QApplication::translate("AssemblyRecordItemType", "Name"));
        dict.insert("unit", QApplication::translate("AssemblyRecordItemType", "Unit"));
        dict.insert("acquisition_price", QApplication::translate("AssemblyRecordItemType", "Acquisition price"));
        dict.insert("list_price", QApplication::translate("AssemblyRecordItemType", "List price"));
        dict.insert("discount", QApplication::translate("AssemblyRecordItemType", "Discount"));
        dict.insert("ean", QApplication::translate("AssemblyRecordItemType", "EAN code"));
        dict.insert("category_id", QApplication::translate("AssemblyRecordItemType", "Category"));
    }

    MTDictionary dict;
};

const MTDictionary &AssemblyRecordItemType::attributes()
{
    static AssemblyRecordItemTypeAttributes dict;
    return dict.dict;
}
