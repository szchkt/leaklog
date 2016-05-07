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

#include "repair.h"

#include "inputwidgets.h"
#include "editdialoguewidgets.h"
#include "global.h"

#include <QApplication>

using namespace Global;

Repair::Repair(const QString &uuid, const QVariantMap &savedValues):
    DBRecord(tableName(), "uuid", uuid, savedValues)
{}

void Repair::initEditDialogue(EditDialogueWidgets *md)
{
    MTDictionary refrigerants(listRefrigerantsToString().split(';'));

    md->setWindowTitle(tr("Repair"));
    MDDateTimeEdit *date_edit = new MDDateTimeEdit("date", tr("Date:"), md->widget(), date());
    if (DBInfo::isDatabaseLocked())
        date_edit->setMinimumDate(QDate::fromString(DBInfo::lockDate(), DATE_FORMAT));
    md->addInputWidget(date_edit);
    MDLineEdit *customer_edit = new MDLineEdit("customer", tr("Customer:"), md->widget(), customer());
    if (!customerUUID().isEmpty())
        customer_edit->setEnabled(false);
    md->addInputWidget(customer_edit);
    md->addInputWidget(new MDLineEdit("device", tr("Device:"), md->widget(), device()));
    md->addInputWidget(new MDComboBox("field", tr("Field of application:"), md->widget(), field(), fieldsOfApplication()));
    md->addInputWidget(new MDComboBox("refrigerant", tr("Refrigerant:"), md->widget(), refrigerant(), refrigerants));
    MDComboBox *repairman = new MDComboBox("inspector_uuid", tr("Inspector:"), md->widget(), inspectorUUID(), listInspectors());
    repairman->setNullValue(QVariant(QVariant::Int));
    md->addInputWidget(repairman);
    md->addInputWidget(new MDLineEdit("arno", tr("Assembly record No.:"), md->widget(), arno()));
    md->addInputWidget(new MDDoubleSpinBox("refrigerant_amount", tr("Amount of refrigerant:"), md->widget(), 0.0, 999999.9, refrigerantAmount(), QApplication::translate("Units", "kg")));
    md->addInputWidget(new MDDoubleSpinBox("refr_add_am", tr("Refrigerant addition:"), md->widget(), -999999999.9, 999999999.9, refrigerantAddition(), QApplication::translate("Units", "kg")));
    md->addInputWidget(new MDDoubleSpinBox("refr_reco", tr("Refrigerant recovery:"), md->widget(), -999999999.9, 999999999.9, refrigerantRecovery(), QApplication::translate("Units", "kg")));
    QStringList used_ids; MTSqlQuery query_used_ids;
    query_used_ids.setForwardOnly(true);
    query_used_ids.prepare("SELECT date FROM repairs" + QString(id().isEmpty() ? "" : " WHERE date <> :date"));
    if (!id().isEmpty()) { query_used_ids.bindValue(":date", id()); }
    if (query_used_ids.exec()) {
        while (query_used_ids.next()) {
            used_ids << query_used_ids.value(0).toString();
        }
    }
    md->setUsedIds(used_ids);
}

QString Repair::tableName()
{
    return "repairs";
}

class RepairColumns
{
public:
    RepairColumns() {
        columns << Column("uuid", "UUID PRIMARY KEY");
        columns << Column("customer_uuid", "UUID");
        columns << Column("inspector_uuid", "UUID");
        columns << Column("date", "TEXT");
        columns << Column("customer", "TEXT");
        columns << Column("device", "TEXT");
        columns << Column("field", "TEXT");
        columns << Column("refrigerant", "TEXT");
        columns << Column("refrigerant_amount", "NUMERIC");
        columns << Column("refr_add_am", "NUMERIC");
        columns << Column("refr_reco", "NUMERIC");
        columns << Column("arno", "TEXT");
        columns << Column("date_updated", "TEXT");
        columns << Column("updated_by", "TEXT");
    }

    ColumnList columns;
};

const ColumnList &Repair::columns()
{
    static RepairColumns columns;
    return columns.columns;
}

class RepairAttributes
{
public:
    RepairAttributes() {
        dict.insert("date", QApplication::translate("Repair", "Date"));
        dict.insert("customer", QApplication::translate("Repair", "Customer"));
        dict.insert("device", QApplication::translate("Repair", "Device"));
        dict.insert("field", QApplication::translate("Repair", "Field of application"));
        dict.insert("refrigerant", QApplication::translate("Repair", "Refrigerant"));
        dict.insert("refrigerant_amount", QApplication::translate("Repair", "Amount of refrigerant"));
        dict.insert("refr_add_am", QApplication::translate("Repair", "Refrigerant addition"));
        dict.insert("refr_reco", QApplication::translate("Repair", "Refrigerant recovery"));
        dict.insert("inspector_uuid", QApplication::translate("Repair", "Inspector"));
        dict.insert("arno", QApplication::translate("Repair", "Assembly record No."));
    }

    MTDictionary dict;
};

const MTDictionary &Repair::attributes()
{
    static RepairAttributes dict;
    return dict.dict;
}
