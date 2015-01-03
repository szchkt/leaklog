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

#include "assemblyrecordsview.h"

#include "global.h"
#include "mttextstream.h"
#include "records.h"
#include "viewtabsettings.h"
#include "mainwindowsettings.h"
#include "toolbarstack.h"
#include "htmlbuilder.h"

using namespace Global;

AssemblyRecordsView::AssemblyRecordsView(ViewTabSettings *settings):
    CircuitsView(settings)
{
}

QString AssemblyRecordsView::renderHTML()
{
    QString customer_id = settings->selectedCustomer();
    QString circuit_id = settings->selectedCircuit();
    int year = settings->toolBarStack()->filterSinceValue();

    bool show_date_updated = settings->isShowDateUpdatedChecked();
    bool show_owner = settings->isShowOwnerChecked();

    HTMLDiv div;
    HTMLTable *table;
    HTMLTableRow *_tr;
    HTMLTableCell *_td;

    bool customer_given = customer_id.toInt() >= 0, circuit_given = circuit_id.toInt() >= 0;

    MTDictionary inspectors = Global::listInspectors();

    QString html; MTTextStream out(&html);

    if (settings->mainWindowSettings().serviceCompanyInformationVisible()) {
        HTMLTable *service_company = writeServiceCompany();
        out << service_company->html();
        delete service_company;
        out << "<br>";
    }

    if (customer_given) {
        writeCustomersTable(out, customer_id);
        out << "<br>";
    }
    if (circuit_given) {
        writeCircuitsTable(out, customer_id, circuit_id);
        out << "<br>";
    }

    div << html;

    table = new HTMLTable("cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\" class=\"highlight\"");
    _tr = table->addRow();
    _td = _tr->addHeaderCell("colspan=\"9\" style=\"background-color: #DFDFDF; font-size: medium; width:100%; text-align: center;\"");
    *_td << tr("Assembly Records");
    _tr = table->addRow();
    *(_tr->addHeaderCell()->link("allassemblyrecords:/order_by:date")) << tr("Date");
    *(_tr->addHeaderCell()->link("allassemblyrecords:/order_by:arno")) << tr("Assembly record number");
    *(_tr->addHeaderCell()->link("allassemblyrecords:/order_by:record_name")) << tr("Assembly record name");
    if (!customer_given) *(_tr->addHeaderCell()->link("allassemblyrecords:/order_by:customer")) << tr("Customer");
    if (!circuit_given) *(_tr->addHeaderCell()->link("allassemblyrecords:/order_by:circuit")) << tr("Circuit");
    *(_tr->addHeaderCell()->link("allassemblyrecords:/order_by:inspector")) << tr("Inspector");
    *(_tr->addHeaderCell()->link("allassemblyrecords:/order_by:operator")) << tr("Operator");
    if (show_date_updated)
        *(_tr->addHeaderCell()->link("allassemblyrecords:/order_by:inspections.date_updated")) << tr("Date Updated");
    if (show_owner)
        *(_tr->addHeaderCell()->link("allassemblyrecords:/order_by:inspections.updated_by")) << tr("Author");

    MTDictionary parents;
    if (customer_id.toInt() >= 0) parents.insert("customer", customer_id);
    if (circuit_id.toInt() >= 0) parents.insert("circuit", circuit_id);
    MTRecord record("inspections LEFT JOIN assembly_record_types ON inspections.ar_type = assembly_record_types.id"
                    " LEFT JOIN customers ON customers.id = inspections.customer"
                    " LEFT JOIN persons ON inspections.operator = CAST(persons.id AS text)",
                    "inspections.date", "", parents);
    record.setCustomWhere("arno IS NOT NULL AND arno <> '' AND ar_type IS NOT NULL AND ar_type > 0");
    if (!settings->toolBarStack()->isFilterEmpty()) {
        record.addFilter(settings->toolBarStack()->filterColumn(), settings->toolBarStack()->filterKeyword());
    }
    QString order_by = settings->mainWindowSettings().orderByForView(LinkParser::AllAssemblyRecords);
    if (order_by.isEmpty())
        order_by = "date";
    ListOfVariantMaps items = record.listAll("inspections.customer, inspections.circuit,"
                                             " inspections.date, inspections.arno,"
                                             " assembly_record_types.name AS record_name,"
                                             " inspections.inspector, customers.company,"
                                             " persons.name AS operator,"
                                             " inspections.date_updated, inspections.updated_by",
                                             settings->appendDefaultOrderToColumn(order_by));

    for (int i = 0; i < items.count(); ++i) {
        if (year && items.at(i).value("date").toString().split(".").first().toInt() < year) continue;
        _tr = table->addRow(QString("onclick=\"window.location = 'customer:%1/circuit:%2/inspection:%3/assemblyrecord'\" style=\"cursor: pointer;\"")
                            .arg(items.at(i).value("customer").toString())
                            .arg(items.at(i).value("circuit").toString())
                            .arg(items.at(i).value("date").toString()));
        *(_tr->addCell()) << settings->mainWindowSettings().formatDateTime(items.at(i).value("date"));
        *(_tr->addCell()) << escapeString(items.at(i).value("arno"));
        *(_tr->addCell()) << escapeString(items.at(i).value("record_name"));
        if (!customer_given) *(_tr->addCell()) << escapeString(items.at(i).value("company"));
        if (!circuit_given) *(_tr->addCell()) << escapeString(items.at(i).value("circuit").toString().rightJustified(4, '0'));
        *(_tr->addCell()) << escapeString(inspectors.value(items.at(i).value("inspector").toString()));
        *(_tr->addCell()) << escapeString(items.at(i).value("operator"));
        if (show_date_updated)
            *(_tr->addCell()) << settings->mainWindowSettings().formatDateTime(items.at(i).value("date_updated"));
        if (show_owner)
            *(_tr->addCell()) << escapeString(items.at(i).value("updated_by"));
    }
    div << table;

    return viewTemplate("assembly_records").arg(div.html());
}

QString AssemblyRecordsView::title() const
{
    return tr("Assembly Records");
}
