/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2021 Matus & Michal Tomlein

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

#include "style.h"

#include "inputwidgets.h"
#include "editdialoguewidgets.h"

#include <QApplication>

Style::Style(const QString &uuid):
    DBRecord(tableName(), uuid)
{}

void Style::initEditDialogue(EditDialogueWidgets *md)
{
    md->setWindowTitle(tr("Style"));
    md->addInputWidget(new MDLineEdit("name", tr("Name:"), md->widget(), name()));
    md->addInputWidget(new MDPlainTextEdit("content", tr("Style:"), md->widget(), content()));
    md->addInputWidget(new MDCheckBox("div_tables", tr("Use div elements instead of tables"), md->widget(), usesDivElements()));
}

QString Style::tableName()
{
    return "styles";
}

class StyleColumns
{
public:
    StyleColumns() {
        columns << Column("uuid", "UUID PRIMARY KEY");
        columns << Column("name", "TEXT");
        columns << Column("content", "TEXT");
        columns << Column("div_tables", "INTEGER");
        columns << Column("date_updated", "TEXT");
        columns << Column("updated_by", "TEXT");
    }

    ColumnList columns;
};

const ColumnList &Style::columns()
{
    static StyleColumns columns;
    return columns.columns;
}

class StyleAttributes
{
public:
    StyleAttributes() {
        dict.insert("name", QApplication::translate("Style", "Name"));
        dict.insert("content", QApplication::translate("Style", "Content"));
    }

    MTDictionary dict;
};

const MTDictionary &Style::attributes()
{
    static StyleAttributes dict;
    return dict.dict;
}
