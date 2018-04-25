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

#include "customersview.h"

#include "global.h"
#include "mttextstream.h"
#include "records.h"
#include "viewtabsettings.h"
#include "mainwindowsettings.h"
#include "toolbarstack.h"
#include "htmlbuilder.h"

using namespace Global;

CustomersView::CustomersView(ViewTabSettings *settings):
    View(settings)
{
}

QString CustomersView::renderHTML()
{
    QString html; MTTextStream out(&html);
    writeServiceCompany(out);
    writeCustomersTable(out);
    return viewTemplate("customers").arg(html);
}

static void addCustomerHeaderCell(const QString &key, const QString &customer_id, HTMLTableRow *row)
{
    if (customer_id.isEmpty())
        *(row->addHeaderCell()) << "<a href=\"allcustomers:/order_by:" << key << "\">" << Customer::attributes().value(key) << "</a>";
    else
        *(row->addHeaderCell()) << Customer::attributes().value(key);
}

void CustomersView::writeCustomersTable(MTTextStream &out, const QString &customer_id)
{
    HTMLTable *table = writeCustomersTable(customer_id);
    out << table->html();
    delete table;
}

HTMLTable *CustomersView::writeCustomersTable(const QString &customer_id, HTMLTable *table)
{
    bool disable_hiding_details = settings->currentView() == View::AssemblyRecordDetails;
    bool customer_details_visible = settings->mainWindowSettings().customerDetailsVisible() || disable_hiding_details;
    bool show_date_updated = settings->isShowDateUpdatedChecked() && !disable_hiding_details;
    bool show_owner = settings->isShowOwnerChecked() && !disable_hiding_details;
    bool show_notes = settings->isShowNotesChecked() && !customer_id.isEmpty();

    Customer all_customers(customer_id);
    if (customer_id.isEmpty() && !settings->toolBarStack()->isFilterEmpty()) {
        all_customers.addFilter(settings->toolBarStack()->filterColumn(), settings->toolBarStack()->filterKeyword());
    }
    QString order_by;
    if (!customer_id.isEmpty() || settings->mainWindowSettings().orderByForView(LinkParser::AllCustomers).isEmpty())
        order_by = "company ASC, id ASC";
    else
        order_by = settings->appendDefaultOrderToColumn(settings->mainWindowSettings().orderByForView(LinkParser::AllCustomers));
    ListOfVariantMaps list = all_customers.listAll("*,"
                                                   " (SELECT COUNT(id) FROM circuits WHERE parent = customers.id) AS circuits_count,"
                                                   " (SELECT COUNT(date) FROM inspections WHERE customer = customers.id) AS inspections_count",
                                                   order_by);

    if (!table)
        table = new HTMLTable("cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\"");
    table->addClass("customers");
    if (customer_id.isEmpty())
        table->addClass("highlight");

    int thead_colspan = 7;
    HTMLTableRow *row = NULL;

    if (customer_id.isEmpty() || customer_details_visible) {
        row = new HTMLTableRow();

        addCustomerHeaderCell("id", customer_id, row);
        addCustomerHeaderCell("company", customer_id, row);
        addCustomerHeaderCell("address", customer_id, row);
        addCustomerHeaderCell("mail", customer_id, row);
        addCustomerHeaderCell("phone", customer_id, row);
        *(row->addHeaderCell()) << tr("Number of circuits");
        *(row->addHeaderCell()) << tr("Total number of inspections");
        if (show_date_updated) {
            if (customer_id.isEmpty())
                *(row->addHeaderCell()) << "<a href=\"allcustomers:/order_by:date_updated\">" << tr("Date Updated") << "</a>";
            else
                *(row->addHeaderCell()) << tr("Date Updated");
            thead_colspan++;
        }
        if (show_owner) {
            if (customer_id.isEmpty())
                *(row->addHeaderCell()) << "<a href=\"allcustomers:/order_by:updated_by\">" << tr("Author") << "</a>";
            else
                *(row->addHeaderCell()) << tr("Author");
            thead_colspan++;
        }
    }

    HTMLTableCell *cell = table->addRow()->addHeaderCell("colspan=\"" + QString::number(thead_colspan) + "\" style=\"font-size: medium; background-color: floralwhite;\"");

    if (customer_id.isEmpty()) {
        *cell << tr("Customers");
    } else {
        if (!disable_hiding_details)
            *cell << "<a href=\"toggledetailsvisible:customer\">";
        if (customer_details_visible || !list.count()) {
            *cell << tr("Customer");
        } else {
            QString name = list.first().value("company").toString().trimmed();
            if (name.isEmpty())
                name = formatCompanyID(customer_id);
            *cell << escapeString(tr("Customer: %1").arg(name));
        }
        if (!disable_hiding_details)
            *cell << "</a>";
    }

    if (customer_id.isEmpty() || customer_details_visible) {
        *table << row;

        QString id; QString highlighted_id = settings->selectedCustomer();
        for (int i = 0; i < list.count(); ++i) {
            id = list.at(i).value("id").toString();
            QString row_attrs;
            if (customer_id.isEmpty()) {
                row_attrs = QString("id=\"%1\" onclick=\"window.location = '%1'\" style=\"cursor: pointer;\"").arg("customer:" + id);
                if (id == highlighted_id)
                    row_attrs.append(" class=\"selected\"");
            }
            row = table->addRow(row_attrs);
            *(row->addCell()) << toolTipLink("customer", formatCompanyID(id), id);
            *(row->addCell()) << escapeString(list.at(i).value("company"));
            *(row->addCell()) << MTVariant(list.at(i).value("address"), MTVariant::Address);
            *(row->addCell()) << escapeString(list.at(i).value("mail"));
            *(row->addCell()) << escapeString(list.at(i).value("phone"));
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

HTMLTable *CustomersView::customerContactPersons(const QString &customer_id, HTMLTable *table)
{
    if (!table) table = new HTMLTable("cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\"");
    table->addClass("contact_persons");
    HTMLTableRow *_tr;

    _tr = table->addRow();
    *(_tr->addHeaderCell()) << tr("Contact persons");
    *(_tr->addHeaderCell()) << tr("E-mail");
    *(_tr->addHeaderCell()) << tr("Phone");

    Person persons_record(QString(), customer_id);
    persons_record.parents().insert("hidden", "0");
    ListOfVariantMaps persons = persons_record.listAll();
    for (int i = 0; i < persons.count(); ++i) {
        _tr = table->addRow();
        *(_tr->addCell()) << persons.at(i).value("name").toString();
        *(_tr->addCell()) << persons.at(i).value("mail").toString();
        *(_tr->addCell()) << persons.at(i).value("phone").toString();
    }

    return table;
}

QString CustomersView::title() const
{
    return tr("Customers");
}
