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

#include "repair.h"

#include "inputwidgets.h"
#include "editdialoguewidgets.h"
#include "global.h"

#include <QApplication>

using namespace Global;

Repair::Repair(const QString &uuid, const QVariantMap &savedValues):
    DBRecord(tableName(), uuid, savedValues)
{}

void Repair::initEditDialogue(EditDialogueWidgets *md)
{
    MTDictionary refrigerants(listRefrigerants());

    md->setWindowTitle(tr("Repair"));
    md->addInputWidget(new MDComboBox("service_company_uuid", tr("Service company:"), md->widget(), serviceCompanyUUID(), listServiceCompanies()));

    MTDictionary types;
    types.insert(QString::number(NominalRepair), tr("Commissioning"));
    types.insert(QString::number(RegularRepair), tr("Repair"));
    MDComboBox *cb_nominal = new MDComboBox("repair_type", tr("Type:"), md->widget(), QString::number(type()), types);
    md->addInputWidget(cb_nominal);

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
    md->addInputWidget(new MDDoubleSpinBox("refrigerant_amount", tr("Refrigerant amount total:"), md->widget(), 0.0, 999999.9, refrigerantAmount(), QApplication::translate("Units", "kg")));

    MDDoubleSpinBox *iw = new MDDoubleSpinBox("refr_add_am", tr("Refrigerant addition (new):"), md->widget(), -999999999.9, 999999999.9, refrigerantAddition(), QApplication::translate("Units", "kg"));
    iw->label()->setDefaultText(QApplication::translate("Variables", "New charge (new):"));
    iw->label()->toggleAlternativeText(cb_nominal->currentIndex());
    iw->label()->addConnection(cb_nominal, SIGNAL(toggled(bool)), SLOT(toggleAlternativeText(bool)));
    md->addInputWidget(iw);

    iw = new MDDoubleSpinBox("refr_add_am_recy", tr("Refrigerant addition (recycled):"), md->widget(), -999999999.9, 999999999.9, refrigerantAdditionRecycled(), QApplication::translate("Units", "kg"));
    iw->label()->setDefaultText(QApplication::translate("Variables", "New charge (recycled):"));
    iw->label()->toggleAlternativeText(cb_nominal->currentIndex());
    iw->label()->addConnection(cb_nominal, SIGNAL(toggled(bool)), SLOT(toggleAlternativeText(bool)));
    md->addInputWidget(iw);

    iw = new MDDoubleSpinBox("refr_add_am_rege", tr("Refrigerant addition (reclaimed):"), md->widget(), -999999999.9, 999999999.9, refrigerantAdditionReclaimed(), QApplication::translate("Units", "kg"));
    iw->label()->setDefaultText(QApplication::translate("Variables", "New charge (reclaimed):"));
    iw->label()->toggleAlternativeText(cb_nominal->currentIndex());
    iw->label()->addConnection(cb_nominal, SIGNAL(toggled(bool)), SLOT(toggleAlternativeText(bool)));
    md->addInputWidget(iw);

    md->addInputWidget(new MDDoubleSpinBox("refr_reco", tr("Refrigerant recovery:"), md->widget(), -999999999.9, 999999999.9, refrigerantRecovery(), QApplication::translate("Units", "kg")));
}

bool Repair::checkValues(QWidget *parent)
{
    if (refrigerant().isEmpty())
        return showErrorMessage(parent, tr("Add repair - Leaklog"), QApplication::translate("Circuit", "Select a refrigerant."));

    if (field().isEmpty())
        return showErrorMessage(parent, tr("Add repair - Leaklog"), QApplication::translate("Circuit", "Select a field of application."));

    return true;
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
        columns << Column("service_company_uuid", "UUID");
        columns << Column("repair_type", "SMALLINT NOT NULL DEFAULT 0");
        columns << Column("date", "TEXT");
        columns << Column("customer", "TEXT");
        columns << Column("device", "TEXT");
        columns << Column("field", "TEXT");
        columns << Column("refrigerant", "TEXT");
        columns << Column("refrigerant_amount", "NUMERIC");
        columns << Column("refr_add_am", "NUMERIC");
        columns << Column("refr_add_am_recy", "NUMERIC");
        columns << Column("refr_add_am_rege", "NUMERIC");
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
        dict.insert("refrigerant_amount", QApplication::translate("Repair", "Refrigerant amount"));
        dict.insert("refr_add_am", QApplication::translate("Repair", "Refrigerant addition (new)"));
        dict.insert("refr_add_am_recy", QApplication::translate("Repair", "Refrigerant addition (recycled)"));
        dict.insert("refr_add_am_rege", QApplication::translate("Repair", "Refrigerant addition (reclaimed)"));
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
