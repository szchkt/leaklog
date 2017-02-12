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

#include "circuit.h"

#include "inputwidgets.h"
#include "editdialoguewidgets.h"
#include "global.h"
#include "customer.h"

#include <QApplication>
#include <QMessageBox>

using namespace Global;

Circuit::Circuit():
    DBRecord(tableName(), "id", "", MTDictionary())
{}

Circuit::Circuit(const QString &parent, const QString &id):
    DBRecord(tableName(), "id", id, MTDictionary("parent", parent))
{}

void Circuit::initEditDialogue(EditDialogueWidgets *md)
{
    MTDictionary refrigerants(listRefrigerantsToString().split(';'));

    QString customer = Customer(parent("parent")).stringValue("company");
    if (customer.isEmpty())
        customer = formatCompanyID(parent("parent"));
    md->setWindowTitle(tr("Customer: %2 %1 Circuit").arg(rightTriangle()).arg(customer));
    QVariantMap attributes;
    if (!id().isEmpty() || !values().isEmpty()) {
        attributes = list();
    } else {
        attributes.insert("year", QDate::currentDate().year());
    }
    MDLineEdit *id_edit = new MDLineEdit("id", tr("ID:"), md->widget(), id(), 99999);
    md->addInputWidget(id_edit);
    md->addInputWidget(new MDLineEdit("name", tr("Circuit name:"), md->widget(), attributes.value("name").toString()));
    md->addInputWidget(new MDLineEdit("operation", tr("Place of operation:"), md->widget(), attributes.value("operation").toString()));
    md->addInputWidget(new MDLineEdit("building", tr("Building:"), md->widget(), attributes.value("building").toString()));
    md->addInputWidget(new MDLineEdit("device", tr("Device:"), md->widget(), attributes.value("device").toString()));
    md->addInputWidget(new MDLineEdit("manufacturer", tr("Manufacturer:"), md->widget(), attributes.value("manufacturer").toString()));
    md->addInputWidget(new MDLineEdit("type", tr("Type:"), md->widget(), attributes.value("type").toString()));
    md->addInputWidget(new MDLineEdit("sn", tr("Serial number:"), md->widget(), attributes.value("sn").toString()));
    md->addInputWidget(new MDSpinBox("year", tr("Year of purchase:"), md->widget(), 1900, 2999, attributes.value("year").toInt()));
    md->addInputWidget(new MDDateEdit("commissioning", tr("Date of commissioning:"), md->widget(), attributes.value("commissioning").toString()));
    md->addInputWidget(new MDCheckBox("hermetic", tr("Hermetically sealed"), md->widget(), attributes.value("hermetic").toInt()));
    md->addInputWidget(new MDCheckBox("leak_detector", tr("Fixed leakage detector installed"), md->widget(), attributes.value("leak_detector").toInt()));
    md->addInputWidget(new MDComboBox("field", tr("Field of application:"), md->widget(), attributes.value("field").toString(), fieldsOfApplication()));
    md->addInputWidget(new MDComboBox("refrigerant", tr("Refrigerant:"), md->widget(), attributes.value("refrigerant").toString(), refrigerants));
    md->addInputWidget(new MDDoubleSpinBox("refrigerant_amount", tr("Amount of refrigerant:"), md->widget(), 0.0, 999999.9, attributes.value("refrigerant_amount").toDouble(), QApplication::translate("Units", "kg")));
    md->addInputWidget(new MDComboBox("oil", tr("Oil:"), md->widget(), attributes.value("oil", "poe").toString(), oils()));
    md->addInputWidget(new MDDoubleSpinBox("oil_amount", tr("Amount of oil:"), md->widget(), 0.0, 999999.9, attributes.value("oil_amount").toDouble(), QApplication::translate("Units", "kg")));
    md->addInputWidget(new MDDoubleSpinBox("runtime", tr("Run-time per day:"), md->widget(), 0.0, 24.0, attributes.value("runtime").toDouble(), QApplication::translate("Units", "hours")));
    md->addInputWidget(new MDDoubleSpinBox("utilisation", tr("Rate of utilisation:"), md->widget(), 0.0, 100.0, attributes.value("utilisation").toDouble(), QApplication::translate("Units", "%")));
    MDSpinBox *inspection_interval = new MDSpinBox("inspection_interval", tr("Inspection interval:"), md->widget(), 0, 999999, attributes.value("inspection_interval").toInt(), QApplication::translate("Units", "days"));
    inspection_interval->setSpecialValueText(tr("Automatic"));
    md->addInputWidget(inspection_interval);
    MDComboBox *disused = new MDComboBox("disused", tr("Status:"), md->widget(), attributes.value("disused").toString(),
                                         MTDictionary(QStringList()
                                                      << QString::number(Circuit::Commissioned)
                                                      << QString::number(Circuit::ExcludedFromAgenda)
                                                      << QString::number(Circuit::Decommissioned),
                                                      QStringList()
                                                      << tr("Commissioned")
                                                      << tr("Excluded from Agenda")
                                                      << tr("Decommissioned")));
    md->addInputWidget(disused);
    MDDateEdit *decommissioning = new MDDateEdit("decommissioning", tr("Date of decommissioning:"), md->widget(), attributes.value("decommissioning").toString());
    decommissioning->setEnabled(disused->currentIndex());
    QObject::connect(disused, SIGNAL(toggled(bool)), decommissioning, SLOT(setEnabled(bool)));
    md->addInputWidget(decommissioning);
    MDPlainTextEdit *reason = new MDPlainTextEdit("decommissioning_reason", tr("Reason for decommissioning:"), md->widget(), attributes.value("decommissioning_reason").toString());
    reason->setRowSpan(2);
    reason->setEnabled(disused->currentIndex());
    QObject::connect(disused, SIGNAL(toggled(bool)), reason, SLOT(setEnabled(bool)));
    md->addInputWidget(reason);
    MDPlainTextEdit *notes = new MDPlainTextEdit("notes", tr("Notes:"), md->widget(), attributes.value("notes").toString());
    notes->setRowSpan(0);
    md->addInputWidget(notes);
    int min_available_id = 1;
    QStringList used_ids; MTSqlQuery query_used_ids;
    query_used_ids.setForwardOnly(true);
    query_used_ids.prepare(QString("SELECT id FROM circuits WHERE parent = :parent%1 ORDER BY id ASC").arg(id().isEmpty() ? "" : " AND id <> :id"));
    query_used_ids.bindValue(":parent", parent("parent"));
    if (!id().isEmpty()) { query_used_ids.bindValue(":id", id()); }
    if (query_used_ids.exec()) {
        while (query_used_ids.next()) {
            used_ids << query_used_ids.value(0).toString();
            if (min_available_id == query_used_ids.value(0).toInt()) {
                min_available_id++;
            }
        }
    }
    md->setUsedIds(used_ids);
    if (id().isEmpty()) {
        id_edit->setVariantValue(min_available_id);
    }
}

