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

#include "records.h"
#include "inputwidgets.h"
#include "editdialogue.h"
#include "global.h"
#include "variables.h"
#include "warnings.h"
#include "partnerwidgets.h"
#include "editcustomerdialogue.h"

#include <QSqlRecord>
#include <QApplication>
#include <QMessageBox>

using namespace Global;

DBRecord::DBRecord():
    QObject(),
    MTRecord()
{}

DBRecord::DBRecord(const QString &type, const QString &id_field, const QString &id, const MTDictionary &parents):
    QObject(),
    MTRecord(type, id_field, id, parents)
{}

Customer::Customer(const QString &id):
    DBRecord("customers", "id", id, MTDictionary())
{}

void Customer::initEditDialogue(EditDialogueWidgets *md)
{
    md->setWindowTitle(tr("Customer"));
    QVariantMap attributes;
    if (!id().isEmpty() || !values().isEmpty()) {
        attributes = list();
    }
    md->addInputWidget(new MDLineEdit("id", tr("ID:"), md->widget(), id(), 99999999));
    md->addInputWidget(new MDLineEdit("company", tr("Company:"), md->widget(), attributes.value("company").toString()));
    md->addInputWidget(new MDAddressEdit("address", tr("Address:"), md->widget(), attributes.value("address").toString()));
    md->addInputWidget(new MDLineEdit("mail", tr("E-mail:"), md->widget(), attributes.value("mail").toString()));
    md->addInputWidget(new MDLineEdit("phone", tr("Phone:"), md->widget(), attributes.value("phone").toString()));
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
            QVariantMap service_company = ServiceCompany(DBInfoValueForKey("default_service_company")).list();
            values().insert("operator_id", service_company.value("id"));
            values().insert("operator_company", service_company.value("name"));
            values().insert("operator_address", service_company.value("address"));
            values().insert("operator_mail", service_company.value("mail"));
            values().insert("operator_phone", service_company.value("phone"));
        }
        break;
    case 0:
        values().insert("operator_id", value("id"));
        values().insert("operator_company", value("company"));
        values().insert("operator_address", value("address"));
        values().insert("operator_mail", value("mail"));
        values().insert("operator_phone", value("phone"));
        break;
    }
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

Circuit::Circuit():
    DBRecord("circuits", "id", "", MTDictionary())
{}

Circuit::Circuit(const QString &parent, const QString &id):
    DBRecord("circuits", "id", id, MTDictionary("parent", parent))
{}

