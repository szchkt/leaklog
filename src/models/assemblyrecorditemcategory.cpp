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

#include "assemblyrecorditemcategory.h"

#include "inputwidgets.h"
#include "editdialoguewidgets.h"

#include <QApplication>

AssemblyRecordItemCategory::AssemblyRecordItemCategory(const QString &uuid):
    DBRecord(tableName(), "uuid", uuid)
{}

AssemblyRecordItemCategory::AssemblyRecordItemCategory(const MTDictionary &parents):
    DBRecord(tableName(), "uuid", QString(), parents)
{}

bool AssemblyRecordItemCategory::isPredefined()
{
    return intValue("predefined");
}

QString AssemblyRecordItemCategory::name()
{
    return stringValue("name");
}

AssemblyRecordItemCategory::DisplayOptions AssemblyRecordItemCategory::displayOptions()
{
    return (DisplayOptions)intValue("display_options");
}

AssemblyRecordItemCategory::DisplayPosition AssemblyRecordItemCategory::displayPosition()
{
    return (DisplayPosition)intValue("display_position");
}

AssemblyRecordTypeCategory AssemblyRecordItemCategory::typeCategories()
{
    return AssemblyRecordTypeCategory({"ar_item_category_uuid", id()});
}

QString AssemblyRecordItemCategory::tableName()
{
    return "assembly_record_item_categories";
}

class AssemblyRecordItemCategoryColumns
{
public:
    AssemblyRecordItemCategoryColumns() {
        columns << Column("uuid", "UUID PRIMARY KEY");
        columns << Column("predefined", "SMALLINT NOT NULL DEFAULT 0");
        columns << Column("name", "TEXT");
        columns << Column("display_options", "INTEGER NOT NULL DEFAULT 0");
        columns << Column("display_position", "INTEGER NOT NULL DEFAULT 0");
        columns << Column("date_updated", "TEXT");
        columns << Column("updated_by", "TEXT");
    }

    ColumnList columns;
};

const ColumnList &AssemblyRecordItemCategory::columns()
{
    static AssemblyRecordItemCategoryColumns columns;
    return columns.columns;
}

class AssemblyRecordItemCategoryAttributes
{
public:
    AssemblyRecordItemCategoryAttributes() {
        dict.insert("name", QApplication::translate("AssemblyRecordItemCategory", "Name"));
    }

    MTDictionary dict;
};

const MTDictionary &AssemblyRecordItemCategory::attributes()
{
    static AssemblyRecordItemCategoryAttributes dict;
    return dict.dict;
}

void AssemblyRecordItemCategory::initEditDialogue(EditDialogueWidgets *md)
{
    md->setWindowTitle(tr("Assembly Record Item Category"));

    md->addInputWidget(new MDLineEdit("name", tr("Name:"), md->widget(), name()));
    MDGroupedCheckBoxes *md_display_options = new MDGroupedCheckBoxes("display_options", tr("Display options:"), md->widget(), displayOptions());
    md_display_options->addCheckBox(AssemblyRecordItemCategory::ShowValue, tr("Show value"));
    md_display_options->addCheckBox(AssemblyRecordItemCategory::ShowAcquisitionPrice, tr("Show acquisition price"));
    md_display_options->addCheckBox(AssemblyRecordItemCategory::ShowListPrice, tr("Show list price"));
    md_display_options->addCheckBox(AssemblyRecordItemCategory::ShowDiscount, tr("Show discount"));
    md_display_options->addCheckBox(AssemblyRecordItemCategory::ShowTotal, tr("Calculate total"));
    md->addInputWidget(md_display_options);
    MDRadioButtonGroup *md_position = new MDRadioButtonGroup("display_position", tr("Display:"), md->widget(), QString::number(displayPosition()));
    md_position->addRadioButton(tr("In table"), QString::number(AssemblyRecordItemCategory::DisplayAtTop));
    md_position->addRadioButton(tr("Separately"), QString::number(AssemblyRecordItemCategory::DisplayAtBottom));
    md->addInputWidget(md_position);
}
