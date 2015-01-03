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

#include "assemblyrecorditemsview.h"

#include "global.h"
#include "mttextstream.h"
#include "records.h"
#include "viewtabsettings.h"
#include "mainwindowsettings.h"
#include "toolbarstack.h"
#include "htmlbuilder.h"

using namespace Global;

AssemblyRecordItemsView::AssemblyRecordItemsView(ViewTabSettings *settings):
    View(settings)
{
}

QString AssemblyRecordItemsView::renderHTML()
{
    QString highlighted_category_id = settings->selectedAssemblyRecordItemCategory();
    QString highlighted_type_id = settings->selectedAssemblyRecordItemType();

    QString order_by = settings->mainWindowSettings().orderByForView(LinkParser::AllAssemblyRecordItems);
    order_by = AssemblyRecordItemCategory::attributes().contains(order_by) ? order_by : "name";

    AssemblyRecordItemCategory all_item_categories("");
    ListOfVariantMaps item_categories = all_item_categories.listAll("*", order_by);

    HTMLDiv div;

    if (settings->mainWindowSettings().serviceCompanyInformationVisible()) {
        div << writeServiceCompany();
        div.newLine();
    }

    HTMLTable *table = div.table("cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\" class=\"highlight\"");
    int thead_colspan = 5;
    HTMLTableRow *row = new HTMLTableRow();
    for (int n = 0; n < AssemblyRecordItemCategory::attributes().count(); ++n) {
        *(row->addHeaderCell("rowspan=\"2\"")->link("allassemblyrecorditems:/order_by:"
                                  + AssemblyRecordItemCategory::attributes().key(n)))
                << AssemblyRecordItemCategory::attributes().value(n);
        thead_colspan++;
    }
    *(row->addHeaderCell("colspan=\"5\"")) << tr("Show");
    *(table->addRow()->addHeaderCell("colspan=\"" + QString::number(thead_colspan) + "\" style=\"font-size: medium;\""))
        << tr("List of Assembly Record Item Categories and Types");
    *table << row;
    row = table->addRow();
    *(row->addHeaderCell()) << tr("Value");
    *(row->addHeaderCell()) << tr("Acquisition price");
    *(row->addHeaderCell()) << tr("List price");
    *(row->addHeaderCell()) << tr("Discount");
    *(row->addHeaderCell()) << tr("Total");

    QMap<int, HTMLTableCell *> category_cells;

    for (int i = 0; i < item_categories.count(); ++i) {
        QString category_id = item_categories.at(i).value("id").toString();
        row = table->addRow(QString("id=\"%1\" onclick=\"executeLink(this, '%1');\"%2 style=\"cursor: pointer;\"")
                            .arg("assemblyrecorditemcategory:" + category_id)
                            .arg(highlighted_category_id == category_id ? " class=\"selected\"" : ""));

        *(row->addCell()->link(""))
                << category_id;

        for (int n = 1; n < AssemblyRecordItemCategory::attributes().count(); ++n) {
            *(row->addCell("style=\"font-size: small;\"")->bold())
                    << escapeString(item_categories.at(i).value(AssemblyRecordItemCategory::attributes().key(n)).toString());
        }

        addDisplayOptionsCellToCategoriesTable(row, item_categories.at(i).value("display_options").toInt(), AssemblyRecordItemCategory::ShowValue);
        addDisplayOptionsCellToCategoriesTable(row, item_categories.at(i).value("display_options").toInt(), AssemblyRecordItemCategory::ShowAcquisitionPrice);
        addDisplayOptionsCellToCategoriesTable(row, item_categories.at(i).value("display_options").toInt(), AssemblyRecordItemCategory::ShowListPrice);
        addDisplayOptionsCellToCategoriesTable(row, item_categories.at(i).value("display_options").toInt(), AssemblyRecordItemCategory::ShowDiscount);
        addDisplayOptionsCellToCategoriesTable(row, item_categories.at(i).value("display_options").toInt(), AssemblyRecordItemCategory::ShowTotal);

        category_cells.insert(item_categories.at(i).value("id").toInt(),
                              table->addRow("class=\"no_highlight\"")->addCell(QString("colspan=\"%1\" style=\"padding: 15px;\"")
                                                                               .arg(row->childCount())));
    }

    AssemblyRecordItemType all_item_types("");
    if (!settings->toolBarStack()->isFilterEmpty()) {
        all_item_types.addFilter(settings->toolBarStack()->filterColumn(), settings->toolBarStack()->filterKeyword());
    }

    order_by = settings->mainWindowSettings().orderByForView(LinkParser::AllAssemblyRecordItems);
    order_by = order_by.isEmpty() ? "category_id, name" : QString("category_id, %1").arg(order_by);

    ListOfVariantMaps item_types = all_item_types.listAll("*", order_by);

    for (int i = 0; i < item_types.count();) {
        int category_id = item_types.at(i).value("category_id").toInt();

        if (!category_cells.contains(category_id)) {
            i++;
            continue;
        }

        QString html; MTTextStream out(&html);

        out << "<table cellspacing=\"0\" cellpadding=\"4\" class=\"highlight\">";
        out << "<tr>";
        for (int n = 0; n < AssemblyRecordItemType::attributes().count(); ++n) {
            if (AssemblyRecordItemType::attributes().key(n) == "category_id")
                continue;

            out << "<th><a href=\"allassemblyrecorditems:/order_by:"
                << AssemblyRecordItemType::attributes().key(n) << "\">"
                << AssemblyRecordItemType::attributes().value(n) << "</a></th>";
        }
        out << "</tr>";

        for (; i < item_types.count() && item_types.at(i).value("category_id").toInt() == category_id; ++i) {
            QString type_id = item_types.at(i).value("id").toString();
            out << QString("<tr id=\"%1\" onclick=\"executeLink(this, '%1');\"").arg("assemblyrecorditemtype:" + type_id);
            if (highlighted_type_id == type_id)
                out << " class=\"selected\"";
            out << " style=\"cursor: pointer;\"><td><a href=\"\">" << type_id << "</a></td>";

            for (int n = 1; n < AssemblyRecordItemType::attributes().count(); ++n) {
                if (AssemblyRecordItemType::attributes().key(n) != "category_id")
                    out << "<td>" << escapeString(item_types.at(i).value(AssemblyRecordItemType::attributes().key(n)).toString()) << "</td>";
            }
            out << "</tr>";
        }
        out << "</table>";

        *(category_cells.value(category_id)) << html;
    }

    return viewTemplate("assembly_record_items").arg(div.html());
}

void AssemblyRecordItemsView::addDisplayOptionsCellToCategoriesTable(HTMLTableRow *row, int display_options, int value)
{
    *(row->addCell()) << (display_options & value ? tr("Yes") : tr("No"));
}

QString AssemblyRecordItemsView::title() const
{
    return tr("List of Assembly Record Item Categories and Types");
}
