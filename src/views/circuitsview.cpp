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

#include "circuitsview.h"

#include "global.h"
#include "mttextstream.h"
#include "records.h"
#include "viewtabsettings.h"
#include "mainwindowsettings.h"
#include "toolbarstack.h"
#include "htmlbuilder.h"

using namespace Global;

CircuitsView::CircuitsView(ViewTabSettings *settings):
    CustomersView(settings)
{
}

QString CircuitsView::renderHTML()
{
    QString customer_id = settings->selectedCustomer();
    HTMLMain main;
    if (settings->mainWindowSettings().serviceCompanyInformationVisible()) {
        main << writeServiceCompany();
        main.newLine();
    }
    main << writeCustomersTable(customer_id);
    if (settings->mainWindowSettings().customerDetailsVisible()) {
        main.newLine();
        main << customerContactPersons(customer_id);
    }
    main.newLine();
    main << writeCircuitsTable(customer_id);
    return viewTemplate("customer").arg(main.html());
}

void CircuitsView::writeCircuitsTable(MTTextStream &out, const QString &customer_id, const QString &circuit_id, int cols_in_row)
{
    HTMLDiv *div = writeCircuitsTable(customer_id, circuit_id, cols_in_row);
    out << div->html();
    delete div;
}

HTMLDiv *CircuitsView::writeCircuitsTable(const QString &customer_id, const QString &circuit_id, int cols_in_row, HTMLTable *table)
{
    bool disable_hiding_details = settings->currentView() == View::AssemblyRecordDetails;
    bool circuits_details_visible = settings->mainWindowSettings().circuitDetailsVisible() || disable_hiding_details;
    bool circuits_visible = settings->mainWindowSettings().circuitsVisible();
    bool excluded_circuits_visible = settings->mainWindowSettings().excludedCircuitsVisible();
    bool decommissioned_circuits_visible = settings->mainWindowSettings().decommissionedCircuitsVisible();
    bool show_date_updated = settings->isShowDateUpdatedChecked() && !disable_hiding_details;
    bool show_owner = settings->isShowOwnerChecked() && !disable_hiding_details;
    bool all_circuits = circuit_id.isEmpty();

    Circuit circuits_record(customer_id, circuit_id);
    if (all_circuits && !settings->toolBarStack()->isFilterEmpty()) {
        circuits_record.addFilter(settings->toolBarStack()->filterColumn(), settings->toolBarStack()->filterKeyword());
    }
    QString circuits_query_select = "circuits.*, (SELECT date FROM inspections"
            " WHERE inspections.customer = circuits.parent AND inspections.circuit = circuits.id"
            " ORDER BY date DESC LIMIT 1) AS last_inspection, " + circuitRefrigerantAmountQuery();
    QString order_by = settings->mainWindowSettings().orderByForView(LinkParser::Customer);
    if (order_by.isEmpty())
        order_by = "id";
    ListOfVariantMaps circuits = circuits_record.listAll(circuits_query_select,
                                                         all_circuits ? settings->appendDefaultOrderToColumn(order_by) : QString());

    int commissioned_count = 0;
    int excluded_count = 0;
    int decommissioned_count = 0;
    foreach (const QVariantMap &circuit, circuits) {
        int disused = circuit.value("disused").toInt();
        if (disused == Circuit::Commissioned) {
            commissioned_count++;
        } else if (disused <= Circuit::ExcludedFromAgenda) {
            excluded_count++;
        } else if (disused >= Circuit::Decommissioned) {
            decommissioned_count++;
        }
    }

    HTMLDiv *div = new HTMLDiv();
    if (!table) table = new HTMLTable("cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\"");
    table->addClass("circuits");
    if (all_circuits)
        table->addClass("highlight");

    HTMLTableRow *thead = NULL;

    if (all_circuits || circuits_details_visible) {
        thead = new HTMLTableRow();
        writeCircuitsHeader(customer_id, circuit_id, cols_in_row, show_date_updated, show_owner, !all_circuits && circuits.count() && circuits.first().value("disused").toInt() != Circuit::Commissioned, thead);
    }

    HTMLTableRow *_tr = table->addRow();
    HTMLTableCell *_td = _tr->addHeaderCell("colspan=\"" + QString::number(thead ? thead->childCount() : 1) + "\" style=\"font-size: medium; background-color: aliceblue;\"");

    if (all_circuits) {
        *_td << "<a href=\"toggledetailsvisible:circuits\">";
        if (circuits_visible) {
            *_td << tr("Circuits");
        } else {
            *_td << tr("Circuits (%1)").arg(commissioned_count);
        }
        *_td << "</a>";
    } else {
        if (!disable_hiding_details)
            *_td << "<a href=\"toggledetailsvisible:circuit\">";
        if (circuits_details_visible || !circuits.count()) {
            *_td << tr("Circuit");
        } else {
            QVariantMap circuit = circuits.first();
            QString name = circuit.value("name").toString().trimmed();
            if (name.isEmpty())
                name = circuit.value("device").toString().trimmed();
            if (name.isEmpty())
                name = QString("%1 %2").arg(circuit.value("manufacturer").toString()).arg(circuit.value("type").toString()).trimmed();
            if (!name.isEmpty())
                name = QString("(%1)").arg(name);
            *_td << escapeString(tr("Circuit: %1 %2").arg(circuit.value("id").toString().rightJustified(5, '0')).arg(name));
        }
        if (!disable_hiding_details)
            *_td << "</a>";
    }

    if (all_circuits ? circuits_visible : circuits_details_visible) {
        *table << thead;

        foreach (const QVariantMap &circuit, circuits) {
            if (all_circuits && circuit.value("disused").toInt() != Circuit::Commissioned) continue;
            writeCircuitRow(circuit, customer_id, circuit_id, cols_in_row, show_date_updated, show_owner, table);
        }
    }

    if (cols_in_row < 0)
        *div << table;
    else
        *div << table->customHtml(cols_in_row);

    if (all_circuits && excluded_count) {
        *div << "<br>";
        table = new HTMLTable("cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\" class=\"highlight\"");
        _tr = table->addRow();

        if (excluded_circuits_visible) {
            thead = table->addRow();
            writeCircuitsHeader(customer_id, circuit_id, cols_in_row, show_date_updated, show_owner, true, thead);
        } else {
            thead = NULL;
        }

        _td = _tr->addHeaderCell(QString("colspan=\"%1\" style=\"font-size: medium;\"").arg(thead ? thead->childCount() : 1));
        *_td << "<a href=\"toggledetailsvisible:excludedcircuits\">";
        if (excluded_circuits_visible) {
            *_td << tr("Circuits Excluded from Agenda");
        } else {
            *_td << tr("Circuits Excluded from Agenda (%1)").arg(excluded_count);
        }
        *_td << "</a>";

        if (excluded_circuits_visible) {
            foreach (const QVariantMap &circuit, circuits) {
                if (circuit.value("disused").toInt() > Circuit::ExcludedFromAgenda) continue;
                writeCircuitRow(circuit, customer_id, circuit_id, cols_in_row, show_date_updated, show_owner, table);
            }
        }
        *div << table;
    }

    if (all_circuits && decommissioned_count) {
        *div << "<br>";
        table = new HTMLTable("cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\" class=\"highlight\"");
        _tr = table->addRow();

        if (decommissioned_circuits_visible) {
            thead = table->addRow();
            writeCircuitsHeader(customer_id, circuit_id, cols_in_row, show_date_updated, show_owner, true, thead);
        } else {
            thead = NULL;
        }

        _td = _tr->addHeaderCell(QString("colspan=\"%1\" style=\"font-size: medium;\"").arg(thead ? thead->childCount() : 1));
        *_td << "<a href=\"toggledetailsvisible:decommissionedcircuits\">";
        if (decommissioned_circuits_visible) {
            *_td << tr("Decommissioned Circuits");
        } else {
            *_td << tr("Decommissioned Circuits (%1)").arg(decommissioned_count);
        }
        *_td << "</a>";

        if (decommissioned_circuits_visible) {
            foreach (const QVariantMap &circuit, circuits) {
                if (circuit.value("disused").toInt() < Circuit::Decommissioned) continue;
                writeCircuitRow(circuit, customer_id, circuit_id, cols_in_row, show_date_updated, show_owner, table);
            }
        }
        *div << table;
    }

    return div;
}

