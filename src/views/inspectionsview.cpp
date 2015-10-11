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
    QString customer_uuid = settings->selectedCustomerUUID();
    QString circuit_uuid = settings->selectedCircuitUUID();
    bool show_date_updated = settings->isShowDateUpdatedChecked();
    bool show_owner = settings->isShowOwnerChecked();

    QString html; MTTextStream out(&html);

    if (settings->mainWindowSettings().serviceCompanyInformationVisible()) {
        HTMLTable *service_company = writeServiceCompany();
        out << service_company->html();
        delete service_company;
        out << "<br>";
    }

    writeCustomersTable(out, customer_uuid);
    out << "<br>";
    writeCircuitsTable(out, customer_uuid, circuit_uuid, 8);

    if (settings->mainWindowSettings().circuitDetailsVisible()) {
        HTMLTable *compressors_table = circuitCompressorsTable(circuit_uuid);
        if (compressors_table) out << "<br>" << circuitCompressorsTable(circuit_uuid)->html();

        HTMLTable *units_table = circuitUnitsTable(circuit_uuid);
        if (units_table) out << "<br>" << units_table->html();
    }

    Inspection inspection_record({"circuit_uuid", circuit_uuid});
    if (!settings->toolBarStack()->isFilterEmpty()) {
        inspection_record.addFilter(settings->toolBarStack()->filterColumn(), settings->toolBarStack()->filterKeyword());
    }
    QString order_by = settings->mainWindowSettings().orderByForView((LinkParser::Customer << Link::MaxViewBits) | LinkParser::Circuit);
    if (order_by.isEmpty())
        order_by = "date";
    ListOfVariantMaps inspections = inspection_record.listAll("uuid, date, nominal, repair, outside_interval, risks, rmds, arno, inspector_uuid, "
                                                              "person_uuid, refr_add_am, refr_reco, date_updated, updated_by, "
                                                              "inspection_type, inspection_type_data, "
                                                              "(SELECT COUNT(uuid) FROM inspection_images WHERE inspection_uuid = inspections.uuid) AS image_count",
                                                              settings->appendDefaultOrderToColumn(order_by));
    if (year) {
        for (int i = 0; i < inspections.count();) {
            if (inspections.at(i).value("date").toString().split(".").first().toInt() < year) {
                inspections.removeAt(i);
            } else { ++i; }
        }
    }

    MultiMapOfVariantMaps inspectors(Inspector().mapAll("uuid", "person"));
    MultiMapOfVariantMaps persons(Person({"customer_uuid", customer_uuid}).mapAll("uuid", "name"));
    out << "<br><table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\" class=\"highlight\">";
    out << "<tr><th colspan=\"11\" style=\"font-size: medium; background-color: lightgoldenrodyellow;\">";
    out << "<a href=\"customer:" << customer_uuid << "/circuit:" << circuit_uuid << "/table\">";
    out << tr("Inspections and Repairs") << "</a></th></tr>";
    out << "<tr><th><a href=\"customer:" << customer_uuid << "/circuit:" << circuit_uuid << "/order_by:date\">" << tr("Date") << "</a></th>";
    out << "<th><a href=\"customer:" << customer_uuid << "/circuit:" << circuit_uuid << "/order_by:refr_add_am\">" << variableNames().value("refr_add_am") << "</a></th>";
    out << "<th><a href=\"customer:" << customer_uuid << "/circuit:" << circuit_uuid << "/order_by:refr_reco\">" << variableNames().value("refr_reco") << "</a></th>";
    out << "<th><a href=\"customer:" << customer_uuid << "/circuit:" << circuit_uuid << "/order_by:inspector_uuid\">" << variableNames().value("inspector_uuid") << "</a></th>";
    out << "<th><a href=\"customer:" << customer_uuid << "/circuit:" << circuit_uuid << "/order_by:person_uuid\">" << variableNames().value("person_uuid") << "</a></th>";
    out << "<th><a href=\"customer:" << customer_uuid << "/circuit:" << circuit_uuid << "/order_by:risks\">" << variableNames().value("risks") << "</a></th>";
    out << "<th><a href=\"customer:" << customer_uuid << "/circuit:" << circuit_uuid << "/order_by:rmds\">" << variableNames().value("rmds") << "</a></th>";
    out << "<th><a href=\"customer:" << customer_uuid << "/circuit:" << circuit_uuid << "/order_by:arno\">" << variableNames().value("arno") << "</a></th>";
    if (show_date_updated)
        out << "<th><a href=\"customer:" << customer_uuid << "/circuit:" << circuit_uuid << "/order_by:date_updated\">" << tr("Date Updated") << "</a></th>";
    if (show_owner)
        out << "<th><a href=\"customer:" << customer_uuid << "/circuit:" << circuit_uuid << "/order_by:updated_by\">" << tr("Author") << "</a></th>";
    out << "</tr>";

    QString highlighted_uuid = settings->selectedInspectionUUID();
    for (int i = 0; i < inspections.count(); ++i) {
        QString uuid = inspections.at(i).value("uuid").toString();
        QString date = settings->mainWindowSettings().formatDateTime(inspections.at(i).value("date").toString());
        bool is_nominal = inspections.at(i).value("nominal").toInt();
        bool is_repair = inspections.at(i).value("repair").toInt();
        bool is_outside_interval = inspections.at(i).value("outside_interval").toInt();
        Inspection::Type inspection_type = (Inspection::Type)inspections.at(i).value("inspection_type").toInt();

        if (inspection_type != Inspection::DefaultType) {
            QString description = Inspection::descriptionForInspectionType(inspection_type, inspections.at(i).value("inspection_type_data").toString());

            if (!description.isEmpty()) {
                out << QString("<tr><td>%1</td><td colspan=\"10\">%2</td></tr>")
                    .arg(toolTipLink("customer/circuit/inspection", date,
                                     customer_uuid, circuit_uuid, uuid, ToolTipLinkItemRemove))
                    .arg(escapeString(description));
                continue;
            }
        }

        out << QString("<tr id=\"%3\" onclick=\"window.location = 'customer:%1/circuit:%2/%3'\" style=\"cursor: pointer;\"")
               .arg(customer_uuid).arg(circuit_uuid).arg((is_repair ? "repair:" : "inspection:") + uuid);
        if (uuid == highlighted_uuid)
            out << " class=\"selected\"";
        out << "><td>";
        if (is_nominal) { out << "<b>"; }
        else if (is_repair) { out << "<i>"; }
        out << toolTipLink(is_repair ? "customer/circuit/repair" : "customer/circuit/inspection",
                           date, customer_uuid, circuit_uuid, uuid);
        if (is_outside_interval) { out << "*"; }
        if (is_nominal) { out << "</b>"; }
        else if (is_repair) { out << "</i>"; }
        if (inspections.at(i).value("image_count").toInt()) {
            out << "&nbsp;<a href=\"customer:" << customer_uuid << "/circuit:" << circuit_uuid
                << "/" << (is_repair ? "repair:" : "inspection:") << uuid << "/images"
                << "\"><img src=\"data:image/png;base64," << attachmentImage()
                << "\" alt=\"" << tr("Images") << "\" style=\"vertical-align: bottom;\"></a>";
        }
        out << "</td>";
        out << "<td>" << inspections.at(i).value("refr_add_am").toDouble() << "&nbsp;" << QApplication::translate("Units", "kg") << "</td>";
        out << "<td>" << inspections.at(i).value("refr_reco").toDouble() << "&nbsp;" << QApplication::translate("Units", "kg") << "</td>";
        out << "<td>" << escapeString(inspectors.value(inspections.at(i).value("inspector_uuid").toString())
                                      .value("person", inspections.at(i).value("inspector_uuid")).toString()) << "</td>";
        out << "<td>" << escapeString(persons.value(inspections.at(i).value("person_uuid").toString())
                                      .value("name", inspections.at(i).value("person_uuid")).toString()) << "</td>";
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
    out << "</table>";
    return viewTemplate("circuit").arg(html);
}

QString InspectionsView::title() const
{
    Circuit circuit(settings->selectedCircuitUUID());
    circuit.readValues();
    QString title = circuit.circuitName();
    return tr("List of Inspections")
            + " - " + Customer(settings->selectedCustomerUUID()).companyName()
            + " - " + QString(title.isEmpty() ? circuit.circuitID() : title);
}
