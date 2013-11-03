/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2013 Matus & Michal Tomlein

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

#include "toolbarstack.h"

#include "global.h"
#include "viewtabsettings.h"
#include "mainwindowsettings.h"
#include "linkparser.h"
#include "records.h"
#include "mtaddress.h"

using namespace Global;

ToolBarStack::ToolBarStack(QWidget *parent):
    QWidget(parent), _view(View::ViewCount), _settings(NULL)
{
    setupUi(this);

#ifdef Q_OS_MAC
    setStyleSheet(".QWidget { background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #F5F5F5, stop: 1 #CDCDCD);"
                             "color: white; border-color: #A9A9A9; border-style: solid; border-width: 0px 0px 1px 0px; }"
                  "QToolButton { border-color: #888888; border-style: solid; border-width: 1px; border-radius: 3px; min-height: 16px;"
                                "background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #F9F9F9, stop: 1 #D6D6D6); }"
                  "QToolButton:pressed { color: white;"
                                        "background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #6F6F6F, stop: 1 #9C9C9C); }");
#else
    setStyleSheet(".QWidget { background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #F5F5F5, stop: 1 #CDCDCD);"
                             "color: white; border-color: #A9A9A9; border-style: solid; border-width: 0px 0px 1px 0px; }"
                  "QToolButton { min-height: 16px; }");
#endif

    QObject::connect(chb_table_all_circuits, SIGNAL(clicked(bool)), this, SLOT(toggleTableForAllCircuits()));
    QObject::connect(spb_filter_since, SIGNAL(valueChanged(int)), this, SIGNAL(filterChanged()));
    QObject::connect(spb_filter_month_from, SIGNAL(valueChanged(int)), this, SLOT(monthFromChanged(int)));
    QObject::connect(spb_filter_month_until, SIGNAL(valueChanged(int)), this, SLOT(monthUntilChanged(int)));
    QObject::connect(le_filter, SIGNAL(returnPressed()), this, SIGNAL(filterChanged()));
    QObject::connect(cb_filter_column, SIGNAL(currentIndexChanged(int)), this, SLOT(emitFilterChanged()));
    QObject::connect(cb_filter_type, SIGNAL(currentIndexChanged(int)), this, SLOT(emitFilterChanged()));

    QObject::connect(btn_clear_inspector, SIGNAL(clicked()), this, SLOT(clearInspector()));
    QObject::connect(btn_clear_customer, SIGNAL(clicked()), this, SLOT(clearCustomer()));
    QObject::connect(btn_clear_circuit, SIGNAL(clicked()), this, SLOT(clearCircuit()));
    QObject::connect(btn_clear_inspection, SIGNAL(clicked()), this, SLOT(clearInspection()));
    QObject::connect(btn_clear_repair, SIGNAL(clicked()), this, SLOT(clearRepair()));
    QObject::connect(btn_clear_assembly_record_type, SIGNAL(clicked()), this, SLOT(clearAssemblyRecordType()));
    QObject::connect(btn_clear_assembly_record_item_type, SIGNAL(clicked()), this, SLOT(clearAssemblyRecordItemType()));
    QObject::connect(btn_clear_assembly_record_item_category, SIGNAL(clicked()), this, SLOT(clearAssemblyRecordItemCategory()));
    QObject::connect(btn_clear_circuit_unit_type, SIGNAL(clicked()), this, SLOT(clearCircuitUnitType()));
}

void ToolBarStack::setSettings(ViewTabSettings *settings)
{
    _settings = settings;
    viewChanged(View::Store);
    setReportDataGroupBoxVisible(false);
}

