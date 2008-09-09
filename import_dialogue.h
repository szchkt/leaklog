/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008 Matus & Michal Tomlein

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

#include "ui_import_dialogue.h"

#include <QHeaderView>

class ImportDialogue : public QDialog, private Ui::ImportDialogue
{
    Q_OBJECT

public:
    ImportDialogue(QWidget * parent = NULL):
    QDialog(parent) {
        setupUi(this);
        id_trw_variables->header()->setResizeMode(0, QHeaderView::Stretch);
        id_trw_variables->header()->setResizeMode(1, QHeaderView::ResizeToContents);
        id_trw_variables->header()->setResizeMode(2, QHeaderView::ResizeToContents);
        id_trw_variables->header()->setResizeMode(3, QHeaderView::ResizeToContents);
        //id_trw_variables->header()->setResizeMode(4, QHeaderView::ResizeToContents);
        id_trw_variables->header()->setResizeMode(5, QHeaderView::ResizeToContents);
        id_trw_variables->header()->setResizeMode(6, QHeaderView::ResizeToContents);
        id_trw_variables->header()->setResizeMode(7, QHeaderView::ResizeToContents);
        QObject::connect(id_le_search_customers, SIGNAL(textChanged(QLineEdit *, const QString &)), id_lw_customers, SLOT(filterItems(QLineEdit *, const QString &)));
        QObject::connect(id_le_search_circuits, SIGNAL(textChanged(QLineEdit *, const QString &)), id_lw_circuits, SLOT(filterItems(QLineEdit *, const QString &)));
        QObject::connect(id_le_search_inspections, SIGNAL(textChanged(QLineEdit *, const QString &)), id_lw_inspections, SLOT(filterItems(QLineEdit *, const QString &)));
        QObject::connect(id_tbtn_customers_selectall, SIGNAL(clicked()), this, SLOT(selectAllCustomers()));
        QObject::connect(id_tbtn_circuits_selectall, SIGNAL(clicked()), this, SLOT(selectAllCircuits()));
        QObject::connect(id_tbtn_inspections_selectall, SIGNAL(clicked()), this, SLOT(selectAllInspections()));
        QObject::connect(id_tbtn_customers_selectnone, SIGNAL(clicked()), this, SLOT(deselectAllCustomers()));
        QObject::connect(id_tbtn_circuits_selectnone, SIGNAL(clicked()), this, SLOT(deselectAllCircuits()));
        QObject::connect(id_tbtn_inspections_selectnone, SIGNAL(clicked()), this, SLOT(deselectAllInspections()));
        QObject::connect(id_lw_customers, SIGNAL(itemClicked(QListWidgetItem *)), this, SLOT(showCircuits(QListWidgetItem *)));
        QObject::connect(id_lw_circuits, SIGNAL(itemClicked(QListWidgetItem *)), this, SLOT(showInspections(QListWidgetItem *)));
        item_flags = Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
    };
    inline MTListWidget * customers() { return id_lw_customers; };
    inline MTListWidget * circuits() { return id_lw_circuits; };
    inline MTListWidget * inspections() { return id_lw_inspections; };
    inline QTreeWidget * variables() { return id_trw_variables; };

private slots:
    void showCircuits(QListWidgetItem * item) {
        bool hide = item->checkState() == Qt::Unchecked;
        for (int i = 0; i < id_lw_circuits->count(); ++i) {
            if (id_lw_circuits->item(i)->data(Qt::UserRole).toString().startsWith(QString("%1::").arg(item->data(Qt::UserRole).toString()))) {
                id_lw_circuits->item(i)->setHidden(hide);
                id_lw_circuits->item(i)->setFlags(hide ? item_flags : (item_flags | Qt::ItemIsEnabled));
                if (hide) { id_lw_circuits->item(i)->setCheckState(Qt::Unchecked); }
                showInspections(id_lw_circuits->item(i));
            }
        }
    };
    void showInspections(QListWidgetItem * item) {
        bool hide = item->checkState() == Qt::Unchecked;
        for (int i = 0; i < id_lw_inspections->count(); ++i) {
            if (id_lw_inspections->item(i)->data(Qt::UserRole).toString().startsWith(QString("%1::").arg(item->data(Qt::UserRole).toString()))) {
                id_lw_inspections->item(i)->setHidden(hide);
                id_lw_inspections->item(i)->setFlags(hide ? item_flags : (item_flags | Qt::ItemIsEnabled));
            }
        }
    };
    void selectAllCustomers() { selectAll(customers()); };
    void selectAllCircuits() { selectAll(circuits()); };
    void selectAllInspections() { selectAll(inspections()); };
    void deselectAllCustomers() { deselectAll(customers()); };
    void deselectAllCircuits() { deselectAll(circuits()); };
    void deselectAllInspections() { deselectAll(inspections()); };

private:
    void selectAll(MTListWidget * lw) {
        for (int i = 0; i < lw->count(); ++i) {
            lw->item(i)->setCheckState(Qt::Checked);
        }
    };
    void deselectAll(MTListWidget * lw) {
        for (int i = 0; i < lw->count(); ++i) {
            lw->item(i)->setCheckState(Qt::Unchecked);
        }
    };

    QFlags<Qt::ItemFlag> item_flags;
};

#endif // IMPORT_DIALOGUE_H
