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

#include "agendaview.h"

#include "global.h"
#include "mttextstream.h"
#include "records.h"
#include "viewtabsettings.h"
#include "mainwindowsettings.h"
#include "toolbarstack.h"
#include "warnings.h"
#include "htmlbuilder.h"

#include <QDate>

using namespace Global;

AgendaView::AgendaView(ViewTabSettings *settings):
    View(settings)
{
}

QString AgendaView::renderHTML()
{
    bool CO2_equivalent = settings->toolBarStack()->isCO2EquivalentChecked();

    QString html; MTTextStream out(&html);

    writeServiceCompany(out);

    QMultiMap<QString, QList<QVariant> > next_inspections_map;

    MultiMapOfVariantMaps customers(Customer::query().mapAll("uuid", "id, company"));

    MTQuery circuits_query = Circuit::query({"disused", "0"});
    if (!settings->toolBarStack()->isFilterEmpty()) {
        circuits_query.addFilter(settings->toolBarStack()->filterColumn(), settings->toolBarStack()->filterKeyword());
    }
    circuits_query.addJoin("LEFT JOIN (SELECT circuit_uuid, MAX(date) AS date FROM inspections"
                           " WHERE outside_interval = 0 GROUP BY circuit_uuid) AS ins"
                           " ON ins.circuit_uuid = circuits.uuid");
    circuits_query.addJoin("LEFT JOIN (SELECT i.circuit_uuid, i.date, i.nominal, i.refr_add_am FROM inspections AS i"
                           " LEFT JOIN inspections AS j ON i.circuit_uuid = j.circuit_uuid"
                           " AND i.date < j.date WHERE j.date IS NULL) AS all_ins"
                           " ON all_ins.circuit_uuid = circuits.uuid");
    MTSqlQuery circuits = circuits_query.select("circuits.customer_uuid, circuits.uuid, circuits.id, circuits.name, circuits.operation, circuits.refrigerant, "
                                                + circuitRefrigerantAmountQuery()
                                                + ", circuits.hermetic, circuits.leak_detector, circuits.inspection_interval,"
                                                " COALESCE(ins.date, circuits.commissioning) AS last_regular_inspection,"
                                                " COALESCE(all_ins.date, circuits.commissioning) AS last_inspection,"
                                                " all_ins.nominal, all_ins.refr_add_am");
    circuits.setForwardOnly(true);
    circuits.exec();
    while (circuits.next()) {
        QString refrigerant = circuits.stringValue("refrigerant");
        double refrigerant_amount = circuits.doubleValue("refrigerant_amount");
        int inspection_interval = Warnings::circuitInspectionInterval(refrigerant, refrigerant_amount, CO2_equivalent,
                                                                      circuits.intValue("hermetic"),
                                                                      circuits.intValue("leak_detector"),
                                                                      circuits.intValue("inspection_interval"));
        if (inspection_interval) {
            QString last_regular_inspection_date = circuits.stringValue("last_regular_inspection");
            if (last_regular_inspection_date.isEmpty())
                continue;
            QString next_regular_inspection_date = QDate::fromString(last_regular_inspection_date.split("-").first(), DATE_FORMAT)
                    .addDays(inspection_interval).toString(DATE_FORMAT);
            QString last_inspection_date = circuits.stringValue("last_inspection");
            if (!last_inspection_date.isEmpty()) {
                QString next_inspection_date = QDate::fromString(last_inspection_date.split("-").first(), DATE_FORMAT)
                        .addDays(30).toString(DATE_FORMAT);
                if (next_inspection_date < next_regular_inspection_date &&
                        circuits.intValue("nominal") == 0 && circuits.doubleValue("refr_add_am") > 0.0)
                    next_inspections_map.insert(next_inspection_date,
                                                QList<QVariant>()
                                                    << circuits.stringValue("customer_uuid")
                                                    << circuits.stringValue("uuid")
                                                    << circuits.stringValue("id")
                                                    << circuits.stringValue("name")
                                                    << circuits.stringValue("operation")
                                                    << refrigerant
                                                    << refrigerant_amount
                                                    << last_inspection_date
                                                    << true);
            }
            next_inspections_map.insert(next_regular_inspection_date,
                                        QList<QVariant>()
                                            << circuits.stringValue("customer_uuid")
                                            << circuits.stringValue("uuid")
                                            << circuits.stringValue("id")
                                            << circuits.stringValue("name")
                                            << circuits.stringValue("operation")
                                            << refrigerant
                                            << refrigerant_amount
                                            << last_regular_inspection_date
                                            << false);
        }
    }

    out << "<table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\"><tr>";
    out << "<th colspan=\"7\" style=\"font-size: medium;\">" << tr("Agenda") << "</th></tr>";
    out << "<tr><th>" << tr("Next inspection") << "</th><th>" << tr("Customer") << "</th>";
    out << "<th>" << tr("Circuit") << "</th><th>" << QApplication::translate("Circuit", "Place of operation") << "</th>";
    out << "<th>" << QApplication::translate("Circuit", "Refrigerant") << "</th>";
    out << "<th>" << replaceUnsupportedCharacters(QApplication::translate("MainWindow", "CO\342\202\202 equivalent")) << "</th>";
    out << "<th>" << tr("Last inspection") << "</th></tr>";

    QMapIterator<QString, QList<QVariant> > i(next_inspections_map);
    while (i.hasNext()) { i.next();
        QString customer_uuid = i.value().value(0).toString();
        QString circuit_uuid = i.value().value(1).toString();
        QString circuit_id = i.value().value(2).toString();
        QString circuit_name = i.value().value(3).toString();
        QString operation = i.value().value(4).toString();
        QString refrigerant = i.value().value(5).toString();
        double refrigerant_amount = i.value().value(6).toDouble();
        QString last_inspection_date = i.value().value(7).toString();
        bool reinspection = i.value().value(8).toBool();
        qint64 days_to = QDate::currentDate().daysTo(QDate::fromString(i.key(), DATE_FORMAT));
        QString next_inspection;
        switch (days_to) {
            case -1: next_inspection = tr("Yesterday"); break;
            case 0: next_inspection = tr("Today"); break;
            case 1: next_inspection = tr("Tomorrow"); break;
            default: next_inspection = settings->mainWindowSettings().formatDate(i.key()); break;
        }
        QString style;
        if (!settings->isPrinterFriendlyVersionChecked()) {
            if (days_to < 0)
                style = "background-color: #eacccc;";
            else if (days_to < 31)
                style = "background-color: #f2f27f;";
        }
        out << "<tr><td style=\"" << style << "\">";
        if (reinspection)
            out << "<i>";
        out << next_inspection;
        if (reinspection)
            out << "*</i>";
        out << "</td><td style=\"" << style << "\"><a href=\"customer:" << customer_uuid << "\">";
        out << escapeString(customers.value(customer_uuid).value("id").toString());
        out << " (" << escapeString(customers.value(customer_uuid).value("company").toString()) << ")</a></td>";
        out << "<td style=\"" << style << "\"><a href=\"customer:" << customer_uuid << "/circuit:" << circuit_uuid << "\">";
        out << circuit_id.rightJustified(5, '0');
        if (!circuit_name.isEmpty()) {
            out << " (" << escapeString(circuit_name) << ")";
        }
        out << "</a></td>";
        out << "<td style=\"" << style << "\">" << escapeString(operation) << "</td>";
        out << "<td style=\"" << style << "\">" << refrigerant_amount << "&nbsp;" << QApplication::translate("Units", "kg")
            << " " << escapeString(refrigerant) << "</td>";
        out << "<td style=\"" << style << "\">" << CO2Equivalent(refrigerant, refrigerant_amount)
            << "&nbsp;" << QApplication::translate("Units", "t") << "</td>";
        out << "<td style=\"" << style << "\">";
        if (last_inspection_date.contains("-"))
            out << "<a href=\"customer:" << customer_uuid << "/circuit:" << circuit_uuid << "/inspection:"
                << last_inspection_date << "\">" << settings->mainWindowSettings().formatDateTime(last_inspection_date) << "</a>";
        else
            out << settings->mainWindowSettings().formatDate(last_inspection_date);
        out << "</td></tr>";
    }
    out << "</table>";

    return viewTemplate("agenda").arg(html);
}

QString AgendaView::title() const
{
    return tr("Agenda");
}