void ToolBarStack::connectSlots(QObject *receiver)
{
    QObject::connect(this, SIGNAL(filterChanged()), receiver, SLOT(refreshView()));
    QObject::connect(tbtn_edit_service_company, SIGNAL(clicked()), receiver, SLOT(editServiceCompany()));
    QObject::connect(tbtn_add_record_of_refrigerant_management, SIGNAL(clicked()), receiver, SLOT(addRecordOfRefrigerantManagement()));
    QObject::connect(tbtn_add_inspector, SIGNAL(clicked()), receiver, SLOT(addInspector()));
    QObject::connect(tbtn_edit_inspector, SIGNAL(clicked()), receiver, SLOT(editInspector()));
    QObject::connect(tbtn_remove_inspector, SIGNAL(clicked()), receiver, SLOT(removeInspector()));
    QObject::connect(tbtn_add_customer, SIGNAL(clicked()), receiver, SLOT(addCustomer()));
    QObject::connect(tbtn_edit_customer, SIGNAL(clicked()), receiver, SLOT(editCustomer()));
    QObject::connect(tbtn_remove_customer, SIGNAL(clicked()), receiver, SLOT(removeCustomer()));
    QObject::connect(tbtn_add_repair, SIGNAL(clicked()), receiver, SLOT(addRepair()));
    QObject::connect(tbtn_edit_repair, SIGNAL(clicked()), receiver, SLOT(editRepair()));
    QObject::connect(tbtn_remove_repair, SIGNAL(clicked()), receiver, SLOT(removeRepair()));
    QObject::connect(tbtn_add_circuit, SIGNAL(clicked()), receiver, SLOT(addCircuit()));
    QObject::connect(tbtn_edit_circuit, SIGNAL(clicked()), receiver, SLOT(editCircuit()));
    QObject::connect(tbtn_remove_circuit, SIGNAL(clicked()), receiver, SLOT(removeCircuit()));
    QObject::connect(tbtn_add_inspection, SIGNAL(clicked()), receiver, SLOT(addInspection()));
    QObject::connect(tbtn_edit_inspection, SIGNAL(clicked()), receiver, SLOT(editInspection()));
    QObject::connect(tbtn_remove_inspection, SIGNAL(clicked()), receiver, SLOT(removeInspection()));
    QObject::connect(tbtn_add_assembly_record_type, SIGNAL(clicked()), receiver, SLOT(addAssemblyRecordType()));
    QObject::connect(tbtn_edit_assembly_record_type, SIGNAL(clicked()), receiver, SLOT(editAssemblyRecordType()));
    QObject::connect(tbtn_remove_assembly_record_type, SIGNAL(clicked()), receiver, SLOT(removeAssemblyRecordType()));
    QObject::connect(tbtn_edit_assembly_record_item_type, SIGNAL(clicked()), receiver, SLOT(editAssemblyRecordItemType()));
    QObject::connect(tbtn_add_assembly_record_item_type, SIGNAL(clicked()), receiver, SLOT(addAssemblyRecordItemType()));
    QObject::connect(tbtn_remove_assembly_record_item_type, SIGNAL(clicked()), receiver, SLOT(removeAssemblyRecordItemType()));
    QObject::connect(tbtn_edit_assembly_record_item_category, SIGNAL(clicked()), receiver, SLOT(editAssemblyRecordItemCategory()));
    QObject::connect(tbtn_add_assembly_record_item_category, SIGNAL(clicked()), receiver, SLOT(addAssemblyRecordItemCategory()));
    QObject::connect(tbtn_remove_assembly_record_item_category, SIGNAL(clicked()), receiver, SLOT(removeAssemblyRecordItemCategory()));
    QObject::connect(tbtn_add_circuit_unit_type, SIGNAL(clicked()), receiver, SLOT(addCircuitUnitType()));
    QObject::connect(tbtn_remove_circuit_unit_type, SIGNAL(clicked()), receiver, SLOT(removeCircuitUnitType()));
    QObject::connect(tbtn_edit_circuit_unit_type, SIGNAL(clicked()), receiver, SLOT(editCircuitUnitType()));
    QObject::connect(chb_by_field, SIGNAL(clicked()), receiver, SLOT(refreshView()));
    QObject::connect(chb_assembly_record_acquisition_price, SIGNAL(clicked()), receiver, SLOT(refreshView()));
    QObject::connect(chb_assembly_record_list_price, SIGNAL(clicked()), receiver, SLOT(refreshView()));
    QObject::connect(chb_assembly_record_total, SIGNAL(clicked()), receiver, SLOT(refreshView()));
}

