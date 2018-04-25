/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2018 Matus & Michal Tomlein

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

#include "inspectionsview.h"

#include "global.h"
#include "mttextstream.h"
#include "records.h"
#include "viewtabsettings.h"
#include "mainwindowsettings.h"
#include "toolbarstack.h"
#include "htmlbuilder.h"

#include <QBuffer>

using namespace Global;

static QString attachmentImage()
{
    static QString image;
    if (image.isEmpty()) {
        QByteArray bytes;
        QBuffer buffer(&bytes);
        QIcon(":/images/images/attachment16.png").pixmap(16, 16).toImage().save(&buffer, "PNG");
        image = bytes.toBase64();
    }
    return image;
}

InspectionsView::InspectionsView(ViewTabSettings *settings):
    CircuitsView(settings)
{
}

QString InspectionsView::renderHTML()
{
    int year = settings->toolBarStack()->filterSinceValue();
    QString customer_id = settings->selectedCustomer();
    QString circuit_id = settings->selectedCircuit();
    bool show_date_updated = settings->isShowDateUpdatedChecked();
    bool show_owner = settings->isShowOwnerChecked();
    bool most_recent_first = settings->isShowMostRecentFirstChecked();

    QString html; MTTextStream out(&html);

    writeServiceCompany(out);

    writeCustomersTable(out, customer_id);
    out << "<br>";
    writeCircuitsTable(out, customer_id, circuit_id, 8);

    if (settings->mainWindowSettings().circuitDetailsVisible()) {
        HTMLTable *compressors_table = circuitCompressorsTable(customer_id, circuit_id);
        if (compressors_table) out << "<br>" << circuitCompressorsTable(customer_id, circuit_id)->html();

        HTMLTable *units_table = circuitUnitsTable(customer_id, circuit_id);
        if (units_table) out << "<br>" << units_table->html();
    }

    Inspection inspection_record(customer_id, circuit_id, "");
    if (!settings->toolBarStack()->isFilterEmpty()) {
        inspection_record.addFilter(settings->toolBarStack()->filterColumn(), settings->toolBarStack()->filterKeyword());
    }
    QString order_by = settings->mainWindowSettings().orderByForView((LinkParser::Customer << Link::MaxViewBits) | LinkParser::Circuit);
    if (order_by.isEmpty())
        order_by = "date";
    ListOfVariantMaps inspections = inspection_record.listAll("date, nominal, repair, outside_interval, risks, rmds, arno, inspector, "
                                                              "operator, refr_add_am, refr_reco, date_updated, updated_by, "
                                                              "inspection_type, inspection_type_data, "
                                                              "(SELECT COUNT(file_id) FROM inspection_images WHERE customer = inspections.customer AND circuit = inspections.circuit AND date = inspections.date) AS image_count",
                                                              settings->appendDefaultOrderToColumn(order_by));
    if (year) {
        for (int i = 0; i < inspections.count();) {
            if (inspections.at(i).value("date").toString().split(".").first().toInt() < year) {
                inspections.removeAt(i);
            } else { ++i; }
        }
    }

    Inspector inspectors_record("");
    MultiMapOfVariantMaps inspectors(inspectors_record.mapAll("id", "person"));
    Person operators_record(QString(), customer_id);
    MultiMapOfVariantMaps operators(operators_record.mapAll("id", "name"));
    out << "<br><table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\" class=\"highlight\">";
    out << "<tr><th colspan=\"11\" style=\"font-size: medium; background-color: lightgoldenrodyellow;\">";
    out << "<a href=\"customer:" << customer_id << "/circuit:" << circuit_id << "/table\">";
    out << tr("Inspections and Repairs") << "</a></th></tr>";
    out << "<tr><th><a href=\"customer:" << customer_id << "/circuit:" << circuit_id << "/order_by:date\">" << tr("Date") << "</a></th>";
    out << "<th><a href=\"customer:" << customer_id << "/circuit:" << circuit_id << "/order_by:refr_add_am\">" << variableNames().value("refr_add_am") << "</a></th>";
    out << "<th><a href=\"customer:" << customer_id << "/circuit:" << circuit_id << "/order_by:refr_reco\">" << variableNames().value("refr_reco") << "</a></th>";
    out << "<th><a href=\"customer:" << customer_id << "/circuit:" << circuit_id << "/order_by:inspector\">" << variableNames().value("inspector") << "</a></th>";
    out << "<th><a href=\"customer:" << customer_id << "/circuit:" << circuit_id << "/order_by:operator\">" << variableNames().value("operator") << "</a></th>";
    out << "<th><a href=\"customer:" << customer_id << "/circuit:" << circuit_id << "/order_by:risks\">" << variableNames().value("risks") << "</a></th>";
    out << "<th><a href=\"customer:" << customer_id << "/circuit:" << circuit_id << "/order_by:rmds\">" << variableNames().value("rmds") << "</a></th>";
    out << "<th><a href=\"customer:" << customer_id << "/circuit:" << circuit_id << "/order_by:arno\">" << variableNames().value("arno") << "</a></th>";
    if (show_date_updated)
        out << "<th><a href=\"customer:" << customer_id << "/circuit:" << circuit_id << "/order_by:date_updated\">" << tr("Date Updated") << "</a></th>";
    if (show_owner)
        out << "<th><a href=\"customer:" << customer_id << "/circuit:" << circuit_id << "/order_by:updated_by\">" << tr("Author") << "</a></th>";
    out << "</tr>";

    if (most_recent_first)
        writeCircuitDecommissioningReason(out, customer_id, circuit_id);

    QString highlighted_id = settings->selectedInspection();
    for (int i = 0; i < inspections.count(); ++i) {
        QString id = inspections.at(i).value("date").toString();
        bool is_nominal = inspections.at(i).value("nominal").toInt();
        bool is_repair = inspections.at(i).value("repair").toInt() == Inspection::IsRepair;
        bool is_outside_interval = inspections.at(i).value("outside_interval").toInt();
        Inspection::Type inspection_type = (Inspection::Type)inspections.at(i).value("inspection_type").toInt();

        if (inspection_type != Inspection::DefaultType) {
            QString description = Inspection::descriptionForInspectionType(inspection_type, inspections.at(i).value("inspection_type_data").toString());

            if (!description.isEmpty()) {
                out << QString("<tr><td>%1</td><td colspan=\"10\">%2</td></tr>")
                    .arg(toolTipLink("customer/circuit/inspection",
                                     settings->mainWindowSettings().formatDateTime(id),
                                     customer_id, circuit_id, id, ToolTipLinkItemRemove))
                    .arg(escapeString(description));
                continue;
            }
        }

        out << QString("<tr id=\"%3\" onclick=\"window.location = 'customer:%1/circuit:%2/%3'\" style=\"cursor: pointer;\"")
               .arg(customer_id).arg(circuit_id).arg((is_repair ? "repair:" : "inspection:") + id);
        if (id == highlighted_id)
            out << " class=\"selected\"";
        out << "><td>";
        if (is_nominal) { out << "<b>"; }
        else if (is_repair) { out << "<i>"; }
        out << toolTipLink(is_repair ? "customer/circuit/repair" : "customer/circuit/inspection",
                           settings->mainWindowSettings().formatDateTime(id), customer_id, circuit_id, id);
        if (is_outside_interval) { out << "*"; }
        if (is_nominal) { out << "</b>"; }
        else if (is_repair) { out << "</i>"; }
        if (inspections.at(i).value("image_count").toInt()) {
            out << "&nbsp;<a href=\"customer:" << customer_id << "/circuit:" << circuit_id
                << "/" << (is_repair ? "repair:" : "inspection:") << id << "/images"
                << "\"><img src=\"data:image/png;base64," << attachmentImage()
                << "\" alt=\"" << tr("Images") << "\" style=\"vertical-align: bottom;\"></a>";
        }
        out << "</td>";
        out << "<td>" << inspections.at(i).value("refr_add_am").toDouble() << "&nbsp;" << QApplication::translate("Units", "kg") << "</td>";
        out << "<td>" << inspections.at(i).value("refr_reco").toDouble() << "&nbsp;" << QApplication::translate("Units", "kg") << "</td>";
        out << "<td>" << escapeString(inspectors.value(inspections.at(i).value("inspector").toString())
                                      .value("person", inspections.at(i).value("inspector")).toString()) << "</td>";
        out << "<td>" << escapeString(operators.value(inspections.at(i).value("operator").toString())
                                      .value("name", inspections.at(i).value("operator")).toString()) << "</td>";
        if (!inspections.at(i).value("risks").toString().isEmpty()) {
            out << "<td onmouseover=\"Tip('" << escapeString(escapeString(inspections.at(i).value("risks").toString()), true, true);
            out << "')\" onmouseout=\"UnTip()\">" << escapeString(elideRight(inspections.at(i).value("risks").toString(), 50)) << "</td>";
        } else {
            out << "<td></td>";
        }
        if (!inspections.at(i).value("rmds").toString().isEmpty()) {
            out << "<td onmouseover=\"Tip('" << escapeString(escapeString(inspections.at(i).value("rmds").toString()), true, true);
            out << "')\" onmouseout=\"UnTip()\">" << escapeString(elideRight(inspections.at(i).value("rmds").toString(), 50)) << "</td>";
        } else {
            out << "<td></td>";
        }
        out << "<td>" << MTVariant(inspections.at(i).value("arno")) << "</td>";
        if (show_date_updated)
            out << "<td>" << settings->mainWindowSettings().formatDateTime(inspections.at(i).value("date_updated")) << "</td>";
        if (show_owner)
            out << "<td>" << escapeString(inspections.at(i).value("updated_by")) << "</td>";
        out << "</tr>";
    }

    if (!most_recent_first)
        writeCircuitDecommissioningReason(out, customer_id, circuit_id);

    out << "</table>";

    return viewTemplate("circuit").arg(html);
}

void InspectionsView::writeCircuitDecommissioningReason(MTTextStream &out, const QString &customer_id, const QString &circuit_id)
{
    Circuit circuit(customer_id, circuit_id);
    circuit.readValues();
    if (circuit.intValue("disused") != Circuit::Commissioned && !circuit.stringValue("decommissioning_reason").isEmpty()) {
        out << QString("<tr><td>%1</td><td colspan=\"10\">%2</td></tr>")
            .arg(settings->mainWindowSettings().formatDate(circuit.stringValue("decommissioning")))
            .arg(escapeString(circuit.stringValue("decommissioning_reason")));
    }
}

QString InspectionsView::title() const
{
    QString title = Circuit(settings->selectedCustomer(), settings->selectedCircuit()).stringValue("name");
    return tr("Inspections")
            + " - " + Customer(settings->selectedCustomer()).stringValue("company")
            + " - " + QString(title.isEmpty() ? settings->selectedCircuit().rightJustified(5, '0') : title);
}
