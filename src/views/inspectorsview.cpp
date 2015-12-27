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

QString InspectorsView::renderHTML()
{
    QString highlighted_id = settings->selectedInspector();
    HTMLDiv div;

    if (settings->mainWindowSettings().serviceCompanyInformationVisible()) {
        div << writeServiceCompany();
        div.newLine();
    }

    div << writeInspectorsTable(highlighted_id);
    return viewTemplate("inspectors").arg(div.html());
}

HTMLTable *InspectorsView::writeInspectorsTable(const QString &highlighted_id, const QString &inspector_id)
{
    Inspector inspectors_record(inspector_id);
    if (!settings->toolBarStack()->isFilterEmpty()) {
        inspectors_record.addFilter(settings->toolBarStack()->filterColumn(), settings->toolBarStack()->filterKeyword());
    }
    ListOfVariantMaps inspectors(inspectors_record.listAll("*,"
       " (SELECT COUNT(date) FROM inspections WHERE inspector = CAST(inspectors.id AS text)) AS inspections_count,"
       " (SELECT COUNT(date) FROM repairs WHERE repairman = CAST(inspectors.id AS text)) AS repairs_count"));

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
            << (inspector_id.isEmpty() ? tr("Inspectors") : tr("Inspector"));
    *table << _tr;
    for (int i = 0; i < inspectors.count(); ++i) {
        QString id = inspectors.at(i).value("id").toString();
        QString tr_attr;
        if (inspector_id.isEmpty()) {
            tr_attr = QString("id=\"%1\" onclick=\"executeLink(this, '%1');\"").arg("inspector:" + id);
            if (highlighted_id == id)
                tr_attr.append(" class=\"selected\"");
            tr_attr.append(" style=\"cursor: pointer;\"");
        }
        _tr = table->addRow(tr_attr);
        *(_tr->addCell("onmouseover=\"Tip('" + tr("View inspector activity") + "')\" onmouseout=\"UnTip()\"")
                ->link("inspectorreport:" + id)) << id.rightJustified(4, '0');
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
