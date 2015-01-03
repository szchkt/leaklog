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

#include "viewtabsettings.h"
#include "mainwindowsettings.h"
#include "records.h"

#include <QSettings>

ViewTabSettings::ViewTabSettings():
    m_customer(-1),
    m_circuit(-1),
    m_compressor(-1),
    m_inspector(-1),
    m_assembly_record_type(-1),
    m_assembly_record_item_type(-1),
    m_assembly_record_item_category(-1),
    m_circuit_unit_type(-1)
{
}

void ViewTabSettings::saveSettings(QSettings &settings) const
{
    if (isCustomerSelected())
        settings.setValue("selected_customer", m_customer);
    if (isCircuitSelected())
        settings.setValue("selected_circuit", m_circuit);
    if (isCompressorSelected())
        settings.setValue("selected_compressor", m_compressor);
    if (isInspectionSelected())
        settings.setValue("selected_inspection", m_inspection);
    if (isRepairSelected())
        settings.setValue("selected_repair", m_repair);
    if (isInspectorSelected())
        settings.setValue("selected_inspector", m_inspector);
    if (isAssemblyRecordTypeSelected())
        settings.setValue("selected_assembly_record_type", m_assembly_record_type);
    if (isAssemblyRecordItemTypeSelected())
        settings.setValue("selected_assembly_record_item_type", m_assembly_record_item_type);
    if (isAssemblyRecordItemCategorySelected())
        settings.setValue("selected_assembly_record_item_category", m_assembly_record_item_category);
    if (isCircuitUnitTypeSelected())
        settings.setValue("selected_circuit_unit_type", m_circuit_unit_type);

    settings.setValue("current_view", currentView());
    if (!currentTable().isEmpty())
        settings.setValue("current_table", currentTable());
}

void ViewTabSettings::restoreSettings(QSettings &settings)
{
    m_customer = settings.value("selected_customer", -1).toInt();
    m_circuit = settings.value("selected_circuit", -1).toInt();
    m_inspection = settings.value("selected_inspection").toString();
    m_compressor = settings.value("selected_compressor", -1).toInt();
    m_repair = settings.value("selected_repair").toString();
    m_inspector = settings.value("selected_inspector", -1).toInt();
    m_assembly_record_type = settings.value("selected_assembly_record_type", -1).toInt();
    m_assembly_record_item_type = settings.value("selected_assembly_record_item_type", -1).toInt();
    m_assembly_record_item_category = settings.value("selected_assembly_record_item_category", -1).toInt();
    m_circuit_unit_type = settings.value("selected_circuit_unit_type", -1).toInt();

    validateSelection();

    QMetaObject::invokeMethod(object(), "setView", Qt::QueuedConnection,
                              Q_ARG(int, settings.value("current_view").toInt()),
                              Q_ARG(QString, settings.value("current_table").toString()));
}

void ViewTabSettings::validateSelection()
{
    if (isCustomerSelected() && !Customer(selectedCustomer()).exists()) {
        clearSelectedCustomer();
    }
    if (isCircuitSelected() && !Circuit(selectedCustomer(), selectedCircuit()).exists()) {
        clearSelectedCircuit();
    }
    if (isInspectionSelected() && !Inspection(selectedCustomer(), selectedCircuit(), selectedInspection()).exists()) {
        clearSelectedInspection();
    }
    if (isCompressorSelected() && !Compressor(selectedCompressor()).exists()) {
        clearSelectedCompressor();
    }
    if (isRepairSelected() && !Repair(selectedRepair()).exists()) {
        clearSelectedRepair();
    }
    if (isInspectorSelected() && !Inspector(selectedInspector()).exists()) {
        clearSelectedInspector();
    }
    if (isAssemblyRecordTypeSelected() && !AssemblyRecordType(selectedAssemblyRecordType()).exists()) {
        clearSelectedAssemblyRecordType();
    }
    if (isAssemblyRecordItemTypeSelected() && !AssemblyRecordItemType(selectedAssemblyRecordItemType()).exists()) {
        clearSelectedAssemblyRecordItemType();
    }
    if (isAssemblyRecordItemCategorySelected() && !AssemblyRecordItemCategory(selectedAssemblyRecordItemCategory()).exists()) {
        clearSelectedAssemblyRecordItemCategory();
    }
    if (isCircuitUnitTypeSelected() && !CircuitUnitType(selectedCircuitUnitType()).exists()) {
        clearSelectedCircuitUnitType();
    }
}

