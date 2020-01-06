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

QString AssemblyRecordsView::renderHTML(bool)
{
    QString customer_uuid = settings->selectedCustomerUUID();
    QString circuit_uuid = settings->selectedCircuitUUID();
    int year = settings->toolBarStack()->filterSinceValue();

    bool show_date_updated = settings->isShowDateUpdatedChecked();
    bool show_owner = settings->isShowOwnerChecked();

    HTMLDiv div;
    HTMLTable *table;
    HTMLTableRow *_tr;
    HTMLTableCell *_td;

    bool customer_selected = !customer_uuid.isEmpty();
    bool circuit_selected = !circuit_uuid.isEmpty();

    MTDictionary inspectors = Global::listInspectors();

    QString html; MTTextStream out(&html);

    writeServiceCompany(out);

    if (customer_selected) {
        writeCustomersTable(out, customer_uuid);
        out << "<br>";
    }
    if (circuit_selected) {
        writeCircuitsTable(out, customer_uuid, circuit_uuid);
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
    *(_tr->addHeaderCell()->link("allassemblyrecords:/order_by:inspector_uuid")) << tr("Inspector");
    *(_tr->addHeaderCell()->link("allassemblyrecords:/order_by:person_uuid")) << tr("Contact person");
    if (show_date_updated)
        *(_tr->addHeaderCell()->link("allassemblyrecords:/order_by:inspections.date_updated")) << tr("Date Updated");
    if (show_owner)
        *(_tr->addHeaderCell()->link("allassemblyrecords:/order_by:inspections.updated_by")) << tr("Author");

    QVariantMap parents;
    if (!circuit_uuid.isEmpty()) parents.insert("circuit_uuid", circuit_uuid);
    MTQuery query("inspections LEFT JOIN assembly_record_types ON inspections.ar_type_uuid = assembly_record_types.uuid"
                  " LEFT JOIN customers ON customers.uuid = inspections.customer_uuid"
                  " LEFT JOIN persons ON inspections.person_uuid = persons.uuid",
                  parents);
    query.setPredicate("arno IS NOT NULL AND arno <> '' AND ar_type_uuid IS NOT NULL");
    if (!settings->toolBarStack()->isFilterEmpty()) {
        query.addFilter(settings->toolBarStack()->filterColumn(), settings->toolBarStack()->filterKeyword());
    }
    QString order_by = settings->mainWindowSettings().orderByForView(LinkParser::AllAssemblyRecords);
    if (order_by.isEmpty())
        order_by = "date";
    ListOfVariantMaps items = query.listAll("inspections.customer_uuid, inspections.circuit_uuid,"
                                            " inspections.uuid, inspections.date, inspections.arno,"
                                            " assembly_record_types.name AS record_name,"
                                            " inspections.inspector_uuid, customers.company,"
                                            " persons.name AS operator,"
                                            " inspections.date_updated, inspections.updated_by",
                                            settings->appendDefaultOrderToColumn(order_by));

    foreach (const QVariantMap &item, items) {
        if (year && item.value("date").toString().split(".").first().toInt() < year) continue;
        _tr = table->addRow(QString("onclick=\"window.location = 'customer:%1/circuit:%2/inspection:%3/assemblyrecord'\" style=\"cursor: pointer;\"")
                            .arg(item.value("customer_uuid").toString())
                            .arg(item.value("circuit_uuid").toString())
                            .arg(item.value("uuid").toString()));
        *(_tr->addCell()) << settings->mainWindowSettings().formatDateTime(item.value("date"));
        *(_tr->addCell()) << escapeString(item.value("arno"));
        *(_tr->addCell()) << escapeString(item.value("record_name"));
        *(_tr->addCell()) << escapeString(inspectors.value(item.value("inspector_uuid").toString()));
        *(_tr->addCell()) << escapeString(item.value("operator"));
        if (show_date_updated)
            *(_tr->addCell()) << settings->mainWindowSettings().formatDateTime(item.value("date_updated"));
        if (show_owner)
            *(_tr->addCell()) << escapeString(item.value("updated_by"));
    }
    div << table;

    return viewTemplate("assembly_records").arg(div.html());
}

QString AssemblyRecordsView::title() const
{
    return tr("Assembly Records");
}
