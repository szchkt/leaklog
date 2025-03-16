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

#include "importdialogue.h"
#include "records.h"

#include <QHeaderView>

ImportDialogue::ImportDialogue(QWidget *parent):
QDialog(parent, Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowMaximizeButtonHint) {
    setupUi(this);
    trw_customers_new->setColumnCount(Customer::attributes().count());
    trw_customers_new->setHeaderItem(new QTreeWidgetItem(Customer::attributes().values()));
    trw_customers_new->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    trw_customers_modified->setColumnCount(Customer::attributes().count());
    trw_customers_modified->setHeaderItem(new QTreeWidgetItem(Customer::attributes().values()));
    trw_customers_modified->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    QStringList persons_header_items = Person::attributes().values();
    persons_header_items.prepend(QApplication::translate("Customer", "Customer"));
    trw_persons_new->setColumnCount(Person::attributes().count());
    trw_persons_new->setHeaderItem(new QTreeWidgetItem(persons_header_items));
    trw_persons_new->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    trw_persons_modified->setColumnCount(Person::attributes().count());
    trw_persons_modified->setHeaderItem(new QTreeWidgetItem(persons_header_items));
    trw_persons_modified->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    trw_circuits_new->setColumnCount(Circuit::attributes().count() + 1);
    trw_circuits_new->setHeaderItem(new QTreeWidgetItem(QStringList()
                                                        << QApplication::translate("Customer", "Customer")
                                                        << Circuit::attributes().values()));
    trw_circuits_new->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    trw_circuits_modified->setColumnCount(Circuit::attributes().count() + 1);
    trw_circuits_modified->setHeaderItem(new QTreeWidgetItem(QStringList()
                                                             << QApplication::translate("Customer", "Customer")
                                                             << Circuit::attributes().values()));
    trw_circuits_modified->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    trw_inspections_new->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    trw_inspections_modified->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    trw_repairs_new->setColumnCount(Repair::attributes().count());
    trw_repairs_new->setHeaderItem(new QTreeWidgetItem(Repair::attributes().values()));
    trw_repairs_new->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    trw_repairs_modified->setColumnCount(Repair::attributes().count());
    trw_repairs_modified->setHeaderItem(new QTreeWidgetItem(Repair::attributes().values()));
    trw_repairs_modified->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    trw_refrigerant_management_new->setColumnCount(RefrigerantRecord::attributes().count());
    trw_refrigerant_management_new->setHeaderItem(new QTreeWidgetItem(RefrigerantRecord::attributes().values()));
    trw_refrigerant_management_new->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    trw_refrigerant_management_modified->setColumnCount(RefrigerantRecord::attributes().count());
    trw_refrigerant_management_modified->setHeaderItem(new QTreeWidgetItem(RefrigerantRecord::attributes().values()));
    trw_refrigerant_management_modified->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    trw_inspectors_new->setColumnCount(Inspector::attributes().count());
    trw_inspectors_new->setHeaderItem(new QTreeWidgetItem(Inspector::attributes().values()));
    trw_inspectors_new->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    trw_inspectors_modified->setColumnCount(Inspector::attributes().count());
    trw_inspectors_modified->setHeaderItem(new QTreeWidgetItem(Inspector::attributes().values()));
    trw_inspectors_modified->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    trw_variables->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    trw_variables->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    trw_variables->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    trw_variables->header()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    trw_variables->header()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    //trw_variables->header()->setSectionResizeMode(5, QHeaderView::ResizeToContents);
    trw_variables->header()->setSectionResizeMode(6, QHeaderView::ResizeToContents);
    trw_variables->header()->setSectionResizeMode(7, QHeaderView::ResizeToContents);
    trw_variables->header()->setSectionResizeMode(8, QHeaderView::ResizeToContents);
    trw_variables->header()->setSectionResizeMode(9, QHeaderView::ResizeToContents);
    QObject::connect(tbtn_customers_new_selectall, SIGNAL(clicked()), this, SLOT(selectAllNewCustomers()));
    QObject::connect(tbtn_customers_new_selectnone, SIGNAL(clicked()), this, SLOT(deselectAllNewCustomers()));
    QObject::connect(tbtn_customers_modified_selectall, SIGNAL(clicked()), this, SLOT(selectAllModifiedCustomers()));
    QObject::connect(tbtn_customers_modified_selectnone, SIGNAL(clicked()), this, SLOT(deselectAllModifiedCustomers()));
    QObject::connect(tbtn_persons_new_selectall, SIGNAL(clicked()), this, SLOT(selectAllNewPersons()));
    QObject::connect(tbtn_persons_new_selectnone, SIGNAL(clicked()), this, SLOT(deselectAllNewPersons()));
    QObject::connect(tbtn_persons_modified_selectall, SIGNAL(clicked()), this, SLOT(selectAllModifiedPersons()));
    QObject::connect(tbtn_persons_modified_selectnone, SIGNAL(clicked()), this, SLOT(deselectAllModifiedPersons()));
    QObject::connect(tbtn_circuits_new_selectall, SIGNAL(clicked()), this, SLOT(selectAllNewCircuits()));
    QObject::connect(tbtn_circuits_new_selectnone, SIGNAL(clicked()), this, SLOT(deselectAllNewCircuits()));
    QObject::connect(tbtn_circuits_modified_selectall, SIGNAL(clicked()), this, SLOT(selectAllModifiedCircuits()));
    QObject::connect(tbtn_circuits_modified_selectnone, SIGNAL(clicked()), this, SLOT(deselectAllModifiedCircuits()));
    QObject::connect(tbtn_inspections_new_selectall, SIGNAL(clicked()), this, SLOT(selectAllNewInspections()));
    QObject::connect(tbtn_inspections_new_selectnone, SIGNAL(clicked()), this, SLOT(deselectAllNewInspections()));
    QObject::connect(tbtn_inspections_modified_selectall, SIGNAL(clicked()), this, SLOT(selectAllModifiedInspections()));
    QObject::connect(tbtn_inspections_modified_selectnone, SIGNAL(clicked()), this, SLOT(deselectAllModifiedInspections()));
    QObject::connect(tbtn_repairs_new_selectall, SIGNAL(clicked()), this, SLOT(selectAllNewRepairs()));
    QObject::connect(tbtn_repairs_new_selectnone, SIGNAL(clicked()), this, SLOT(deselectAllNewRepairs()));
    QObject::connect(tbtn_repairs_modified_selectall, SIGNAL(clicked()), this, SLOT(selectAllModifiedRepairs()));
    QObject::connect(tbtn_repairs_modified_selectnone, SIGNAL(clicked()), this, SLOT(deselectAllModifiedRepairs()));
    QObject::connect(tbtn_inspectors_new_selectall, SIGNAL(clicked()), this, SLOT(selectAllNewInspectors()));
    QObject::connect(tbtn_inspectors_new_selectnone, SIGNAL(clicked()), this, SLOT(deselectAllNewInspectors()));
    QObject::connect(tbtn_inspectors_modified_selectall, SIGNAL(clicked()), this, SLOT(selectAllModifiedInspectors()));
    QObject::connect(tbtn_inspectors_modified_selectnone, SIGNAL(clicked()), this, SLOT(deselectAllModifiedInspectors()));
    QObject::connect(tbtn_refrigerant_management_new_selectall, SIGNAL(clicked()), this, SLOT(selectAllNewRefrigerantManagement()));
    QObject::connect(tbtn_refrigerant_management_new_selectnone, SIGNAL(clicked()), this, SLOT(deselectAllNewRefrigerantManagement()));
    QObject::connect(tbtn_refrigerant_management_modified_selectall, SIGNAL(clicked()), this, SLOT(selectAllModifiedRefrigerantManagement()));
    QObject::connect(tbtn_refrigerant_management_modified_selectnone, SIGNAL(clicked()), this, SLOT(deselectAllModifiedRefrigerantManagement()));
    QObject::connect(trw_customers_new, SIGNAL(itemChanged(QTreeWidgetItem *, int)), this, SLOT(customerChanged(QTreeWidgetItem *, int)));
    QObject::connect(trw_customers_modified, SIGNAL(itemChanged(QTreeWidgetItem *, int)), this, SLOT(customerChanged(QTreeWidgetItem *, int)));
    QObject::connect(trw_circuits_new, SIGNAL(itemChanged(QTreeWidgetItem *, int)), this, SLOT(circuitChanged(QTreeWidgetItem *, int)));
    QObject::connect(trw_circuits_modified, SIGNAL(itemChanged(QTreeWidgetItem *, int)), this, SLOT(circuitChanged(QTreeWidgetItem *, int)));
}