void CircuitsView::writeCircuitsHeader(const QString &customer_id, const QString &circuit_id, int cols_in_row, bool show_date_updated, bool show_owner, bool disused, HTMLTableRow *thead)
{
    for (int n = 0; n < Circuit::numBasicAttributes(); ++n) {
        if (circuit_id.isEmpty())
            *(thead->addHeaderCell()) << "<a href=\"customer:" << customer_id << "/order_by:"
                                      << Circuit::attributes().key(n) << "\">" << Circuit::attributes().value(n).split("||").first() << "</a>";
        else
            *(thead->addHeaderCell()) << Circuit::attributes().value(n).split("||").first();
    }
    *(thead->addHeaderCell()) << Circuit::attributes().value("refrigerant");
    if (cols_in_row >= 0) {
        *(thead->addHeaderCell()) << replaceUnsupportedCharacters(QApplication::translate("MainWindow", "CO\342\202\202 equivalent"));
        *(thead->addHeaderCell()) << QApplication::translate("MainWindow", "GWP");
    }
    *(thead->addHeaderCell()) << Circuit::attributes().value("oil");
    if (disused) {
        *(thead->addHeaderCell()) << Circuit::attributes().value("decommissioning");
    } else {
        *(thead->addHeaderCell()) << tr("Last inspection");
    }
    if (show_date_updated) {
        if (circuit_id.isEmpty())
            *(thead->addHeaderCell()) << "<a href=\"customer:" << customer_id << "/order_by:date_updated\">" << tr("Date Updated") << "</a>";
        else
            *(thead->addHeaderCell()) << tr("Date Updated");
    }
    if (show_owner) {
        if (circuit_id.isEmpty())
            *(thead->addHeaderCell()) << "<a href=\"customer:" << customer_id << "/order_by:updated_by\">" << tr("Author") << "</a>";
        else
            *(thead->addHeaderCell()) << tr("Author");
    }
}

