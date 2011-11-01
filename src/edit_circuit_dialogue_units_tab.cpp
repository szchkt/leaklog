/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2011 Matus & Michal Tomlein

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

#include "edit_circuit_dialogue_units_tab.h"
#include "global.h"
#include "mtsqlquery.h"

#include <QPushButton>
#include <QToolButton>
#include <QHeaderView>

EditCircuitDialogueUnitsTab::EditCircuitDialogueUnitsTab(const QString & customer_id, const QString & circuit_id, QWidget * parent)
    : EditDialogueTab(parent)
{
    setName(tr("Units"));
    this->customer_id = customer_id;

    QGridLayout * grid = new QGridLayout(this);
    grid->setContentsMargins(9, 9, 9, 9);

    tree = new QTreeWidget(this);
    tree->setColumnCount(3);
    QStringList header_labels;
    header_labels << tr("Available unit types");
    header_labels << tr("Location");
    header_labels << "";
    tree->setHeaderLabels(header_labels);
    tree->setSelectionMode(QAbstractItemView::NoSelection);
    tree->header()->setStretchLastSection(false);
    tree->header()->setResizeMode(0, QHeaderView::Stretch);
    tree->header()->setResizeMode(1, QHeaderView::ResizeToContents);
    tree->header()->setStretchLastSection(false);
    tree->header()->setResizeMode(2, QHeaderView::Custom);
    tree->header()->resizeSection(2, 24);
    tree->setColumnWidth(2, 24);
    QObject::connect(tree, SIGNAL(itemExpanded(QTreeWidgetItem*)), this, SLOT(manufacturerItemExpanded(QTreeWidgetItem*)));
    grid->addWidget(tree, 0, 0);

    QList<EditDialogueTableCell *> header;
    header.append(new EditDialogueTableCell(tr("Manufacturer"), "manufacturer"));
    header.append(new EditDialogueTableCell(tr("Type"), "type"));
    header.append(new EditDialogueTableCell(tr("Location"), "location"));
    header.append(new EditDialogueTableCell(tr("Serial number"), "sn"));

    table = new EditCircuitDialogueTable(tr("Used circuit units"), header, this);
    QObject::connect(table, SIGNAL(updateCircuit(MTDictionary)), this, SIGNAL(updateCircuit(MTDictionary)));

    grid->addWidget(table, 0, 1);

    loadManufacturers();
    loadRows(customer_id, circuit_id);
}

void EditCircuitDialogueUnitsTab::loadRows(const QString & customer_id, const QString & circuit_id)
{
    QMap<QString, EditDialogueTableCell *> cells;

    enum QUERY_RESULTS
    {
        SN = 0,
        MANUFACTURER = 1,
        TYPE = 2,
        LOCATION = 3,
        UNIT_TYPE_ID = 4,
        UNIT_ID = 5
    };
    MTSqlQuery query(QString("SELECT circuit_units.sn, circuit_unit_types.manufacturer, circuit_unit_types.type,"
                            " circuit_unit_types.location, circuit_unit_types.id, circuit_units.id AS unit_id"
                            " FROM circuit_units"
                            " LEFT JOIN circuit_unit_types ON circuit_units.unit_type_id = circuit_unit_types.id"
                            " WHERE circuit_units.company_id = %1 AND circuit_units.circuit_id = %2")
                    .arg(customer_id.toInt()).arg(circuit_id.toInt()));
    while (query.next()) {
        former_ids.append(query.value(UNIT_ID).toInt());
        cells.insert("manufacturer", new EditDialogueTableCell(query.value(MANUFACTURER), "manufacturer"));
        cells.insert("type", new EditDialogueTableCell(query.value(TYPE), "type"));
        cells.insert("unit_type_id", new EditDialogueTableCell(query.value(UNIT_TYPE_ID), "unit_type_id"));
        cells.insert("location", new EditDialogueTableCell(CircuitUnitType::locationToString(query.value(LOCATION).toInt()), "location"));
        cells.insert("sn", new EditDialogueTableCell(query.value(SN), "sn", Global::String));
        cells.insert("id", new EditDialogueTableCell(query.value(UNIT_ID), "id"));
        table->addRow(cells);
    }
}

void EditCircuitDialogueUnitsTab::save(const QVariant & circuit_id)
{
    QList<MTDictionary> all_values = table->allValues();

    int id = 1;
    MTSqlQuery query("SELECT MAX(id) FROM circuit_units");
    if (query.last()) {
        id = query.value(0).toInt() + 1;
    }

    CircuitUnit unit;
    for (int i = 0; i < all_values.count(); ++i) {
        QVariantMap map;

        if (all_values.at(i).contains("id")) {
            unit = CircuitUnit(all_values.at(i).value("id"));
            if (former_ids.contains(all_values.at(i).value("id").toInt()))
                former_ids.removeAll(all_values.at(i).value("id").toInt());
        } else {
            unit = CircuitUnit();
            map.insert("id", QString::number(id++));
        }

        map.insert("company_id", customer_id);
        map.insert("circuit_id", circuit_id);
        map.insert("unit_type_id", all_values.at(i).value("unit_type_id"));
        map.insert("sn", all_values.at(i).value("sn"));
        unit.update(map);
    }

    for (int i = 0; i < former_ids.count(); ++i)
        CircuitUnit(QString::number(former_ids.at(i))).remove();
}