void Circuit::initEditDialogue(EditDialogueWidgets *md)
{
    MTDictionary refrigerants(listRefrigerantsToString().split(';'));

    QString customer = Customer(parent("parent")).stringValue("company");
    if (customer.isEmpty())
        customer = parent("parent").rightJustified(8, '0');
    md->setWindowTitle(tr("Customer: %2 %1 Circuit").arg(rightTriangle()).arg(customer));
    QVariantMap attributes;
    if (!id().isEmpty() || !values().isEmpty()) {
        attributes = list();
    } else {
        attributes.insert("year", QDate::currentDate().year());
    }
    md->addInputWidget(new MDLineEdit("id", tr("ID:"), md->widget(), id(), 99999));
    md->addInputWidget(new MDLineEdit("name", tr("Circuit name:"), md->widget(), attributes.value("name").toString()));
    md->addInputWidget(new MDLineEdit("operation", tr("Place of operation:"), md->widget(), attributes.value("operation").toString()));
    md->addInputWidget(new MDLineEdit("building", tr("Building:"), md->widget(), attributes.value("building").toString()));
    md->addInputWidget(new MDLineEdit("device", tr("Device:"), md->widget(), attributes.value("device").toString()));
    md->addInputWidget(new MDCheckBox("hermetic", tr("Hermetically sealed"), md->widget(), attributes.value("hermetic").toInt()));
    md->addInputWidget(new MDLineEdit("manufacturer", tr("Manufacturer:"), md->widget(), attributes.value("manufacturer").toString()));
    md->addInputWidget(new MDLineEdit("type", tr("Type:"), md->widget(), attributes.value("type").toString()));
    md->addInputWidget(new MDLineEdit("sn", tr("Serial number:"), md->widget(), attributes.value("sn").toString()));
    md->addInputWidget(new MDSpinBox("year", tr("Year of purchase:"), md->widget(), 1900, 2999, attributes.value("year").toInt()));
    md->addInputWidget(new MDDateEdit("commissioning", tr("Date of commissioning:"), md->widget(), attributes.value("commissioning").toString()));
    MDCheckBox *disused = new MDCheckBox("disused", tr("Disused"), md->widget(), attributes.value("disused").toInt());
    md->addInputWidget(disused);
    MDDateEdit *decommissioning = new MDDateEdit("decommissioning", tr("Date of decommissioning:"), md->widget(), attributes.value("decommissioning").toString());
    decommissioning->setEnabled(disused->isChecked());
    QObject::connect(disused, SIGNAL(toggled(bool)), decommissioning, SLOT(setEnabled(bool)));
    md->addInputWidget(decommissioning);
    md->addInputWidget(new MDComboBox("field", tr("Field of application:"), md->widget(), attributes.value("field").toString(), fieldsOfApplication()));
    md->addInputWidget(new MDComboBox("refrigerant", tr("Refrigerant:"), md->widget(), attributes.value("refrigerant").toString(), refrigerants));
    md->addInputWidget(new MDDoubleSpinBox("refrigerant_amount", tr("Amount of refrigerant:"), md->widget(), 0.0, 999999.9, attributes.value("refrigerant_amount").toDouble(), QApplication::translate("Units", "kg")));
    md->addInputWidget(new MDComboBox("oil", tr("Oil:"), md->widget(), attributes.value("oil").toString(), oils()));
    md->addInputWidget(new MDDoubleSpinBox("oil_amount", tr("Amount of oil:"), md->widget(), 0.0, 999999.9, attributes.value("oil_amount").toDouble(), QApplication::translate("Units", "kg")));
    md->addInputWidget(new MDCheckBox("leak_detector", tr("Fixed leakage detector installed"), md->widget(), attributes.value("leak_detector").toInt()));
    md->addInputWidget(new MDDoubleSpinBox("runtime", tr("Run-time per day:"), md->widget(), 0.0, 24.0, attributes.value("runtime").toDouble(), QApplication::translate("Units", "hours")));
    md->addInputWidget(new MDDoubleSpinBox("utilisation", tr("Rate of utilisation:"), md->widget(), 0.0, 100.0, attributes.value("utilisation").toDouble(), QApplication::translate("Units", "%")));
    MDSpinBox *inspection_interval = new MDSpinBox("inspection_interval", tr("Inspection interval:"), md->widget(), 0, 999999, attributes.value("inspection_interval").toInt(), QApplication::translate("Units", "days"));
    inspection_interval->setSpecialValueText(tr("Automatic"));
    md->addInputWidget(inspection_interval);
    QStringList used_ids; MTSqlQuery query_used_ids;
    query_used_ids.setForwardOnly(true);
    query_used_ids.prepare("SELECT id FROM circuits WHERE parent = :parent" + QString(id().isEmpty() ? "" : " AND id <> :id"));
    query_used_ids.bindValue(":parent", parent("parent"));
    if (!id().isEmpty()) { query_used_ids.bindValue(":id", id()); }
    if (query_used_ids.exec()) {
        while (query_used_ids.next()) {
            used_ids << query_used_ids.value(0).toString();
        }
    }
    md->setUsedIds(used_ids);
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
        dict.insert("disused", QApplication::translate("Circuit", "Disused"));
        dict.insert("decommissioning", QApplication::translate("Circuit", "Date of decommissioning"));
        dict.insert("refrigerant", QApplication::translate("Circuit", "Refrigerant"));
        dict.insert("refrigerant_amount", QApplication::translate("Circuit", "Amount of refrigerant") + "||" + QApplication::translate("Units", "kg"));
        dict.insert("oil", QApplication::translate("Circuit", "Oil"));
        dict.insert("oil_amount", QApplication::translate("Circuit", "Amount of oil") + "||" + QApplication::translate("Units", "kg"));
        dict.insert("leak_detector", QApplication::translate("Circuit", "Fixed leakage detector installed"));
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

QString Inspection::descriptionForInspectionType(Inspection::Type type, const QString &type_data)
{
    QStringList data = type_data.split(UNIT_SEPARATOR);

    switch (type) {
        case Inspection::CircuitMovedType: {
            int from_id = data.value(0).toInt();
            QString from = from_id ? Customer(QString::number(from_id)).stringValue("company") : QString();
            if (from.isEmpty())
                from = data.value(2);

            from.append(QString(" (%1)").arg(QString::number(from_id).rightJustified(8, '0')));

            int to_id = data.value(1).toInt();
            QString to = to_id ? Customer(QString::number(to_id)).stringValue("company") : QString();
            if (to.isEmpty())
                to = data.value(3);

            to.append(QString(" (%1)").arg(QString::number(to_id).rightJustified(8, '0')));

            return tr("Circuit moved from customer %1 to %2.").arg(from).arg(to);
        }

        case Inspection::InspectionSkippedType:
            return data.value(0);

        default:
            break;
    }

    return QString();
}

Inspection::Inspection():
    DBRecord("inspections", "date", "", MTDictionary()),
    m_scope(Variable::Inspection)
{}

Inspection::Inspection(const QString &customer, const QString &circuit, const QString &date):
    DBRecord("inspections", "date", date, MTDictionary(QStringList() << "customer" << "circuit", QStringList() << customer << circuit)),
    m_scope(Variable::Inspection)
{}

Inspection::Inspection(const QString &table, const QString &id_column, const QString &id, const MTDictionary &parents):
    DBRecord(table, id_column, id, parents),
    m_scope(Variable::Inspection)
{}

void Inspection::initEditDialogue(EditDialogueWidgets *md)
{
    QString customer = Customer(parent("customer")).stringValue("company");
    if (customer.isEmpty())
        customer = parent("customer").rightJustified(8, '0');
    QString circuit = Circuit(parent("customer"), parent("circuit")).stringValue("name");
    if (circuit.isEmpty())
        circuit = parent("circuit").rightJustified(5, '0');
    md->setWindowTitle(tr("Customer: %2 %1 Circuit: %3 %1 Inspection").arg(rightTriangle()).arg(customer).arg(circuit));
    md->setMaximumRowCount(10);
    QVariantMap attributes;
    if (!id().isEmpty() || !values().isEmpty()) {
        attributes = list();
    }
    bool nominal_found = false;
    QStringList used_ids; MTSqlQuery query_used_ids;
    query_used_ids.setForwardOnly(true);
    query_used_ids.prepare("SELECT date, nominal FROM inspections WHERE customer = :customer AND circuit = :circuit" + QString(id().isEmpty() ? "" : " AND date <> :date"));
    query_used_ids.bindValue(":customer", parent("customer"));
    query_used_ids.bindValue(":circuit", parent("circuit"));
    if (!id().isEmpty()) { query_used_ids.bindValue(":date", id()); }
    if (query_used_ids.exec()) {
        while (query_used_ids.next()) {
            used_ids << query_used_ids.value(0).toString();
            if (!nominal_found && query_used_ids.value(1).toInt())
                nominal_found = true;
        }
    }
    md->setUsedIds(used_ids);
    MDDateTimeEdit *date = new MDDateTimeEdit("date", tr("Date:"), md->widget(), id());
    if (isDatabaseLocked()) {
        date->setMinimumDate(QDate::fromString(lockDate(), DATE_FORMAT));
    }
    md->addInputWidget(date);
    MTCheckBoxGroup *chbgrp_i_type = new MTCheckBoxGroup(md->widget());
    MDCheckBox *chb_nominal = new MDCheckBox("nominal", tr("Nominal inspection"), md->widget(), attributes.value("nominal").toInt(), true);
    if (nominal_found)
        QObject::connect(chb_nominal, SIGNAL(toggled(MTCheckBox *, bool)), this, SLOT(showSecondNominalInspectionWarning(MTCheckBox *, bool)));

    md->addInputWidget(chb_nominal);
    chbgrp_i_type->addCheckBox((MTCheckBox *)chb_nominal->widget());
    MDCheckBox *chb_repair = new MDCheckBox("repair", tr("Repair"), md->widget(), attributes.value("repair").toInt(), true);
    md->addInputWidget(chb_repair);
    chbgrp_i_type->addCheckBox((MTCheckBox *)chb_repair->widget());
    md->addInputWidget(new MDCheckBox("outside_interval", tr("Outside the inspection interval"), md->widget(), attributes.value("outside_interval").toInt()));

    if (!id().isEmpty()) {
        MTDictionary parents("customer_id", parent("customer"));
        parents.insert("circuit_id", parent("circuit"));
        parents.insert("date", id());
        if (!InspectionsCompressor(QString(), parents).exists()) {
            m_scope |= Variable::Compressor;
            md->setMaximumRowCount(14);
        }
    }

    Variables query(QSqlDatabase(), m_scope);
    query.initEditDialogueWidgets(md, attributes, this, date->variantValue().toDateTime(), chb_repair, chb_nominal);
}

void Inspection::showSecondNominalInspectionWarning(MTCheckBox *checkbox, bool state)
{
    if (state) {
        QMessageBox message(checkbox->parentWidget());
        message.setWindowTitle(tr("Nominal inspection already exists - Leaklog"));
        message.setWindowModality(Qt::WindowModal);
        message.setWindowFlags(message.windowFlags() | Qt::Sheet);
        message.setIcon(QMessageBox::Information);
        message.setText(tr("This circuit already has a nominal inspection."));
        message.setInformativeText(tr("Are you sure you want to add another?"));
        message.addButton(tr("&Add Another Nominal Inspection"), QMessageBox::AcceptRole);
        message.setDefaultButton(message.addButton(tr("Cancel"), QMessageBox::RejectRole));
        switch (message.exec()) {
            case 0: // Add
                break;
            case 1: // Cancel
                QMetaObject::invokeMethod(checkbox, "toggle", Qt::QueuedConnection);
                break;
        }
    }
}

InspectionByInspector::InspectionByInspector(const QString &inspector_id):
    Inspection("inspections LEFT JOIN customers ON inspections.customer = customers.id"
           " LEFT JOIN circuits ON inspections.circuit = circuits.id AND circuits.parent = inspections.customer", "date", "", MTDictionary("inspector", inspector_id))
{}

Repair::Repair(const QString &date):
    DBRecord("repairs", "date", date, MTDictionary())
{}

void Repair::initEditDialogue(EditDialogueWidgets *md)
{
    MTDictionary refrigerants(listRefrigerantsToString().split(';'));

    md->setWindowTitle(tr("Repair"));
    QVariantMap attributes;
    if (!id().isEmpty() || !values().isEmpty()) {
        attributes = list();
    } else {
        for (int i = 0; i < parents().count(); ++i) {
            attributes.insert(parents().key(i), parents().value(i));
        }
    }
    MDDateTimeEdit *date = new MDDateTimeEdit("date", tr("Date:"), md->widget(), id());
    if (isDatabaseLocked())
        date->setMinimumDate(QDate::fromString(lockDate(), DATE_FORMAT));
    md->addInputWidget(date);
    MDLineEdit *customer = new MDLineEdit("customer", tr("Customer:"), md->widget(), attributes.value("customer").toString());
    if (!attributes.value("parent").toString().isEmpty())
        customer->setEnabled(false);
    md->addInputWidget(customer);
    md->addInputWidget(new MDLineEdit("device", tr("Device:"), md->widget(), attributes.value("device").toString()));
    md->addInputWidget(new MDComboBox("field", tr("Field of application:"), md->widget(), attributes.value("field").toString(), fieldsOfApplication()));
    md->addInputWidget(new MDComboBox("refrigerant", tr("Refrigerant:"), md->widget(), attributes.value("refrigerant").toString(), refrigerants));
    MDComboBox *repairman = new MDComboBox("repairman", tr("Repairman:"), md->widget(), attributes.value("repairman").toString(), listInspectors());
    repairman->setNullValue(QVariant(QVariant::Int));
    md->addInputWidget(repairman);
    md->addInputWidget(new MDLineEdit("arno", tr("Assembly record No.:"), md->widget(), attributes.value("arno").toString()));
    md->addInputWidget(new MDDoubleSpinBox("refrigerant_amount", tr("Amount of refrigerant:"), md->widget(), 0.0, 999999.9, attributes.value("refrigerant_amount").toDouble(), QApplication::translate("Units", "kg")));
    md->addInputWidget(new MDDoubleSpinBox("refr_add_am", tr("Refrigerant addition:"), md->widget(), -999999999.9, 999999999.9, attributes.value("refr_add_am").toDouble(), QApplication::translate("Units", "kg")));
    md->addInputWidget(new MDDoubleSpinBox("refr_reco", tr("Refrigerant recovery:"), md->widget(), -999999999.9, 999999999.9, attributes.value("refr_reco").toDouble(), QApplication::translate("Units", "kg")));
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
        dict.insert("repairman", QApplication::translate("Repair", "Repairman"));
        dict.insert("arno", QApplication::translate("Repair", "Assembly record No."));
    }

    MTDictionary dict;
};