void ToolBarStack::addFilterItems(const QString &column, const MTDictionary &items)
{
    cb_filter_column->insertSeparator(cb_filter_column->count());

    QSet<QString> used;
    for (int i = 0; i < items.count(); ++i) {
        if (!used.contains(items.value(i))) {
            used << items.value(i);
            cb_filter_column->addItem(items.value(i), QString("%1 = '%2' AND ? IS NOT NULL").arg(column).arg(items.key(i)));
        }
    }
}

void ToolBarStack::viewChanged(View::ViewID view)
{
    lbl_view->setText(_settings->currentViewTitle());

    if (_view == view)
        return;

    _view = view;

    tbtn_add_assembly_record_item_category->setVisible(view == View::AssemblyRecordItems);
    tbtn_add_assembly_record_item_type->setVisible(view == View::AssemblyRecordItems);
    tbtn_add_assembly_record_type->setVisible(view == View::AssemblyRecordTypes);
    tbtn_add_circuit->setVisible(view == View::Circuits);
    tbtn_add_circuit_unit_type->setVisible(view == View::CircuitUnitTypes);
    tbtn_add_customer->setVisible(view == View::Customers);
    tbtn_add_inspection->setVisible(view == View::Inspections);
    tbtn_add_inspector->setVisible(view == View::Inspectors);
    tbtn_add_record_of_refrigerant_management->setVisible(view == View::Store || view == View::RefrigerantManagement);
    tbtn_add_repair->setVisible(view == View::Repairs);
    tbtn_edit_service_company->setVisible(view == View::Store);

    enableTools();

    lbl_filter_since->setText(tr("Since:"));
    spb_filter_since->setSpecialValueText(tr("All"));
    cb_filter_column->clear();
    le_filter->clear();

    bool filter_by_field_visible = false;
    bool filter_all_circuits_visible = false;
    bool filter_since_visible = false;
    bool filter_month_visible = false;
    bool assembly_record_widgets_visible = false;

    switch (view) {
        case View::Store:
            filter_by_field_visible = true;
            filter_since_visible = true;
            break;
        case View::RefrigerantManagement:
            filter_since_visible = true;
            cb_filter_column->addItem(QApplication::translate("RecordOfRefrigerantManagement", "Date"), "date");
            cb_filter_column->addItem(QApplication::translate("RecordOfRefrigerantManagement", "Business partner"), "partner");
            cb_filter_column->addItem(QApplication::translate("RecordOfRefrigerantManagement", "Business partner (ID)"), "partner_id");
            cb_filter_column->addItem(QApplication::translate("RecordOfRefrigerantManagement", "Refrigerant"), "refrigerant");
            break;
        case View::Customers:
            cb_filter_column->addItem(QApplication::translate("Customer", "ID"), "id");
            cb_filter_column->addItem(QApplication::translate("Customer", "Company"), "company");
            cb_filter_column->addItem(QApplication::translate("Customer", "Address"), "address");
            cb_filter_column->addItem(QApplication::translate("Customer", "E-mail"), "mail");
            cb_filter_column->addItem(QApplication::translate("Customer", "Phone"), "phone");
            break;
        case View::Circuits:
            updateView_ListOfCircuits_CircuitAttributes:
            cb_filter_column->addItem(QApplication::translate("Circuit", "ID"), "id");
            cb_filter_column->addItem(QApplication::translate("Circuit", "Circuit name"), "name");
            cb_filter_column->addItem(QApplication::translate("Circuit", "Place of operation"), "operation");
            cb_filter_column->addItem(QApplication::translate("Circuit", "Building"), "building");
            cb_filter_column->addItem(QApplication::translate("Circuit", "Device"), "device");
            cb_filter_column->addItem(QApplication::translate("Circuit", "Manufacturer"), "manufacturer");
            cb_filter_column->addItem(QApplication::translate("Circuit", "Type"), "type");
            cb_filter_column->addItem(QApplication::translate("Circuit", "Serial number"), "sn");
            cb_filter_column->addItem(QApplication::translate("Circuit", "Year of purchase"), "year");
            cb_filter_column->addItem(QApplication::translate("Circuit", "Date of commissioning"), "commissioning");
            cb_filter_column->addItem(QApplication::translate("Circuit", "Refrigerant"), "refrigerant");
            cb_filter_column->addItem(QApplication::translate("Circuit", "Oil"), "oil");
            addFilterItems("field", fieldsOfApplication());
            break;
        case View::Inspections:
            filter_since_visible = true;
            cb_filter_column->addItem(QApplication::translate("Inspection", "Date"), "date");
            cb_filter_column->addItem(QApplication::translate("Inspection", "Operator"), "operator");
            cb_filter_column->addItem(QApplication::translate("Inspection", "Remedies"), "rmds");
            cb_filter_column->addItem(QApplication::translate("Inspection", "Assembly record No."), "arno");
            cb_filter_column->addItem(QApplication::translate("Inspection", "Assembly record type"), "ar_type");
            break;
        case View::Repairs:
            filter_since_visible = true;
            cb_filter_column->addItem(QApplication::translate("Repair", "Date"), "date");
            cb_filter_column->addItem(QApplication::translate("Repair", "Customer"), "customer");
            cb_filter_column->addItem(QApplication::translate("Repair", "Device"), "device");
            cb_filter_column->addItem(QApplication::translate("Repair", "Refrigerant"), "refrigerant");
            cb_filter_column->addItem(QApplication::translate("Repair", "Assembly record No."), "arno");
            break;
        case View::Inspectors:
            cb_filter_column->addItem(QApplication::translate("Inspector", "ID"), "id");
            cb_filter_column->addItem(QApplication::translate("Inspector", "Name"), "person");
            cb_filter_column->addItem(QApplication::translate("Inspector", "E-mail"), "mail");
            cb_filter_column->addItem(QApplication::translate("Inspector", "Phone"), "phone");
            break;
        case View::InspectorDetails:
            filter_since_visible = true;
            cb_filter_column->addItem(QApplication::translate("Inspection", "Date"), "date");
            cb_filter_column->addItem(QApplication::translate("Inspector", "Customer ID"), "customer");
            cb_filter_column->addItem(QApplication::translate("Circuit", "Circuit ID"), "circuit");
            break;
        case View::TableOfInspections:
            filter_since_visible = true;
            filter_all_circuits_visible = true;
            break;
        case View::Agenda:
            cb_filter_column->addItem(QApplication::translate("Customer", "ID"), "parent");
            goto updateView_ListOfCircuits_CircuitAttributes;
            break;
        case View::OperatorReport:
            filter_since_visible = true;
            filter_month_visible = true;
            lbl_filter_since->setText(tr("Year:"));
            spb_filter_since->setSpecialValueText(tr("Last"));
            cb_filter_column->addItem(QApplication::translate("Circuit", "ID"), "id");
            cb_filter_column->addItem(QApplication::translate("Circuit", "Refrigerant"), "refrigerant");
            cb_filter_column->addItem(QApplication::translate("Circuit", "Place of operation"), "operation");
            addFilterItems("field", fieldsOfApplication());
            break;
        case View::AssemblyRecordTypes:
            cb_filter_column->addItem(QApplication::translate("AssemblyRecord", "Name"), "name");
            cb_filter_column->addItem(QApplication::translate("AssemblyRecord", "Description"), "description");
            break;
        case View::AssemblyRecordItems:
            cb_filter_column->addItem(QApplication::translate("AssemblyRecord", "Name"), "name");
            break;
        case View::CircuitUnitTypes:
            cb_filter_column->addItem(QApplication::translate("AssemblyRecord", "Manufacturer"), "manufacturer");
            cb_filter_column->addItem(QApplication::translate("AssemblyRecord", "Type"), "type");
            cb_filter_column->addItem(QApplication::translate("AssemblyRecord", "Refrigerant"), "refrigerant");
            cb_filter_column->addItem(QApplication::translate("AssemblyRecord", "Oil"), "oil");
            cb_filter_column->addItem(QApplication::translate("AssemblyRecord", "Notes"), "notes");
            break;
        case View::AssemblyRecords:
            filter_since_visible = true;
            cb_filter_column->addItem(QApplication::translate("AssemblyRecord", "Date"), "date");
            cb_filter_column->addItem(QApplication::translate("AssemblyRecord", "Inspector ID"), "inspector");
            cb_filter_column->addItem(QApplication::translate("AssemblyRecord", "Assembly record No."), "arno");
            cb_filter_column->addItem(QApplication::translate("AssemblyRecord", "Assembly record type ID"), "ar_type");
            break;
        case View::AssemblyRecordDetails:
            assembly_record_widgets_visible = true;
            break;
        case View::InspectionDetails:
        case View::InspectionImages:
        case View::LeakagesByApplication:
        default:
            break;
    }

    chb_by_field->setVisible(filter_by_field_visible);

    chb_table_all_circuits->setVisible(filter_all_circuits_visible);

    cb_filter_column->setVisible(cb_filter_column->count());
    cb_filter_type->setVisible(cb_filter_column->count());
    le_filter->setVisible(cb_filter_column->count());

    lbl_filter_since->setVisible(filter_since_visible);
    spb_filter_since->setVisible(filter_since_visible);

    lbl_filter_month->setVisible(filter_month_visible);
    spb_filter_month_from->setVisible(filter_month_visible);
    lbl_filter_month_dash->setVisible(filter_month_visible);
    spb_filter_month_until->setVisible(filter_month_visible);

    lbl_assembly_records_show->setVisible(assembly_record_widgets_visible);
    chb_assembly_record_acquisition_price->setVisible(assembly_record_widgets_visible);
    chb_assembly_record_list_price->setVisible(assembly_record_widgets_visible);
    chb_assembly_record_total->setVisible(assembly_record_widgets_visible);

    widget_filter->setVisible(cb_filter_column->count() ||
                              filter_by_field_visible ||
                              filter_all_circuits_visible ||
                              filter_since_visible ||
                              filter_month_visible ||
                              assembly_record_widgets_visible);
}

