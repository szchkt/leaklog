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

#include "modify_circuit_dialogue_compressors_tab.h"
#include "records.h"

#include <QTreeWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QHeaderView>
#include <QToolButton>

ModifyCircuitDialogueCompressorsTab::ModifyCircuitDialogueCompressorsTab(const QString & customer_id, const QString & circuit_id, QWidget * parent)
    : ModifyDialogueTab(parent),
      customer_id(customer_id)
{
    setName(tr("Compressors"));

    QVBoxLayout * layout = new QVBoxLayout(this);
    layout->setContentsMargins(9, 9, 9, 9);
    layout->setSpacing(9);

    tree = new QTreeWidget(this);
    tree->setColumnCount(5);
    QStringList header_labels;
    header_labels << tr("Name");
    header_labels << tr("Manufacturer");
    header_labels << tr("Type");
    header_labels << tr("Serial number");
    header_labels << tr("Remove");
    tree->setHeaderLabels(header_labels);
    tree->setIndentation(0);
    tree->setSelectionMode(QAbstractItemView::NoSelection);
    tree->header()->setResizeMode(0, QHeaderView::Stretch);
    tree->header()->setResizeMode(1, QHeaderView::Stretch);
    tree->header()->setResizeMode(2, QHeaderView::Stretch);
    tree->header()->setResizeMode(3, QHeaderView::Stretch);
    tree->header()->setResizeMode(4, QHeaderView::ResizeToContents);
    layout->addWidget(tree);

    QPushButton * add_row_btn = new QPushButton(tr("Add"), this);
    QObject::connect(add_row_btn, SIGNAL(clicked()), this, SLOT(addRow()));
    QHBoxLayout * buttons_layout = new QHBoxLayout(this);
    buttons_layout->addWidget(add_row_btn);
    buttons_layout->addStretch();
    buttons_layout->setContentsMargins(0, 0, 0, 0);
    layout->addLayout(buttons_layout);

    load(circuit_id);
}

void ModifyCircuitDialogueCompressorsTab::save(const QVariant & circuit_id)
{
    for (int i = 0; i < rows.count(); ++i) {
        if (!rows.at(i)->hasId() && rows.at(i)->isEmpty()) continue;

        Compressor compressor_rec;
        if (rows.at(i)->hasId()) {
            compressor_rec = Compressor(QString::number(rows.at(i)->id()));
        } else {
            compressor_rec = Compressor(QString());
        }

        QVariantMap map;
        map.insert("customer_id", customer_id);
        map.insert("circuit_id", circuit_id);
        map.insert("name", rows.at(i)->name());
        map.insert("manufacturer", rows.at(i)->manufacturer());
        map.insert("type", rows.at(i)->type());
        map.insert("sn", rows.at(i)->sn());
        compressor_rec.update(map);
    }
    for (int i = 0; i < deleted_ids.count(); ++i) {
        Compressor compressor_rec(QString::number(deleted_ids.at(i)));
        compressor_rec.remove();
    }
}

void ModifyCircuitDialogueCompressorsTab::load(const QString & circuit_id)
{
    if (!circuit_id.isEmpty()) {
        Compressor compressor_rec(QString(), MTDictionary(QStringList() << "customer_id" << "circuit_id",
                                                                       QStringList() << customer_id << circuit_id));

        ListOfVariantMaps compressors = compressor_rec.listAll();
        for (int i = 0; i < compressors.count(); ++i) {
            CompressorsTableRow * row = addRow(compressors.at(i).value("id").toInt());
            row->setName(compressors.at(i).value("name").toString());
            row->setManufacturer(compressors.at(i).value("manufacturer").toString());
            row->setType(compressors.at(i).value("type").toString());
            row->setSn(compressors.at(i).value("sn").toString());
        }
    }

    if (!rows.count()) addRow();
}

void ModifyCircuitDialogueCompressorsTab::rowWantsToBeRemoved(CompressorsTableRow * row)
{
    for (int i = 0; i < rows.count(); ++i) {
        if (rows.at(i) == row) {
            rows.takeAt(i);
            break;
        }
    }
    if (row->hasId()) deleted_ids << row->id();
    delete row->treeItem();
    delete row;
}

CompressorsTableRow * ModifyCircuitDialogueCompressorsTab::addRow(int row_id)
{
    CompressorsTableRow * row = new CompressorsTableRow(tree, row_id, this);
    rows.append(row);
    QObject::connect(row, SIGNAL(wantsToBeRemoved(CompressorsTableRow*)), this, SLOT(rowWantsToBeRemoved(CompressorsTableRow*)));
    return row;
}

CompressorsTableRow::CompressorsTableRow(QTreeWidget * tree, int id, QWidget * parent)
    : QWidget(parent),
      m_id(id)
{
    tree_item = new QTreeWidgetItem(tree);

    name_le = new QLineEdit(this);
    tree->setItemWidget(tree_item, 0, name_le);

    manufacturer_le = new QLineEdit(this);
    tree->setItemWidget(tree_item, 1, manufacturer_le);

    type_le = new QLineEdit(this);
    tree->setItemWidget(tree_item, 2, type_le);

    sn_le = new QLineEdit(this);
    tree->setItemWidget(tree_item, 3, sn_le);

    remove_btn = new QToolButton(this);
    remove_btn->setIcon(QIcon(QString::fromUtf8(":/images/images/remove16.png")));
    QObject::connect(remove_btn, SIGNAL(clicked()), this, SLOT(remove()));
    tree->setItemWidget(tree_item, 4, remove_btn);
}

void CompressorsTableRow::remove()
{
    emit wantsToBeRemoved(this);
}
