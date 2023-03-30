/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2023 Matus & Michal Tomlein

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

#include "customersview.h"

#include "global.h"
#include "mttextstream.h"
#include "records.h"
#include "viewtabsettings.h"
#include "mainwindowsettings.h"
#include "toolbarstack.h"
#include "htmlbuilder.h"

#include <QUuid>

using namespace Global;

CustomersView::CustomersView(ViewTabSettings *settings):
    View(settings)
{
}

QString CustomersView::renderHTML(bool)
{
    QString html; MTTextStream out(&html);
    writeServiceCompany(out);
    writeCustomersTable(out);
    return viewTemplate("customers").arg(html);
}

static void addCustomerHeaderCell(const QString &key, const QString &title, const QString &customer_uuid, HTMLTableRow *row)
{
    if (customer_uuid.isEmpty())
        *(row->addHeaderCell()) << "<a href=\"allcustomers:/order_by:" << key << "\">" << title << "</a>";
    else
        *(row->addHeaderCell()) << title;
}

static void addCustomerHeaderCell(const QString &key, const QString &customer_uuid, HTMLTableRow *row)
{
    addCustomerHeaderCell(key, Customer::attributes().value(key), customer_uuid, row);
}

void CustomersView::writeCustomersTable(MTTextStream &out, const QString &customer_uuid)
{
    HTMLTable *table = writeCustomersTable(customer_uuid);
    out << table->html();
    delete table;
}

