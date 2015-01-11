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

QString InspectionImagesView::renderHTML()
{
    QString customer_id = settings->selectedCustomer();
    QString circuit_id = settings->selectedCircuit();
    QString inspection_date = settings->selectedInspection();

    QString html; MTTextStream out(&html);

    if (settings->mainWindowSettings().serviceCompanyInformationVisible()) {
        HTMLTable *service_company = writeServiceCompany();
        out << service_company->html();
        delete service_company;
        out << "<br>";
    }

    writeCustomersTable(out, customer_id);
    out << "<br>";
    writeCircuitsTable(out, customer_id, circuit_id, 7);

    Inspection inspection_record(customer_id, circuit_id, inspection_date);
    QVariantMap inspection = inspection_record.list();
    bool nominal = inspection.value("nominal").toInt();
    bool repair = inspection.value("repair").toInt();

    HTMLParentElement *el;
    HTMLDiv div;

    div << html;
    div.newLine();

    HTMLTable *table = div.table("cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\" class=\"no_border\"");
    el = table->addRow()->addHeaderCell("colspan=\"2\" style=\"font-size: medium; background-color: lightgoldenrodyellow;\"")
         ->link("customer:" + customer_id + "/circuit:" + circuit_id + (repair ? "/repair:" : "/inspection:") + inspection_date + "/edit");
    if (nominal) *el << tr("Nominal inspection:");
    else if (repair) *el << tr("Repair:");
    else *el << tr("Inspection:");
    *el << "&nbsp;" << settings->mainWindowSettings().formatDateTime(inspection_date);

    InspectionImage images_record(customer_id, circuit_id, inspection_date);
    ListOfVariantMaps images = images_record.listAll("*", "file_id");

    for (int i = 0; i < images.count(); ++i) {
        QByteArray byte_array = DBFile(images.at(i).value("file_id").toInt()).data().toBase64();
        if (!byte_array.isNull()) {
            *(table->addRow()->addCell()) << "<img src=\"data:image/jpeg;base64," << byte_array << "\">";
        }
        *(table->addRow()->addCell()) << images.at(i).value("description").toString();
    }

    return viewTemplate("inspection_images").arg(div.html());
}

QString InspectionImagesView::title() const
{
    return tr("Inspection Images");
}