void EditCircuitDialogueUnitsTab::loadManufacturers()
{
    EditCircuitDialogueTreeItem * item;
    MTSqlQuery query("SELECT DISTINCT manufacturer FROM circuit_unit_types ORDER BY manufacturer");
    while (query.next()) {
        item = new EditCircuitDialogueTreeItem(tree);
        item->setText(0, query.value(0).toString());
        item->setIsType(false);
        item->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
    }
}

void EditCircuitDialogueUnitsTab::manufacturerItemExpanded(QTreeWidgetItem * qitem)
{
    EditCircuitDialogueTreeItem * parent_item = (EditCircuitDialogueTreeItem *) qitem;
    EditCircuitDialogueTreeItem * item = NULL;
    if (parent_item->isType() || parent_item->childCount()) return;

    MTSqlQuery query(QString("SELECT id, type, location FROM circuit_unit_types WHERE manufacturer = '%1' ORDER BY type")
                    .arg(parent_item->text(0)));

    while (query.next()) {
        item = new EditCircuitDialogueTreeItem(parent_item);
        item->setText(0, query.value(1).toString());
        item->setText(1, CircuitUnitType::locationToString(query.value(2).toInt()));
        QToolButton * add_btn = new QToolButton;
        add_btn->setIcon(QIcon(QString::fromUtf8(":/images/images/add16.png")));
        QObject::connect(add_btn, SIGNAL(clicked()), item, SLOT(addClicked()));
        tree->setItemWidget(item, 2, add_btn);
        item->setUnitType(query.value(0).toString());
        item->setManufacturer(parent_item->text(0));
        item->setLocation(query.value(2).toInt());
        QObject::connect(item, SIGNAL(itemWantsToBeAdded(EditCircuitDialogueTreeItem*)), this, SLOT(addToTable(EditCircuitDialogueTreeItem*)));
    }
}

void EditCircuitDialogueUnitsTab::addToTable(EditCircuitDialogueTreeItem * item)
{
    QMap<QString, EditDialogueTableCell *> cells;
    cells.insert("manufacturer", new EditDialogueTableCell(item->manufacturer(), "manufacturer"));
    cells.insert("type", new EditDialogueTableCell(item->text(0), "type"));
    cells.insert("unit_type_id", new EditDialogueTableCell(item->unitType(), "unit_type_id"));
    cells.insert("location", new EditDialogueTableCell(CircuitUnitType::locationToString(item->location()), "location"));
    cells.insert("sn", new EditDialogueTableCell(QString(), "sn", Global::String));
    table->addRow(cells);
}

EditCircuitDialogueTable::EditCircuitDialogueTable(const QString & name, const QList<EditDialogueTableCell *> & header, QWidget * parent)
    : EditDialogueTable(name, header, parent)
{
    QPushButton * update_circuit_btn = new QPushButton(tr("Update circuit"), this);
    QObject::connect(update_circuit_btn, SIGNAL(clicked()), this, SLOT(updateCircuit()));

    QHBoxLayout * hlayout = new QHBoxLayout;
    hlayout->addStretch();
    hlayout->addWidget(update_circuit_btn);
    hlayout->setContentsMargins(0, 0, 0, 0);

    layout->addLayout(hlayout);
}

void EditCircuitDialogueTable::updateCircuit()
{
    MTDictionary circuit_vars;
    MTDictionary oils = Global::oils();
    double refr_amount = 0;
    double oil_amount = 0;

    for (int i = 0; i < rows.count(); ++i) {
        CircuitUnitType unit_type_record(rows.at(i)->value("unit_type_id"));
        QVariantMap unit_type = unit_type_record.list();

        if (!unit_type.value("refrigerant").toString().isEmpty())
            circuit_vars.setValue("refrigerant", unit_type.value("refrigerant").toString());
        refr_amount += unit_type.value("refrigerant_amount").toDouble();

        if (!unit_type.value("oil").toString().isEmpty())
            circuit_vars.setValue("oil", oils.key(oils.indexOfValue(unit_type.value("oil").toString())));
        oil_amount += unit_type.value("oil_amount").toDouble();

        if (unit_type.value("location").toInt() == CircuitUnitType::External) {
            circuit_vars.setValue("manufacturer", unit_type.value("manufacturer").toString());
            circuit_vars.setValue("type", unit_type.value("type").toString());
        }
    }

    circuit_vars.setValue("refrigerant_amount", QString::number(refr_amount));
    circuit_vars.setValue("oil_amount", QString::number(oil_amount));

    updateCircuit(circuit_vars);
}

void EditCircuitDialogueTreeItem::addClicked()
{
    emit itemWantsToBeAdded(this);
}
