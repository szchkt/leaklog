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

#include "table.h"

#include "inputwidgets.h"
#include "editdialoguewidgets.h"
#include "variables.h"

#include <QApplication>

Table::Table(const QString &uuid):
    DBRecord(tableName(), "uuid", uuid)
{}

void Table::initEditDialogue(EditDialogueWidgets *md)
{
    md->setWindowTitle(tr("Table"));

    md->addInputWidget(new MDLineEdit("name", tr("Name:"), md->widget(), name()));
    md->addInputWidget(new MDCheckBox("highlight_nominal", tr("Highlight nominal inspections"), md->widget(), highlightNominal()));
    md->addInputWidget(new MDComboBox("scope", tr("Scope:"), md->widget(), stringValue("scope"), {
        {QString::number(Variable::Inspection), tr("Inspection")},
        {QString::number(Variable::Compressor), tr("Compressor")}
    }));

    QStringList used_ids; MTSqlQuery query_used_ids;
    query_used_ids.setForwardOnly(true);
    query_used_ids.prepare("SELECT name FROM tables" + QString(name().isEmpty() ? "" : " WHERE name <> :name"));
    if (!id().isEmpty()) { query_used_ids.bindValue(":name", name()); }
    if (query_used_ids.exec()) {
        while (query_used_ids.next()) {
            used_ids << query_used_ids.value(0).toString();
        }
    }
    md->setUsedIds(used_ids);
}

QString Table::tableName()
{
    return "tables";
}

class TableColumns
{
public:
    TableColumns() {
        columns << Column("uuid", "UUID PRIMARY KEY");
        columns << Column("name", "TEXT");
        columns << Column("position", "INTEGER NOT NULL DEFAULT 0");
        columns << Column("highlight_nominal", "INTEGER NOT NULL DEFAULT 0");
        columns << Column("scope", "INTEGER NOT NULL DEFAULT 1");
        columns << Column("variables", "TEXT");
        columns << Column("sum", "TEXT");
        columns << Column("avg", "TEXT");
        columns << Column("date_updated", "TEXT");
        columns << Column("updated_by", "TEXT");
    }

    ColumnList columns;
};

const ColumnList &Table::columns()
{
    static TableColumns columns;
    return columns.columns;
}
