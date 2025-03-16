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

#include "circuitunittypesview.h"

#include "global.h"
#include "mttextstream.h"
#include "records.h"
#include "viewtabsettings.h"
#include "mainwindowsettings.h"
#include "toolbarstack.h"
#include "htmlbuilder.h"

using namespace Global;

CircuitUnitTypesView::CircuitUnitTypesView(ViewTabSettings *settings):
    View(settings)
{
}

QString CircuitUnitTypesView::renderHTML(bool)
{
    QString highlighted_uuid = settings->selectedCircuitUnitTypeUUID();

    QString html; MTTextStream out(&html);

    writeServiceCompany(out);

    MTQuery all_items = CircuitUnitType::query();
    if (!settings->toolBarStack()->isFilterEmpty()) {
        all_items.addFilter(settings->toolBarStack()->filterColumn(), settings->toolBarStack()->filterKeyword());
    }
    ListOfVariantMaps items = all_items.listAll("*", settings->mainWindowSettings().orderByForView(LinkParser::AllCircuitUnitTypes));

    out << "<table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\" class=\"highlight\">";
    QString thead = "<tr>"; int thead_colspan = 2;
    for (int n = 1; n < CircuitUnitType::attributes().count(); ++n) {
        thead.append("<th><a href=\"allcircuitunittypes:/order_by:"
                     + CircuitUnitType::attributes().key(n) + "\">"
                     + CircuitUnitType::attributes().value(n) + "</a></th>");
        thead_colspan++;
    }
    thead.append("</tr>");
    out << "<tr><th colspan=\"" << thead_colspan << "\" style=\"font-size: medium;\">" << tr("Circuit Unit Types") << "</th></tr>";
    out << thead;

    for (int i = 0; i < items.count(); ++i) {
        QString uuid = items.at(i).value("uuid").toString();
        out << QString("<tr id=\"%1\" onclick=\"executeLink(this, '%1');\"").arg("circuitunittype:" + uuid);
        if (highlighted_uuid == uuid)
            out << " class=\"selected\"";
        out << " style=\"cursor: pointer;\">";
        for (int n = 1; n < CircuitUnitType::attributes().count(); ++n) {
            out << "<td>";
            QString key = CircuitUnitType::attributes().key(n);
            if (key == "location")
                out << CircuitUnitType::locationToString(items.at(i).value("location").toInt());
            else if (key == "oil")
                out << items.at(i).value(key).toString().toUpper();
            else if (key == "output")
                out << escapeString(QString("%L1 %2").arg(FLOAT_ARG(items.at(i).value("output").toDouble())).arg(items.at(i).value("output_unit").toString()));
            else if (key.endsWith("_price") || key.endsWith("_amount") || key == "discount" || key == "output_t0_tc")
                out << items.at(i).value(key).toDouble();
            else
                out << escapeString(items.at(i).value(key).toString());
            out << "</td>";
        }
        out << "</tr>";
    }
    out << "</table>";
    return viewTemplate("circuit_unit_types").arg(html);
}

QString CircuitUnitTypesView::title() const
{
    return tr("Circuit Unit Types");
}
