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

#include "inspector.h"

#include "global.h"
#include "inputwidgets.h"
#include "editdialoguewidgets.h"

#include <QApplication>

Inspector::Inspector(const QString &id):
    DBRecord(tableName(), "id", id, MTDictionary())
{}

void Inspector::initEditDialogue(EditDialogueWidgets *md)
{
    QString currency = DBInfo::valueForKey("currency", "EUR");

    md->setWindowTitle(tr("Inspector"));
    QVariantMap attributes;
    if (!id().isEmpty()) {
        attributes = list();
    }
    md->addInputWidget(new MDLineEdit("id", tr("Certificate number:"), md->widget(), attributes.value("id").toString(), 99999));
    md->addInputWidget(new MDLineEdit("certificate_number", tr("Foreign certificate number:"), md->widget(), attributes.value("certificate_number").toString()));
    md->addInputWidget(new MDComboBox("certificate_country", tr("Country of issue:"), md->widget(), attributes.value("certificate_country").toString(), Global::countries()));
    md->addInputWidget(new MDLineEdit("person", tr("Full name:"), md->widget(), attributes.value("person").toString()));
    md->addInputWidget(new MDLineEdit("mail", tr("E-mail:"), md->widget(), attributes.value("mail").toString()));
    md->addInputWidget(new MDLineEdit("phone", tr("Phone:"), md->widget(), attributes.value("phone").toString()));
    MDInputWidget *iw = new MDDoubleSpinBox("acquisition_price", tr("Acquisition price:"), md->widget(), 0.0, 999999999.9, attributes.value("acquisition_price").toDouble(), currency);
    iw->setShowInForm(false);
    md->addInputWidget(iw);
    iw = new MDDoubleSpinBox("list_price", tr("List price:"), md->widget(), 0.0, 999999999.9, attributes.value("list_price").toDouble(), currency);
    iw->setShowInForm(false);
    md->addInputWidget(iw);
    QStringList used_ids; MTSqlQuery query_used_ids;
    query_used_ids.setForwardOnly(true);
    query_used_ids.prepare("SELECT id FROM inspectors" + QString(id().isEmpty() ? "" : " WHERE id <> :id"));
    if (!id().isEmpty()) { query_used_ids.bindValue(":id", id()); }
    if (query_used_ids.exec()) {
        while (query_used_ids.next()) {
            used_ids << query_used_ids.value(0).toString();
        }
    }
    md->setUsedIds(used_ids);
}

QString Inspector::tableName()
{
    return "inspectors";
}

class InspectorColumns
{
public:
    InspectorColumns() {
        columns << Column("uuid", "UUID PRIMARY KEY");
        columns << Column("certificate_number", "TEXT");
        columns << Column("certificate_country", "TEXT");
        columns << Column("person", "TEXT");
        columns << Column("mail", "TEXT");
        columns << Column("phone", "TEXT");
        columns << Column("list_price", "NUMERIC");
        columns << Column("acquisition_price", "NUMERIC");
        columns << Column("date_updated", "TEXT");
        columns << Column("updated_by", "TEXT");
    }

    ColumnList columns;
};

const ColumnList &Inspector::columns()
{
    static InspectorColumns columns;
    return columns.columns;
}

class InspectorAttributes
{
public:
    InspectorAttributes() {
        dict.insert("id", QApplication::translate("Inspector", "Certificate number"));
        dict.insert("certificate_number", QApplication::translate("Inspector", "Foreign certificate number"));
        dict.insert("certificate_country", QApplication::translate("Inspector", "Country of issue"));
        dict.insert("person", QApplication::translate("Inspector", "Full name"));
        dict.insert("mail", QApplication::translate("Inspector", "E-mail"));
        dict.insert("phone", QApplication::translate("Inspector", "Phone"));
    }

    MTDictionary dict;
};

const MTDictionary &Inspector::attributes()
{
    static InspectorAttributes dict;
    return dict.dict;
}
