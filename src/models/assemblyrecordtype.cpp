/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2025 Matus & Michal Tomlein

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

#include "assemblyrecordtype.h"

#include "inputwidgets.h"
#include "editdialoguewidgets.h"
#include "global.h"
#include "style.h"

#include <QApplication>

using namespace Global;

AssemblyRecordType::AssemblyRecordType(const QString &uuid):
    DBRecord(tableName(), uuid)
{}

void AssemblyRecordType::initEditDialogue(EditDialogueWidgets *md)
{
    md->setWindowTitle(tr("Assembly Record Type"));

    QStringList keywords;
    keywords << "customer_id";
    keywords << "customer_name";
    keywords << "circuit_id";
    keywords << "circuit_name";
    keywords << "day";
    keywords << "month";
    keywords << "year";

    md->addInputWidget(new MDLineEdit("name", tr("Name:"), md->widget(), name()));
    md->addInputWidget(new MDLineEdit("description", tr("Description:"), md->widget(), description()));
    MDGroupedCheckBoxes *md_display_options = new MDGroupedCheckBoxes("display_options", tr("Display options:"), md->widget(), displayOptions());
    md_display_options->addCheckBox(AssemblyRecordType::ShowServiceCompany, tr("Show service company"));
    md_display_options->addCheckBox(AssemblyRecordType::ShowCustomer, tr("Show customer"));
    md_display_options->addCheckBox(AssemblyRecordType::ShowCustomerContactPersons, tr("Show customer contact persons"));
    md_display_options->addCheckBox(AssemblyRecordType::ShowCircuit, tr("Show circuit"));
    md_display_options->addCheckBox(AssemblyRecordType::ShowCompressors, tr("Show compressors"));
    md_display_options->addCheckBox(AssemblyRecordType::ShowCircuitUnits, tr("Show circuit units"));
    md->addInputWidget(md_display_options);
    md->addInputWidget(new MDHighlightedPlainTextEdit("name_format", tr("Name format:"), md->widget(), nameFormat(), keywords));
    md->addInputWidget(new MDComboBox("style", tr("Visual style:"), md->widget(), stringValue("style_uuid"), listStyles()));
}

QString AssemblyRecordType::name()
{
    return stringValue("name");
}

QString AssemblyRecordType::description()
{
    return stringValue("description");
}

AssemblyRecordType::DisplayOptions AssemblyRecordType::displayOptions()
{
    return (DisplayOptions)intValue("display_options");
}

QString AssemblyRecordType::nameFormat()
{
    return stringValue("name_format");
}

Style AssemblyRecordType::style()
{
    return stringValue("style_uuid");
}

MTRecordQuery<AssemblyRecordTypeCategory> AssemblyRecordType::typeCategories() const
{
    return AssemblyRecordTypeCategory::query({{"ar_type_uuid", uuid()}});
}

QString AssemblyRecordType::tableName()
{
    return "assembly_record_types";
}

class AssemblyRecordTypeColumns
{
public:
    AssemblyRecordTypeColumns() {
        columns << Column("uuid", "UUID PRIMARY KEY");
        columns << Column("style_uuid", "UUID");
        columns << Column("name", "TEXT");
        columns << Column("description", "TEXT");
        columns << Column("display_options", "INTEGER");
        columns << Column("name_format", "TEXT");
        columns << Column("date_updated", "TEXT");
        columns << Column("updated_by", "TEXT");
    }

    ColumnList columns;
};

const ColumnList &AssemblyRecordType::columns()
{
    static AssemblyRecordTypeColumns columns;
    return columns.columns;
}

class AssemblyRecordTypeAttributes
{
public:
    AssemblyRecordTypeAttributes() {
        dict.insert("name", QApplication::translate("AssemblyRecordType", "Name"));
        dict.insert("description", QApplication::translate("AssemblyRecordType", "Description"));
    }

    MTDictionary dict;
};

const MTDictionary &AssemblyRecordType::attributes()
{
    static AssemblyRecordTypeAttributes dict;
    return dict.dict;
}

bool AssemblyRecordType::remove() const
{
    typeCategories().removeAll();
    return MTRecord::remove();
}
