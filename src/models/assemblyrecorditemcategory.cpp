/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2016 Matus & Michal Tomlein

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

AssemblyRecordItemCategory::AssemblyRecordItemCategory(const QString &id):
    DBRecord(tableName(), "id", id, MTDictionary())
{
}

QString AssemblyRecordItemCategory::tableName()
{
    return "assembly_record_item_categories";
}

class AssemblyRecordItemCategoryColumns
{
public:
    AssemblyRecordItemCategoryColumns() {
        columns << Column("id", "INTEGER PRIMARY KEY");
        columns << Column("name", "TEXT");
        columns << Column("display_options", "INTEGER");
        columns << Column("display_position", "INTEGER");
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
        dict.insert("id", QApplication::translate("AssemblyRecordItemCategory", "ID"));
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

    QVariantMap attributes;
    if (!id().isEmpty() || !values().isEmpty()) {
        attributes = list();
    }

    md->addInputWidget(new MDHiddenIdField("id", md->widget(), attributes.value("id").toString()));
    md->addInputWidget(new MDLineEdit("name", tr("Name:"), md->widget(), attributes.value("name").toString()));
    MDGroupedCheckBoxes *md_display_options = new MDGroupedCheckBoxes("display_options", tr("Display options:"), md->widget(), attributes.value("display_options").toInt());
    md_display_options->addCheckBox(AssemblyRecordItemCategory::ShowValue, tr("Show value"));
    md_display_options->addCheckBox(AssemblyRecordItemCategory::ShowAcquisitionPrice, tr("Show acquisition price"));
    md_display_options->addCheckBox(AssemblyRecordItemCategory::ShowListPrice, tr("Show list price"));
    md_display_options->addCheckBox(AssemblyRecordItemCategory::ShowDiscount, tr("Show discount"));
    md_display_options->addCheckBox(AssemblyRecordItemCategory::ShowTotal, tr("Calculate total"));
    md->addInputWidget(md_display_options);
    MDRadioButtonGroup *md_position = new MDRadioButtonGroup("display_position", tr("Display:"), md->widget(), QString::number(attributes.value("display_position").toInt()));
    md_position->addRadioButton(tr("In table"), QString::number(AssemblyRecordItemCategory::DisplayAtTop));
    md_position->addRadioButton(tr("Separately"), QString::number(AssemblyRecordItemCategory::DisplayAtBottom));
    md->addInputWidget(md_position);
    QStringList used_ids; MTSqlQuery query_used_ids;
    query_used_ids.setForwardOnly(true);
    query_used_ids.prepare("SELECT id FROM assembly_record_item_categories" + QString(id().isEmpty() ? "" : " WHERE id <> :id"));
    if (!id().isEmpty()) { query_used_ids.bindValue(":id", id()); }
    if (query_used_ids.exec()) {
        while (query_used_ids.next()) {
            used_ids << query_used_ids.value(0).toString();
        }
    }
    md->setUsedIds(used_ids);
}
