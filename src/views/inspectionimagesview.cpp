/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2017 Matus & Michal Tomlein

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

#include "inspectionimagesview.h"

#include "global.h"
#include "mttextstream.h"
#include "records.h"
#include "viewtabsettings.h"
#include "mainwindowsettings.h"
#include "htmlbuilder.h"
#include "dbfile.h"

#include <QApplication>

using namespace Global;

InspectionImagesView::InspectionImagesView(ViewTabSettings *settings):
    CircuitsView(settings)
{
}

QString InspectionImagesView::renderHTML(bool for_export)
{
    QString customer_uuid = settings->selectedCustomerUUID();
    QString circuit_uuid = settings->selectedCircuitUUID();
    QString inspection_uuid = settings->selectedInspectionUUID();

    QString html; MTTextStream out(&html);

    writeServiceCompany(out);

    writeCustomersTable(out, customer_uuid);
    out << "<br>";
    writeCircuitsTable(out, customer_uuid, circuit_uuid, 8);

    Inspection inspection(inspection_uuid);
    bool nominal = inspection.isNominal();
    bool repair = inspection.isRepair();

    HTMLParentElement *el;
    HTMLDiv div;

    div << html;
    div.newLine();

    HTMLTable *table = div.table("cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\" class=\"no_border\"");
    el = table->addRow()->addHeaderCell("colspan=\"2\" style=\"font-size: medium; background-color: lightgoldenrodyellow;\"")
         ->link("customer:" + customer_uuid + "/circuit:" + circuit_uuid + (repair ? "/repair:" : "/inspection:") + inspection_uuid + "/edit");
    if (nominal) *el << tr("Nominal inspection:");
    else if (repair) *el << tr("Repair:");
    else *el << tr("Inspection:");
    *el << "&nbsp;" << settings->mainWindowSettings().formatDateTime(inspection_uuid);

    ListOfVariantMaps images = inspection.images().listAll("*", "file_uuid");

    for (int i = 0; i < images.count(); ++i) {
        QString uuid = images.at(i).value("file_uuid").toString();
        if (for_export) {
            QByteArray byte_array = DBFile(uuid).data().toBase64();
            if (!byte_array.isNull()) {
                *(table->addRow()->addCell()) << "<img style=\"max-width: 100%;\" src=\"data:image/jpeg;base64," << byte_array << "\">";
            }
        } else {
            *(table->addRow()->addCell()) << "<img style=\"max-width: 100%;\" src=\"dbfile://" << uuid << "\">";
        }
        *(table->addRow()->addCell()) << images.at(i).value("description").toString();
    }

    return viewTemplate("inspection_images").arg(div.html());
}

QString InspectionImagesView::title() const
{
    return tr("Inspection Images");
}
