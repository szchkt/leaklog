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

#include "servicecompany.h"

#include "inputwidgets.h"
#include "editdialoguewidgets.h"

#include <QApplication>

ServiceCompany::ServiceCompany(const QString &uuid):
    DBRecord(tableName(), uuid)
{}

void ServiceCompany::initEditDialogue(EditDialogueWidgets *md)
{
    md->setWindowTitle(tr("Service Company"));
    md->addInputWidget(new MDLineEdit("name", tr("Name:"), md->widget(), name()));
    md->addInputWidget(new MDCompanyIDEdit("id", tr("ID:"), md->widget(), companyID()));
    md->addInputWidget(new MDAddressEdit("address", tr("Address:"), md->widget(), stringValue("address")));
    md->addInputWidget(new MDLineEdit("phone", tr("Phone:"), md->widget(), phone()));
    md->addInputWidget(new MDLineEdit("mail", tr("E-mail:"), md->widget(), mail()));
    md->addInputWidget(new MDLineEdit("website", tr("Website:"), md->widget(), website()));
    md->addInputWidget(new MDFileChooser("image", tr("Image:"), md->widget(), imageFileUUID()));
}

MTAddress ServiceCompany::address()
{
    return stringValue("address");
}

void ServiceCompany::setAddress(const MTAddress &value)
{
    setValue("address", value.toString());
}

QString ServiceCompany::tableName()
{
    return "service_companies";
}

class ServiceCompanyColumns
{
public:
    ServiceCompanyColumns() {
        columns << Column("uuid", "UUID PRIMARY KEY");
        columns << Column("image_file_uuid", "UUID");
        columns << Column("id", "TEXT");
        columns << Column("name", "TEXT");
        columns << Column("address", "TEXT");
        columns << Column("mail", "TEXT");
        columns << Column("phone", "TEXT");
        columns << Column("website", "TEXT");
        columns << Column("date_updated", "TEXT");
        columns << Column("updated_by", "TEXT");
    }

    ColumnList columns;
};

const ColumnList &ServiceCompany::columns()
{
    static ServiceCompanyColumns columns;
    return columns.columns;
}

class ServiceCompanyAttributes
{
public:
    ServiceCompanyAttributes() {
        dict.insert("name", QApplication::translate("ServiceCompany", "Name:"));
        dict.insert("id", QApplication::translate("ServiceCompany", "ID:"));
        dict.insert("address", QApplication::translate("ServiceCompany", "Address:"));
        dict.insert("phone", QApplication::translate("ServiceCompany", "Phone:"));
        dict.insert("mail", QApplication::translate("ServiceCompany", "E-mail:"));
        dict.insert("website", QApplication::translate("ServiceCompany", "Website:"));
    }

    MTDictionary dict;
};

const MTDictionary &ServiceCompany::attributes()
{
    static ServiceCompanyAttributes dict;
    return dict.dict;
}
