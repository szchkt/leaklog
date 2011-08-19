/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2011 Matus & Michal Tomlein

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

MainWindowSettings::MainWindowSettings():
    m_customer(-1),
    m_circuit(-1),
    m_compressor(-1),
    m_inspection_is_repair(false),
    m_inspector(-1),
    m_assembly_record_type(-1),
    m_assembly_record_item_type(-1),
    m_assembly_record_item_category(-1),
    m_circuit_unit_type(-1)
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
