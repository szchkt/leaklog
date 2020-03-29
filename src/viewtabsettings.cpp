/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2020 Matus & Michal Tomlein

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

#include "viewtabsettings.h"
#include "mainwindowsettings.h"
#include "records.h"

#include <QSettings>

ViewTabSettings::ViewTabSettings()
{
}

void ViewTabSettings::saveSettings(QSettings &settings) const
{
    if (isServiceCompanySelected())
        settings.setValue("selected_service_company_uuid", selectedServiceCompanyUUID());
    if (isCustomerSelected())
        settings.setValue("selected_customer_uuid", _customer_uuid);
    if (isCircuitSelected())
        settings.setValue("selected_circuit_uuid", _circuit_uuid);
    if (isCompressorSelected())
        settings.setValue("selected_compressor_uuid", _compressor_uuid);
    if (isInspectionSelected())
        settings.setValue("selected_inspection_uuid", _inspection_uuid);
    if (isRepairSelected())
        settings.setValue("selected_repair_uuid", _repair_uuid);
    if (isInspectorSelected())
        settings.setValue("selected_inspector_uuid", _inspector_uuid);
    if (isAssemblyRecordTypeSelected())
        settings.setValue("selected_ar_type_uuid", _ar_type_uuid);
    if (isAssemblyRecordItemTypeSelected())
        settings.setValue("selected_ar_item_type_uuid", _ar_item_type_uuid);
    if (isAssemblyRecordItemCategorySelected())
        settings.setValue("selected_ar_item_category_uuid", _ar_item_category_uuid);
    if (isCircuitUnitTypeSelected())
        settings.setValue("selected_circuit_unit_type_uuid", _circuit_unit_type_uuid);

    settings.setValue("current_view", currentView());
    if (!currentTableUUID().isEmpty())
        settings.setValue("current_table_uuid", currentTableUUID());
}

void ViewTabSettings::restoreSettings(QSettings &settings)
{
    setSelectedServiceCompanyUUID(settings.value("selected_service_company_uuid").toString());
    _customer_uuid = settings.value("selected_customer_uuid").toString();
    _circuit_uuid = settings.value("selected_circuit_uuid").toString();
    _inspection_uuid = settings.value("selected_inspection_uuid").toString();
    _compressor_uuid = settings.value("selected_compressor_uuid").toString();
    _repair_uuid = settings.value("selected_repair_uuid").toString();
    _inspector_uuid = settings.value("selected_inspector_uuid").toString();
    _ar_type_uuid = settings.value("selected_ar_type_uuid").toString();
    _ar_item_type_uuid = settings.value("selected_ar_item_type_uuid").toString();
    _ar_item_category_uuid = settings.value("selected_ar_item_category_uuid").toString();
    _circuit_unit_type_uuid = settings.value("selected_circuit_unit_type_uuid").toString();

    validateSelection();

    QMetaObject::invokeMethod(object(), "setView", Qt::QueuedConnection,
                              Q_ARG(int, settings.value("current_view").toInt()),
                              Q_ARG(QString, settings.value("current_table_uuid").toString()));
}

void ViewTabSettings::validateSelection()
{
    if (isCustomerSelected() && !Customer(selectedCustomerUUID()).exists()) {
        clearSelectedCustomer();
    }
    if (isCircuitSelected() && !Circuit(selectedCircuitUUID()).exists()) {
        clearSelectedCircuit();
    }
    if (isInspectionSelected() && !Inspection(selectedInspectionUUID()).exists()) {
        clearSelectedInspection();
    }
    if (isCompressorSelected() && !Compressor(selectedCompressorUUID()).exists()) {
        clearSelectedCompressor();
    }
    if (isRepairSelected() && !Repair(selectedRepairUUID()).exists()) {
        clearSelectedRepair();
    }
    if (isInspectorSelected() && !Inspector(selectedInspectorUUID()).exists()) {
        clearSelectedInspector();
    }
    if (isAssemblyRecordTypeSelected() && !AssemblyRecordType(selectedAssemblyRecordTypeUUID()).exists()) {
        clearSelectedAssemblyRecordType();
    }
    if (isAssemblyRecordItemTypeSelected() && !AssemblyRecordItemType(selectedAssemblyRecordItemTypeUUID()).exists()) {
        clearSelectedAssemblyRecordItemType();
    }
    if (isAssemblyRecordItemCategorySelected() && !AssemblyRecordItemCategory(selectedAssemblyRecordItemCategoryUUID()).exists()) {
        clearSelectedAssemblyRecordItemCategory();
    }
    if (isCircuitUnitTypeSelected() && !CircuitUnitType(selectedCircuitUnitTypeUUID()).exists()) {
        clearSelectedCircuitUnitType();
    }
}

