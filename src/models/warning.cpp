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

#include "warning.h"

#include "inputwidgets.h"
#include "editdialoguewidgets.h"
#include "global.h"
#include "variables.h"
#include "warnings.h"

#include <QApplication>

using namespace Global;

WarningRecord::WarningRecord(const QString &id):
    DBRecord(tableName(), "id", id, MTDictionary())
{}

void WarningRecord::initEditDialogue(EditDialogueWidgets *md)
{
    md->setWindowTitle(tr("Warning"));
    QVariantMap attributes; bool enable_all = true;
    if (!id().isEmpty()) {
        Warning query(id().toInt());
        if (query.next()) {
            for (int i = 0; i < query.record().count(); ++i) {
                attributes.insert(query.record().fieldName(i), query.value(query.record().fieldName(i)));
            }
        }
        if (!attributes.value("name").toString().isEmpty()) {
            md->setWindowTitle(tr("%1: %2").arg(tr("Warning")).arg(attributes.value("name").toString()));
        }
        enable_all = id().toInt() < 1000;
    }
    md->addInputWidget(new MDCheckBox("enabled", tr("Enabled"), md->widget(), attributes.value("enabled").toInt()));
    md->addInputWidget(new MDLineEdit("name", tr("Name:"), md->widget(), attributes.value("name").toString(), 0, "", enable_all));
    md->addInputWidget(new MDLineEdit("description", tr("Description:"), md->widget(), attributes.value("description").toString(), 0, "", enable_all));
    md->addInputWidget(new MDSpinBox("delay", tr("Delay:"), md->widget(), 0, 999999, attributes.value("delay").toInt(), tr("days"), "", enable_all));
    md->addInputWidget(new MDComboBox("scope", tr("Scope:"), md->widget(), attributes.value("scope").toString(),
                                      MTDictionary(QStringList()
                                                   << QString::number(Variable::Inspection)
                                                   << QString::number(Variable::Compressor),
                                                   QStringList() << tr("Inspection") << tr("Compressor")),
                                      QString(), enable_all));
    QStringList used_ids;
    used_ids << "refrigerant_amount" << "oil_amount" << "sum" << "p_to_t" << "p_to_t_vap";
    used_ids << listSupportedFunctions();
    used_ids << listVariableIds();
    md->setUsedIds(used_ids);
}

QString WarningRecord::tableName()
{
    return "warnings";
}

class WarningColumns
{
public:
    WarningColumns() {
        columns << Column("uuid", "UUID PRIMARY KEY");
        columns << Column("scope", "INTEGER DEFAULT 1 NOT NULL");
        columns << Column("enabled", "INTEGER");
        columns << Column("name", "TEXT");
        columns << Column("description", "TEXT");
        columns << Column("delay", "INTEGER");
        columns << Column("date_updated", "TEXT");
        columns << Column("updated_by", "TEXT");
    }

    ColumnList columns;
};

const ColumnList &WarningRecord::columns()
{
    static WarningColumns columns;
    return columns.columns;
}
