/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2026 Matus & Michal Tomlein

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

#include "businesspartner.h"

#include "inputwidgets.h"
#include "editdialoguewidgets.h"

#include <QApplication>

BusinessPartner::BusinessPartner(const QString &uuid, const QVariantMap &savedValues):
    DBRecord(tableName(), uuid, savedValues)
{}

void BusinessPartner::initEditDialogue(EditDialogueWidgets *md)
{
    md->setWindowTitle(tr("Business Partner"));
    md->addInputWidget(new MDLineEdit("name", tr("Name:"), md->widget(), name()));
    md->addInputWidget(new MDCompanyIDEdit("company_id", tr("ID:"), md->widget(), companyID()));
    md->addInputWidget(new MDLineEdit("company_vatin", tr("VAT ID:"), md->widget(), companyVATIN()));
    md->addInputWidget(new MDAddressEdit("address", tr("Address:"), md->widget(), address()));
    md->addInputWidget(new MDLineEdit("phone", tr("Phone:"), md->widget(), phone()));
    md->addInputWidget(new MDLineEdit("mail", tr("E-mail:"), md->widget(), mail()));
    md->addInputWidget(new MDLineEdit("website", tr("Website:"), md->widget(), website()));
    md->addInputWidget(new MDPlainTextEdit("notes", tr("Notes:"), md->widget(), notes()));
}

QString BusinessPartner::tableName()
{
    return "partners";
}

class BusinessPartnerColumns
{
public:
    BusinessPartnerColumns() {
        columns << Column("uuid", "UUID PRIMARY KEY");
        columns << Column("name", "TEXT");
        columns << Column("company_id", "TEXT");
        columns << Column("company_vatin", "TEXT");
        columns << Column("address", "TEXT");
        columns << Column("phone", "TEXT");
        columns << Column("mail", "TEXT");
        columns << Column("website", "TEXT");
        columns << Column("notes", "TEXT");
        columns << Column("date_updated", "TEXT");
        columns << Column("updated_by", "TEXT");
    }

    ColumnList columns;
};

const ColumnList &BusinessPartner::columns()
{
    static BusinessPartnerColumns columns;
    return columns.columns;
}

class BusinessPartnerAttributes
{
public:
    BusinessPartnerAttributes() {
        dict.insert("name", QApplication::translate("BusinessPartner", "Name"));
        dict.insert("company_id", QApplication::translate("BusinessPartner", "ID"));
        dict.insert("company_vatin", QApplication::translate("BusinessPartner", "VAT ID"));
        dict.insert("address", QApplication::translate("BusinessPartner", "Address"));
        dict.insert("phone", QApplication::translate("BusinessPartner", "Phone"));
        dict.insert("mail", QApplication::translate("BusinessPartner", "E-mail"));
        dict.insert("website", QApplication::translate("BusinessPartner", "Website"));
        dict.insert("notes", QApplication::translate("BusinessPartner", "Notes"));
    }

    MTDictionary dict;
};

const MTDictionary &BusinessPartner::attributes()
{
    static BusinessPartnerAttributes dict;
    return dict.dict;
}
