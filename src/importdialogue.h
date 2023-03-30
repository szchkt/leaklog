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

#ifndef IMPORT_DIALOGUE_H
#define IMPORT_DIALOGUE_H

#include "ui_importdialogue.h"

class ImportDialogue : public QDialog, private Ui::ImportDialogue
{
    Q_OBJECT

public:
    ImportDialogue(QWidget *parent = NULL);
    inline QTreeWidget *newCustomers() { return trw_customers_new; }
    inline QTreeWidget *modifiedCustomers() { return trw_customers_modified; }
    inline QTreeWidget *newPersons() { return trw_persons_new; }
    inline QTreeWidget *modifiedPersons() { return trw_persons_modified; }
    inline QTreeWidget *newCircuits() { return trw_circuits_new; }
    inline QTreeWidget *modifiedCircuits() { return trw_circuits_modified; }
    inline QTreeWidget *newInspections() { return trw_inspections_new; }
    inline QTreeWidget *modifiedInspections() { return trw_inspections_modified; }
    inline QTreeWidget *newRepairs() { return trw_repairs_new; }
    inline QTreeWidget *modifiedRepairs() { return trw_repairs_modified; }
    inline QTreeWidget *newRefrigerantManagement() { return trw_refrigerant_management_new; }
    inline QTreeWidget *modifiedRefrigerantManagement() { return trw_refrigerant_management_modified; }
    inline QTreeWidget *newInspectors() { return trw_inspectors_new; }
    inline QTreeWidget *modifiedInspectors() { return trw_inspectors_modified; }
    inline QTreeWidget *variables() { return trw_variables; }

private slots:
    void selectAllNewCustomers() { setCheckState(trw_customers_new, Qt::Checked); }
    void deselectAllNewCustomers() { setCheckState(trw_customers_new, Qt::Unchecked); }
    void selectAllModifiedCustomers() { setCheckState(trw_customers_modified, Qt::Checked); }
    void deselectAllModifiedCustomers() { setCheckState(trw_customers_modified, Qt::Unchecked); }
    void selectAllNewPersons() { setCheckState(trw_persons_new, Qt::Checked); }
    void deselectAllNewPersons() { setCheckState(trw_persons_new, Qt::Unchecked); }
    void selectAllModifiedPersons() { setCheckState(trw_persons_modified, Qt::Checked); }
    void deselectAllModifiedPersons() { setCheckState(trw_persons_modified, Qt::Unchecked); }
    void selectAllNewCircuits() { setCheckState(trw_circuits_new, Qt::Checked); }
    void deselectAllNewCircuits() { setCheckState(trw_circuits_new, Qt::Unchecked); }
    void selectAllModifiedCircuits() { setCheckState(trw_circuits_modified, Qt::Checked); }
    void deselectAllModifiedCircuits() { setCheckState(trw_circuits_modified, Qt::Unchecked); }
    void selectAllNewInspections() { setCheckState(trw_inspections_new, Qt::Checked); }
    void deselectAllNewInspections() { setCheckState(trw_inspections_new, Qt::Unchecked); }
    void selectAllModifiedInspections() { setCheckState(trw_inspections_modified, Qt::Checked); }
    void deselectAllModifiedInspections() { setCheckState(trw_inspections_modified, Qt::Unchecked); }
    void selectAllNewRepairs() { setCheckState(trw_repairs_new, Qt::Checked); }
    void deselectAllNewRepairs() { setCheckState(trw_repairs_new, Qt::Unchecked); }
    void selectAllModifiedRepairs() { setCheckState(trw_repairs_modified, Qt::Checked); }
    void deselectAllModifiedRepairs() { setCheckState(trw_repairs_modified, Qt::Unchecked); }
    void selectAllNewRefrigerantManagement() { setCheckState(trw_refrigerant_management_new, Qt::Checked); }
    void deselectAllNewRefrigerantManagement() { setCheckState(trw_refrigerant_management_new, Qt::Unchecked); }
    void selectAllModifiedRefrigerantManagement() { setCheckState(trw_refrigerant_management_modified, Qt::Checked); }
    void deselectAllModifiedRefrigerantManagement() { setCheckState(trw_refrigerant_management_modified, Qt::Unchecked); }
    void selectAllNewInspectors() { setCheckState(trw_inspectors_new, Qt::Checked); }
    void deselectAllNewInspectors() { setCheckState(trw_inspectors_new, Qt::Unchecked); }
    void selectAllModifiedInspectors() { setCheckState(trw_inspectors_modified, Qt::Checked); }
    void deselectAllModifiedInspectors() { setCheckState(trw_inspectors_modified, Qt::Unchecked); }

    void customerChanged(QTreeWidgetItem *, int);
    void circuitChanged(QTreeWidgetItem *, int);

private:
    void setCheckState(QTreeWidget *trw, Qt::CheckState state);
};

#endif // IMPORT_DIALOGUE_H