const MTDictionary &Repair::attributes()
{
    static RepairAttributes dict;
    return dict.dict;
}

VariableRecord::VariableRecord(const QString &var_id, const QString &parent_id):
    DBRecord("variables", "id", var_id, parent_id.isEmpty() ? MTDictionary() : MTDictionary("parent_id", parent_id))
{}

void VariableRecord::initEditDialogue(EditDialogueWidgets *md)
{
    md->setWindowTitle(tr("Variable"));

    QVariantMap attributes;
    bool enable_all = true;

    if (!id().isEmpty()) {
        Variable variable(id());
        if (variable.next()) {
            attributes.insert("parent_id", variable.value(Variable::ParentID));
            attributes.insert("id", variable.value(Variable::ID));
            attributes.insert("name", variable.value(Variable::Name));
            attributes.insert("type", variable.value(Variable::Type));
            attributes.insert("unit", variable.value(Variable::Unit));
            attributes.insert("value", variable.value(Variable::Value));
            attributes.insert("compare_nom", variable.value(Variable::CompareNom));
            attributes.insert("tolerance", variable.value(Variable::Tolerance));
            attributes.insert("col_bg", variable.value(Variable::ColBg));
        }

        if (variableNames().contains(id()))
            enable_all = false;
    }

    QStringList used_ids;
    used_ids << "refrigerant_amount" << "oil_amount" << "sum" << "p_to_t" << "p_to_t_vap";
    used_ids << listSupportedFunctions();
    used_ids << listVariableIds(true);
    if (!id().isEmpty())
        used_ids.removeAll(id());
    md->setUsedIds(used_ids);

    md->addInputWidget(new MDLineEdit("id", tr("ID:"), md->widget(), attributes.value("id").toString(), 0, "", enable_all));
    md->addInputWidget(new MDLineEdit("name", tr("Name:"), md->widget(), attributes.value("name").toString(), 0, "", enable_all));
    md->addInputWidget(new MDLineEdit("unit", tr("Unit:"), md->widget(), attributes.value("unit").toString(), 0, "", enable_all));
    MDComboBox *cb_type = new MDComboBox("type", tr("Type:"), md->widget(), attributes.value("type").toString(), variableTypes(), "", enable_all);
    if (attributes.value("type").toString() == "group")
        cb_type->setEnabled(false);
    md->addInputWidget(cb_type);
    md->addInputWidget(new MDComboBox("scope", tr("Scope:"), md->widget(), attributes.value("scope").toString(),
                                      MTDictionary(QStringList()
                                                   << QString::number(Variable::Inspection)
                                                   << QString::number(Variable::Compressor),
                                                   QStringList() << tr("Inspection") << tr("Compressor")),
                                      "", id().isEmpty()));
    md->addInputWidget(new MDHighlightedPlainTextEdit("value", tr("Value:"), md->widget(), attributes.value("value").toString(), used_ids, enable_all));
    md->addInputWidget(new MDCheckBox("compare_nom", tr("Compare value with the nominal inspection"), md->widget(), attributes.value("compare_nom").toInt()));
    md->addInputWidget(new MDDoubleSpinBox("tolerance", tr("Tolerance:"), md->widget(), 0.0, 999999.9, attributes.value("tolerance").toDouble()));
    if (attributes.value("parent_id").toString().isEmpty()) {
        md->addInputWidget(new MDColourComboBox("col_bg", tr("Colour:"), md->widget(), attributes.value("col_bg").toString()));
    }
}