void ViewTabSettings::loadCustomer(const QString &customer_uuid, bool refresh)
{
    if (customer_uuid.isEmpty()) { return; }
    setSelectedCustomerUUID(customer_uuid);
    enableAllTools();
    if (refresh) {
        setView(View::Circuits);
    }
}

void ViewTabSettings::loadCircuit(const QString &circuit_uuid, bool refresh)
{
    if (!isCustomerSelected()) { return; }
    if (circuit_uuid.isEmpty()) { return; }
    setSelectedCircuitUUID(circuit_uuid);
    enableAllTools();
    if (refresh) {
        setView(View::Inspections);
    }
}

void ViewTabSettings::loadInspection(const QString &inspection_uuid, bool refresh)
{
    if (!isCustomerSelected()) { return; }
    if (!isCircuitSelected()) { return; }
    if (inspection_uuid.isEmpty()) { return; }
    if (Inspection(inspection_uuid).intValue("inspection_type") < 0) {
        clearSelectedInspection();
        enableAllTools();
        if (refresh) {
            setView(View::Inspections);
        }
        return;
    }
    setSelectedInspectionUUID(inspection_uuid);
    enableAllTools();
    if (refresh) {
        setView(View::InspectionDetails);
    }
}

void ViewTabSettings::loadRepair(const QString &repair_uuid, bool refresh)
{
    if (repair_uuid.isEmpty()) { return; }
    setSelectedRepairUUID(repair_uuid);
    enableAllTools();
    if (refresh) {
        refreshView();
    }
}

void ViewTabSettings::loadInspector(const QString &inspector_uuid, bool refresh)
{
    if (inspector_uuid < 0) { return; }
    setSelectedInspectorUUID(inspector_uuid);
    enableAllTools();
    if (refresh) {
        setView(View::Inspectors);
    }
}

void ViewTabSettings::loadInspectorReport(const QString &inspector_uuid, bool refresh)
{
    if (inspector_uuid.isEmpty()) { return; }
    setSelectedInspectorUUID(inspector_uuid);
    enableAllTools();
    if (refresh) {
        setView(View::InspectorDetails);
    }
}

void ViewTabSettings::loadAssemblyRecordType(const QString &ar_type_uuid, bool refresh)
{
    if (ar_type_uuid.isEmpty()) { return; }
    setSelectedAssemblyRecordTypeUUID(ar_type_uuid);
    enableAllTools();
    if (refresh) {
        setView(View::AssemblyRecordTypes);
    }
}

void ViewTabSettings::loadAssemblyRecordItemType(const QString &ar_item_type_uuid, bool refresh)
{
    if (ar_item_type_uuid.isEmpty()) { return; }
    setSelectedAssemblyRecordItemTypeUUID(ar_item_type_uuid);
    enableAllTools();
    if (refresh) {
        setView(View::AssemblyRecordItems);
    }
}

void ViewTabSettings::loadAssemblyRecordItemCategory(const QString &ar_item_category_uuid, bool refresh)
{
    if (ar_item_category_uuid.isEmpty()) { return; }
    setSelectedAssemblyRecordItemCategoryUUID(ar_item_category_uuid);
    enableAllTools();
    if (refresh) {
        setView(View::AssemblyRecordItems);
    }
}

void ViewTabSettings::loadAssemblyRecord(const QString &inspection_uuid, bool refresh)
{
    if (!isCustomerSelected()) { return; }
    if (!isCircuitSelected()) { return; }
    if (inspection_uuid.isEmpty()) { return; }
    setSelectedInspectionUUID(inspection_uuid);
    enableAllTools();
    if (refresh) {
        setView(View::AssemblyRecordDetails);
    }
}

void ViewTabSettings::loadCircuitUnitType(const QString &circuit_unit_type_uuid, bool refresh)
{
    if (circuit_unit_type_uuid < 0) { return; }
    setSelectedCircuitUnitTypeUUID(circuit_unit_type_uuid);
    enableAllTools();
    if (refresh) {
        setView(View::CircuitUnitTypes);
    }
}
