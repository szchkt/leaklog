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

#include "inspectordetailsview.h"

#include "global.h"
#include "mttextstream.h"
#include "records.h"
#include "viewtabsettings.h"
#include "mainwindowsettings.h"
#include "toolbarstack.h"
#include "htmlbuilder.h"

using namespace Global;

InspectorDetailsView::InspectorDetailsView(ViewTabSettings *settings):
    InspectorsView(settings)
{
}

QString InspectorDetailsView::renderHTML()
{
    QString inspector_uuid = settings->selectedInspectorUUID();

    HTMLDiv div;

    if (settings->mainWindowSettings().serviceCompanyInformationVisible()) {
        div << writeServiceCompany();
        div.newLine();
    }

    div << writeInspectorsTable(QString(), inspector_uuid);
    div.newLine();

    HTMLTable *table;
    HTMLTableRow *_tr;
    HTMLTableCell *_td;
    HTMLParentElement *elem;

    bool show_acquisition_price = DBInfo::isOperationPermitted("access_assembly_record_acquisition_price") > 0;
    bool show_list_price = DBInfo::isOperationPermitted("access_assembly_record_list_price") > 0;

    double absolute_total = 0.0, total = 0.0, acquisition_total = 0.0;

    MTQuery ar_item_record = AssemblyRecordItem::queryByInspector(inspector_uuid);
    if (!settings->toolBarStack()->isFilterEmpty()) {
        ar_item_record.addFilter(settings->toolBarStack()->filterColumn(), settings->toolBarStack()->filterKeyword());
    }
    ListOfVariantMaps ar_items(ar_item_record.listAll("inspections.customer_uuid, inspections.circuit_uuid, inspections.uuid AS inspection_uuid, inspections.date, customers.id AS customer_id, circuits.id AS circuit_id, assembly_record_items.*"));

    table = div.table("cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\" class=\"highlight\"");
    *(table->addRow()->addHeaderCell("colspan=\"10\" style=\"font-size: medium;\"")) << tr("Assembly Records");
    _tr = table->addRow();
    *(_tr->addHeaderCell()) << tr("Date");
    *(_tr->addHeaderCell()) << tr("Customer ID");
    *(_tr->addHeaderCell()) << tr("Circuit ID");
    *(_tr->addHeaderCell()) << tr("Assembly record");
    *(_tr->addHeaderCell()) << tr("Value");
    if (show_acquisition_price)
        *(_tr->addHeaderCell()) << tr("Acquisition price");
    if (show_list_price) {
        *(_tr->addHeaderCell()) << tr("List price");
        *(_tr->addHeaderCell()) << tr("Discount");
        *(_tr->addHeaderCell()) << tr("Total");
    }

    for (int i = 0; i < ar_items.count(); ++i) {
        QString date = settings->mainWindowSettings().formatDateTime(ar_items.at(i).value("date").toString());
        QString uuid = ar_items.at(i).value("inspection_uuid").toString();
        QString customer_uuid = ar_items.at(i).value("customer_uuid").toString();
        QString circuit_uuid = ar_items.at(i).value("circuit_uuid").toString();
        bool is_nominal = ar_items.at(i).value("nominal").toInt();
        bool is_repair = ar_items.at(i).value("repair").toInt();
        bool is_outside_interval = ar_items.at(i).value("outside_interval").toInt();

        _tr = table->addRow(QString("onclick=\"window.location = 'customer:%1/circuit:%2/%3:%4/assemblyrecord'\" style=\"cursor: pointer;\"")
                            .arg(customer_uuid)
                            .arg(circuit_uuid)
                            .arg(is_repair ? "repair" : "inspection")
                            .arg(uuid));
        _td = _tr->addCell();

        if (is_nominal) elem = _td->bold();
        else if (is_repair) elem = _td->italics();
        else elem = _td;
        *elem << toolTipLink(is_repair ? "customer/circuit/repair" : "customer/circuit/inspection", date, customer_uuid, circuit_uuid, uuid);
        if (is_outside_interval) { *elem << "*"; }

        *(_tr->addCell()) << ar_items.at(i).value("customer_id").toString();
        *(_tr->addCell()) << ar_items.at(i).value("circuit_id").toString().rightJustified(5, '0');
        *(_tr->addCell()) << ar_items.at(i).value("arno").toString();
        *(_tr->addCell()) << ar_items.at(i).value("value").toString();
        if (show_acquisition_price) {
            acquisition_total += ar_items.at(i).value("value").toDouble() * ar_items.at(i).value("acquisition_price").toDouble();

            *(_tr->addCell()) << QString::number(ar_items.at(i).value("acquisition_price").toDouble());
        }
        if (show_list_price) {
            total = ar_items.at(i).value("value").toDouble();
            total *= ar_items.at(i).value("list_price").toDouble();
            double total_discount = ar_items.at(i).value("discount").toDouble();
            total *= 1 - total_discount / 100;
            absolute_total += total;

            *(_tr->addCell()) << QString::number(ar_items.at(i).value("list_price").toDouble());
            *(_tr->addCell()) << QString::number(total_discount) << " %";
            *(_tr->addCell()) << QString::number(total);
        }
    }
    if (show_list_price || show_acquisition_price) {
        _tr = table->addRow();
        *(_tr->addHeaderCell("colspan=\"5\"")) << tr("Total");
        _td = _tr->addCell();
        if (show_acquisition_price) *_td << QString::number(acquisition_total);
        _td = _tr->addCell("colspan=\"3\"");
        if (show_list_price) *_td << QString::number(absolute_total);
    }
    div.newLine();

    table = div.table("cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\" class=\"highlight\"");
    *(table->addRow()->addHeaderCell("colspan=\"5\" style=\"font-size: medium;\"")) << tr("Inspections and Repairs");
    _tr = table->addRow();
    *(_tr->addHeaderCell()) << tr("Date");
    *(_tr->addHeaderCell()) << tr("Customer ID");
    *(_tr->addHeaderCell()) << tr("Customer");
    *(_tr->addHeaderCell()) << tr("Circuit ID");
    *(_tr->addHeaderCell()) << tr("Circuit name");

    MTQuery inspections_query = Inspection::queryByInspector(inspector_uuid);
    if (!settings->toolBarStack()->isFilterEmpty()) {
        inspections_query.addFilter(settings->toolBarStack()->filterColumn(), settings->toolBarStack()->filterKeyword());
    }
    ListOfVariantMaps inspections(inspections_query.listAll("inspections.uuid, date, inspections.customer_uuid, customers.id AS customer_id, customers.company, circuit_uuid, circuits.id AS circuit_id, circuits.name AS circuit_name, repair, nominal"));

    for (int i = 0; i < inspections.count(); ++i) {
        QString date = settings->mainWindowSettings().formatDateTime(inspections.at(i).value("date").toString());
        QString uuid = inspections.at(i).value("uuid").toString();
        QString customer_uuid = inspections.at(i).value("customer_uuid").toString();
        QString circuit_uuid = inspections.at(i).value("circuit_uuid").toString();
        bool is_nominal = inspections.at(i).value("nominal").toInt();
        bool is_repair = inspections.at(i).value("repair").toInt();
        bool is_outside_interval = inspections.at(i).value("outside_interval").toInt();

        QString inspection_link = "onclick=\"window.location = 'customer:" + customer_uuid + "/circuit:" + circuit_uuid;
        inspection_link.append((is_repair ? "/repair:" : "/inspection:") + uuid + "'\" style=\"cursor: pointer;\"");

        _tr = table->addRow(inspection_link);
        _td = _tr->addCell();

        if (is_nominal) elem = _td->bold();
        else if (is_repair) elem = _td->italics();
        else elem = _td;
        *elem << toolTipLink(is_repair ? "customer/circuit/repair" : "customer/circuit/inspection", date, customer_uuid, circuit_uuid, uuid);
        if (is_outside_interval) { *elem << "*"; }

        *(_tr->addCell()) << inspections.at(i).value("customer_id").toString();
        *(_tr->addCell()) << inspections.at(i).value("company").toString();
        *(_tr->addCell()) << inspections.at(i).value("circuit_id").toString().rightJustified(5, '0');
        *(_tr->addCell()) << inspections.at(i).value("circuit_name").toString();
    }

    return viewTemplate("inspector").arg(div.html());
}

QString InspectorDetailsView::title() const
{
    return tr("Inspector");
}
