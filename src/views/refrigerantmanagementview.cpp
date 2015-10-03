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

#include "refrigerantmanagementview.h"

#include "global.h"
#include "mttextstream.h"
#include "records.h"
#include "viewtabsettings.h"
#include "mainwindowsettings.h"
#include "toolbarstack.h"
#include "htmlbuilder.h"

using namespace Global;

RefrigerantManagementView::RefrigerantManagementView(ViewTabSettings *settings):
    View(settings)
{
}

QString RefrigerantManagementView::renderHTML()
{
    int since = settings->toolBarStack()->filterSinceValue();
    bool show_date_updated = settings->isShowDateUpdatedChecked();
    bool show_owner = settings->isShowOwnerChecked();

    QString html; MTTextStream out(&html);

    if (settings->mainWindowSettings().serviceCompanyInformationVisible()) {
        HTMLTable *service_company = writeServiceCompany();
        out << service_company->html();
        delete service_company;
        out << "<br>";
    }

    out << "<table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\" class=\"highlight\">";
    out << "<tr><th colspan=\"14\" style=\"font-size: medium;\">";
    out << tr("Refrigerant Management") << "</th></tr>";
    out << "<tr><th rowspan=\"2\"><a href=\"refrigerantmanagement:/order_by:date\">" << tr("Date") << "</a></th>";
    out << "<th colspan=\"2\">" << QApplication::translate("RefrigerantRecord", "Business partner") << "</th>";
    out << "<th rowspan=\"2\">" << tr("Refrigerant") << "</th>";
    out << "<th colspan=\"2\">" << tr("Purchased") << "</th>";
    out << "<th colspan=\"2\">" << tr("Sold") << "</th>";
    out << "<th rowspan=\"2\">" << tr("Reclaimed") << "</th>";
    out << "<th rowspan=\"2\">" << tr("Disposed of") << "</th>";
    out << "<th colspan=\"2\">" << tr("Leaked in store") << "</th>";
    if (show_date_updated)
        out << "<th rowspan=\"2\"><a href=\"refrigerantmanagement:/order_by:date_updated\">" << tr("Date Updated") << "</a></th>";
    if (show_owner)
        out << "<th rowspan=\"2\"><a href=\"refrigerantmanagement:/order_by:updated_by\">" << tr("Author") << "</a></th>";
    out << "</tr><tr>";
    out << "<th>" << QApplication::translate("Customer", "Company") << "</th>";
    out << "<th>" << QApplication::translate("Customer", "ID") << "</th>";
    out << "<th>" << QApplication::translate("VariableNames", "New") << "</th>";
    out << "<th>" << QApplication::translate("VariableNames", "Recovered") << "</th>";
    out << "<th>" << QApplication::translate("VariableNames", "New") << "</th>";
    out << "<th>" << QApplication::translate("VariableNames", "Recovered") << "</th>";
    out << "<th>" << QApplication::translate("VariableNames", "New") << "</th>";
    out << "<th>" << QApplication::translate("VariableNames", "Recovered") << "</th>";
    out << "</tr>";
    RefrigerantRecord records("");
    if (!settings->toolBarStack()->isFilterEmpty()) {
        records.addFilter(settings->toolBarStack()->filterColumn(), settings->toolBarStack()->filterKeyword());
    }
    QString order_by = settings->mainWindowSettings().orderByForView(LinkParser::RefrigerantManagement);
    if (order_by.isEmpty())
        order_by = "date";
    MTSqlQuery query = records.select("*", settings->appendDefaultOrderToColumn(order_by));
    query.setForwardOnly(true);
    query.exec();
    QString date;
    while (query.next()) {
        date = query.stringValue("date");
        if (since && date.left(4).toInt() < since) continue;
        out << "<tr onclick=\"window.location = 'refrigerantrecord:" << date << "/edit'\" style=\"cursor: pointer;\">";
        out << "<td>" << settings->mainWindowSettings().formatDateTime(date) << "</td>";
        for (int n = 1; n < RefrigerantRecord::attributes().count(); ++n) {
            QString key = RefrigerantRecord::attributes().key(n);
            if (key == "partner_id") {
                out << "<td>" << formatCompanyID(query.value(key)) << "</td>";
            } else {
                out << "<td>" << MTVariant(query.value(key)) << "</td>";
            }
        }
        if (show_date_updated)
            out << "<td>" << settings->mainWindowSettings().formatDateTime(query.value("date_updated")) << "</th>";
        if (show_owner)
            out << "<td>" << escapeString(query.value("updated_by")) << "</th>";
        out << "</tr>";
    }
    out << "</table>";
    return viewTemplate("refrigerant_management").arg(html);
}

QString RefrigerantManagementView::title() const
{
    return tr("Refrigerant Management");
}
