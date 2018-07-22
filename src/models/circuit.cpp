/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2018 Matus & Michal Tomlein

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

#include "circuit.h"

#include "inputwidgets.h"
#include "editdialoguewidgets.h"
#include "global.h"
#include "customer.h"

#include <QApplication>
#include <QMessageBox>

using namespace Global;

Circuit::Circuit(const QString &uuid, const QVariantMap &savedValues):
    DBRecord(tableName(), uuid, savedValues)
{}

void Circuit::initEditDialogue(EditDialogueWidgets *md)
{
    MTDictionary refrigerants(listRefrigerants());

    Customer customer(customerUUID());
    md->setWindowTitle(tr("Customer: %2 %1 Circuit").arg(rightTriangle())
                       .arg(customer.companyName().isEmpty() ? customer.companyID() : customer.companyName()));
    if (!year()) {
        setValue("year", QDate::currentDate().year());
    }

    QString id = stringValue("id");
    MDLineEdit *id_edit = new MDLineEdit("id", tr("ID:"), md->widget(), id, 99999);
    md->addInputWidget(id_edit);
    md->addInputWidget(new MDLineEdit("name", tr("Circuit name:"), md->widget(), circuitName()));
    md->addInputWidget(new MDLineEdit("operation", tr("Place of operation:"), md->widget(), placeOfOperation()));
    md->addInputWidget(new MDLineEdit("building", tr("Building:"), md->widget(), building()));
    md->addInputWidget(new MDLineEdit("device", tr("Device:"), md->widget(), device()));
    md->addInputWidget(new MDLineEdit("manufacturer", tr("Manufacturer:"), md->widget(), manufacturer()));
    md->addInputWidget(new MDLineEdit("type", tr("Type:"), md->widget(), type()));
    md->addInputWidget(new MDLineEdit("sn", tr("Serial number:"), md->widget(), serialNumber()));
    md->addInputWidget(new MDSpinBox("year", tr("Year of purchase:"), md->widget(), 1900, 2999, year()));
    md->addInputWidget(new MDDateEdit("commissioning", tr("Date of commissioning:"), md->widget(), dateOfCommissioning()));
    md->addInputWidget(new MDCheckBox("hermetic", tr("Hermetically sealed"), md->widget(), hermetic()));
    md->addInputWidget(new MDCheckBox("leak_detector", tr("Fixed leakage detector installed"), md->widget(), leakDetectorInstalled()));
    md->addInputWidget(new MDComboBox("field", tr("Field of application:"), md->widget(), field(), fieldsOfApplication()));
    md->addInputWidget(new MDComboBox("refrigerant", tr("Refrigerant:"), md->widget(), refrigerant(), refrigerants));
    md->addInputWidget(new MDDoubleSpinBox("refrigerant_amount", tr("Amount of refrigerant:"), md->widget(), 0.0, 999999.9, refrigerantAmount(), QApplication::translate("Units", "kg")));
    md->addInputWidget(new MDComboBox("oil", tr("Oil:"), md->widget(), stringValue("oil", "poe"), oils()));
    md->addInputWidget(new MDDoubleSpinBox("oil_amount", tr("Amount of oil:"), md->widget(), 0.0, 999999.9, oilAmount(), QApplication::translate("Units", "kg")));
    md->addInputWidget(new MDDoubleSpinBox("runtime", tr("Run-time per day:"), md->widget(), 0.0, 24.0, runtime(), QApplication::translate("Units", "hours")));
    md->addInputWidget(new MDDoubleSpinBox("utilisation", tr("Rate of utilisation:"), md->widget(), 0.0, 100.0, utilisation(), QApplication::translate("Units", "%")));
    MDSpinBox *inspection_interval = new MDSpinBox("inspection_interval", tr("Inspection interval:"), md->widget(), 0, 999999, inspectionInterval(), QApplication::translate("Units", "days"));
    inspection_interval->setSpecialValueText(tr("Automatic"));
    md->addInputWidget(inspection_interval);
    MDComboBox *disused = new MDComboBox("disused", tr("Status:"), md->widget(), stringValue("disused"), {
        {QString::number(Circuit::Commissioned), tr("Commissioned")},
        {QString::number(Circuit::ExcludedFromAgenda), tr("Excluded from Agenda")},
        {QString::number(Circuit::Decommissioned), tr("Decommissioned")}
    });
    md->addInputWidget(disused);
    MDDateEdit *decommissioning = new MDDateEdit("decommissioning", tr("Date of decommissioning:"), md->widget(), dateOfDecommissioning());
    decommissioning->setEnabled(disused->currentIndex());
    QObject::connect(disused, SIGNAL(toggled(bool)), decommissioning, SLOT(setEnabled(bool)));
    md->addInputWidget(decommissioning);
    MDPlainTextEdit *reason = new MDPlainTextEdit("decommissioning_reason", tr("Reason for decommissioning:"), md->widget(), reasonForDecommissioning());
    reason->setRowSpan(2);
    reason->setEnabled(disused->currentIndex());
    QObject::connect(disused, SIGNAL(toggled(bool)), reason, SLOT(setEnabled(bool)));
    md->addInputWidget(reason);
    MDPlainTextEdit *notes = new MDPlainTextEdit("notes", tr("Notes:"), md->widget(), this->notes());
    notes->setRowSpan(0);
    md->addInputWidget(notes);

    int min_available_id = 1;
    QStringList used_ids; MTSqlQuery query_used_ids;
    query_used_ids.prepare(QString("SELECT id FROM circuits WHERE customer_uuid = :customer_uuid%1 ORDER BY id ASC").arg(QString(id.isEmpty() ? "" : " AND id <> :id")));
    query_used_ids.bindValue(":customer_uuid", customerUUID());
    if (!id.isEmpty()) { query_used_ids.bindValue(":id", id); }
    if (query_used_ids.exec()) {
        while (query_used_ids.next()) {
            used_ids << query_used_ids.value(0).toString();
            if (min_available_id == query_used_ids.value(0).toInt()) {
                min_available_id++;
            }
        }
    }
    md->setUsedIds(used_ids);
    if (id.isEmpty()) {
        id_edit->setVariantValue(min_available_id);
    }
}

