/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2013 Matus & Michal Tomlein

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

#include "assemblyrecorditemcategoriesview.h"

#include "global.h"
#include "mttextstream.h"
#include "records.h"
#include "viewtabsettings.h"
#include "mainwindowsettings.h"
#include "navigation.h"
#include "htmlbuilder.h"

using namespace Global;

AssemblyRecordItemCategoriesView::AssemblyRecordItemCategoriesView(ViewTabSettings *settings):
    View(settings)
{
}

QString AssemblyRecordItemCategoriesView::renderHTML()
{
    QString highlighted_id = settings->selectedAssemblyRecordItemCategory();

    AssemblyRecordItemCategory all_items("");
    if (!settings->navigation()->isFilterEmpty()) {
        all_items.addFilter(settings->navigation()->filterColumn(), settings->navigation()->filterKeyword());
    }
    ListOfVariantMaps items = all_items.listAll("*", settings->mainWindowSettings().orderByForView(LinkParser::AllAssemblyRecordItemCategories));

    HTMLTable table("cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\" class=\"highlight\"");
    int thead_colspan = 5;
    HTMLTableRow * row = new HTMLTableRow();
    for (int n = 0; n < AssemblyRecordItemCategory::attributes().count(); ++n) {
        *(row->addHeaderCell("rowspan=\"2\"")->link("assemblyrecorditemcategory:/order_by:"
                                  + AssemblyRecordItemCategory::attributes().key(n)))
                << AssemblyRecordItemCategory::attributes().value(n);
        thead_colspan++;
    }
    *(row->addHeaderCell("colspan=\"5\"")) << tr("Show");
    *(table.addRow()->addHeaderCell("colspan=\"" + QString::number(thead_colspan) + "\" style=\"font-size: medium;\""))
        << tr("List of Assembly Record Item Categories");
    table << row;
    row = table.addRow();
    *(row->addHeaderCell()) << tr("Value");
    *(row->addHeaderCell()) << tr("Acquisition price");
    *(row->addHeaderCell()) << tr("List price");
    *(row->addHeaderCell()) << tr("Discount");
    *(row->addHeaderCell()) << tr("Total");

    QString id;
    for (int i = 0; i < items.count(); ++i) {
        id = items.at(i).value("id").toString();
        row = table.addRow(QString("id=\"%1\" onclick=\"executeLink(this, '%1');\"%2 style=\"cursor: pointer;\"")
                           .arg("assemblyrecorditemcategory:" + id)
                           .arg(highlighted_id == id ? " class=\"selected\"" : ""));
        *(row->addCell()->link(""))
                << id;
        for (int n = 1; n < AssemblyRecordItemCategory::attributes().count(); ++n) {
            *(row->addCell())
                    << escapeString(items.at(i).value(AssemblyRecordItemCategory::attributes().key(n)).toString());
        }
        addDisplayOptionsCellToCategoriesTable(row, items.at(i).value("display_options").toInt(), AssemblyRecordItemCategory::ShowValue);
        addDisplayOptionsCellToCategoriesTable(row, items.at(i).value("display_options").toInt(), AssemblyRecordItemCategory::ShowAcquisitionPrice);
        addDisplayOptionsCellToCategoriesTable(row, items.at(i).value("display_options").toInt(), AssemblyRecordItemCategory::ShowListPrice);
        addDisplayOptionsCellToCategoriesTable(row, items.at(i).value("display_options").toInt(), AssemblyRecordItemCategory::ShowDiscount);
        addDisplayOptionsCellToCategoriesTable(row, items.at(i).value("display_options").toInt(), AssemblyRecordItemCategory::ShowTotal);
    }
    return viewTemplate("assembly_record_item_categories").arg(table.html());
}

void AssemblyRecordItemCategoriesView::addDisplayOptionsCellToCategoriesTable(HTMLTableRow * row, int display_options, int value)
{
    *(row->addCell()) << (display_options & value ? tr("Yes") : tr("No"));
}

QString AssemblyRecordItemCategoriesView::title() const
{
    return QApplication::translate("Navigation", "List of Assembly Record Item Categories");
}
