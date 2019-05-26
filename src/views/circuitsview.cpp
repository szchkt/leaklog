/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2019 Matus & Michal Tomlein

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

QString CircuitsView::renderHTML(bool)
{
    QString customer_uuid = settings->selectedCustomerUUID();
    HTMLMain main;
    writeServiceCompany(main);
    main << writeCustomersTable(customer_uuid);
    if (settings->mainWindowSettings().customerDetailsVisible()) {
        main.newLine();
        main << customerContactPersons(customer_uuid);
    }
    main.newLine();
    main << writeCircuitsTable(customer_uuid);
    return viewTemplate("customer").arg(main.html());
}

void CircuitsView::writeCircuitsTable(MTTextStream &out, const QString &customer_uuid, const QString &circuit_uuid, int cols_in_row, bool title)
{
    HTMLDiv *div = writeCircuitsTable(customer_uuid, circuit_uuid, cols_in_row, title);
    out << div->html();
    delete div;
}

HTMLDiv *CircuitsView::writeCircuitsTable(const QString &customer_uuid, const QString &circuit_uuid, int cols_in_row, bool title, HTMLTable *table)
{
    bool disable_hiding_details = settings->currentView() == View::AssemblyRecordDetails;
    bool circuits_details_visible = settings->mainWindowSettings().circuitDetailsVisible() || disable_hiding_details;
    bool circuits_visible = settings->mainWindowSettings().circuitsVisible();
    bool excluded_circuits_visible = settings->mainWindowSettings().excludedCircuitsVisible();
    bool decommissioned_circuits_visible = settings->mainWindowSettings().decommissionedCircuitsVisible();
    CircuitsColumns columns;
    columns.date_updated = title && settings->isShowDateUpdatedChecked() && !disable_hiding_details;
    columns.owner = title && settings->isShowOwnerChecked() && !disable_hiding_details;
    bool all_circuits = circuit_uuid.isEmpty();
    columns.notes = title && settings->isShowNotesChecked() && !all_circuits && cols_in_row > 0;
    columns.operation = cols_in_row > 0 || settings->isShowPlaceOfOperationChecked();
    columns.building = cols_in_row > 0 || settings->isShowBuildingChecked();
    columns.device = cols_in_row > 0 || settings->isShowDeviceChecked();
    columns.manufacturer = cols_in_row > 0 || settings->isShowManufacturerChecked();
    columns.type = cols_in_row > 0 || settings->isShowTypeChecked();
    columns.sn = cols_in_row > 0 || settings->isShowSerialNumberChecked();
    columns.year = cols_in_row > 0 || settings->isShowYearOfPurchaseChecked();
    columns.commissioning = cols_in_row > 0 || settings->isShowDateOfCommissioningChecked();
    columns.field = cols_in_row > 0 || settings->isShowFieldOfApplicationChecked();
    columns.oil = cols_in_row > 0 || settings->isShowOilChecked();
    columns.star = settings->currentView() != View::TableOfInspections;

    MTQuery circuits_query = Circuit::query();
    circuits_query.parents().insert("customer_uuid", customer_uuid);
    if (!all_circuits) {
        circuits_query.parents().insert("uuid", circuit_uuid);
    } else {
        if (!settings->toolBarStack()->isFilterEmpty()) {
            circuits_query.addFilter(settings->toolBarStack()->filterColumn(), settings->toolBarStack()->filterKeyword());
        }
        if (settings->toolBarStack()->starredOnly()) {
            circuits_query.addFilter("starred <> ?", "0");
        }
    }
    QString circuits_query_select = "circuits.*, (SELECT uuid FROM inspections"
        " WHERE inspections.circuit_uuid = circuits.uuid ORDER BY date DESC LIMIT 1) AS last_inspection_uuid, (SELECT date FROM inspections"
        " WHERE inspections.circuit_uuid = circuits.uuid ORDER BY date DESC LIMIT 1) AS last_inspection_date, " + circuitRefrigerantAmountQuery();
    QString order_by = settings->mainWindowSettings().orderByForView(LinkParser::Customer);
    if (order_by.isEmpty())
        order_by = "id";
    else if (order_by == "refrigerant")
        order_by = settings->appendDefaultOrderToColumn(order_by) + ", refrigerant_amount";
    else if (order_by == "oil")
        order_by = settings->appendDefaultOrderToColumn(order_by) + ", oil_amount";
    ListOfVariantMaps circuits = circuits_query.listAll(circuits_query_select,
                                                        all_circuits ? settings->appendDefaultOrderToColumn(order_by) : QString());

    if (columns.notes && (circuits.isEmpty() || circuits.first().value("notes").toString().isEmpty()))
        columns.notes = false;

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
    if (!table) table = new HTMLTable(title ? "cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\"" : QString());
    table->addClass("circuits");
    if (all_circuits)
        table->addClass("highlight");

    HTMLTableRow *thead = NULL;

    if (all_circuits || circuits_details_visible) {
        thead = new HTMLTableRow();
        writeCircuitsHeader(customer_uuid, circuit_uuid, cols_in_row, columns, !all_circuits && circuits.count() ? circuits.first().value("disused").toInt() : 0, thead);
    }

    HTMLTableRow *_tr = table->addRow();
    if (title) {
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
    }

    if (all_circuits ? circuits_visible : circuits_details_visible) {
        *table << thead;

        foreach (const QVariantMap &circuit, circuits) {
            if (all_circuits && circuit.value("disused").toInt() != Circuit::Commissioned) continue;
            writeCircuitRow(circuit, customer_uuid, circuit_uuid, cols_in_row, columns, table);
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
            writeCircuitsHeader(customer_uuid, circuit_uuid, cols_in_row, columns, Circuit::ExcludedFromAgenda, thead);
        } else {
            thead = NULL;
        }

        HTMLTableCell *_td = _tr->addHeaderCell(QString("colspan=\"%1\" style=\"font-size: medium;\"").arg(thead ? thead->childCount() : 1));
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
                writeCircuitRow(circuit, customer_uuid, circuit_uuid, cols_in_row, columns, table);
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
            writeCircuitsHeader(customer_uuid, circuit_uuid, cols_in_row, columns, Circuit::Decommissioned, thead);
        } else {
            thead = NULL;
        }

        HTMLTableCell *_td = _tr->addHeaderCell(QString("colspan=\"%1\" style=\"font-size: medium;\"").arg(thead ? thead->childCount() : 1));
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
                writeCircuitRow(circuit, customer_uuid, circuit_uuid, cols_in_row, columns, table);
            }
        }
        *div << table;
    }

    return div;
}

