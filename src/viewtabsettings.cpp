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

#include "viewtabsettings.h"
#include "mainwindowsettings.h"
#include "records.h"

#include <QSettings>

#define MAX_LINKS 10

ViewTabSettings::ViewTabSettings():
    m_customer(-1),
    m_circuit(-1),
    m_compressor(-1),
    m_inspector(-1),
    m_assembly_record_type(-1),
    m_assembly_record_item_type(-1),
    m_assembly_record_item_category(-1),
    m_circuit_unit_type(-1),
    m_last_link(NULL),
    m_received_link(NULL)
{
}

void ViewTabSettings::restoreDefaults()
{
    clearSelectedCustomer();
    clearSelectedInspector();
    clearSelectedAssemblyRecordType();
    clearSelectedAssemblyRecordItemType();
    clearSelectedAssemblyRecordItemCategory();
    clearSelectedCircuitUnitType();

    if (m_last_link) delete m_last_link;
    if (m_received_link) delete m_received_link;
    m_last_link = NULL; m_received_link = NULL;

    for (int i = m_previous_links.count() - 1; i >= 0; --i)
        delete m_previous_links.takeAt(i);

    for (int i = m_next_links.count() - 1; i >= 0; --i)
        delete m_next_links.takeAt(i);

    emitEnableBackAndForwardButtons();
}

void ViewTabSettings::saveSettings(QSettings & settings) const
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

void ViewTabSettings::restoreSettings(QSettings & settings)
{
    m_customer = settings.value("selected_customer", -1).toInt();
    m_circuit = settings.value("selected_circuit", -1).toInt();
    m_compressor = settings.value("selected_compressor", -1).toInt();
    m_inspection = settings.value("selected_inspection").toString();
    m_repair = settings.value("selected_repair").toString();
    m_inspector = settings.value("selected_inspector", -1).toInt();
    m_assembly_record_type = settings.value("selected_assembly_record_type", -1).toInt();
    m_assembly_record_item_type = settings.value("selected_assembly_record_item_type", -1).toInt();
    m_assembly_record_item_category = settings.value("selected_assembly_record_item_category", -1).toInt();
    m_circuit_unit_type = settings.value("selected_circuit_unit_type", -1).toInt();

    enableTools();
    QMetaObject::invokeMethod(object(), "setView", Qt::QueuedConnection,
                              Q_ARG(int, settings.value("current_view").toInt()),
                              Q_ARG(QString, settings.value("current_table").toString()));
}

void ViewTabSettings::loadCustomer(int customer, bool refresh)
{
    if (customer < 0) { return; }
    setSelectedCustomer(customer);
    setSelectedCircuit(-1);
    setSelectedCompressor(-1);
    clearSelectedInspection();
    enableTools();
    if (refresh) {
        setView(View::Circuits);
    }
}

void ViewTabSettings::loadCircuit(int circuit, bool refresh)
{
    if (!isCustomerSelected()) { return; }
    if (circuit < 0) { return; }
    setSelectedCircuit(circuit);
    enableTools();
    if (refresh) {
        setView(View::Inspections);
    }
}

void ViewTabSettings::loadInspection(const QString & inspection, bool refresh)
{
    if (!isCustomerSelected()) { return; }
    if (!isCircuitSelected()) { return; }
    if (inspection.isEmpty()) { return; }
    setSelectedInspection(inspection);
    enableTools();
    if (refresh) {
        setView(View::InspectionDetails);
    }
}

void ViewTabSettings::loadRepair(const QString & date, bool refresh)
{
    if (date.isEmpty()) { return; }
    setSelectedRepair(date);
    enableTools();
    if (refresh) {
        refreshView();
    }
}

void ViewTabSettings::loadInspector(int inspector, bool refresh)
{
    if (inspector < 0) { return; }
    setSelectedInspector(inspector);
    enableTools();
    if (refresh) {
        setView(View::Inspectors);
    }
}

void ViewTabSettings::loadInspectorReport(int inspector, bool refresh)
{
    if (inspector < 0) { return; }
    setSelectedInspector(inspector);
    enableTools();
    if (refresh) {
        setView(View::InspectorDetails);
    }
}

void ViewTabSettings::loadAssemblyRecordType(int assembly_record, bool refresh)
{
    if (assembly_record < 0) { return; }
    setSelectedAssemblyRecordType(assembly_record);
    enableTools();
    if (refresh) {
        setView(View::AssemblyRecordTypes);
    }
}

void ViewTabSettings::loadAssemblyRecordItemType(int assembly_record_item, bool refresh)
{
    if (assembly_record_item < 0) { return; }
    setSelectedAssemblyRecordItemType(assembly_record_item);
    enableTools();
    if (refresh) {
        setView(View::AssemblyRecordItems);
    }
}

void ViewTabSettings::loadAssemblyRecordItemCategory(int assembly_record_item_category, bool refresh)
{
    if (assembly_record_item_category < 0) { return; }
    setSelectedAssemblyRecordItemCategory(assembly_record_item_category);
    enableTools();
    if (refresh) {
        setView(View::AssemblyRecordItems);
    }
}

void ViewTabSettings::loadAssemblyRecord(const QString & inspection, bool refresh)
{
    if (!isCustomerSelected()) { return; }
    if (!isCircuitSelected()) { return; }
    if (inspection.isEmpty()) { return; }
    setSelectedInspection(inspection);
    enableTools();
    if (refresh) {
        setView(View::AssemblyRecordDetails);
    }
}

void ViewTabSettings::loadCircuitUnitType(int circuit_unit_type, bool refresh)
{
    if (circuit_unit_type < 0) { return; }
    setSelectedCircuitUnitType(circuit_unit_type);
    enableTools();
    if (refresh) {
        setView(View::CircuitUnitTypes);
    }
}

void ViewTabSettings::setLastLink(Link * link)
{
    saveToPreviousLinks();

    m_last_link = link;
    m_received_link = NULL;

    while (m_next_links.count())
        delete m_next_links.takeLast();

    emitEnableBackAndForwardButtons();
}

void ViewTabSettings::setReceivedLink(Link * link)
{
     m_received_link = link;
     if (!link->orderBy().isEmpty())
         mainWindowSettings().setOrderByForView(link->views(), link->orderBy());
}

void ViewTabSettings::loadReceivedLink()
{
    updateLastLink();
    emitEnableBackAndForwardButtons();
}

void ViewTabSettings::updateLastLink()
{
    if (m_received_link) {
        saveToPreviousLinks();

        m_last_link = m_received_link;
        m_received_link = NULL;
    }
}

void ViewTabSettings::loadPreviousLink()
{
    updateLastLink();
    if (m_previous_links.count()) {
        saveToNextLinks();
        setReceivedLink(m_previous_links.takeLast());
    }
    emitEnableBackAndForwardButtons();
}

void ViewTabSettings::saveToNextLinks()
{
    if (m_last_link) {
        m_next_links.append(m_last_link);
        if (m_next_links.count() > MAX_LINKS)
            delete m_next_links.takeFirst();
    }
    m_last_link = NULL;
}

void ViewTabSettings::loadNextLink()
{
    updateLastLink();
    if (m_next_links.count()) {
        saveToPreviousLinks();
        setReceivedLink(m_next_links.takeLast());
    }
    emitEnableBackAndForwardButtons();
}

void ViewTabSettings::saveToPreviousLinks()
{
    if (m_last_link) {
        m_previous_links.append(m_last_link);
        if (m_previous_links.count() > MAX_LINKS)
            delete m_previous_links.takeFirst();
    }
    m_last_link = NULL;
}
