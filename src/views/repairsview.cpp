/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2025 Matus & Michal Tomlein

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

QString RepairsView::renderHTML(bool)
{
    QString service_company_uuid = settings->filterServiceCompanyUUID();
    MTDictionary service_companies = listServiceCompanies();
    bool show_service_company = service_companies.count() > 1;
    QString customer_uuid = settings->selectedCustomerUUID();
    QString highlighted_uuid = settings->selectedRepairUUID();
    int year = settings->toolBarStack()->filterSinceValue();

    bool show_date_updated = settings->isShowDateUpdatedChecked();
    bool show_owner = settings->isShowOwnerChecked();

    QString html; MTTextStream out(&html);

    writeServiceCompany(out);

    QVariantMap parent;
    if (!service_company_uuid.isEmpty()) {
        parent.insert("service_company_uuid", service_company_uuid);
    }
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
    repairs.exec();
    out << "<table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\" class=\"highlight\">";
    out << "<tr><th colspan=\"15\" style=\"font-size: medium;\">" << tr("Repairs") << "</th></tr><tr>";
    auto th = [&out](const QString &key, const QString &title) {
        out << "<th rowspan=\"2\"><a href=\"allrepairs:/order_by:" << key << "\">" << title << "</a></th>";
    };
    th("date", QApplication::translate("Repair", "Date"));
    if (show_service_company)
        th("service_company_uuid", QApplication::translate("Repair", "Service company"));
    th("customer", QApplication::translate("Repair", "Customer"));
    th("device", QApplication::translate("Repair", "Device"));
    th("field", QApplication::translate("Repair", "Field of application"));
    th("refrigerant", QApplication::translate("Repair", "Refrigerant"));
    th("refrigerant_amount", QApplication::translate("Repair", "Refrigerant amount"));
    out << "<th colspan=\"3\">" << QApplication::translate("Repair", "Refrigerant addition") << "</th>";
    th("refr_reco", QApplication::translate("Repair", "Refrigerant recovery"));
    th("inspector_uuid", QApplication::translate("Repair", "Inspector"));
    th("arno", QApplication::translate("Repair", "Assembly record No."));
    if (show_date_updated)
        th("date_updated", tr("Date Updated"));
    if (show_owner)
        th("updated_by", tr("Author"));
    out << "</tr>";
    out << "<tr>";
    out << "<th><a href=\"allrepairs:/order_by:refr_add_am\">" << QApplication::translate("Repair", "New") << "</a></th>";
    out << "<th><a href=\"allrepairs:/order_by:refr_add_am_recy\">" << QApplication::translate("Repair", "Recycled") << "</a></th>";
    out << "<th><a href=\"allrepairs:/order_by:refr_add_am_rege\">" << QApplication::translate("Repair", "Reclaimed") << "</a></th>";
    out << "</tr>";
    MultiMapOfVariantMaps inspectors(Inspector::query().mapAll("uuid", "person"));
    while (repairs.next()) {
        QString uuid = repairs.stringValue("uuid");
        QString date = repairs.stringValue("date");
        Repair::Type repair_type = (Repair::Type)repairs.intValue("repair_type");
        if (date.split(".").first().toInt() < year) continue;
        out << QString("<tr id=\"%1\" onclick=\"executeLink(this, '%1');\"").arg("repair:" + uuid);
        if (highlighted_uuid == uuid)
            out << " class=\"selected\"";
        out << " style=\"cursor: pointer;\"><td>";
        if (repair_type == Repair::NominalRepair) { out << "<b>"; }
        out << settings->mainWindowSettings().formatDateTime(date);
        if (repair_type == Repair::NominalRepair) { out << "</b>"; }
        out << "</td>";
        if (show_service_company)
            out << "<td>" << escapeString(service_companies.value(repairs.stringValue("service_company_uuid"))) << "</td>";
        out << "<td>" << escapeString(repairs.stringValue("customer")) << "</td>";
        out << "<td>" << escapeString(repairs.stringValue("device")) << "</td>";
        QString field = repairs.stringValue("field");
        out << "<td>" << escapeString(attributeValues().value("field::" + field, field)) << "</td>";
        out << "<td>" << escapeString(repairs.stringValue("refrigerant")) << "</td>";
        out << "<td>" << repairs.doubleValue("refrigerant_amount") << "</td>";
        out << "<td>" << repairs.doubleValue("refr_add_am") << "</td>";
        out << "<td>" << repairs.doubleValue("refr_add_am_recy") << "</td>";
        out << "<td>" << repairs.doubleValue("refr_add_am_rege") << "</td>";
        out << "<td>" << repairs.doubleValue("refr_reco") << "</td>";
        out << "<td>" << escapeString(inspectors.value(repairs.stringValue("inspector_uuid")).value("person").toString()) << "</td>";
        out << "<td>" << escapeString(repairs.stringValue("arno")) << "</td>";
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