Table::Table(const QString &id, const QString &uid, const MTDictionary &parents):
    DBRecord("tables", uid.isEmpty() ? "id" : "uid", uid.isEmpty() ? id : uid, parents)
{}

void Table::initEditDialogue(EditDialogueWidgets *md)
{
    md->setWindowTitle(tr("Table"));
    QVariantMap attributes;
    if (!id().isEmpty()) {
        attributes = list();
    }
    md->addInputWidget(new MDLineEdit("id", tr("Name:"), md->widget(), attributes.value("id").toString()));
    md->addInputWidget(new MDCheckBox("highlight_nominal", tr("Highlight nominal inspections"), md->widget(), attributes.value("highlight_nominal").toInt()));
    md->addInputWidget(new MDComboBox("scope", tr("Scope:"), md->widget(), attributes.value("scope").toString(),
                                      MTDictionary(QStringList()
                                                   << QString::number(Variable::Inspection)
                                                   << QString::number(Variable::Compressor),
                                                   QStringList() << tr("Inspection") << tr("Compressor"))));
    QStringList used_ids; MTSqlQuery query_used_ids;
    query_used_ids.setForwardOnly(true);
    query_used_ids.prepare("SELECT id FROM tables" + QString(id().isEmpty() ? "" : " WHERE id <> :id"));
    if (!id().isEmpty()) { query_used_ids.bindValue(":id", id()); }
    if (query_used_ids.exec()) {
        while (query_used_ids.next()) {
            used_ids << query_used_ids.value(0).toString();
        }
    }
    md->setUsedIds(used_ids);
}

Inspector::Inspector(const QString &id):
    DBRecord("inspectors", "id", id, MTDictionary())
{}