static void addCircuitHeaderCell(const QString &key, const QString &title, const QString &customer_uuid, const QString &circuit_uuid, HTMLTableRow *thead)
{
    if (circuit_uuid.isEmpty())
        *(thead->addHeaderCell()) << "<a href=\"customer:" << customer_uuid << "/order_by:"
                                  << key << "\">" << title << "</a>";
    else
        *(thead->addHeaderCell()) << title;
}

static void addCircuitHeaderCell(const QString &key, const QString &customer_uuid, const QString &circuit_uuid, HTMLTableRow *thead)
{
    addCircuitHeaderCell(key, Circuit::attributes().value(key).split("||").first(), customer_uuid, circuit_uuid, thead);
}

void CircuitsView::writeCircuitsHeader(const QString &customer_uuid, const QString &circuit_uuid, int cols_in_row, CircuitsColumns columns, int disused, HTMLTableRow *thead)
{
    addCircuitHeaderCell("id", customer_uuid, circuit_uuid, thead);
    addCircuitHeaderCell("name", customer_uuid, circuit_uuid, thead);
    if (columns.operation)
        addCircuitHeaderCell("operation", customer_uuid, circuit_uuid, thead);
    if (columns.building)
        addCircuitHeaderCell("building", customer_uuid, circuit_uuid, thead);
    if (columns.device)
        addCircuitHeaderCell("device", customer_uuid, circuit_uuid, thead);
    if (columns.manufacturer)
        addCircuitHeaderCell("manufacturer", customer_uuid, circuit_uuid, thead);
    if (columns.type)
        addCircuitHeaderCell("type", customer_uuid, circuit_uuid, thead);
    if (columns.sn)
        addCircuitHeaderCell("sn", customer_uuid, circuit_uuid, thead);
    if (columns.year)
        addCircuitHeaderCell("year", customer_uuid, circuit_uuid, thead);
    if (columns.commissioning)
        addCircuitHeaderCell("commissioning", customer_uuid, circuit_uuid, thead);
    if (columns.field)
        addCircuitHeaderCell("field", customer_uuid, circuit_uuid, thead);
    addCircuitHeaderCell("refrigerant", customer_uuid, circuit_uuid, thead);
    *(thead->addHeaderCell()) << QApplication::translate("MainWindow", "CO\342\202\202 equivalent");
    if (cols_in_row >= 0) {
        *(thead->addHeaderCell()) << QApplication::translate("MainWindow", "GWP");
    }
    if (columns.oil)
        addCircuitHeaderCell("oil", customer_uuid, circuit_uuid, thead);
    switch (disused) {
        case Circuit::ExcludedFromAgenda:
            addCircuitHeaderCell("decommissioning", QApplication::translate("Circuit", "Date excluded"), customer_uuid, circuit_uuid, thead);
            break;
        case Circuit::Decommissioned:
            addCircuitHeaderCell("decommissioning", Circuit::attributes().value("decommissioning"), customer_uuid, circuit_uuid, thead);
            break;
        default:
            addCircuitHeaderCell("last_inspection_date", tr("Last inspection"), customer_uuid, circuit_uuid, thead);
            break;
    }
    if (columns.date_updated) {
        addCircuitHeaderCell("date_updated", tr("Date Updated"), customer_uuid, circuit_uuid, thead);
    }
    if (columns.owner) {
        addCircuitHeaderCell("updated_by", tr("Author"), customer_uuid, circuit_uuid, thead);
    }
    if (columns.notes) {
        *(thead->addHeaderCell(QString("colspan=\"%1\"").arg(cols_in_row > 0 ? cols_in_row : 1))) << tr("Notes");
    }
}

