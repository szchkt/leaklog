/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2012 Matus & Michal Tomlein

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

#include "main_window_settings.h"
#include "records.h"

#define MAX_LINKS 10

MainWindowSettings::MainWindowSettings():
    QObject(),
    m_customer(-1),
    m_circuit(-1),
    m_compressor(-1),
    m_inspection_is_repair(false),
    m_inspector(-1),
    m_assembly_record_type(-1),
    m_assembly_record_item_type(-1),
    m_assembly_record_item_category(-1),
    m_circuit_unit_type(-1),
    m_has_assembly_record(false),
    m_last_link(NULL),
    m_received_link(NULL)
{
}

void MainWindowSettings::setSelectedCustomer(int customer, const QString & company) {
    clearSelectedCircuit();
    m_customer = customer;
    if (customer >= 0 && company.isNull())
        m_customer_company = Customer(QString::number(customer)).stringValue("company");
    else
        m_customer_company = company;
}

void MainWindowSettings::setSelectedInspector(int inspector, const QString & inspector_name)
{
    m_inspector = inspector;
    if (inspector >= 0 && inspector_name.isNull())
        m_inspector_name = Inspector(QString::number(inspector)).stringValue("person");
    else
        m_inspector_name = inspector_name;
}

void MainWindowSettings::setLastLink(Link * link)
{
    saveToPreviousLinks();

    m_last_link = link;
    m_received_link = NULL;

    while (m_next_links.count())
        delete m_next_links.takeLast();

    enableBackAndForwardButtons();
}

void MainWindowSettings::loadReceivedLink()
{
    updateLastLink();
    enableBackAndForwardButtons();
}

void MainWindowSettings::updateLastLink()
{
    if (m_received_link) {
        saveToPreviousLinks();

        m_last_link = m_received_link;
        m_received_link = NULL;
    }
}

void MainWindowSettings::loadPreviousLink()
{
    updateLastLink();
    if (m_previous_links.count()) {
        saveToNextLinks();
        setReceivedLink(m_previous_links.takeLast());
    }
    enableBackAndForwardButtons();
}

void MainWindowSettings::saveToNextLinks()
{
    if (m_last_link) {
        m_next_links.append(m_last_link);
        if (m_next_links.count() > MAX_LINKS)
            delete m_next_links.takeFirst();
    }
    m_last_link = NULL;
}

void MainWindowSettings::loadNextLink()
{
    updateLastLink();
    if (m_next_links.count()) {
        saveToPreviousLinks();
        setReceivedLink(m_next_links.takeLast());
    }
    enableBackAndForwardButtons();
}

void MainWindowSettings::saveToPreviousLinks()
{
    if (m_last_link) {
        m_previous_links.append(m_last_link);
        if (m_previous_links.count() > MAX_LINKS)
            delete m_previous_links.takeFirst();
    }
    m_last_link = NULL;
}

void MainWindowSettings::enableBackAndForwardButtons()
{
    emit enableBackButton(m_previous_links.count() > 0);
    emit enableForwardButton(m_next_links.count() > 0);
}

void MainWindowSettings::clear()
{
    clearSelectedCustomer();
    setSelectedInspector(-1);
    setSelectedAssemblyRecordType(-1);
    setSelectedAssemblyRecordItemType(-1);
    setSelectedAssemblyRecordItemCategory(-1);
    setSelectedCircuitUnitType(-1);

    if (m_last_link) delete m_last_link;
    if (m_received_link) delete m_received_link;
    m_last_link = NULL; m_received_link = NULL;

    for (int i = m_previous_links.count() - 1; i >= 0; --i)
        delete m_previous_links.takeAt(i);

    for (int i = m_next_links.count() - 1; i >= 0; --i)
        delete m_next_links.takeAt(i);

    enableBackAndForwardButtons();
}
