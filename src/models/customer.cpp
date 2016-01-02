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

#include "customer.h"

#include "servicecompany.h"
#include "inputwidgets.h"
#include "editdialoguewidgets.h"
#include "editcustomerdialogue.h"

#include <QApplication>

Customer::Customer(const QString &uuid):
    DBRecord(tableName(), "uuid", uuid)
{}

void Customer::initEditDialogue(EditDialogueWidgets *md)
{
    md->setWindowTitle(tr("Customer"));
    md->addInputWidget(new MDCompanyIDEdit("id", tr("ID:"), md->widget(), companyID()));
    md->addInputWidget(new MDLineEdit("company", tr("Company:"), md->widget(), companyName()));
    md->addInputWidget(new MDAddressEdit("address", tr("Address:"), md->widget(), stringValue("address")));
    md->addInputWidget(new MDLineEdit("mail", tr("E-mail:"), md->widget(), mail()));
    md->addInputWidget(new MDLineEdit("phone", tr("Phone:"), md->widget(), phone()));
    (new OperatorInputWidget(savedValues(), md->widget()))->addToEditDialogue(*md);
}

void Customer::readOperatorValues()
{
    switch (operatorType()) {
        case OperatorTypeServiceCompany: {
            ServiceCompany service_company = ServiceCompany(DBInfo::valueForKey("default_service_company_uuid"));
            setValue("operator_id", service_company.value("id"));
            setValue("operator_company", service_company.value("name"));
            setValue("operator_address", service_company.value("address"));
            setValue("operator_mail", service_company.value("mail"));
            setValue("operator_phone", service_company.value("phone"));
            break;
        }

        case OperatorTypeCustomer:
            setValue("operator_id", value("id"));
            setValue("operator_company", value("company"));
            setValue("operator_address", value("address"));
            setValue("operator_mail", value("mail"));
            setValue("operator_phone", value("phone"));
            break;

        case OperatorTypeOther:
            break;
    }
}

QString Customer::companyID()
{
    return stringValue("id");
}

QString Customer::companyName()
{
    return stringValue("company");
}

MTAddress Customer::address()
{
    return stringValue("address");
}

QString Customer::mail()
{
    return stringValue("mail");
}

QString Customer::phone()
{
    return stringValue("phone");
}

Customer::OperatorType Customer::operatorType()
{
    return (OperatorType)intValue("operatorType");
}

QString Customer::operatorCompanyID()
{
    return stringValue("operator_id");
}

QString Customer::operatorCompanyName()
{
    return stringValue("operator_company");
}

MTAddress Customer::operatorAddress()
{
    return stringValue("operator_address");
}

QString Customer::operatorMail()
{
    return stringValue("operator_mail");
}

QString Customer::operatorPhone()
{
    return stringValue("operator_phone");
}

Circuit Customer::circuits()
{
    Circuit circuits;
    circuits.parents().insert("customer_uuid", id());
    return circuits;
}

Person Customer::persons()
{
    return Person({"customer_uuid", id()});
}

QString Customer::tableName()
{
    return "customers";
}

class CustomerColumns
{
public:
    CustomerColumns() {
        columns << Column("uuid", "UUID PRIMARY KEY");
        columns << Column("id", "TEXT");
        columns << Column("company", "TEXT");
        columns << Column("address", "TEXT");
        columns << Column("mail", "TEXT");
        columns << Column("phone", "TEXT");
        columns << Column("operator_type", "SMALLINT NOT NULL DEFAULT 0");
        columns << Column("operator_id", "TEXT");
        columns << Column("operator_company", "TEXT");
        columns << Column("operator_address", "TEXT");
        columns << Column("operator_mail", "TEXT");
        columns << Column("operator_phone", "TEXT");
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
        // numBasicAttributes: 5
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

int Customer::numBasicAttributes()
{
    return 5;
}