void ToolBarStack::toggleTableForAllCircuits()
{
    _settings->refreshView();
}

void ToolBarStack::emitFilterChanged()
{
    bool enabled = !filterColumn().contains('?');
    bool changed = !isFilterEmpty() || enabled != le_filter->isEnabled();
    cb_filter_type->setEnabled(enabled);
    le_filter->setEnabled(enabled);

    if (changed && cb_filter_column->count())
        emit filterChanged();
}

void ToolBarStack::monthFromChanged(int value)
{
    if (spb_filter_month_until->value() < value)
        spb_filter_month_until->setValue(value);
    emit filterChanged();
}

void ToolBarStack::monthUntilChanged(int value)
{
    if (spb_filter_month_from->value() > value)
        spb_filter_month_from->setValue(value);
    emit filterChanged();
}

void ToolBarStack::enableTools()
{
    if (_settings->isInspectorSelected()) {
        Inspector inspector(_settings->selectedInspector());
        inspector.readValues("person");

        QStringList description;
        description << _settings->selectedInspector().rightJustified(4, '0');
        if (!inspector.stringValue("person").isEmpty())
            description << QString("<b>%1</b>").arg(escapeString(inspector.stringValue("person")));

        lbl_inspector->setText(tr("Inspector: %1").arg(description.join(", ")));
    }
    widget_inspector->setVisible((_view == View::Inspectors || _view == View::InspectorDetails) && _settings->isInspectorSelected());

    if (_settings->isCustomerSelected()) {
        Customer customer(_settings->selectedCustomer());
        customer.readValues("company, address");

        QStringList description;
        description << _settings->selectedCustomer().rightJustified(8, '0');
        if (!customer.stringValue("company").isEmpty())
            description << QString("<b>%1</b>").arg(escapeString(customer.stringValue("company")));

        MTAddress address(customer.stringValue("address"));
        if (!address.isEmpty())
            description << address.toHtml();

        lbl_customer->setText(tr("Customer: %1").arg(description.join(", ")));
    }
    widget_customer->setVisible((_view == View::Customers ||
                                 _view == View::Circuits ||
                                 _view == View::Repairs ||
                                 _view == View::Inspections ||
                                 _view == View::InspectionDetails ||
                                 _view == View::InspectionImages ||
                                 _view == View::AssemblyRecords ||
                                 _view == View::AssemblyRecordDetails) &&
                                _settings->isCustomerSelected());

    if (_settings->isRepairSelected())
        lbl_repair->setText(tr("Repair: <b>%1</b>").arg(_settings->mainWindowSettings().formatDateTime(_settings->selectedRepair())));
    widget_repair->setVisible(_view == View::Repairs && _settings->isRepairSelected());

    if (_settings->isCircuitSelected()) {
        QStringList attributes;
        attributes << "name" << "operation" << "building" << "device";

        Circuit circuit(_settings->selectedCustomer(), _settings->selectedCircuit());
        circuit.readValues(attributes.join(", ") + ", field, refrigerant, " + circuitRefrigerantAmountQuery());

        QStringList description;
        description << _settings->selectedCircuit().rightJustified(5, '0');

        foreach (const QString &attribute, attributes)
            if (!circuit.stringValue(attribute).isEmpty())
                description << QString(attribute == "name" ? "<b>%1</b>" : "%1").arg(escapeString(circuit.stringValue(attribute)));

        if (attributeValues().contains("field::" + circuit.stringValue("field")))
            description << attributeValues().value("field::" + circuit.stringValue("field"));

        description << QString("%1 %2 %3")
                       .arg(circuit.stringValue("refrigerant_amount"))
                       .arg(QApplication::translate("Units", "kg"))
                       .arg(circuit.stringValue("refrigerant"));

        lbl_circuit->setText(tr("Circuit: %1").arg(description.join(", ")));
    }
    widget_circuit->setVisible((_view == View::Circuits ||
                                _view == View::Inspections ||
                                _view == View::InspectionDetails ||
                                _view == View::InspectionImages ||
                                _view == View::AssemblyRecords ||
                                _view == View::AssemblyRecordDetails) &&
                               _settings->isCircuitSelected());

    if (_settings->isInspectionSelected())
        lbl_inspection->setText(tr("Inspection: <b>%1</b>").arg(_settings->mainWindowSettings().formatDateTime(_settings->selectedInspection())));
    widget_inspection->setVisible((_view == View::Inspections ||
                                   _view == View::InspectionDetails ||
                                   _view == View::InspectionImages ||
                                   _view == View::AssemblyRecords ||
                                   _view == View::AssemblyRecordDetails) &&
                                  _settings->isInspectionSelected());

    if (_settings->isAssemblyRecordTypeSelected()) {
        AssemblyRecordType type(_settings->selectedAssemblyRecordType());
        type.readValues("name");

        QStringList description;
        description << _settings->selectedAssemblyRecordType();
        if (!type.stringValue("name").isEmpty())
            description << QString("<b>%1</b>").arg(escapeString(type.stringValue("name")));

        lbl_ar_type->setText(tr("Assembly Record Type: %1").arg(description.join(", ")));
    }
    widget_ar_type->setVisible(_view >= View::AssemblyRecordsRelated && _view <= View::AssemblyRecordsRelatedEnd
                               && _settings->isAssemblyRecordTypeSelected());

    if (_settings->isAssemblyRecordItemCategorySelected()) {
        AssemblyRecordItemCategory category(_settings->selectedAssemblyRecordItemCategory());
        category.readValues("name");

        QStringList description;
        description << _settings->selectedAssemblyRecordItemCategory();
        if (!category.stringValue("name").isEmpty())
            description << QString("<b>%1</b>").arg(escapeString(category.stringValue("name")));

        lbl_ar_item_category->setText(tr("Assembly Record Item Category: %1").arg(description.join(", ")));
    }
    widget_ar_item_category->setVisible(_view >= View::AssemblyRecordsRelated && _view <= View::AssemblyRecordsRelatedEnd
                                        && _settings->isAssemblyRecordItemCategorySelected());

    if (_settings->isAssemblyRecordItemTypeSelected()) {
        AssemblyRecordItemType item_type(_settings->selectedAssemblyRecordItemType());
        item_type.readValues("name");

        QStringList description;
        description << _settings->selectedAssemblyRecordItemType();
        if (!item_type.stringValue("name").isEmpty())
            description << QString("<b>%1</b>").arg(escapeString(item_type.stringValue("name")));

        lbl_ar_item_type->setText(tr("Assembly Record Item Type: %1").arg(description.join(", ")));
    }
    widget_ar_item_type->setVisible(_view >= View::AssemblyRecordsRelated && _view <= View::AssemblyRecordsRelatedEnd
                                    && _settings->isAssemblyRecordItemTypeSelected());

    if (_settings->isCircuitUnitTypeSelected()) {
        CircuitUnitType unit_type(_settings->selectedCircuitUnitType());
        unit_type.readValues("manufacturer, type, refrigerant, refrigerant_amount");

        QStringList description;
        description << _settings->selectedCircuitUnitType();
        if (!unit_type.stringValue("manufacturer").isEmpty())
            description << QString("<b>%1</b>").arg(escapeString(unit_type.stringValue("manufacturer")));
        if (!unit_type.stringValue("type").isEmpty())
            description << QString("<b>%1</b>").arg(escapeString(unit_type.stringValue("type")));

        description << QString("%1 %2 %3")
                       .arg(unit_type.stringValue("refrigerant_amount"))
                       .arg(QApplication::translate("Units", "kg"))
                       .arg(unit_type.stringValue("refrigerant"));

        lbl_circuit_unit_type->setText(tr("Circuit Unit Type: %1").arg(description.join(", ")));
    }
    widget_circuit_unit_type->setVisible(_view >= View::AssemblyRecordsRelated && _view <= View::AssemblyRecordsRelatedEnd
                                         && _settings->isCircuitUnitTypeSelected());

    tbtn_edit_inspector->setEnabled(_settings->isInspectorSelected());
    tbtn_remove_inspector->setEnabled(_settings->isInspectorSelected());
    tbtn_edit_customer->setEnabled(_settings->isCustomerSelected());
    tbtn_remove_customer->setEnabled(_settings->isCustomerSelected());
    tbtn_edit_repair->setEnabled(_settings->isRepairSelected());
    tbtn_remove_repair->setEnabled(_settings->isRepairSelected());
    tbtn_edit_circuit->setEnabled(_settings->isCircuitSelected());
    tbtn_remove_circuit->setEnabled(_settings->isCircuitSelected());
    tbtn_edit_inspection->setEnabled(_settings->isInspectionSelected());
    tbtn_remove_inspection->setEnabled(_settings->isInspectionSelected());
    tbtn_edit_assembly_record_type->setEnabled(_settings->isAssemblyRecordTypeSelected());
    tbtn_remove_assembly_record_type->setEnabled(_settings->isAssemblyRecordTypeSelected());
    tbtn_edit_assembly_record_item_type->setEnabled(_settings->isAssemblyRecordItemTypeSelected());
    tbtn_remove_assembly_record_item_type->setEnabled(_settings->isAssemblyRecordItemTypeSelected());
    tbtn_edit_assembly_record_item_category->setEnabled(_settings->isAssemblyRecordItemCategorySelected());
    tbtn_remove_assembly_record_item_category->setEnabled(_settings->isAssemblyRecordItemCategorySelected());
    tbtn_edit_circuit_unit_type->setEnabled(_settings->isCircuitUnitTypeSelected());
    tbtn_remove_circuit_unit_type->setEnabled(_settings->isCircuitUnitTypeSelected());
    chb_table_all_circuits->setEnabled(_settings->isCircuitSelected());
    chb_table_all_circuits->setChecked(!_settings->isCircuitSelected());

    bool enabled = isOperationPermitted("access_assembly_record_acquisition_price") > 0;
    chb_assembly_record_acquisition_price->setEnabled(enabled);
    chb_assembly_record_acquisition_price->setChecked(enabled);

    enabled = isOperationPermitted("access_assembly_record_list_price") > 0;
    chb_assembly_record_list_price->setEnabled(enabled);
    chb_assembly_record_list_price->setChecked(enabled);

    chb_assembly_record_total->setEnabled(enabled);
    chb_assembly_record_total->setChecked(enabled);
}