void Inspector::initEditDialogue(EditDialogueWidgets *md)
{
    QString currency = Global::DBInfoValueForKey("currency", "EUR");

    md->setWindowTitle(tr("Inspector"));
    QVariantMap attributes;
    if (!id().isEmpty()) {
        attributes = list();
    }
    md->addInputWidget(new MDLineEdit("id", tr("ID:"), md->widget(), attributes.value("id").toString(), 9999));
    md->addInputWidget(new MDLineEdit("person", tr("Certified person:"), md->widget(), attributes.value("person").toString()));
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

class InspectorAttributes
{
public:
    InspectorAttributes() {
        dict.insert("id", QApplication::translate("Inspector", "ID"));
        dict.insert("person", QApplication::translate("Inspector", "Certified person"));
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

ServiceCompany::ServiceCompany(const QString &id):
    DBRecord("service_companies", "id", id, MTDictionary())
{}

void ServiceCompany::initEditDialogue(EditDialogueWidgets *md)
{
    md->setWindowTitle(tr("Service Company"));
    QVariantMap attributes;
    if (!id().isEmpty()) {
        attributes = list();
    }
    md->addInputWidget(new MDLineEdit("name", tr("Name:"), md->widget(), attributes.value("name").toString()));
    md->addInputWidget(new MDLineEdit("id", tr("ID:"), md->widget(), attributes.value("id").toString(), 99999999));
    md->addInputWidget(new MDAddressEdit("address", tr("Address:"), md->widget(), attributes.value("address").toString()));
    md->addInputWidget(new MDLineEdit("phone", tr("Phone:"), md->widget(), attributes.value("phone").toString()));
    md->addInputWidget(new MDLineEdit("mail", tr("E-mail:"), md->widget(), attributes.value("mail").toString()));
    md->addInputWidget(new MDLineEdit("website", tr("Website:"), md->widget(), attributes.value("website").toString()));
    md->addInputWidget(new MDFileChooser("image", tr("Image:"), md->widget(), attributes.value("image").toInt()));
    QStringList used_ids; MTSqlQuery query_used_ids;
    query_used_ids.setForwardOnly(true);
    query_used_ids.prepare("SELECT id FROM service_companies" + QString(id().isEmpty() ? "" : " WHERE id <> :id"));
    if (!id().isEmpty()) { query_used_ids.bindValue(":id", id()); }
    if (query_used_ids.exec()) {
        while (query_used_ids.next()) {
            used_ids << query_used_ids.value(0).toString();
        }
    }
    md->setUsedIds(used_ids);
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

RecordOfRefrigerantManagement::RecordOfRefrigerantManagement(const QString &date):
    DBRecord("refrigerant_management", "date", date, MTDictionary())
{}

void RecordOfRefrigerantManagement::initEditDialogue(EditDialogueWidgets *md)
{
    MTDictionary refrigerants(listRefrigerantsToString().split(';'));

    md->setWindowTitle(tr("Record of Refrigerant Management"));
    QVariantMap attributes;
    if (!id().isEmpty()) {
        attributes = list();
    }
    MDDateTimeEdit *date = new MDDateTimeEdit("date", tr("Date:"), md->widget(), attributes.value("date").toString());
    if (isDatabaseLocked()) {
        date->setMinimumDate(QDate::fromString(lockDate(), DATE_FORMAT));
    }
    md->addInputWidget(date);

    PartnerWidgets *partner_widgets = new PartnerWidgets(attributes.value("partner").toString(), attributes.value("partner_id").toString(), md->widget());
    md->addInputWidget(partner_widgets->partnersWidget());
    md->addInputWidget(partner_widgets->partnerNameWidget());
    md->addInputWidget(partner_widgets->partnerIdWidget());
    md->addInputWidget(new MDComboBox("refrigerant", tr("Refrigerant:"), md->widget(), attributes.value("refrigerant").toString(), refrigerants));
    md->addInputWidget(new MDDoubleSpinBox("purchased", tr("Purchased (new):"), md->widget(), 0.0, 999999999.9, attributes.value("purchased").toDouble(), QApplication::translate("Units", "kg")));
    md->addInputWidget(new MDDoubleSpinBox("purchased_reco", tr("Purchased (recovered):"), md->widget(), 0.0, 999999999.9, attributes.value("purchased_reco").toDouble(), QApplication::translate("Units", "kg")));
    md->addInputWidget(new MDDoubleSpinBox("sold", tr("Sold (new):"), md->widget(), 0.0, 999999999.9, attributes.value("sold").toDouble(), QApplication::translate("Units", "kg")));
    md->addInputWidget(new MDDoubleSpinBox("sold_reco", tr("Sold (recovered):"), md->widget(), 0.0, 999999999.9, attributes.value("sold_reco").toDouble(), QApplication::translate("Units", "kg")));
    md->addInputWidget(new MDDoubleSpinBox("refr_rege", tr("Reclaimed:"), md->widget(), 0.0, 999999999.9, attributes.value("refr_rege").toDouble(), QApplication::translate("Units", "kg")));
    md->addInputWidget(new MDDoubleSpinBox("refr_disp", tr("Disposed of:"), md->widget(), 0.0, 999999999.9, attributes.value("refr_disp").toDouble(), QApplication::translate("Units", "kg")));
    md->addInputWidget(new MDDoubleSpinBox("leaked", tr("Leaked (new):"), md->widget(), 0.0, 999999999.9, attributes.value("leaked").toDouble(), QApplication::translate("Units", "kg")));
    md->addInputWidget(new MDDoubleSpinBox("leaked_reco", tr("Leaked (recovered):"), md->widget(), 0.0, 999999999.9, attributes.value("leaked_reco").toDouble(), QApplication::translate("Units", "kg")));
    QStringList used_ids; MTSqlQuery query_used_ids;
    query_used_ids.setForwardOnly(true);
    query_used_ids.prepare("SELECT date FROM refrigerant_management" + QString(id().isEmpty() ? "" : " WHERE date <> :date"));
    if (!id().isEmpty()) { query_used_ids.bindValue(":date", id()); }
    if (query_used_ids.exec()) {
        while (query_used_ids.next()) {
            used_ids << query_used_ids.value(0).toString();
        }
    }
    md->setUsedIds(used_ids);
}

class RecordOfRefrigerantManagementAttributes
{
public:
    RecordOfRefrigerantManagementAttributes() {
        dict.insert("date", QApplication::translate("RecordOfRefrigerantManagement", "Date"));
        dict.insert("partner", QApplication::translate("RecordOfRefrigerantManagement", "Business partner"));
        dict.insert("partner_id", QApplication::translate("RecordOfRefrigerantManagement", "Business partner (ID)"));
        dict.insert("refrigerant", QApplication::translate("RecordOfRefrigerantManagement", "Refrigerant"));
        dict.insert("purchased", QApplication::translate("RecordOfRefrigerantManagement", "Purchased (new)"));
        dict.insert("purchased_reco", QApplication::translate("RecordOfRefrigerantManagement", "Purchased (recovered)"));
        dict.insert("sold", QApplication::translate("RecordOfRefrigerantManagement", "Sold (new)"));
        dict.insert("sold_reco", QApplication::translate("RecordOfRefrigerantManagement", "Sold (recovered)"));
        dict.insert("refr_rege", QApplication::translate("RecordOfRefrigerantManagement", "Reclaimed"));
        dict.insert("refr_disp", QApplication::translate("RecordOfRefrigerantManagement", "Disposed of"));
        dict.insert("leaked", QApplication::translate("RecordOfRefrigerantManagement", "Leaked (new)"));
        dict.insert("leaked_reco", QApplication::translate("RecordOfRefrigerantManagement", "Leaked (recovered)"));
    }

    MTDictionary dict;
};

const MTDictionary &RecordOfRefrigerantManagement::attributes()
{
    static RecordOfRefrigerantManagementAttributes dict;
    return dict.dict;
}

WarningRecord::WarningRecord(const QString &id):
    DBRecord("warnings", "id", id, MTDictionary())
{}

void WarningRecord::initEditDialogue(EditDialogueWidgets *md)
{
    md->setWindowTitle(tr("Warning"));
    QVariantMap attributes; bool enable_all = true;
    if (!id().isEmpty()) {
        Warning query(id().toInt());
        if (query.next()) {
            for (int i = 0; i < query.record().count(); ++i) {
                attributes.insert(query.record().fieldName(i), query.value(query.record().fieldName(i)));
            }
        }
        if (!attributes.value("name").toString().isEmpty()) {
            md->setWindowTitle(tr("%1: %2").arg(tr("Warning")).arg(attributes.value("name").toString()));
        }
        enable_all = id().toInt() < 1000;
    }
    md->addInputWidget(new MDCheckBox("enabled", tr("Enabled"), md->widget(), attributes.value("enabled").toInt()));
    md->addInputWidget(new MDLineEdit("name", tr("Name:"), md->widget(), attributes.value("name").toString(), 0, "", enable_all));
    md->addInputWidget(new MDLineEdit("description", tr("Description:"), md->widget(), attributes.value("description").toString(), 0, "", enable_all));
    md->addInputWidget(new MDSpinBox("delay", tr("Delay:"), md->widget(), 0, 999999, attributes.value("delay").toInt(), tr("days"), "", enable_all));
    md->addInputWidget(new MDComboBox("scope", tr("Scope:"), md->widget(), attributes.value("scope").toString(),
                                      MTDictionary(QStringList()
                                                   << QString::number(Variable::Inspection)
                                                   << QString::number(Variable::Compressor),
                                                   QStringList() << tr("Inspection") << tr("Compressor")),
                                      QString(), enable_all));
    QStringList used_ids;
    used_ids << "refrigerant_amount" << "oil_amount" << "sum" << "p_to_t" << "p_to_t_vap";
    used_ids << listSupportedFunctions();
    used_ids << listVariableIds();
    md->setUsedIds(used_ids);
}

AssemblyRecordType::AssemblyRecordType(const QString &id):
    DBRecord("assembly_record_types", "id", id, MTDictionary())
{}

void AssemblyRecordType::initEditDialogue(EditDialogueWidgets *md)
{
    md->setWindowTitle(tr("Assembly Record Type"));

    QStringList keywords;
    keywords << "customer_id";
    keywords << "customer_name";
    keywords << "circuit_id";
    keywords << "circuit_name";
    keywords << "day";
    keywords << "month";
    keywords << "year";

    QVariantMap attributes;
    if (!id().isEmpty() || !values().isEmpty()) {
        attributes = list();
    }

    md->addInputWidget(new MDHiddenIdField("id", md->widget(), attributes.value("id")));
    md->addInputWidget(new MDLineEdit("name", tr("Name:"), md->widget(), attributes.value("name").toString()));
    md->addInputWidget(new MDLineEdit("description", tr("Description:"), md->widget(), attributes.value("description").toString()));
    MDGroupedCheckBoxes *md_display_options = new MDGroupedCheckBoxes("display_options", tr("Display options:"), md->widget(), attributes.value("display_options").toInt());
    md_display_options->addCheckBox(AssemblyRecordType::ShowServiceCompany, tr("Show service company"));
    md_display_options->addCheckBox(AssemblyRecordType::ShowCustomer, tr("Show customer"));
    md_display_options->addCheckBox(AssemblyRecordType::ShowCustomerContactPersons, tr("Show customer contact persons"));
    md_display_options->addCheckBox(AssemblyRecordType::ShowCircuit, tr("Show circuit"));
    md_display_options->addCheckBox(AssemblyRecordType::ShowCompressors, tr("Show compressors"));
    md_display_options->addCheckBox(AssemblyRecordType::ShowCircuitUnits, tr("Show circuit units"));
    md->addInputWidget(md_display_options);
    md->addInputWidget(new MDHighlightedPlainTextEdit("name_format", tr("Name format:"), md->widget(), attributes.value("name_format").toString(), keywords));
    md->addInputWidget(new MDComboBox("style", tr("Visual style:"), md->widget(), attributes.value("style").toString(), listStyles()));
    QStringList used_ids; MTSqlQuery query_used_ids;
    query_used_ids.setForwardOnly(true);
    query_used_ids.prepare("SELECT id FROM assembly_record_types" + QString(id().isEmpty() ? "" : " WHERE id <> :id"));
    if (!id().isEmpty()) { query_used_ids.bindValue(":id", id()); }
    if (query_used_ids.exec()) {
        while (query_used_ids.next()) {
            used_ids << query_used_ids.value(0).toString();
        }
    }
    md->setUsedIds(used_ids);
}

class AssemblyRecordTypeAttributes
{
public:
    AssemblyRecordTypeAttributes() {
        dict.insert("id", QApplication::translate("AssemblyRecordType", "ID"));
        dict.insert("name", QApplication::translate("AssemblyRecordType", "Name"));
        dict.insert("description", QApplication::translate("AssemblyRecordType", "Description"));
    }

    MTDictionary dict;
};

const MTDictionary &AssemblyRecordType::attributes()
{
    static AssemblyRecordTypeAttributes dict;
    return dict.dict;
}

bool AssemblyRecordType::remove()
{
    AssemblyRecordTypeCategory type_categories(id());
    return type_categories.remove() && MTRecord::remove();
}

AssemblyRecordItemType::AssemblyRecordItemType(const QString &id):
    DBRecord("assembly_record_item_types", "id", id, MTDictionary())
{}

void AssemblyRecordItemType::initEditDialogue(EditDialogueWidgets *md)
{
    QString currency = Global::DBInfoValueForKey("currency", "EUR");

    md->setWindowTitle(tr("Assembly Record Item Type"));

    QVariantMap attributes;
    if (!id().isEmpty() || !values().isEmpty()) {
        attributes = list();
    }

    md->addInputWidget(new MDHiddenIdField("id", md->widget(), attributes.value("id")));
    md->addInputWidget(new MDLineEdit("name", tr("Name:"), md->widget(), attributes.value("name").toString()));
    md->addInputWidget(new MDLineEdit("unit", tr("Unit:"), md->widget(), attributes.value("unit").toString()));
    md->addInputWidget(new MDDoubleSpinBox("acquisition_price", tr("Acquisition price:"), md->widget(), 0.0, 999999999.9, attributes.value("acquisition_price").toDouble(), currency));
    md->addInputWidget(new MDDoubleSpinBox("list_price", tr("List price:"), md->widget(), 0.0, 999999999.9, attributes.value("list_price").toDouble(), currency));
    md->addInputWidget(new MDDoubleSpinBox("discount", tr("Discount:"), md->widget(), 0.0, 100.0, attributes.value("discount").toDouble(), "%"));
    md->addInputWidget(new MDSpinBox("ean", tr("EAN code:"), md->widget(), 0, 99999999, attributes.value("ean").toInt()));
    md->addInputWidget(new MDCheckBox("auto_show", tr("Automatically add to assembly record"), md->widget(), attributes.value("auto_show").toBool()));
    md->addInputWidget(new MDComboBox("category_id", tr("Category:"), md->widget(), attributes.value("category_id").toString(), listAssemblyRecordItemCategories(true)));
    md->addInputWidget(new MDComboBox("inspection_variable_id", tr("Get value from inspection:"), md->widget(), attributes.value("inspection_variable_id").toString(), listAllVariables()));
    md->addInputWidget(new MDComboBox("value_data_type", tr("Data type:"), md->widget(), attributes.value("value_data_type").toString(), listDataTypes()));

    QStringList used_ids; MTSqlQuery query_used_ids;
    query_used_ids.setForwardOnly(true);
    query_used_ids.prepare("SELECT id FROM assembly_record_item_types" + QString(id().isEmpty() ? "" : " WHERE id <> :id"));
    if (!id().isEmpty()) { query_used_ids.bindValue(":id", id()); }
    if (query_used_ids.exec()) {
        while (query_used_ids.next()) {
            used_ids << query_used_ids.value(0).toString();
        }
    }
    md->setUsedIds(used_ids);
}

class AssemblyRecordItemTypeAttributes
{
public:
    AssemblyRecordItemTypeAttributes() {
        dict.insert("id", QApplication::translate("AssemblyRecordItemType", "ID"));
        dict.insert("name", QApplication::translate("AssemblyRecordItemType", "Name"));
        dict.insert("unit", QApplication::translate("AssemblyRecordItemType", "Unit"));
        dict.insert("acquisition_price", QApplication::translate("AssemblyRecordItemType", "Acquisition price"));
        dict.insert("list_price", QApplication::translate("AssemblyRecordItemType", "List price"));
        dict.insert("discount", QApplication::translate("AssemblyRecordItemType", "Discount"));
        dict.insert("ean", QApplication::translate("AssemblyRecordItemType", "EAN code"));
        dict.insert("category_id", QApplication::translate("AssemblyRecordItemType", "Category"));
    }

    MTDictionary dict;
};

const MTDictionary &AssemblyRecordItemType::attributes()
{
    static AssemblyRecordItemTypeAttributes dict;
    return dict.dict;
}

AssemblyRecordTypeCategory::AssemblyRecordTypeCategory(const QString &record_type_id):
    MTRecord("assembly_record_type_categories", "", "", record_type_id.isEmpty() ? MTDictionary() : MTDictionary("record_type_id", record_type_id))
{}

class AssemblyRecordTypeCategoryAttributes
{
public:
    AssemblyRecordTypeCategoryAttributes() {
        dict.insert("record_type_id", QApplication::translate("AssemblyRecordTypeCategory", "Assembly record type ID"));
        dict.insert("record_category_id", QApplication::translate("AssemblyRecordTypeCategory", "Assembly record category type ID"));
    }

    MTDictionary dict;
};

const MTDictionary &AssemblyRecordTypeCategory::attributes()
{
    static AssemblyRecordTypeCategoryAttributes dict;
    return dict.dict;
}

AssemblyRecordItemCategory::AssemblyRecordItemCategory(const QString &id):
    DBRecord("assembly_record_item_categories", "id", id, MTDictionary())
{
}

class AssemblyRecordItemCategoryAttributes
{
public:
    AssemblyRecordItemCategoryAttributes() {
        dict.insert("id", QApplication::translate("AssemblyRecordItemCategory", "ID"));
        dict.insert("name", QApplication::translate("AssemblyRecordItemCategory", "Name"));
    }

    MTDictionary dict;
};

const MTDictionary &AssemblyRecordItemCategory::attributes()
{
    static AssemblyRecordItemCategoryAttributes dict;
    return dict.dict;
}

void AssemblyRecordItemCategory::initEditDialogue(EditDialogueWidgets *md)
{
    md->setWindowTitle(tr("Assembly Record Item Category"));

    QVariantMap attributes;
    if (!id().isEmpty() || !values().isEmpty()) {
        attributes = list();
    }

    md->addInputWidget(new MDHiddenIdField("id", md->widget(), attributes.value("id").toString()));
    md->addInputWidget(new MDLineEdit("name", tr("Name:"), md->widget(), attributes.value("name").toString()));
    MDGroupedCheckBoxes *md_display_options = new MDGroupedCheckBoxes("display_options", tr("Display options:"), md->widget(), attributes.value("display_options").toInt());
    md_display_options->addCheckBox(AssemblyRecordItemCategory::ShowValue, tr("Show value"));
    md_display_options->addCheckBox(AssemblyRecordItemCategory::ShowAcquisitionPrice, tr("Show acquisition price"));
    md_display_options->addCheckBox(AssemblyRecordItemCategory::ShowListPrice, tr("Show list price"));
    md_display_options->addCheckBox(AssemblyRecordItemCategory::ShowDiscount, tr("Show discount"));
    md_display_options->addCheckBox(AssemblyRecordItemCategory::ShowTotal, tr("Calculate total"));
    md->addInputWidget(md_display_options);
    MDRadioButtonGroup *md_position = new MDRadioButtonGroup("display_position", tr("Display:"), md->widget(), QString::number(attributes.value("display_position").toInt()));
    md_position->addRadioButton(tr("In table"), QString::number(AssemblyRecordItemCategory::DisplayAtTop));
    md_position->addRadioButton(tr("Separately"), QString::number(AssemblyRecordItemCategory::DisplayAtBottom));
    md->addInputWidget(md_position);
    QStringList used_ids; MTSqlQuery query_used_ids;
    query_used_ids.setForwardOnly(true);
    query_used_ids.prepare("SELECT id FROM assembly_record_item_categories" + QString(id().isEmpty() ? "" : " WHERE id <> :id"));
    if (!id().isEmpty()) { query_used_ids.bindValue(":id", id()); }
    if (query_used_ids.exec()) {
        while (query_used_ids.next()) {
            used_ids << query_used_ids.value(0).toString();
        }
    }
    md->setUsedIds(used_ids);
}

AssemblyRecordItem::AssemblyRecordItem(const QString &record_id):
    MTRecord("assembly_record_items", "", "", MTDictionary("arno", record_id))
{}

AssemblyRecordItem::AssemblyRecordItem(const QString &table, const QString &id_column, const QString &id, const MTDictionary &parents):
    MTRecord(table, id_column, id, parents)
{}

class AssemblyRecordItemAttributes
{
public:
    AssemblyRecordItemAttributes() {
        dict.insert("arno", QApplication::translate("AssemblyRecordItem", "Assembly record number"));
        dict.insert("item_type_id", QApplication::translate("AssemblyRecordItem", "Record item type ID"));
        dict.insert("value", QApplication::translate("AssemblyRecordItem", "Value"));
        dict.insert("acquisition_price", QApplication::translate("AssemblyRecordItem", "Acquisition price"));
        dict.insert("list_price", QApplication::translate("AssemblyRecordItem", "List price"));
    }

    MTDictionary dict;
};

const MTDictionary &AssemblyRecordItem::attributes()
{
    static AssemblyRecordItemAttributes dict;
    return dict.dict;
}

AssemblyRecordItemByInspector::AssemblyRecordItemByInspector(const QString &inspector_id):
    AssemblyRecordItem("assembly_record_items LEFT JOIN inspections ON assembly_record_items.arno = inspections.arno", "", "",
        MTDictionary(QStringList() << "item_type_id" << "source", QStringList() << inspector_id << QString::number(AssemblyRecordItem::Inspectors)))
{}

File::File(const QString &file_id):
    MTRecord("files", "id", file_id, MTDictionary())
{}

Person::Person(const QString &person_id):
    MTRecord("persons", "id", person_id, MTDictionary())
{}

Person::Person(const QString &person_id, const QString &customer_id):
    MTRecord("persons", "id", person_id, MTDictionary("company_id", customer_id))
{}

class PersonAttributes
{
public:
    PersonAttributes() {
        dict.insert("id", QApplication::translate("Person", "ID"));
        dict.insert("company_id", QApplication::translate("Person", "Company ID"));
        dict.insert("name", QApplication::translate("Person", "Name"));
        dict.insert("mail", QApplication::translate("Person", "E-mail"));
        dict.insert("phone", QApplication::translate("Person", "Phone"));
        dict.insert("hidden", QApplication::translate("Person", "Hidden"));
    }

    MTDictionary dict;
};

const MTDictionary &Person::attributes()
{
    static PersonAttributes dict;
    return dict.dict;
}

CircuitUnitType::CircuitUnitType(const QString &id):
    DBRecord("circuit_unit_types", "id", id, MTDictionary())
{}

void CircuitUnitType::initEditDialogue(EditDialogueWidgets *md)
{
    QString currency = Global::DBInfoValueForKey("currency", "EUR");

    md->setWindowTitle(tr("Circuit Unit Type"));
    MTDictionary refrigerants(listRefrigerantsToString().split(';'));
    MTDictionary locations;
    locations.insert(QString::number(CircuitUnitType::External), tr("External"));
    locations.insert(QString::number(CircuitUnitType::Internal), tr("Internal"));
    MTDictionary output_units;
    output_units.insert("kW", "kW");
    output_units.insert("m3", "m3");

    QVariantMap attributes;
    if (!id().isEmpty() || !values().isEmpty()) {
        attributes = list();
    }

    md->addInputWidget(new MDHiddenIdField("id", md->widget(), attributes.value("id").toString()));
    md->addInputWidget(new MDLineEdit("manufacturer", tr("Manufacturer:"), md->widget(), attributes.value("manufacturer").toString()));
    md->addInputWidget(new MDLineEdit("type", tr("Type:"), md->widget(), attributes.value("type").toString()));
    md->addInputWidget(new MDComboBox("refrigerant", tr("Refrigerant:"), md->widget(), attributes.value("refrigerant").toString(), refrigerants));
    md->addInputWidget(new MDDoubleSpinBox("refrigerant_amount", tr("Amount of refrigerant:"), md->widget(), 0.0, 999999.9, attributes.value("refrigerant_amount").toDouble(), QApplication::translate("Units", "kg")));
    md->addInputWidget(new MDComboBox("oil", tr("Oil:"), md->widget(), attributes.value("oil").toString(), oils()));
    md->addInputWidget(new MDDoubleSpinBox("oil_amount", tr("Amount of oil:"), md->widget(), 0.0, 999999.9, attributes.value("oil_amount").toDouble(), QApplication::translate("Units", "kg")));
    md->addInputWidget(new MDDoubleSpinBox("acquisition_price", tr("Acquisition price:"), md->widget(), 0.0, 999999999.9, attributes.value("acquisition_price").toDouble(), currency));
    md->addInputWidget(new MDDoubleSpinBox("list_price", tr("List price:"), md->widget(), 0.0, 999999999.9, attributes.value("list_price").toDouble(), currency));
    md->addInputWidget(new MDDoubleSpinBox("discount", tr("Discount:"), md->widget(), 0.0, 100.0, attributes.value("discount").toDouble(), "%"));
    md->addInputWidget(new MDComboBox("location", tr("Location:"), md->widget(), attributes.value("location").toString(), locations));
    md->addInputWidget(new MDLineEdit("unit", tr("Unit of measure:"), md->widget(), attributes.value("unit").toString()));
    QList<MDAbstractInputWidget *> gw_list;
    gw_list.append(new MDDoubleSpinBox("output", tr("Value:"), md->widget(), 0.0, 999999.9, attributes.value("output").toDouble()));
    gw_list.append(new MDComboBox("output_unit", tr("Unit:"), md->widget(), attributes.value("output_unit").toString(), output_units));
    gw_list.append(new MDDoubleSpinBox("output_t0_tc", tr("At t0/tc:"), md->widget(), 0.0, 999999.9, attributes.value("output_t0_tc").toDouble()));
    md->addGroupedInputWidgets(tr("Output:"), gw_list);
    md->addInputWidget(new MDPlainTextEdit("notes", tr("Notes:"), md->widget(), attributes.value("notes").toString()));

    QStringList used_ids; MTSqlQuery query_used_ids;
    query_used_ids.setForwardOnly(true);
    query_used_ids.prepare("SELECT id FROM circuit_unit_types" + QString(id().isEmpty() ? "" : " WHERE id <> :id"));
    if (!id().isEmpty()) { query_used_ids.bindValue(":id", id()); }
    if (query_used_ids.exec()) {
        while (query_used_ids.next()) {
            used_ids << query_used_ids.value(0).toString();
        }
    }
    md->setUsedIds(used_ids);
}

class CircuitUnitTypeAttributes
{
public:
    CircuitUnitTypeAttributes() {
        dict.insert("id", QApplication::translate("CircuitUnitType", "ID"));
        dict.insert("manufacturer", QApplication::translate("CircuitUnitType", "Manufacturer"));
        dict.insert("type", QApplication::translate("CircuitUnitType", "Type"));
        dict.insert("refrigerant", QApplication::translate("CircuitUnitType", "Refrigerant"));
        dict.insert("refrigerant_amount", QApplication::translate("CircuitUnitType", "Amount of refrigerant"));
        dict.insert("oil", QApplication::translate("CircuitUnitType", "Oil"));
        dict.insert("oil_amount", QApplication::translate("CircuitUnitType", "Amount of oil"));
        dict.insert("acquisition_price", QApplication::translate("CircuitUnitType", "Acquisition price"));
        dict.insert("list_price", QApplication::translate("CircuitUnitType", "List price"));
        dict.insert("discount", QApplication::translate("CircuitUnitType", "Discount"));
        dict.insert("location", QApplication::translate("CircuitUnitType", "Location"));
        dict.insert("unit", QApplication::translate("CircuitUnitType", "Unit of measure"));
        dict.insert("output", QApplication::translate("CircuitUnitType", "Output"));
        dict.insert("output_t0_tc", QApplication::translate("CircuitUnitType", "Output at t0/tc"));
        dict.insert("category_id", QApplication::translate("CircuitUnitType", "Category"));
        dict.insert("notes", QApplication::translate("CircuitUnitType", "Notes"));
    }

    MTDictionary dict;
};

const MTDictionary &CircuitUnitType::attributes()
{
    static CircuitUnitTypeAttributes dict;
    return dict.dict;
}

const QString CircuitUnitType::locationToString(int id)
{
    switch (id) {
    case CircuitUnitType::External:
        return tr("External");
    case CircuitUnitType::Internal:
        return tr("Internal");
    }
    return QString();
}

CircuitUnit::CircuitUnit(const QString &id, const MTDictionary &dict):
    MTRecord("circuit_units", "id", id, dict)
{}

InspectionImage::InspectionImage(const QString &customer_id, const QString &circuit_id, const QString &inspection_id):
    MTRecord("inspection_images", "", "", MTDictionary(QStringList() << "customer" << "circuit" << "date", QStringList() << customer_id << circuit_id << inspection_id))
{}

Style::Style(const QString &id):
    DBRecord("styles", "id", id, MTDictionary())
{}

void Style::initEditDialogue(EditDialogueWidgets *md)
{
    md->setWindowTitle(tr("Style"));
    QVariantMap attributes;
    if (!id().isEmpty()) {
        attributes = list();
    }
    md->addInputWidget(new MDLineEdit("name", tr("Name:"), md->widget(), attributes.value("name").toString()));
    md->addInputWidget(new MDPlainTextEdit("content", tr("Style:"), md->widget(), attributes.value("content").toString()));
    md->addInputWidget(new MDCheckBox("div_tables", tr("Use div elements instead of tables"), md->widget(), attributes.value("div_tables").toBool()));
    QStringList used_ids; MTSqlQuery query_used_ids;
    query_used_ids.setForwardOnly(true);
    query_used_ids.prepare("SELECT id FROM styles" + QString(id().isEmpty() ? "" : " WHERE id <> :id"));
    if (!id().isEmpty()) { query_used_ids.bindValue(":id", id()); }
    if (query_used_ids.exec()) {
        while (query_used_ids.next()) {
            used_ids << query_used_ids.value(0).toString();
        }
    }
    md->setUsedIds(used_ids);
}

class StyleAttributes
{
public:
    StyleAttributes() {
        dict.insert("name", QApplication::translate("CircuitUnitType", "ID"));
        dict.insert("content", QApplication::translate("CircuitUnitType", "Content"));
    }

    MTDictionary dict;
};

const MTDictionary &Style::attributes()
{
    static StyleAttributes dict;
    return dict.dict;
}

Compressor::Compressor(const QString &id, const MTDictionary &dict):
    MTRecord("compressors", "id", id, dict)
{
    setSerialId(true);
}

class CompressorAttributes
{
public:
    CompressorAttributes() {
        dict.insert("id", QApplication::translate("Compressor", "ID"));
        dict.insert("name", QApplication::translate("Compressor", "Compressor name"));
        dict.insert("manufacturer", QApplication::translate("Compressor", "Manufacturer"));
        dict.insert("type", QApplication::translate("Compressor", "Type"));
        dict.insert("sn", QApplication::translate("Compressor", "Serial number"));
    }

    MTDictionary dict;
};

const MTDictionary &Compressor::attributes()
{
    static CompressorAttributes dict;
    return dict.dict;
}

InspectionsCompressor::InspectionsCompressor(const QString &id, const MTDictionary &dict):
    MTRecord("inspections_compressors", "id", id, dict)
{
    setSerialId(true);
}