HTMLTable *CustomersView::writeCustomersTable(const QString &customer_uuid, HTMLTable *table)
{
    bool disable_hiding_details = settings->currentView() == View::AssemblyRecordDetails;
    bool customer_details_visible = settings->mainWindowSettings().customerDetailsVisible() || disable_hiding_details;
    bool show_date_updated = settings->isShowDateUpdatedChecked() && !disable_hiding_details;
    bool show_owner = settings->isShowOwnerChecked() && !disable_hiding_details;
    bool show_notes = settings->isShowNotesChecked() && !customer_uuid.isEmpty();

    MTQuery all_customers = Customer::query();
    if (!customer_uuid.isEmpty()) {
        all_customers.parents().insert("uuid", customer_uuid);
    } else {
        QString service_company_uuid = settings->filterServiceCompanyUUID();
        if (!QUuid(service_company_uuid).isNull()) {
            all_customers.addJoin(QString("JOIN (SELECT customer_uuid, COUNT(uuid) AS serviced_circuit_count FROM circuits WHERE service_company_uuid = '%1' GROUP BY customer_uuid) AS serviced_circuits ON customers.uuid = serviced_circuits.customer_uuid AND serviced_circuit_count > 0").arg(service_company_uuid));
        }
        if (!settings->toolBarStack()->isFilterEmpty()) {
            all_customers.addFilter(settings->toolBarStack()->filterColumn(), settings->toolBarStack()->filterKeyword());
        }
        if (settings->toolBarStack()->starredOnly()) {
            all_customers.addFilter("starred <> ?", "0");
        }
    }
    QString order_by;
    if (!customer_uuid.isEmpty() || settings->mainWindowSettings().orderByForView(LinkParser::AllCustomers).isEmpty())
        order_by = "company ASC, id ASC";
    else
        order_by = settings->appendDefaultOrderToColumn(settings->mainWindowSettings().orderByForView(LinkParser::AllCustomers));
    ListOfVariantMaps list = all_customers.listAll("*,"
                                                   " (SELECT COUNT(uuid) FROM circuits WHERE customer_uuid = customers.uuid) AS circuits_count,"
                                                   " (SELECT COUNT(uuid) FROM inspections WHERE customer_uuid = customers.uuid) AS inspections_count",
                                                   order_by);

    if (!table)
        table = new HTMLTable("cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\"");
    table->addClass("customers");
    if (customer_uuid.isEmpty())
        table->addClass("highlight");

    int thead_colspan = 9;
    HTMLTableRow *row = NULL;

    if (customer_uuid.isEmpty() || customer_details_visible) {
        row = new HTMLTableRow();

        addCustomerHeaderCell("id", customer_uuid, row);
        addCustomerHeaderCell("company", customer_uuid, row);
        addCustomerHeaderCell("address", customer_uuid, row);
        addCustomerHeaderCell("mail", customer_uuid, row);
        addCustomerHeaderCell("phone", customer_uuid, row);
        addCustomerHeaderCell("website_url", customer_uuid, row);
        addCustomerHeaderCell("maps_url", customer_uuid, row);
        addCustomerHeaderCell("circuits_count", tr("Number of circuits"), customer_uuid, row);
        addCustomerHeaderCell("inspections_count", tr("Total number of inspections"), customer_uuid, row);
        if (show_date_updated) {
            if (customer_uuid.isEmpty())
                *(row->addHeaderCell()) << "<a href=\"allcustomers:/order_by:date_updated\">" << tr("Date Updated") << "</a>";
            else
                *(row->addHeaderCell()) << tr("Date Updated");
            thead_colspan++;
        }
        if (show_owner) {
            if (customer_uuid.isEmpty())
                *(row->addHeaderCell()) << "<a href=\"allcustomers:/order_by:updated_by\">" << tr("Author") << "</a>";
            else
                *(row->addHeaderCell()) << tr("Author");
            thead_colspan++;
        }
    }

    HTMLTableCell *cell = table->addRow()->addHeaderCell("colspan=\"" + QString::number(thead_colspan) + "\" style=\"font-size: medium; background-color: floralwhite;\"");

    if (customer_uuid.isEmpty()) {
        *cell << tr("Customers");
    } else {
        if (!disable_hiding_details)
            *cell << "<a href=\"toggledetailsvisible:customer\">";
        if (customer_details_visible || !list.count()) {
            *cell << tr("Customer");
        } else {
            QString name = list.first().value("company").toString().trimmed();
            if (name.isEmpty())
                name = list.first().value("id").toString();
            *cell << escapeString(tr("Customer: %1").arg(name));
        }
        if (!disable_hiding_details)
            *cell << "</a>";
    }

    if (customer_uuid.isEmpty() || customer_details_visible) {
        *table << row;

        QString highlighted_uuid = settings->selectedCustomerUUID();
        for (int i = 0; i < list.count(); ++i) {
            QString uuid = list.at(i).value("uuid").toString();
            QString row_attrs;
            if (customer_uuid.isEmpty()) {
                row_attrs = QString("id=\"%1\" onclick=\"window.location = '%1'\" style=\"cursor: pointer;\"").arg("customer:" + uuid);
                if (uuid == highlighted_uuid)
                    row_attrs.append(" class=\"selected\"");
            }
            row = table->addRow(row_attrs);
            *(row->addCell())
                << QString("<span class=\"screen_only\"><a href=\"customer:%1/star\" class=\"no_underline\">%2</a>&nbsp;</span>")
                    .arg(uuid)
                    .arg(QChar(list.at(i).value("starred").toInt() ? 0x2605 : 0x2606))
                << toolTipLink("customer", list.at(i).value("id").toString(), uuid);
            *(row->addCell("class=\"wrap\"")) << ellipsis(list.at(i).value("company"));
            *(row->addCell("class=\"wrap\"")) << ellipsis(MTVariant(list.at(i).value("address"), MTVariant::Address));
            QString mail = escapeString(list.at(i).value("mail"));
            *(row->addCell()) << "<a href=\"mailto:" << mail << "\">" << mail << "</a>";
            *(row->addCell()) << escapeString(list.at(i).value("phone"));
            *(row->addCell()) << formatURL(list.at(i).value("website_url").toString());
            *(row->addCell()) << formatURL(list.at(i).value("maps_url").toString());
            *(row->addCell()) << list.at(i).value("circuits_count").toString();
            *(row->addCell()) << list.at(i).value("inspections_count").toString();
            if (show_date_updated)
                *(row->addCell()) << settings->mainWindowSettings().formatDateTime(list.at(i).value("date_updated"));
            if (show_owner)
                *(row->addCell()) << escapeString(list.at(i).value("updated_by"));

            if (show_notes) {
                QString notes = list.at(i).value("notes").toString();
                if (!notes.isEmpty()) {
                    row = table->addRow();
                    *(row->addHeaderCell(QString("colspan=\"%1\"").arg(thead_colspan))) << tr("Notes");
                    row = table->addRow();
                    *(row->addCell(QString("colspan=\"%1\"").arg(thead_colspan))) << escapeString(notes, false, true);
                }
            }
        }
    }

    return table;
}

HTMLTable *CustomersView::customerContactPersons(const QString &customer_uuid, HTMLTable *table)
{
    if (!table) table = new HTMLTable("cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\"");
    table->addClass("contact_persons");
    HTMLTableRow *_tr;

    _tr = table->addRow();
    *(_tr->addHeaderCell()) << tr("Contact persons");
    *(_tr->addHeaderCell()) << tr("E-mail");
    *(_tr->addHeaderCell()) << tr("Phone");

    MTQuery persons_query = Person::query({{"customer_uuid", customer_uuid}, {"hidden", "0"}});
    ListOfVariantMaps persons = persons_query.listAll();
    for (int i = 0; i < persons.count(); ++i) {
        _tr = table->addRow();
        *(_tr->addCell()) << persons.at(i).value("name").toString();
        QString mail = persons.at(i).value("mail").toString();
        *(_tr->addCell()) << "<a href=\"mailto:" << mail << "\">" << mail << "</a>";
        *(_tr->addCell()) << persons.at(i).value("phone").toString();
    }

    return table;
}

QString CustomersView::title() const
{
    return tr("Customers");
}
