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

#include "assemblyrecordtype.h"

#include "inputwidgets.h"
#include "editdialoguewidgets.h"
#include "global.h"

#include <QApplication>

using namespace Global;

AssemblyRecordType::AssemblyRecordType(const QString &id):
    DBRecord("assembly_record_types", "id", id, MTDictionary())
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

    QVariantMap attributes;
    if (!id().isEmpty() || !values().isEmpty()) {
        attributes = list();
    }

    md->addInputWidget(new MDHiddenIdField("id", md->widget(), attributes.value("id")));
    md->addInputWidget(new MDLineEdit("name", tr("Name:"), md->widget(), attributes.value("name").toString()));
    md->addInputWidget(new MDLineEdit("description", tr("Description:"), md->widget(), attributes.value("description").toString()));
    MDGroupedCheckBoxes *md_display_options = new MDGroupedCheckBoxes("display_options", tr("Display options:"), md->widget(), attributes.value("display_options").toInt());
    md_display_options->addCheckBox(AssemblyRecordType::ShowServiceCompany, tr("Show service company"));
    md_display_options->addCheckBox(AssemblyRecordType::ShowCustomer, tr("Show customer"));
    md_display_options->addCheckBox(AssemblyRecordType::ShowCustomerContactPersons, tr("Show customer contact persons"));
    md_display_options->addCheckBox(AssemblyRecordType::ShowCircuit, tr("Show circuit"));
    md_display_options->addCheckBox(AssemblyRecordType::ShowCompressors, tr("Show compressors"));
    md_display_options->addCheckBox(AssemblyRecordType::ShowCircuitUnits, tr("Show circuit units"));
    md->addInputWidget(md_display_options);
    md->addInputWidget(new MDHighlightedPlainTextEdit("name_format", tr("Name format:"), md->widget(), attributes.value("name_format").toString(), keywords));
    md->addInputWidget(new MDComboBox("style", tr("Visual style:"), md->widget(), attributes.value("style").toString(), listStyles()));
    QStringList used_ids; MTSqlQuery query_used_ids;
    query_used_ids.setForwardOnly(true);
    query_used_ids.prepare("SELECT id FROM assembly_record_types" + QString(id().isEmpty() ? "" : " WHERE id <> :id"));
    if (!id().isEmpty()) { query_used_ids.bindValue(":id", id()); }
    if (query_used_ids.exec()) {
        while (query_used_ids.next()) {
            used_ids << query_used_ids.value(0).toString();
        }
    }
    md->setUsedIds(used_ids);
}

class AssemblyRecordTypeAttributes
{
public:
    AssemblyRecordTypeAttributes() {
        dict.insert("id", QApplication::translate("AssemblyRecordType", "ID"));
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

bool AssemblyRecordType::remove()
{
    AssemblyRecordTypeCategory type_categories(id());
    return type_categories.remove() && MTRecord::remove();
}