void CircuitsView::writeCircuitRow(const QVariantMap &circuit, const QString &customer_uuid, const QString &circuit_uuid, int cols_in_row, CircuitsColumns columns, HTMLTable *table)
{
    QString uuid = circuit.value("uuid").toString();
    QString id = circuit.value("id").toString().rightJustified(5, '0');
    QString tr_attr;
    if (circuit_uuid.isEmpty()) {
        tr_attr = QString("id=\"%2\" onclick=\"window.location = 'customer:%1/%2'\" style=\"cursor: pointer;\"").arg(customer_uuid).arg("circuit:" + uuid);
        if (uuid == settings->selectedCircuitUUID())
            tr_attr.append(" class=\"selected\"");
    }
    HTMLTableRow *_tr = table->addRow(tr_attr);
    HTMLTableCell *_td = _tr->addCell();
    if (columns.star) {
        *_td << QString("<span class=\"screen_only\"><a href=\"customer:%1/circuit:%2/star\" class=\"no_underline\">%3</a>&nbsp;</span>")
            .arg(customer_uuid).arg(uuid)
            .arg(QChar(circuit.value("starred").toInt() ? 0x2605 : 0x2606));
    }
    *_td << toolTipLink("customer/circuit", id, customer_uuid, uuid);
    *(_tr->addCell("class=\"wrap\"")) << ellipsis(circuit.value("name"));
    if (columns.operation)
        *(_tr->addCell("class=\"wrap\"")) << ellipsis(circuit.value("operation"));
    if (columns.building)
        *(_tr->addCell("class=\"wrap\"")) << ellipsis(circuit.value("building"));
    if (columns.device)
        *(_tr->addCell("class=\"wrap\"")) << ellipsis(circuit.value("device"));
    if (columns.manufacturer)
        *(_tr->addCell("class=\"wrap\"")) << ellipsis(circuit.value("manufacturer"));
    if (columns.type)
        *(_tr->addCell("class=\"wrap\"")) << ellipsis(circuit.value("type"));
    if (columns.sn)
        *(_tr->addCell("class=\"wrap\"")) << ellipsis(circuit.value("sn"));
    if (columns.year)
        *(_tr->addCell()) << escapeString(circuit.value("year").toString());
    if (columns.commissioning)
        *(_tr->addCell()) << settings->mainWindowSettings().formatDate(circuit.value("commissioning").toString());
    if (columns.field) {
        QString attr_value = circuit.value("field").toString();
        attr_value = attributeValues().value("field::" + attr_value, attr_value);
        *(_tr->addCell("class=\"wrap\"")) << ellipsis(attr_value);
    }
    _td = _tr->addCell();
    *_td << circuit.value("refrigerant_amount").toDouble() << "&nbsp;" << QApplication::translate("Units", "kg");
    QString refrigerant = circuit.value("refrigerant").toString();
    *_td << " " << refrigerant;
    _td = _tr->addCell();
    double GWP = refrigerantGWP(refrigerant);
    double CO2_equivalent = circuit.value("refrigerant_amount").toDouble() * GWP / 1000.0;
    *_td << CO2_equivalent << "&nbsp;" << QApplication::translate("Units", "t");
    if (cols_in_row >= 0) {
        _td = _tr->addCell();
        *_td << GWP;
    }
    if (columns.oil) {
        _td = _tr->addCell();
        *_td << circuit.value("oil_amount").toDouble() << "&nbsp;" << QApplication::translate("Units", "kg");
        *_td << " " << circuit.value("oil").toString().toUpper();
    }
    if (circuit.value("disused").toInt() == Circuit::Commissioned) {
        *(_tr->addCell()->link(QString("customer:%1/circuit:%2/inspection:%3")
                               .arg(customer_uuid)
                               .arg(uuid)
                               .arg(circuit.value("last_inspection_uuid").toString())))
            << settings->mainWindowSettings().formatDate(circuit.value("last_inspection_date").toString().split('-').first());
    } else {
        *(_tr->addCell()) << settings->mainWindowSettings().formatDate(circuit.value("decommissioning"));
    }
    QString align = columns.notes ? "style=\"vertical-align: top;\"" : QString();
    if (columns.date_updated)
        *(_tr->addCell(align)) << settings->mainWindowSettings().formatDateTime(circuit.value("date_updated"));
    if (columns.owner)
        *(_tr->addCell(align)) << escapeString(circuit.value("updated_by"));
    if (columns.notes)
        *(_tr->addCell(QString("colspan=\"%1\"").arg(cols_in_row > 0 ? cols_in_row : 1))) << escapeString(circuit.value("notes").toString(), false, true);
}

HTMLTable *CircuitsView::circuitCompressorsTable(const QString &circuit_uuid, HTMLTable *table)
{
    ListOfVariantMaps compressors = Compressor::query({{"circuit_uuid", circuit_uuid}}).listAll();
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

HTMLTable *CircuitsView::circuitUnitsTable(const QString &circuit_uuid, HTMLTable *table)
{
    enum QUERY_RESULTS {
        SN,
        MANUFACTURER,
        TYPE,
        LOCATION
    };

    MTSqlQuery query;
    query.prepare("SELECT circuit_units.sn, circuit_unit_types.manufacturer,"
                  " circuit_unit_types.type, circuit_unit_types.location"
                  " FROM circuit_units"
                  " LEFT JOIN circuit_unit_types ON circuit_units.unit_type_uuid = circuit_unit_types.uuid"
                  " WHERE circuit_units.circuit_uuid = :circuit_uuid");
    query.bindValue(":circuit_uuid", circuit_uuid);
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
    return tr("Circuits") + " - " + Customer(settings->selectedCustomerUUID()).companyName();
}