bool Circuit::checkValues(QWidget *parent)
{
    if (!uuid().isEmpty() && value("refrigerant") != savedValue("refrigerant") && !superuserModeEnabled()) {
        MTSqlQuery query;
        query.prepare("SELECT date FROM inspections"
                      " WHERE circuit_uuid = :circuit_uuid"
                      " AND ((refr_add_am IS NOT NULL AND CAST(refr_add_am AS NUMERIC) <> 0)"
                      " OR (refr_reco IS NOT NULL AND CAST(refr_reco AS NUMERIC) <> 0)) LIMIT 1");
        query.bindValue(":circuit_uuid", uuid());

        if (query.exec() && query.next()) {
            QMessageBox message(parent);
            message.setWindowTitle(tr("Edit circuit - Leaklog"));
            message.setWindowModality(Qt::WindowModal);
            message.setWindowFlags(message.windowFlags() | Qt::Sheet);
            message.setIcon(QMessageBox::Warning);
            message.setText(tr("You cannot change the refrigerant in this circuit."));
            message.setInformativeText(tr("Changing the refrigerant would affect the store."));
            message.addButton(QApplication::translate("MainWindow", "OK"), QMessageBox::AcceptRole);
            message.exec();
            return false;
        }
    }
    return true;
}

Customer Circuit::customer()
{
    return Customer(customerUUID());
}

MTRecordQuery<Compressor> Circuit::compressors() const
{
    return Compressor::query({{"circuit_uuid", uuid()}});
}

MTRecordQuery<CircuitUnit> Circuit::units() const
{
    return CircuitUnit::query({{"circuit_uuid", uuid()}});
}

MTRecordQuery<Inspection> Circuit::inspections() const
{
    return Inspection::query({{"circuit_uuid", uuid()}});
}

QString Circuit::tableName()
{
    return "circuits";
}