void CircuitsView::writeCircuitRow(const QVariantMap &circuit, const QString &customer_id, const QString &circuit_id, int cols_in_row, bool show_date_updated, bool show_owner, HTMLTable *table)
{
    QString id = circuit.value("id").toString();
    QString tr_attr;
    if (circuit_id.isEmpty()) {
        tr_attr = QString("id=\"%2\" onclick=\"window.location = 'customer:%1/%2'\" style=\"cursor: pointer;\"").arg(customer_id).arg("circuit:" + id);
        if (id == settings->selectedCircuit())
            tr_attr.append(" class=\"selected\"");
    }
    HTMLTableRow *_tr = table->addRow(tr_attr);
    *(_tr->addCell()) << toolTipLink("customer/circuit", id.rightJustified(5, '0'), customer_id, id);
    for (int n = 1; n < Circuit::numBasicAttributes(); ++n) {
        QStringList dict_value = Circuit::attributes().value(n).split("||");
        QString attr_value = circuit.value(Circuit::attributes().key(n)).toString();
        if (Circuit::attributes().key(n) == "field") {
            if (attributeValues().contains("field::" + attr_value)) {
                attr_value = attributeValues().value("field::" + attr_value);
            }
        } else if (Circuit::attributes().key(n) == "hermetic") {
            attr_value = attr_value.toInt() ? tr("Yes") : tr("No");
        } else if (Circuit::attributes().key(n) == "commissioning") {
            attr_value = settings->mainWindowSettings().formatDate(attr_value);
        }
        HTMLTableCell *_td = _tr->addCell();
        *_td << escapeString(attr_value);
        if (dict_value.count() > 1) { *_td << "&nbsp;" << dict_value.last(); }
    }
    HTMLTableCell *_td = _tr->addCell();
    *_td << circuit.value("refrigerant_amount").toDouble() << "&nbsp;" << QApplication::translate("Units", "kg");
    QString refrigerant = circuit.value("refrigerant").toString();
    *_td << " " << refrigerant;
    if (cols_in_row >= 0) {
        _td = _tr->addCell();
        double GWP = refrigerantGWP(refrigerant);
        double CO2_equivalent = circuit.value("refrigerant_amount").toDouble() * GWP / 1000.0;
        *_td << CO2_equivalent << "&nbsp;" << QApplication::translate("Units", "t");
        _td = _tr->addCell();
        *_td << GWP;
    }
    _td = _tr->addCell();
    *_td << circuit.value("oil_amount").toDouble() << "&nbsp;" << QApplication::translate("Units", "kg");
    *_td << " " << circuit.value("oil").toString().toUpper();
    if (circuit.value("disused").toInt() == Circuit::Commissioned) {
        *(_tr->addCell()->link(QString("customer:%1/circuit:%2/inspection:%3")
                               .arg(customer_id)
                               .arg(id)
                               .arg(circuit.value("last_inspection").toString())))
        << settings->mainWindowSettings().formatDate(circuit.value("last_inspection").toString().split('-').first());
    } else {
        *(_tr->addCell()) << settings->mainWindowSettings().formatDate(circuit.value("decommissioning"));
    }
    if (show_date_updated)
        *(_tr->addCell()) << settings->mainWindowSettings().formatDateTime(circuit.value("date_updated"));
    if (show_owner)
        *(_tr->addCell()) << escapeString(circuit.value("updated_by"));
}