void ToolBarStack::clearInspector()
{
    _settings->clearSelectedInspector();
    _settings->enableAllTools();
    _settings->refreshView();
}

void ToolBarStack::clearCustomer()
{
    _settings->clearSelectedCustomer();
    _settings->enableAllTools();
    _settings->refreshView();
}

void ToolBarStack::clearCircuit()
{
    _settings->clearSelectedCircuit();
    _settings->enableAllTools();
    _settings->refreshView();
}

void ToolBarStack::clearInspection()
{
    _settings->clearSelectedInspection();
    _settings->enableAllTools();
    _settings->refreshView();
}

void ToolBarStack::clearRepair()
{
    _settings->clearSelectedRepair();
    _settings->enableAllTools();
    _settings->refreshView();
}

void ToolBarStack::clearAssemblyRecordType()
{
    _settings->clearSelectedAssemblyRecordType();
    _settings->enableAllTools();
    _settings->refreshView();
}

void ToolBarStack::clearAssemblyRecordItemType()
{
    _settings->clearSelectedAssemblyRecordItemType();
    _settings->enableAllTools();
    _settings->refreshView();
}

void ToolBarStack::clearAssemblyRecordItemCategory()
{
    _settings->clearSelectedAssemblyRecordItemCategory();
    _settings->enableAllTools();
    _settings->refreshView();
}

