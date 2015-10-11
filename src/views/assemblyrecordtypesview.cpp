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

#include "assemblyrecordtypesview.h"

#include "global.h"
#include "mttextstream.h"
#include "records.h"
#include "viewtabsettings.h"
#include "mainwindowsettings.h"
#include "toolbarstack.h"
#include "htmlbuilder.h"

using namespace Global;

AssemblyRecordTypesView::AssemblyRecordTypesView(ViewTabSettings *settings):
    View(settings)
{
}

QString AssemblyRecordTypesView::renderHTML()
{
    QString highlighted_uuid = settings->selectedAssemblyRecordTypeUUID();

    QString html; MTTextStream out(&html);

    if (settings->mainWindowSettings().serviceCompanyInformationVisible()) {
        HTMLTable *service_company = writeServiceCompany();
        out << service_company->html();
        delete service_company;
        out << "<br>";
    }

    AssemblyRecordType all_items("");
    if (!settings->toolBarStack()->isFilterEmpty()) {
        all_items.addFilter(settings->toolBarStack()->filterColumn(), settings->toolBarStack()->filterKeyword());
    }
    ListOfVariantMaps items = all_items.listAll("*", settings->mainWindowSettings().orderByForView(LinkParser::AllAssemblyRecordTypes));

    out << "<table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\" class=\"highlight\">";
    QString thead = "<tr>"; int thead_colspan = 2;
    for (int n = 0; n < AssemblyRecordType::attributes().count(); ++n) {
        thead.append("<th><a href=\"allassemblyrecordtypes:/order_by:"
                     + AssemblyRecordType::attributes().key(n) + "\">"
                     + AssemblyRecordType::attributes().value(n) + "</a></th>");
        thead_colspan++;
    }
    thead.append("</tr>");
    out << "<tr><th colspan=\"" << thead_colspan << "\" style=\"font-size: medium;\">"
        << tr("List of Assembly Record Types") << "</th></tr>";
    out << thead;
    for (int i = 0; i < items.count(); ++i) {
        QString uuid = items.at(i).value("uuid").toString();
        out << QString("<tr id=\"%1\" onclick=\"executeLink(this, '%1');\"").arg("assemblyrecordtype:" + uuid);
        if (highlighted_uuid == uuid) {
            out << " class=\"selected\"";
        }
        out << " style=\"cursor: pointer;\">";
        for (int n = 0; n < AssemblyRecordType::attributes().count(); ++n) {
            out << "<td>" << escapeString(items.at(i).value(AssemblyRecordType::attributes().key(n)).toString()) << "</td>";
        }
        out << "</tr>";
    }
    out << "</table>";
    return viewTemplate("assembly_record_types").arg(html);
}

QString AssemblyRecordTypesView::title() const
{
    return tr("List of Assembly Record Types");
}
