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

#include "variable.h"

#include "inputwidgets.h"
#include "editdialoguewidgets.h"
#include "variables.h"
#include "global.h"

#include <QApplication>
#include <QMessageBox>

using namespace Global;

VariableRecord::VariableRecord(const QString &uuid, const QVariantMap &savedValues):
    DBRecord(tableName(), uuid, savedValues)
{}

QStringList VariableRecord::highlightedVariableIDs()
{
    QStringList used_ids;
    used_ids << "refrigerant_amount" << "oil_amount" << "sum" << "p_to_t" << "p_to_t_vap" << "gwp" << "co2_equivalent";
    used_ids << listSupportedFunctions();
    used_ids << listVariableIds(true);
    return used_ids;
}

void VariableRecord::initEditDialogue(EditDialogueWidgets *md)
{
    md->setWindowTitle(tr("Variable"));

    QVariantMap attributes;
    bool enable_all = true;
    bool enable_id = true;

    if (!uuid().isEmpty()) {
        enable_id = false;

        Variable variable(variableID());
        if (variable.next()) {
            attributes.insert("parent_uuid", variable.value(Variable::ParentUUID));
            attributes.insert("id", variable.value(Variable::ID));
            attributes.insert("name", variable.value(Variable::Name));
            attributes.insert("type", variable.value(Variable::Type));
            attributes.insert("unit", variable.value(Variable::Unit));
            attributes.insert("value", variable.value(Variable::Value));
            attributes.insert("compare_nom", variable.value(Variable::CompareNom));
            attributes.insert("tolerance", variable.value(Variable::Tolerance));
            attributes.insert("col_bg", variable.value(Variable::ColBg));
        }

        if (variableNames().contains(variableID()))
            enable_all = false;
    }

    md->addInputWidget(new MDLineEdit("id", tr("ID:"), md->widget(), attributes.value("id").toString(), 0, "", enable_id));
    md->addInputWidget(new MDLineEdit("name", tr("Name:"), md->widget(), attributes.value("name").toString(), 0, "", enable_all));
    md->addInputWidget(new MDLineEdit("unit", tr("Unit:"), md->widget(), attributes.value("unit").toString(), 0, "", enable_all));
    MDComboBox *cb_type = new MDComboBox("type", tr("Type:"), md->widget(), attributes.value("type").toString(), variableTypes(), "", enable_all);
    if (attributes.value("type").toString() == "group")
        cb_type->setEnabled(false);
    md->addInputWidget(cb_type);
    md->addInputWidget(new MDComboBox("scope", tr("Scope:"), md->widget(), attributes.value("scope").toString(), {
        {QString::number(Variable::Inspection), tr("Inspection")},
        {QString::number(Variable::Compressor), tr("Compressor")}
    }, "", uuid().isEmpty()));
    md->addInputWidget(new MDHighlightedPlainTextEdit("value", tr("Value:"), md->widget(), attributes.value("value").toString(), highlightedVariableIDs(), enable_all));
    md->addInputWidget(new MDCheckBox("compare_nom", tr("Compare value with the nominal inspection"), md->widget(), attributes.value("compare_nom").toInt()));
    md->addInputWidget(new MDDoubleSpinBox("tolerance", tr("Tolerance:"), md->widget(), 0.0, 999999.9, attributes.value("tolerance").toDouble()));
    if (attributes.value("parent_uuid").toString().isEmpty()) {
        md->addInputWidget(new MDColourComboBox("col_bg", tr("Colour:"), md->widget(), attributes.value("col_bg").toString()));
    }
}

bool VariableRecord::checkValues(QWidget *)
{
    if (uuid().isEmpty()) {
        QString id = variableID();
        if (id.isEmpty()) {
            QMessageBox::information(NULL, tr("Save changes"), tr("Invalid ID."));
            return false;
        } else if (highlightedVariableIDs().contains(id)) {
            QMessageBox::information(NULL, tr("Save changes"), tr("This ID is not available. Please choose a different ID."));
            return false;
        }
        uuid() = createUUIDv5(DBInfo::databaseUUID(), id);
    }

    return true;
}

QString VariableRecord::tableName()
{
    return "variables";
}

class VariableColumns
{
public:
    VariableColumns() {
        columns << Column("uuid", "UUID PRIMARY KEY");
        columns << Column("parent_uuid", "UUID");
        columns << Column("id", "TEXT");
        columns << Column("name", "TEXT");
        columns << Column("type", "TEXT");
        columns << Column("unit", "TEXT");
        columns << Column("scope", "INTEGER NOT NULL DEFAULT 1");
        columns << Column("value", "TEXT");
        columns << Column("compare_nom", "INTEGER");
        columns << Column("tolerance", "NUMERIC");
        columns << Column("col_bg", "TEXT");
        columns << Column("date_updated", "TEXT");
        columns << Column("updated_by", "TEXT");
    }

    ColumnList columns;
};

const ColumnList &VariableRecord::columns()
{
    static VariableColumns columns;
    return columns.columns;
}
