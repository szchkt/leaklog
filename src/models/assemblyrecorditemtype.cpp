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

#include "assemblyrecorditemtype.h"

#include "inputwidgets.h"
#include "editdialoguewidgets.h"
#include "global.h"

#include <QApplication>

using namespace Global;

AssemblyRecordItemType::AssemblyRecordItemType(const QString &uuid):
    DBRecord(tableName(), uuid)
{}

void AssemblyRecordItemType::initEditDialogue(EditDialogueWidgets *md)
{
    QString currency = DBInfo::valueForKey("currency", "EUR");

    md->setWindowTitle(tr("Assembly Record Item Type"));

    md->addInputWidget(new MDLineEdit("name", tr("Name:"), md->widget(), name()));
    md->addInputWidget(new MDLineEdit("unit", tr("Unit:"), md->widget(), unit()));
    md->addInputWidget(new MDDoubleSpinBox("acquisition_price", tr("Acquisition price:"), md->widget(), 0.0, 999999999.9, acquisitionPrice(), currency));
    md->addInputWidget(new MDDoubleSpinBox("list_price", tr("List price:"), md->widget(), 0.0, 999999999.9, listPrice(), currency));
    md->addInputWidget(new MDDoubleSpinBox("discount", tr("Discount:"), md->widget(), 0.0, 100.0, discount(), "%"));
    md->addInputWidget(new MDSpinBox("ean", tr("EAN code:"), md->widget(), 0, 99999999, ean()));
    md->addInputWidget(new MDCheckBox("auto_show", tr("Automatically add to assembly record"), md->widget(), autoShow()));
    md->addInputWidget(new MDComboBox("ar_item_category_uuid", tr("Category:"), md->widget(), stringValue("ar_item_category_uuid"), listAssemblyRecordItemCategories(true)));
    md->addInputWidget(new MDComboBox("inspection_variable_id", tr("Get value from inspection:"), md->widget(), inspectionVariableID(), listAllVariables()));
    md->addInputWidget(new MDComboBox("value_data_type", tr("Data type:"), md->widget(), value("value_data_type", Global::Numeric).toString(), listDataTypes()));
}

QString AssemblyRecordItemType::name()
{
    return stringValue("name");
}

double AssemblyRecordItemType::acquisitionPrice()
{
    return doubleValue("acquisition_price");
}

double AssemblyRecordItemType::listPrice()
{
    return doubleValue("list_price");
}

int AssemblyRecordItemType::ean()
{
    return intValue("ean");
}

QString AssemblyRecordItemType::unit()
{
    return stringValue("unit");
}

QString AssemblyRecordItemType::inspectionVariableID()
{
    return stringValue("inspection_variable_id");
}

Global::DataType AssemblyRecordItemType::valueDataType()
{
    return (Global::DataType)intValue("value_data_type");
}

double AssemblyRecordItemType::discount()
{
    return doubleValue("discount");
}

bool AssemblyRecordItemType::autoShow()
{
    return intValue("auto_show");
}

QString AssemblyRecordItemType::tableName()
{
    return "assembly_record_item_types";
}

class AssemblyRecordItemTypeColumns
{
public:
    AssemblyRecordItemTypeColumns() {
        columns << Column("uuid", "UUID PRIMARY KEY");
        columns << Column("ar_item_category_uuid", "UUID");
        columns << Column("name", "TEXT");
        columns << Column("acquisition_price", "NUMERIC");
        columns << Column("list_price", "NUMERIC");
        columns << Column("ean", "INTEGER");
        columns << Column("unit", "TEXT");
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
        dict.insert("name", QApplication::translate("AssemblyRecordItemType", "Name"));
        dict.insert("unit", QApplication::translate("AssemblyRecordItemType", "Unit"));
        dict.insert("acquisition_price", QApplication::translate("AssemblyRecordItemType", "Acquisition price"));
        dict.insert("list_price", QApplication::translate("AssemblyRecordItemType", "List price"));
        dict.insert("discount", QApplication::translate("AssemblyRecordItemType", "Discount"));
        dict.insert("ean", QApplication::translate("AssemblyRecordItemType", "EAN code"));
        dict.insert("ar_item_category_uuid", QApplication::translate("AssemblyRecordItemType", "Category"));
    }

    MTDictionary dict;
};

const MTDictionary &AssemblyRecordItemType::attributes()
{
    static AssemblyRecordItemTypeAttributes dict;
    return dict.dict;
}