bool Circuit::checkValues(const QVariantMap &values, QWidget *parent)
{
    if (!id().isEmpty() && values.value("refrigerant") != stringValue("refrigerant")) {
        QMessageBox message(parent);
        message.setWindowTitle(tr("Change refrigerant - Leaklog"));
        message.setWindowModality(Qt::WindowModal);
        message.setWindowFlags(message.windowFlags() | Qt::Sheet);
        message.setIcon(QMessageBox::Information);
        message.setText(tr("Changing the refrigerant will affect previous inspections of this circuit."));
        message.setInformativeText(QApplication::translate("MainWindow", "Do you want to save your changes?"));
        message.addButton(QApplication::translate("MainWindow", "&Save"), QMessageBox::AcceptRole);
        message.addButton(QApplication::translate("MainWindow", "Cancel"), QMessageBox::RejectRole);
        switch (message.exec()) {
            case 1: // Cancel
                return false;
        }
    }
    return true;
}

void Circuit::cascadeIDChange(int customer_id, int old_id, int new_id, int new_customer_id, bool compressors_and_units)
{
    MTSqlQuery update_inspections;
    if (new_customer_id < 0) {
        update_inspections.prepare("UPDATE inspections SET circuit = :new_id WHERE customer = :customer_id AND circuit = :old_id");
    } else {
        update_inspections.prepare("UPDATE inspections SET customer = :new_customer_id, circuit = :new_id WHERE customer = :customer_id AND circuit = :old_id");
        update_inspections.bindValue(":new_customer_id", new_customer_id);
    }
    update_inspections.bindValue(":customer_id", customer_id);
    update_inspections.bindValue(":old_id", old_id);
    update_inspections.bindValue(":new_id", new_id);
    update_inspections.exec();
    MTSqlQuery update_inspections_compressors;
    if (new_customer_id < 0) {
        update_inspections_compressors.prepare("UPDATE inspections_compressors SET circuit_id = :new_id WHERE customer_id = :customer_id AND circuit_id = :old_id");
    } else {
        update_inspections_compressors.prepare("UPDATE inspections_compressors SET customer_id = :new_customer_id, circuit_id = :new_id WHERE customer_id = :customer_id AND circuit_id = :old_id");
        update_inspections_compressors.bindValue(":new_customer_id", new_customer_id);
    }
    update_inspections_compressors.bindValue(":customer_id", customer_id);
    update_inspections_compressors.bindValue(":old_id", old_id);
    update_inspections_compressors.bindValue(":new_id", new_id);
    update_inspections_compressors.exec();
    MTSqlQuery update_inspection_images;
    if (new_customer_id < 0) {
        update_inspection_images.prepare("UPDATE inspection_images SET circuit = :new_id WHERE customer = :customer_id AND circuit = :old_id");
    } else {
        update_inspection_images.prepare("UPDATE inspection_images SET customer = :new_customer_id, circuit = :new_id WHERE customer = :customer_id AND circuit = :old_id");
        update_inspection_images.bindValue(":new_customer_id", new_customer_id);
    }
    update_inspection_images.bindValue(":customer_id", customer_id);
    update_inspection_images.bindValue(":old_id", old_id);
    update_inspection_images.bindValue(":new_id", new_id);
    update_inspection_images.exec();
    if (compressors_and_units || new_customer_id >= 0) {
        MTSqlQuery update_compressors;
        if (new_customer_id < 0) {
            update_compressors.prepare("UPDATE compressors SET circuit_id = :new_id WHERE customer_id = :customer_id AND circuit_id = :old_id");
        } else {
            update_compressors.prepare("UPDATE compressors SET customer_id = :new_customer_id, circuit_id = :new_id WHERE customer_id = :customer_id AND circuit_id = :old_id");
            update_compressors.bindValue(":new_customer_id", new_customer_id);
        }
        update_compressors.bindValue(":customer_id", customer_id);
        update_compressors.bindValue(":old_id", old_id);
        update_compressors.bindValue(":new_id", new_id);
        update_compressors.exec();
        MTSqlQuery update_circuit_units;
        if (new_customer_id < 0) {
            update_circuit_units.prepare("UPDATE circuit_units SET circuit_id = :new_id WHERE company_id = :customer_id AND circuit_id = :old_id");
        } else {
            update_circuit_units.prepare("UPDATE circuit_units SET company_id = :new_customer_id, circuit_id = :new_id WHERE company_id = :customer_id AND circuit_id = :old_id");
            update_circuit_units.bindValue(":new_customer_id", new_customer_id);
        }
        update_circuit_units.bindValue(":customer_id", customer_id);
        update_circuit_units.bindValue(":old_id", old_id);
        update_circuit_units.bindValue(":new_id", new_id);
        update_circuit_units.exec();
    }
}

QString Circuit::tableName()
{
    return "circuits";
}

class CircuitColumns
{
public:
    CircuitColumns() {
        columns << Column("parent", "INTEGER");
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
        // numBasicAttributes: 11
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

int Circuit::numBasicAttributes()
{
    return 11;
}
