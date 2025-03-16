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

#include "inspector.h"

#include "global.h"
#include "inputwidgets.h"
#include "editdialoguewidgets.h"

#include <QApplication>

using namespace Global;

Inspector::Inspector(const QString &uuid):
    DBRecord(tableName(), uuid)
{}

void Inspector::initEditDialogue(EditDialogueWidgets *md)
{
    QString currency = DBInfo::valueForKey("currency", "EUR");

    md->setWindowTitle(tr("Inspector"));
    md->addInputWidget(new MDComboBox("service_company_uuid", tr("Service company:"), md->widget(), serviceCompanyUUID(), listServiceCompanies()));
    md->addInputWidget(new MDLineEdit("certificate_number", tr("Certificate number:"), md->widget(), certificateNumber()));
    md->addInputWidget(new MDComboBox("certificate_country", tr("Country of issue:"), md->widget(), certificateCountry(), Global::countries()));
    md->addInputWidget(new MDLineEdit("person", tr("Full name:"), md->widget(), personName()));
    md->addInputWidget(new MDLineEdit("mail", tr("E-mail:"), md->widget(), mail()));
    md->addInputWidget(new MDLineEdit("phone", tr("Phone:"), md->widget(), phone()));
    MDInputWidget *iw = new MDDoubleSpinBox("acquisition_price", tr("Acquisition price:"), md->widget(), 0.0, 999999999.9, acquisitionPrice(), currency);
    iw->setRowSpan(0);
    md->addInputWidget(iw);
    iw = new MDDoubleSpinBox("list_price", tr("List price:"), md->widget(), 0.0, 999999999.9, listPrice(), currency);
    iw->setRowSpan(0);
    md->addInputWidget(iw);
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
        columns << Column("service_company_uuid", "UUID");
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
        dict.insert("certificate_number", QApplication::translate("Inspector", "Certificate number"));
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