class CircuitColumns
{
public:
    CircuitColumns() {
        columns << Column("uuid", "UUID PRIMARY KEY");
        columns << Column("customer_uuid", "UUID");
        columns << Column("starred", "SMALLINT NOT NULL DEFAULT 0");
        columns << Column("id", "INTEGER");
        columns << Column("name", "TEXT");
        columns << Column("disused", "INTEGER");
        columns << Column("operation", "TEXT");
        columns << Column("building", "TEXT");
        columns << Column("device", "TEXT");
        columns << Column("hermetic", "INTEGER");
        columns << Column("manufacturer", "TEXT");
        columns << Column("type", "TEXT");
        columns << Column("sn", "TEXT");
        columns << Column("year", "INTEGER");
        columns << Column("commissioning", "TEXT");
        columns << Column("decommissioning", "TEXT");
        columns << Column("decommissioning_reason", "TEXT");
        columns << Column("field", "TEXT");
        columns << Column("refrigerant", "TEXT");
        columns << Column("refrigerant_amount", "NUMERIC");
        columns << Column("oil", "TEXT");
        columns << Column("oil_amount", "NUMERIC");
        columns << Column("leak_detector", "INTEGER");
        columns << Column("runtime", "NUMERIC");
        columns << Column("utilisation", "NUMERIC");
        columns << Column("inspection_interval", "INTEGER");
        columns << Column("notes", "TEXT");
        columns << Column("operator_link", "TEXT");
        columns << Column("date_updated", "TEXT");
        columns << Column("updated_by", "TEXT");
    }

    ColumnList columns;
};

const ColumnList &Circuit::columns()
{
    static CircuitColumns columns;
    return columns.columns;
}

class CircuitAttributes
{
public:
    CircuitAttributes() {
        dict.insert("id", QApplication::translate("Circuit", "ID"));
        dict.insert("name", QApplication::translate("Circuit", "Circuit name"));
        dict.insert("operation", QApplication::translate("Circuit", "Place of operation"));
        dict.insert("building", QApplication::translate("Circuit", "Building"));
        dict.insert("device", QApplication::translate("Circuit", "Device"));
        dict.insert("manufacturer", QApplication::translate("Circuit", "Manufacturer"));
        dict.insert("type", QApplication::translate("Circuit", "Type"));
        dict.insert("sn", QApplication::translate("Circuit", "Serial number"));
        dict.insert("year", QApplication::translate("Circuit", "Year of purchase"));
        dict.insert("commissioning", QApplication::translate("Circuit", "Date of commissioning"));
        dict.insert("field", QApplication::translate("Circuit", "Field of application"));
        dict.insert("hermetic", QApplication::translate("Circuit", "Hermetically sealed"));
        dict.insert("leak_detector", QApplication::translate("Circuit", "Fixed leakage detector installed"));
        dict.insert("disused", QApplication::translate("Circuit", "Status"));
        dict.insert("decommissioning", QApplication::translate("Circuit", "Date of decommissioning"));
        dict.insert("decommissioning_reason", QApplication::translate("Circuit", "Reason for decommissioning"));
        dict.insert("refrigerant", QApplication::translate("Circuit", "Refrigerant"));
        dict.insert("refrigerant_amount", QApplication::translate("Circuit", "Amount of refrigerant") + "||" + QApplication::translate("Units", "kg"));
        dict.insert("oil", QApplication::translate("Circuit", "Oil"));
        dict.insert("oil_amount", QApplication::translate("Circuit", "Amount of oil") + "||" + QApplication::translate("Units", "kg"));
        dict.insert("runtime", QApplication::translate("Circuit", "Run-time per day") + "||" + QApplication::translate("Units", "hours"));
        dict.insert("utilisation", QApplication::translate("Circuit", "Rate of utilisation") + "||%");
        dict.insert("inspection_interval", QApplication::translate("Circuit", "Inspection interval") + "||" + QApplication::translate("Units", "days"));
    }

    MTDictionary dict;
};

const MTDictionary &Circuit::attributes()
{
    static CircuitAttributes dict;
    return dict.dict;
}

bool Circuit::remove() const
{
    compressors().removeAll();
    units().removeAll();
    inspections().removeAll();
    return MTRecord::remove();
}
