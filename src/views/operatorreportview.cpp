/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2014 Matus & Michal Tomlein

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

#include "operatorreportview.h"

#include "global.h"
#include "mttextstream.h"
#include "records.h"
#include "viewtabsettings.h"
#include "mainwindowsettings.h"
#include "toolbarstack.h"
#include "htmlbuilder.h"

#include <QDate>
#include <QApplication>

using namespace Global;

OperatorReportView::OperatorReportView(ViewTabSettings *settings):
    View(settings)
{
}

QString OperatorReportView::renderHTML()
{
    QString customer_id = settings->selectedCustomer();
    int year = settings->toolBarStack()->filterSinceValue();
    int month_from = settings->toolBarStack()->filterMonthFromValue();
    int month_until = settings->toolBarStack()->filterMonthUntilValue();
    bool show_circuit_name = settings->toolBarStack()->isShowCircuitNameChecked();

    if (year == 0)
        year = QDate::currentDate().year() - 1;
    QString date_from = QString("%1.%2").arg(year).arg(month_from, 2, 10, QChar('0'));
    QString date_until = QString("%1.%2").arg(year).arg(month_until + 1, 2, 10, QChar('0'));

    QString interval_string = QString::number(year);
    if (month_from > 1 || month_until < 12) {
        if (month_from == month_until)
            interval_string.prepend(QString("%1 ").arg(longMonthName(month_from)));
        else
            interval_string.prepend(QString::fromUtf8("%1 \342\200\223 %2 ")
                                    .arg(longMonthName(month_from))
                                    .arg(longMonthName(month_until)));
    }

    QString html; MTTextStream out(&html);

    if (settings->mainWindowSettings().serviceCompanyInformationVisible()) {
        HTMLTable *service_company = writeServiceCompany();
        out << service_company->html();
        delete service_company;
        out << "<br>";
    }

    Customer customer(customer_id);
    customer.readOperatorValues();
    out << "<table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">";
    out << "<tr><th style=\"font-size: medium; background-color: floralwhite;\">";
    out << tr("Operator Report: %1").arg(interval_string) << "</th></tr></table><br>";
    out << "<table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">";
    out << "<tr><th colspan=\"3\">" << tr("Owner information") << "</th></tr><tr>";
    out << "<th>" << Customer::attributes().value("id") << "</th>";
    out << "<th>" << Customer::attributes().value("company") << "</th>";
    out << "<th>" << Customer::attributes().value("address") << "</th>";
    out << "</tr><tr>";
    out << "<td>" << toolTipLink("customer", customer_id.rightJustified(8, '0'), customer_id) << "</td>";
    out << "<td>" << MTVariant(customer.value("company")) << "</td>";
    out << "<td>" << MTVariant(customer.value("address"), MTVariant::Address) << "</td>";
    out << "</tr><tr><th colspan=\"3\">" << tr("Operator information") << "</th></tr><tr>";
    out << "<th>" << Customer::attributes().value("id") << "</th>";
    out << "<th>" << Customer::attributes().value("company") << "</th>";
    out << "<th>" << Customer::attributes().value("address") << "</th>";
    out << "</tr><tr>";
    out << "<td>" << customer.stringValue("operator_id").rightJustified(8, '0') << "</td>";
    out << "<td>" << MTVariant(customer.value("operator_company")) << "</td>";
    out << "<td>" << MTVariant(customer.value("operator_address"), MTVariant::Address) << "</td>";
    out << "</tr></table><br>";
    out << "<table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\" class=\"highlight\">";
    out << "<tr><th colspan=\"" << (show_circuit_name ? 9 : 8) << "\" style=\"font-size: medium; background-color: aliceblue;\">";
    out << tr("Circuit information", "Operator report") << "</th></tr><tr>";
    out << "<th rowspan=\"2\">" << QApplication::translate("Circuit", "ID") << "</th>";
    if (show_circuit_name) {
        out << "<th rowspan=\"2\">" << QApplication::translate("Circuit", "Name") << "</th>";
    }
    out << "<th rowspan=\"2\">" << QApplication::translate("Circuit", "Refrigerant") << "</th>";
    out << "<th rowspan=\"2\">" << QApplication::translate("Circuit", "Field of application") << "</th>";
    out << "<th colspan=\"4\">" << QApplication::translate("Circuit", "Amount of refrigerant") << "</th>";
    out << "<th rowspan=\"2\">" << QApplication::translate("Circuit", "Place of operation") << "</th>";
    out << "</tr><tr>";
    out << "<th>" << ((month_from > 1 || month_until < 12) ?
                          tr("At the beginning of the period") :
                          tr("At the beginning of the year")) << "</th>";
    out << "<th>" << tr("Added") << "</th>";
    out << "<th>" << tr("Recovered") << "</th>";
    out << "<th>" << ((month_from > 1 || month_until < 12) ?
                          tr("At the end of the period") :
                          tr("At the end of the year")) << "</th>";
    out << "</tr>";

    MTRecord inspections("inspections", "date", "", MTDictionary("customer", customer_id));
    inspections.parents().insert("nominal", "0");
    if (month_from > 1)
        inspections.addFilter("date >= ?", date_from);
    if (month_until < 12)
        inspections.addFilter("date < ?", date_until);
    if (month_from <= 1 || month_until >= 12)
        inspections.addFilter("date", QString("%1%").arg(year));

    MTDictionary nominal_inpection_parents("customer", customer_id);
    nominal_inpection_parents.insert("nominal", "1");

    QVariantMap sums;
    ListOfVariantMaps nominal_inspections;
    QString nominal_inspection_date, commissioning_date, decommissioning_date;
    double refrigerant_amount, refrigerant_amount_begin, refrigerant_amount_end;
    QString circuit_id;

    Circuit circuits_record(customer_id, "");
    if (!settings->toolBarStack()->isFilterEmpty()) {
        circuits_record.addFilter(settings->toolBarStack()->filterColumn(), settings->toolBarStack()->filterKeyword());
    }

    MTSqlQuery circuits = circuits_record.select("id, name, refrigerant, refrigerant_amount, field, operation, disused, commissioning, decommissioning");
    circuits.setForwardOnly(true);
    circuits.exec();
    while (circuits.next()) {
        circuit_id = circuits.stringValue("id");

        inspections.parents().insert("circuit", circuit_id);
        sums = inspections.sumAll("refr_add_am, refr_reco");

        commissioning_date = circuits.stringValue("commissioning").left(7);
        if (commissioning_date >= date_until)
            continue;
        decommissioning_date = circuits.stringValue("decommissioning").left(7);
        if (circuits.intValue("disused") == 0)
            decommissioning_date = "9999";
        else if (decommissioning_date.isEmpty())
            decommissioning_date = QString::number(QDate::currentDate().year());
        if (decommissioning_date < date_from)
            continue;
        refrigerant_amount = circuits.doubleValue("refrigerant_amount");
        refrigerant_amount_begin = 0.0;
        refrigerant_amount_end = refrigerant_amount;
        if (commissioning_date < date_from)
            refrigerant_amount_begin += refrigerant_amount;

        nominal_inpection_parents.insert("circuit", circuit_id);
        nominal_inspections = MTRecord("inspections", "date", "", nominal_inpection_parents).listAll("date, refr_add_am, refr_reco", "date ASC");
        foreach (const QVariantMap &nominal_inspection, nominal_inspections) {
            nominal_inspection_date = nominal_inspection.value("date", "9999").toString().left(7);
            if (nominal_inspection_date < date_from)
                refrigerant_amount_begin += nominal_inspection.value("refr_add_am", 0.0).toDouble()
                        - nominal_inspection.value("refr_reco", 0.0).toDouble();
            if (nominal_inspection_date < date_until)
                refrigerant_amount_end += nominal_inspection.value("refr_add_am", 0.0).toDouble()
                        - nominal_inspection.value("refr_reco", 0.0).toDouble();
        }

        if (decommissioning_date >= date_from && decommissioning_date < date_until)
            refrigerant_amount_end = 0.0;

        out << "<tr onclick=\"window.location = 'customer:" << customer_id
            << "/circuit:" << circuit_id << "'\" style=\"cursor: pointer;\">";
        out << "<td>" << toolTipLink("customer/circuit", circuit_id.rightJustified(5, '0'), customer_id, circuit_id) << "</td>";
        if (show_circuit_name) {
            out << "<td>" << MTVariant(circuits.stringValue("name")) << "</td>";
        }
        out << "<td>" << circuits.stringValue("refrigerant") << "</td>";
        out << "<td>" << fieldsOfApplication().value(circuits.stringValue("field")) << "</td>";
        out << "<td>" << refrigerant_amount_begin << "</td>";
        out << "<td>" << sums.value("refr_add_am").toDouble() << "</td>";
        out << "<td>" << sums.value("refr_reco").toDouble() << "</td>";
        out << "<td>" << refrigerant_amount_end << "</td>";
        out << "<td>" << MTVariant(circuits.value("operation")) << "</td>";
        out << "</tr>";
    }
    out << "</table><br>";

    QVariantMap inspector;
    if (settings->isInspectorSelected())
        inspector = Inspector(settings->selectedInspector()).list("person, mail, phone");

    HTMLTable compiled_by;
    HTMLTableRow *row = compiled_by.addRow();
    *(row->addCell()) << tr("Compiled by:") << " <input type=\"text\" style=\"border: 0;\" size=\"30\" value=\""
                      << escapeString(inspector.value("person").toString()) << "\">";
    *(row->addCell()) << tr("Phone:") << " <input type=\"text\" style=\"border: 0;\" size=\"20\" value=\""
                      << escapeString(inspector.value("phone").toString()) << "\">";
    *(row->addCell()) << tr("E-mail:") << " <input type=\"text\" style=\"border: 0;\" size=\"40\" value=\""
                      << escapeString(inspector.value("mail").toString()) << "\">";
    row = compiled_by.addRow();
    *(row->addCell("colspan=\"2\"")) << tr("Place:")
                                     << " <input type=\"text\" style=\"border: 0;\" size=\"60\">";
    *(row->addCell()) << tr("Date:")
                      << " <input type=\"text\" style=\"border: 0;\">";
    out << compiled_by.html();

    return viewTemplate("operator_report").arg(html);
}

QString OperatorReportView::title() const
{
    return tr("Operator Report") + " - " + Customer(settings->selectedCustomer()).stringValue("company");
}