void ViewTabSettings::loadCustomer(int customer, bool refresh)
{
    if (customer < 0) { return; }
    setSelectedCustomer(customer);
    setSelectedCircuit(-1);
    setSelectedCompressor(-1);
    clearSelectedInspection();
    enableAllTools();
    if (refresh) {
        setView(View::Circuits);
    }
}

void ViewTabSettings::loadCircuit(int circuit, bool refresh)
{
    if (!isCustomerSelected()) { return; }
    if (circuit < 0) { return; }
    setSelectedCircuit(circuit);
    enableAllTools();
    if (refresh) {
        setView(View::Inspections);
    }
}

void ViewTabSettings::loadInspection(const QString &inspection, bool refresh)
{
    if (!isCustomerSelected()) { return; }
    if (!isCircuitSelected()) { return; }
    if (inspection.isEmpty()) { return; }
    if (Inspection(selectedCustomer(), selectedCircuit(), inspection).intValue("inspection_type") != Inspection::DefaultType) {
        clearSelectedInspection();
        enableAllTools();
        if (refresh) {
            setView(View::Inspections);
        }
        return;
    }
    setSelectedInspection(inspection);
    enableAllTools();
    if (refresh) {
        setView(View::InspectionDetails);
    }
}

void ViewTabSettings::loadRepair(const QString &date, bool refresh)
{
    if (date.isEmpty()) { return; }
    setSelectedRepair(date);
    enableAllTools();
    if (refresh) {
        refreshView();
    }
}

void ViewTabSettings::loadInspector(int inspector, bool refresh)
{
    if (inspector < 0) { return; }
    setSelectedInspector(inspector);
    enableAllTools();
    if (refresh) {
        setView(View::Inspectors);
    }
}

void ViewTabSettings::loadInspectorReport(int inspector, bool refresh)
{
    if (inspector < 0) { return; }
    setSelectedInspector(inspector);
    enableAllTools();
    if (refresh) {
        setView(View::InspectorDetails);
    }
}

void ViewTabSettings::loadAssemblyRecordType(int assembly_record, bool refresh)
{
    if (assembly_record < 0) { return; }
    setSelectedAssemblyRecordType(assembly_record);
    enableAllTools();
    if (refresh) {
        setView(View::AssemblyRecordTypes);
    }
}

void ViewTabSettings::loadAssemblyRecordItemType(int assembly_record_item, bool refresh)
{
    if (assembly_record_item < 0) { return; }
    setSelectedAssemblyRecordItemType(assembly_record_item);
    enableAllTools();
    if (refresh) {
        setView(View::AssemblyRecordItems);
    }
}

void ViewTabSettings::loadAssemblyRecordItemCategory(int assembly_record_item_category, bool refresh)
{
    if (assembly_record_item_category < 0) { return; }
    setSelectedAssemblyRecordItemCategory(assembly_record_item_category);
    enableAllTools();
    if (refresh) {
        setView(View::AssemblyRecordItems);
    }
}

void ViewTabSettings::loadAssemblyRecord(const QString &inspection, bool refresh)
{
    if (!isCustomerSelected()) { return; }
    if (!isCircuitSelected()) { return; }
    if (inspection.isEmpty()) { return; }
    setSelectedInspection(inspection);
    enableAllTools();
    if (refresh) {
        setView(View::AssemblyRecordDetails);
    }
}

void ViewTabSettings::loadCircuitUnitType(int circuit_unit_type, bool refresh)
{
    if (circuit_unit_type < 0) { return; }
    setSelectedCircuitUnitType(circuit_unit_type);
    enableAllTools();
    if (refresh) {
        setView(View::CircuitUnitTypes);
    }
}