void ImportDialogue::setCheckState(QTreeWidget *trw, Qt::CheckState state) {
    QApplication::setOverrideCursor(Qt::WaitCursor);
    for (int i = 0; i < trw->topLevelItemCount(); ++i) {
        if (!trw->topLevelItem(i)->isDisabled())
            trw->topLevelItem(i)->setCheckState(0, state);
    }
    QApplication::restoreOverrideCursor();
}

void ImportDialogue::customerChanged(QTreeWidgetItem *item, int column)
{
    if (column) return;
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QString uuid = item->data(0, Qt::UserRole).toString();
    Qt::CheckState checked = item->checkState(0);
    bool modified = item->treeWidget() == trw_customers_modified;
    QList<QTreeWidget *> trws;
    trws << trw_circuits_new;
    if (modified)
        trws << trw_circuits_modified;
    foreach (QTreeWidget *trw, trws) {
        for (int i = 0; i < trw->topLevelItemCount(); ++i) {
            item = trw->topLevelItem(i);
            if (item->data(1, Qt::UserRole).toString() == uuid) {
                item->setCheckState(0, checked);
                item->setDisabled(!modified && checked == Qt::Unchecked);
            }
        }
    }
    trws.clear();
    trws << trw_inspections_new;
    if (modified)
        trws << trw_inspections_modified;
    foreach (QTreeWidget *trw, trws) {
        for (int i = 0; i < trw->topLevelItemCount(); ++i) {
            item = trw->topLevelItem(i);
            if (item->data(1, Qt::UserRole).toString() == uuid) {
                item->setCheckState(0, checked);
                item->setDisabled(!modified && checked == Qt::Unchecked);
            }
        }
    }
    QApplication::restoreOverrideCursor();
}

void ImportDialogue::circuitChanged(QTreeWidgetItem *item, int column)
{
    if (column) return;
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QString uuid = item->data(0, Qt::UserRole).toString();
    Qt::CheckState checked = item->checkState(0);
    bool modified = item->treeWidget() == trw_circuits_modified;
    QList<QTreeWidget *> trws;
    trws << trw_inspections_new;
    if (modified)
        trws << trw_inspections_modified;
    foreach (QTreeWidget *trw, trws) {
        for (int i = 0; i < trw->topLevelItemCount(); ++i) {
            item = trw->topLevelItem(i);
            if (item->data(2, Qt::UserRole).toString() == uuid) {
                item->setCheckState(0, checked);
                item->setDisabled(!modified && checked == Qt::Unchecked);
            }
        }
    }
    QApplication::restoreOverrideCursor();
}
