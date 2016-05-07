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

#include "editcircuitdialogueunitstab.h"
#include "global.h"
#include "mtsqlquery.h"

#include <QPushButton>
#include <QToolButton>
#include <QHeaderView>

EditCircuitDialogueUnitsTab::EditCircuitDialogueUnitsTab(const QString &circuit_uuid, QWidget *parent)
    : EditDialogueTab(parent)
{
    setName(tr("Units"));

    QGridLayout *grid = new QGridLayout(this);
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
    tree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    tree->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    tree->header()->setStretchLastSection(false);
    tree->header()->setSectionResizeMode(2, QHeaderView::Custom);
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
    loadRows(circuit_uuid);
}

void EditCircuitDialogueUnitsTab::loadRows(const QString &circuit_uuid)
{
    QMap<QString, EditDialogueTableCell *> cells;

    enum QUERY_RESULTS {
        SN,
        MANUFACTURER,
        TYPE,
        LOCATION,
        UNIT_TYPE_UUID,
        UNIT_UUID
    };

    MTSqlQuery query;
    query.prepare("SELECT circuit_units.sn, circuit_unit_types.manufacturer, circuit_unit_types.type,"
                  " circuit_unit_types.location, circuit_unit_types.uuid, circuit_units.uuid AS unit_uuid"
                  " FROM circuit_units"
                  " LEFT JOIN circuit_unit_types ON circuit_units.unit_type_uuid = circuit_unit_types.uuid"
                  " WHERE circuit_units.circuit_uuid = :circuit_uuid");
    query.bindValue(":circuit_uuid", circuit_uuid);
    query.exec();

    while (query.next()) {
        former_ids.append(query.value(UNIT_UUID).toString());
        cells.insert("manufacturer", new EditDialogueTableCell(query.value(MANUFACTURER), "manufacturer"));
        cells.insert("type", new EditDialogueTableCell(query.value(TYPE), "type"));
        cells.insert("unit_type_uuid", new EditDialogueTableCell(query.value(UNIT_TYPE_UUID), "unit_type_uuid"));
        cells.insert("location", new EditDialogueTableCell(CircuitUnitType::locationToString(query.value(LOCATION).toInt()), "location"));
        cells.insert("sn", new EditDialogueTableCell(query.value(SN), "sn", Global::String));
        cells.insert("uuid", new EditDialogueTableCell(query.value(UNIT_UUID), "uuid"));
        table->addRow(cells);
    }
}

void EditCircuitDialogueUnitsTab::save(const QString &circuit_uuid)
{
    QList<MTDictionary> all_values = table->allValues();

    CircuitUnit unit;
    for (int i = 0; i < all_values.count(); ++i) {
        if (all_values.at(i).contains("uuid")) {
            QString uuid = all_values.at(i).value("uuid");
            former_ids.removeAll(uuid);
            unit = CircuitUnit(uuid);
        } else {
            unit = CircuitUnit();
            unit.setCircuitUUID(circuit_uuid);
        }

        unit.setUnitTypeUUID(all_values.at(i).value("unit_type_uuid"));
        unit.setSerialNumber(all_values.at(i).value("sn"));
        unit.save();
    }

    for (int i = 0; i < former_ids.count(); ++i)
        CircuitUnit(former_ids.at(i)).remove();
}

void EditCircuitDialogueUnitsTab::loadManufacturers()
{
    EditCircuitDialogueTreeItem *item;
    MTSqlQuery query("SELECT DISTINCT manufacturer FROM circuit_unit_types ORDER BY manufacturer");
    while (query.next()) {
        item = new EditCircuitDialogueTreeItem(tree);
        item->setText(0, query.value(0).toString());
        item->setIsType(false);
        item->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
    }
}

void EditCircuitDialogueUnitsTab::manufacturerItemExpanded(QTreeWidgetItem *qitem)
{
    EditCircuitDialogueTreeItem *parent_item = (EditCircuitDialogueTreeItem *) qitem;
    EditCircuitDialogueTreeItem *item = NULL;
    if (parent_item->isType() || parent_item->childCount()) return;

    MTSqlQuery query;
    query.prepare("SELECT uuid, type, location FROM circuit_unit_types WHERE manufacturer = :manufacturer ORDER BY type");
    query.bindValue(":manufacturer", parent_item->text(0));
    query.exec();

    while (query.next()) {
        item = new EditCircuitDialogueTreeItem(parent_item);
        item->setText(0, query.value(1).toString());
        item->setText(1, CircuitUnitType::locationToString(query.value(2).toInt()));
        QToolButton *add_btn = new QToolButton;
        add_btn->setIcon(QIcon(QString::fromUtf8(":/images/images/add16.png")));
        QObject::connect(add_btn, SIGNAL(clicked()), item, SLOT(addClicked()));
        tree->setItemWidget(item, 2, add_btn);
        item->setUnitType(query.value(0).toString());
        item->setManufacturer(parent_item->text(0));
        item->setLocation(query.value(2).toInt());
        QObject::connect(item, SIGNAL(itemWantsToBeAdded(EditCircuitDialogueTreeItem*)), this, SLOT(addToTable(EditCircuitDialogueTreeItem*)));
    }
}

void EditCircuitDialogueUnitsTab::addToTable(EditCircuitDialogueTreeItem *item)
{
    QMap<QString, EditDialogueTableCell *> cells;
    cells.insert("manufacturer", new EditDialogueTableCell(item->manufacturer(), "manufacturer"));
    cells.insert("type", new EditDialogueTableCell(item->text(0), "type"));
    cells.insert("unit_type_uuid", new EditDialogueTableCell(item->unitType(), "unit_type_uuid"));
    cells.insert("location", new EditDialogueTableCell(CircuitUnitType::locationToString(item->location()), "location"));
    cells.insert("sn", new EditDialogueTableCell(QString(), "sn", Global::String));
    table->addRow(cells);
}

EditCircuitDialogueTable::EditCircuitDialogueTable(const QString &name, const QList<EditDialogueTableCell *> &header, QWidget *parent)
    : EditDialogueTable(name, header, parent)
{
    QPushButton *update_circuit_btn = new QPushButton(tr("Update circuit"), this);
    QObject::connect(update_circuit_btn, SIGNAL(clicked()), this, SLOT(updateCircuit()));

    QHBoxLayout *hlayout = new QHBoxLayout;
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
        CircuitUnitType unit_type(rows.at(i)->value("unit_type_uuid"));

        if (!unit_type.refrigerant().isEmpty())
            circuit_vars.setValue("refrigerant", unit_type.refrigerant());
        refr_amount += unit_type.refrigerantAmount();

        if (!unit_type.oil().isEmpty())
            circuit_vars.setValue("oil", oils.value(unit_type.oil()));
        oil_amount += unit_type.oilAmount();

        if (unit_type.location() == CircuitUnitType::External) {
            circuit_vars.setValue("manufacturer", unit_type.manufacturer());
            circuit_vars.setValue("type", unit_type.type());
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