void ToolBarStack::clearCircuitUnitType()
{
    _settings->clearSelectedCircuitUnitType();
    _settings->enableAllTools();
    _settings->refreshView();
}

void ToolBarStack::setReportDataGroupBoxVisible(bool visible)
{
    if (visible) {
        widget_inspector->setVisible(!visible);
        widget_customer->setVisible(!visible);
        widget_repair->setVisible(!visible);
        widget_circuit->setVisible(!visible);
        widget_inspection->setVisible(!visible);
        widget_ar_type->setVisible(!visible);
        widget_ar_item_category->setVisible(!visible);
        widget_ar_item_type->setVisible(!visible);
        widget_circuit_unit_type->setVisible(!visible);

        widget_filter->setVisible(!visible);
    } else {
        viewChanged(_view);
    }

    widget_view->setVisible(!visible);
    widget_report_data->setVisible(visible);
}

QString ToolBarStack::filterKeyword() const
{
    if (filterColumn().contains('?'))
        return le_filter->text();
    switch (cb_filter_type->currentIndex()) {
        // contains
        case 0: return "%" + le_filter->text() + "%"; break;
        // is
        case 1: return le_filter->text(); break;
        // starts with
        case 2: return le_filter->text() + "%"; break;
        // ends with
        case 3: return "%" + le_filter->text(); break;
    }
    return le_filter->text();
}
