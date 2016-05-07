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

#include "repairsview.h"

#include "global.h"
#include "mttextstream.h"
#include "records.h"
#include "viewtabsettings.h"
#include "mainwindowsettings.h"
#include "toolbarstack.h"
#include "htmlbuilder.h"

using namespace Global;

RepairsView::RepairsView(ViewTabSettings *settings):
    CustomersView(settings)
{
}

QString RepairsView::renderHTML()
{
    QString customer_uuid = settings->selectedCustomerUUID();
    QString highlighted_uuid = settings->selectedRepairUUID();
    int year = settings->toolBarStack()->filterSinceValue();

    bool show_date_updated = settings->isShowDateUpdatedChecked();
    bool show_owner = settings->isShowOwnerChecked();

    QString html; MTTextStream out(&html);

    if (settings->mainWindowSettings().serviceCompanyInformationVisible()) {
        HTMLTable *service_company = writeServiceCompany();
        out << service_company->html();
        delete service_company;
        out << "<br>";
    }

    MTDictionary parent;
    if (!customer_uuid.isEmpty()) {
        parent.insert("customer_uuid", customer_uuid);
        writeCustomersTable(out, customer_uuid);
        out << "<br>";
    }
    MTQuery repairs_record = Repair::query(parent);
    if (!settings->toolBarStack()->isFilterEmpty()) {
        repairs_record.addFilter(settings->toolBarStack()->filterColumn(), settings->toolBarStack()->filterKeyword());
    }
    QString order_by = settings->mainWindowSettings().orderByForView(LinkParser::AllRepairs);
    if (order_by.isEmpty())
        order_by = "date";
    MTSqlQuery repairs = repairs_record.select("*", settings->appendDefaultOrderToColumn(order_by));
    repairs.setForwardOnly(true);
    repairs.exec();
    out << "<table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\" class=\"highlight\">";
    out << "<tr><th colspan=\"14\" style=\"font-size: medium;\">" << tr("Repairs") << "</th></tr><tr>";
    for (int n = 0; n < Repair::attributes().count(); ++n) {
        out << "<th><a href=\"allrepairs:/order_by:" << Repair::attributes().key(n) << "\">" << Repair::attributes().value(n) << "</a></th>";
    }
    if (show_date_updated)
        out << "<th><a href=\"allrepairs:/order_by:date_updated\">" << tr("Date Updated") << "</a></th>";
    if (show_owner)
        out << "<th><a href=\"allrepairs:/order_by:updated_by\">" << tr("Author") << "</a></th>";
    out << "</tr>";
    MultiMapOfVariantMaps inspectors(Inspector::query().mapAll("uuid", "person"));
    while (repairs.next()) {
        QString uuid = repairs.stringValue("uuid");
        QString date = repairs.stringValue("date");
        if (date.split(".").first().toInt() < year) continue;
        out << QString("<tr id=\"%1\" onclick=\"executeLink(this, '%1');\"").arg("repair:" + uuid);
        if (highlighted_uuid == uuid)
            out << " class=\"selected\"";
        out << " style=\"cursor: pointer;\"><td>" << settings->mainWindowSettings().formatDateTime(date) << "</td>";
        for (int n = 1; n < Repair::attributes().count(); ++n) {
            QString attr_value = repairs.stringValue(Repair::attributes().key(n));
            out << "<td>";
            if (Repair::attributes().key(n) == "field") {
                if (attributeValues().contains("field::" + attr_value)) {
                    attr_value = attributeValues().value("field::" + attr_value);
                }
            } else if (Repair::attributes().key(n) == "inspector_uuid") {
                attr_value = inspectors.value(attr_value).value("person", attr_value).toString();
            }
            out << escapeString(attr_value) << "</td>";
        }
        if (show_date_updated)
            out << "<td>" << settings->mainWindowSettings().formatDateTime(repairs.value("date_updated")) << "</th>";
        if (show_owner)
            out << "<td>" << escapeString(repairs.value("updated_by")) << "</th>";
        out << "</tr>";
    }
    out << "</table>";
    return viewTemplate("repairs").arg(html);
}

QString RepairsView::title() const
{
    return tr("Repairs");
}