HTMLTable *CircuitsView::circuitCompressorsTable(const QString &customer_id, const QString &circuit_id, HTMLTable *table)
{
    Compressor compressors_rec(QString(), MTDictionary(QStringList() << "customer_id" << "circuit_id",
                                                       QStringList() << customer_id << circuit_id));
    ListOfVariantMaps compressors = compressors_rec.listAll();
    if (compressors.count()) {
        if (!table) table = new HTMLTable("cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\"");
        HTMLTableRow *_tr;

        _tr = table->addRow();
        *(_tr->addHeaderCell()) << tr("Compressors");
        *(_tr->addHeaderCell()) << tr("Manufacturer");
        *(_tr->addHeaderCell()) << tr("Type");
        *(_tr->addHeaderCell()) << tr("Serial number");

        for (int i = 0; i < compressors.count(); ++i) {
            _tr = table->addRow();
            *(_tr->addCell()) << compressors.at(i).value("name").toString();
            *(_tr->addCell()) << compressors.at(i).value("manufacturer").toString();
            *(_tr->addCell()) << compressors.at(i).value("type").toString();
            *(_tr->addCell()) << compressors.at(i).value("sn").toString();
        }

        return table;
    }
    return NULL;
}

HTMLTable *CircuitsView::circuitUnitsTable(const QString &customer_id, const QString &circuit_id, HTMLTable *table)
{
    enum QUERY_RESULTS
    {
        SN = 0,
        MANUFACTURER = 1,
        TYPE = 2,
        LOCATION = 3,
        UNIT_TYPE_ID = 4
    };
    MTSqlQuery query;
    query.prepare("SELECT circuit_units.sn, circuit_unit_types.manufacturer,"
                  " circuit_unit_types.type, circuit_unit_types.location, circuit_unit_types.id"
                  " FROM circuit_units"
                  " LEFT JOIN circuit_unit_types ON circuit_units.unit_type_id = circuit_unit_types.id"
                  " WHERE circuit_units.company_id = :customer_id AND circuit_units.circuit_id = :circuit_id");
    query.bindValue(":customer_id", customer_id.toInt());
    query.bindValue(":circuit_id", circuit_id.toInt());
    query.exec();

    if (query.next()) {
        if (!table) table = new HTMLTable("cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\"");
        table->addClass("circuit_units");
        HTMLTableRow *_tr;

        _tr = table->addRow();
        *(_tr->addHeaderCell()) << tr("Circuit units");
        *(_tr->addHeaderCell()) << tr("Manufacturer");
        *(_tr->addHeaderCell()) << tr("Type");
        *(_tr->addHeaderCell()) << tr("Serial number");

        do {
            _tr = table->addRow();
            *(_tr->addCell()) << CircuitUnitType::locationToString(query.value(LOCATION).toInt());
            *(_tr->addCell()) << query.value(MANUFACTURER).toString();
            *(_tr->addCell()) << query.value(TYPE).toString();
            *(_tr->addCell()) << query.value(SN).toString();
        } while (query.next());

        return table;
    }
    return NULL;
}

QString CircuitsView::title() const
{
    return tr("Circuits") + " - " + Customer(settings->selectedCustomer()).stringValue("company");
}
