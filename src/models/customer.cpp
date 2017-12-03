/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2017 Matus & Michal Tomlein

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

#include "customer.h"

#include "servicecompany.h"
#include "inputwidgets.h"
#include "editdialoguewidgets.h"
#include "editcustomerdialogue.h"

#include <QApplication>

Customer::Customer(const QString &id):
    DBRecord(tableName(), "id", id, MTDictionary())
{}

void Customer::initEditDialogue(EditDialogueWidgets *md)
{
    md->setWindowTitle(tr("Customer"));
    QVariantMap attributes;
    if (!id().isEmpty() || !values().isEmpty()) {
        attributes = list();
    }
    md->addInputWidget(new MDCompanyIDEdit("id", tr("ID:"), md->widget(), id()));
    md->addInputWidget(new MDLineEdit("company", tr("Company:"), md->widget(), attributes.value("company").toString()));
    md->addInputWidget(new MDAddressEdit("address", tr("Address:"), md->widget(), attributes.value("address").toString()));
    md->addInputWidget(new MDLineEdit("mail", tr("E-mail:"), md->widget(), attributes.value("mail").toString()));
    md->addInputWidget(new MDLineEdit("phone", tr("Phone:"), md->widget(), attributes.value("phone").toString()));
    MDPlainTextEdit *notes = new MDPlainTextEdit("notes", tr("Notes:"), md->widget(), attributes.value("notes").toString());
    notes->setRowSpan(0);
    md->addInputWidget(notes);
    (new OperatorInputWidget(attributes, md->widget()))->addToEditDialogue(*md);
    QStringList used_ids; MTSqlQuery query_used_ids;
    query_used_ids.setForwardOnly(true);
    query_used_ids.prepare("SELECT id FROM customers" + QString(id().isEmpty() ? "" : " WHERE id <> :id"));
    if (!id().isEmpty()) { query_used_ids.bindValue(":id", id()); }
    if (query_used_ids.exec()) {
        while (query_used_ids.next()) {
            used_ids << query_used_ids.value(0).toString();
        }
    }
    md->setUsedIds(used_ids);
}

void Customer::readOperatorValues()
{
    readValues();
    switch (value("operator_id").toInt()) {
        case -1: {
            QVariantMap service_company = ServiceCompany(DBInfo::valueForKey("default_service_company")).list();
            values().insert("operator_id", service_company.value("id"));
            values().insert("operator_company", service_company.value("name"));
            values().insert("operator_address", service_company.value("address"));
            values().insert("operator_mail", service_company.value("mail"));
            values().insert("operator_phone", service_company.value("phone"));
            break;
        }

        case 0:
            values().insert("operator_id", value("id"));
            values().insert("operator_company", value("company"));
            values().insert("operator_address", value("address"));
            values().insert("operator_mail", value("mail"));
            values().insert("operator_phone", value("phone"));
            break;
    }
}

QString Customer::tableName()
{
    return "customers";
}

class CustomerColumns
{
public:
    CustomerColumns() {
        columns << Column("id", "INTEGER PRIMARY KEY");
        columns << Column("company", "TEXT");
        columns << Column("address", "TEXT");
        columns << Column("mail", "TEXT");
        columns << Column("phone", "TEXT");
        columns << Column("operator_id", "TEXT");
        columns << Column("operator_company", "TEXT");
        columns << Column("operator_address", "TEXT");
        columns << Column("operator_mail", "TEXT");
        columns << Column("operator_phone", "TEXT");
        columns << Column("notes", "TEXT");
        columns << Column("date_updated", "TEXT");
        columns << Column("updated_by", "TEXT");
    }

    ColumnList columns;
};

const ColumnList &Customer::columns()
{
    static CustomerColumns columns;
    return columns.columns;
}

class CustomerAttributes
{
public:
    CustomerAttributes() {
        dict.insert("id", QApplication::translate("Customer", "ID"));
        dict.insert("company", QApplication::translate("Customer", "Company"));
        dict.insert("address", QApplication::translate("Customer", "Address"));
        dict.insert("mail", QApplication::translate("Customer", "E-mail"));
        dict.insert("phone", QApplication::translate("Customer", "Phone"));
        dict.insert("operator_id", QApplication::translate("Customer", "Operator ID"));
        dict.insert("operator_company", QApplication::translate("Customer", "Operator"));
        dict.insert("operator_address", QApplication::translate("Customer", "Operator address"));
        dict.insert("operator_mail", QApplication::translate("Customer", "Operator e-mail"));
        dict.insert("operator_phone", QApplication::translate("Customer", "Operator phone"));
    }

    MTDictionary dict;
};

const MTDictionary &Customer::attributes()
{
    static CustomerAttributes dict;
    return dict.dict;
}
