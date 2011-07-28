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

#ifndef MODIFY_CIRCUIT_DIALOGUE_UNITS_TAB_H
#define MODIFY_CIRCUIT_DIALOGUE_UNITS_TAB_H

#include "tabbed_modify_dialogue.h"
#include "modify_dialogue_table.h"

#include <QTreeWidgetItem>

class ModifyCircuitDialogueTreeItem;
class ModifyCircuitDialogueTable;

class ModifyCircuitDialogueUnitsTab : public ModifyDialogueTab
{
    Q_OBJECT

public:
    ModifyCircuitDialogueUnitsTab(const QString &, const QString &, QWidget * = NULL);

    void save(const QVariant &);
    QWidget * widget() { return this; }

private slots:
    void manufacturerItemExpanded(QTreeWidgetItem *);
    void addToTable(ModifyCircuitDialogueTreeItem *);

signals:
    void updateCircuit(MTDictionary);

private:
    void loadRows(const QString &, const QString &);
    void loadManufacturers();

    ModifyCircuitDialogueTable * table;
    QTreeWidget * tree;
    QString customer_id;
};

class ModifyCircuitDialogueTable : public ModifyDialogueTable
{
    Q_OBJECT

public:
    ModifyCircuitDialogueTable(const QString &, const QList<ModifyDialogueTableCell *> &, QWidget *);

private slots:
    void activateRow() {}
    void updateCircuit();

signals:
    void updateCircuit(MTDictionary);

private:
    void addHiddenRow(ModifyDialogueTableRow *) {}
    QList<ModifyDialogueTableCell *> hiddenAttributes() { return QList<ModifyDialogueTableCell *>(); }
};

class ModifyCircuitDialogueTreeItem : public QObject, public QTreeWidgetItem
{
    Q_OBJECT

public:
    ModifyCircuitDialogueTreeItem(QTreeWidget * parent) : QObject(), QTreeWidgetItem(parent) { is_type = true; }
    ModifyCircuitDialogueTreeItem(QTreeWidgetItem * parent) : QObject(), QTreeWidgetItem(parent) { is_type = true; }

    void setManufacturer(const QString & manufacturer) { this->unit_manufacturer = manufacturer; }
    const QString & manufacturer() { return unit_manufacturer; }
    void setUnitType(const QString & type) { this->unit_type = type; }
    const QString & unitType() { return unit_type; }
    void setIsType(bool is_type) { this->is_type = is_type; }
    bool isType() { return is_type; }
    void setLocation(int loc) { this->unit_location = loc; }
    int location() { return unit_location; }

public slots:
    void addClicked();

signals:
    void itemWantsToBeAdded(ModifyCircuitDialogueTreeItem *);

private:
    QString unit_manufacturer;
    QString unit_type;
    bool is_type;
    int unit_location;
};

#endif // MODIFY_CIRCUIT_DIALOGUE_UNITS_TAB_H
