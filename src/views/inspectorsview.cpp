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

#include "inspectorsview.h"

#include "global.h"
#include "mttextstream.h"
#include "records.h"
#include "viewtabsettings.h"
#include "mainwindowsettings.h"
#include "toolbarstack.h"
#include "htmlbuilder.h"

using namespace Global;

InspectorsView::InspectorsView(ViewTabSettings *settings):
    View(settings)
{
}

QString InspectorsView::renderHTML(bool)
{
    QString highlighted_uuid = settings->selectedInspectorUUID();
    HTMLDiv div;

    writeServiceCompany(div);

    div << writeInspectorsTable(highlighted_uuid);
    return viewTemplate("inspectors").arg(div.html());
}

HTMLTable *InspectorsView::writeInspectorsTable(const QString &highlighted_uuid, const QString &inspector_uuid)
{
    MTQuery inspectors_query = Inspector::query();
    if (!inspector_uuid.isEmpty()) {
        inspectors_query.parents().insert("uuid", inspector_uuid);
    }
    if (!settings->toolBarStack()->isFilterEmpty()) {
        inspectors_query.addFilter(settings->toolBarStack()->filterColumn(), settings->toolBarStack()->filterKeyword());
    }
    ListOfVariantMaps inspectors(inspectors_query.listAll("*,"
       " (SELECT COUNT(date) FROM inspections WHERE inspector_uuid = inspectors.uuid) AS inspections_count,"
       " (SELECT COUNT(date) FROM repairs WHERE inspector_uuid = inspectors.uuid) AS repairs_count"));

    HTMLTable *table = new HTMLTable("cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\"");
    table->addClass("highlight");
    HTMLTableRow *_tr;

    _tr = new HTMLTableRow;
    int thead_colspan = 2;
    for (int n = 0; n < Inspector::attributes().count(); ++n) {
        *(_tr->addHeaderCell()) << Inspector::attributes().value(n);
        thead_colspan++;
    }
    *(_tr->addHeaderCell()) << tr("Number of inspections");
    *(_tr->addHeaderCell()) << tr("Number of repairs");

    *(table->addRow()->addHeaderCell(QString("colspan=\"%1\" style=\"font-size: medium;\"").arg(thead_colspan)))
        << (inspector_uuid.isEmpty() ? tr("Inspectors") : tr("Inspector"));
    *table << _tr;
    for (int i = 0; i < inspectors.count(); ++i) {
        QString uuid = inspectors.at(i).value("uuid").toString();
        QString tr_attr;
        if (inspector_uuid.isEmpty()) {
            tr_attr = QString("id=\"%1\" onclick=\"executeLink(this, '%1');\"").arg("inspector:" + uuid);
            if (highlighted_uuid == uuid)
                tr_attr.append(" class=\"selected\"");
            tr_attr.append(" style=\"cursor: pointer;\"");
        }
        _tr = table->addRow(tr_attr);
        *(_tr->addCell("onmouseover=\"Tip('" + tr("View inspector activity") + "')\" onmouseout=\"UnTip()\"")
                ->link("inspectorreport:" + uuid)) << inspectors.at(i).value("certificate_number").toString();
        for (int n = 1; n < Inspector::attributes().count(); ++n) {
            QString key = Inspector::attributes().key(n);
            *(_tr->addCell()) << MTVariant(inspectors.at(i).value(key).toString(), key);
        }
        *(_tr->addCell()) << inspectors.at(i).value("inspections_count").toString();
        *(_tr->addCell()) << inspectors.at(i).value("repairs_count").toString();
    }

    return table;
}

QString InspectorsView::title() const
{
    return tr("Inspectors");
}
