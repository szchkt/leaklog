/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2026 Matus & Michal Tomlein

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

#include "businesspartnersview.h"

#include "businesspartner.h"
#include "htmlbuilder.h"
#include "mtvariant.h"
#include "toolbarstack.h"
#include "viewtabsettings.h"

BusinessPartnersView::BusinessPartnersView(ViewTabSettings *settings):
    View(settings)
{
}

QString BusinessPartnersView::renderHTML(bool)
{
    HTMLDiv div;
    writeServiceCompany(div);

    MTQuery partners_query = BusinessPartner::query();
    if (!settings->toolBarStack()->isFilterEmpty())
        partners_query.addFilter(settings->toolBarStack()->filterColumn(), settings->toolBarStack()->filterKeyword());
    ListOfVariantMaps partners = partners_query.listAll("*", "name, company_id");
    const MTDictionary &attributes = BusinessPartner::attributes();

    HTMLTable *table = new HTMLTable("cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\"");
    table->addClass("highlight");
    HTMLTableHead *thead = table->thead();
    thead->addClass("sticky");
    *(thead->addRow()->addHeaderCell(QString("colspan=\"%1\" style=\"font-size: medium;\"").arg(attributes.count())))
        << tr("Business Partners");

    HTMLTableRow *row = thead->addRow();
    for (int n = 0; n < attributes.count(); ++n)
        *(row->addHeaderCell()) << attributes.value(n);

    for (int i = 0; i < partners.count(); ++i) {
        QString uuid = partners.at(i).value("uuid").toString();
        row = table->addRow(QString("onclick=\"window.location = 'businesspartner:%1/edit'\" style=\"cursor: pointer;\"").arg(uuid));
        for (int n = 0; n < attributes.count(); ++n) {
            QString key = attributes.key(n);
            *(row->addCell("class=\"wrap\"")) << ellipsis(MTVariant(partners.at(i).value(key), key));
        }
    }

    div << table;
    return viewTemplate("business_partners").arg(div.html());
}

QString BusinessPartnersView::title() const
{
    return tr("Business Partners");
}
