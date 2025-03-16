/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2025 Matus & Michal Tomlein

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

#ifndef EDIT_CIRCUIT_DIALOGUE_UNITS_TAB_H
#define EDIT_CIRCUIT_DIALOGUE_UNITS_TAB_H

#include "tabbededitdialogue.h"
#include "editdialoguetable.h"

#include <QTreeWidgetItem>

class EditCircuitDialogueTreeItem;
class EditCircuitDialogueTable;

class EditCircuitDialogueUnitsTab : public EditDialogueTab
{
    Q_OBJECT

public:
    EditCircuitDialogueUnitsTab(const QString &circuit_uuid, QWidget * = NULL);

    void save(const QString &circuit_uuid);
    QWidget *widget() { return this; }

private slots:
    void manufacturerItemExpanded(QTreeWidgetItem *);
    void addToTable(EditCircuitDialogueTreeItem *);

signals:
    void updateCircuit(const QVariantMap &);

private:
    void loadRows(const QString &circuit_uuid);
    void loadManufacturers();

    EditCircuitDialogueTable *table;
    QTreeWidget *tree;
    QStringList former_ids;
};

class EditCircuitDialogueTable : public EditDialogueTable
{
    Q_OBJECT

public:
    EditCircuitDialogueTable(const QString &, const QList<EditDialogueTableCell *> &, QWidget *);

private slots:
    void activateRow() {}
    void updateCircuit();

signals:
    void updateCircuit(const QVariantMap &);

private:
    void addHiddenRow(EditDialogueTableRow *) {}
    QList<EditDialogueTableCell *> hiddenAttributes() { return QList<EditDialogueTableCell *>(); }
};

class EditCircuitDialogueTreeItem : public QObject, public QTreeWidgetItem
{
    Q_OBJECT

public:
    EditCircuitDialogueTreeItem(QTreeWidget *parent) : QObject(), QTreeWidgetItem(parent) { is_type = true; }
    EditCircuitDialogueTreeItem(QTreeWidgetItem *parent) : QObject(), QTreeWidgetItem(parent) { is_type = true; }

    void setManufacturer(const QString &manufacturer) { this->unit_manufacturer = manufacturer; }
    const QString &manufacturer() { return unit_manufacturer; }
    void setUnitType(const QString &type) { this->unit_type = type; }
    const QString &unitType() { return unit_type; }
    void setIsType(bool is_type) { this->is_type = is_type; }
    bool isType() { return is_type; }
    void setLocation(int loc) { this->unit_location = loc; }
    int location() { return unit_location; }

public slots:
    void addClicked();

signals:
    void itemWantsToBeAdded(EditCircuitDialogueTreeItem *);

private:
    QString unit_manufacturer;
    QString unit_type;
    bool is_type;
    int unit_location;
};

#endif // EDIT_CIRCUIT_DIALOGUE_UNITS_TAB_H
