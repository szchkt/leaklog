/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2018 Matus & Michal Tomlein

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

#include "editassemblyrecorddialogue.h"

#include "records.h"
#include "inputwidgets.h"

#include <QTreeWidgetItem>
#include <QLabel>
#include <QHeaderView>
#include <QSpinBox>

EditAssemblyRecordDialogue::EditAssemblyRecordDialogue(DBRecord *record, UndoStack *undo_stack, QWidget *parent):
    TabbedEditDialogue(record, undo_stack, parent)
{
    main_tabw->setTabText(0, tr("Assembly record type"));
    addTab(new EditAssemblyRecordDialogueTab(md_record->uuid()));
}

void EditAssemblyRecordDialogue::save()
{
    TabbedEditDialogue::save();
}

EditAssemblyRecordDialogueTab::EditAssemblyRecordDialogueTab(const QString &ar_type_uuid, QWidget *parent):
    EditDialogueTab(parent)
{
    setName(tr("Item categories"));

    init(ar_type_uuid);
}

void EditAssemblyRecordDialogueTab::save(const QString &ar_type_uuid)
{
    AssemblyRecordType(ar_type_uuid).typeCategories().removeAll();

    QTreeWidgetItem *item;
    for (int i = 0; i < tree->topLevelItemCount(); ++i) {
        item = tree->topLevelItem(i);

        if (item->checkState(0) == Qt::Checked) {
            QString ar_item_category_uuid = item->data(0, Qt::UserRole).toString();
            int value = ((QSpinBox *) tree->itemWidget(item, 1))->value();
            AssemblyRecordTypeCategory::query({{"ar_type_uuid", ar_type_uuid}, {"ar_item_category_uuid", ar_item_category_uuid}}).each([value](AssemblyRecordTypeCategory &record) {
                record.update("position", value);
            });
        }
    }
}

void EditAssemblyRecordDialogueTab::init(const QString &ar_type_uuid)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    setLayout(layout);

    layout->addWidget(new QLabel(tr("Assembly record categories:")));

    tree = new QTreeWidget;
    tree->setColumnCount(2);
    tree->setHeaderLabels(QStringList() << tr("Category") << tr("Position"));
    tree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    tree->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    layout->addWidget(tree);

    if (ar_type_uuid.isEmpty())
        return;

    MTQuery query(QString("assembly_record_item_categories LEFT JOIN assembly_record_type_categories"
                  " ON assembly_record_type_categories.ar_item_category_uuid = assembly_record_item_categories.uuid"
                  " AND assembly_record_type_categories.ar_type_uuid = '%1'").arg(ar_type_uuid));

    ListOfVariantMaps all_categories(query.listAll("assembly_record_item_categories.name, assembly_record_item_categories.uuid,"
                                                   " assembly_record_type_categories.ar_type_uuid, assembly_record_type_categories.position"));

    QTreeWidgetItem *item;
    for (int i = 0; i < all_categories.count(); ++i) {
        item = new QTreeWidgetItem;
        item->setText(0, all_categories.at(i).value("name").toString());
        item->setData(0, Qt::UserRole, all_categories.at(i).value("uuid"));

        if (!all_categories.at(i).value("ar_type_uuid").isNull()) item->setCheckState(0, Qt::Checked);
        else item->setCheckState(0, Qt::Unchecked);
        tree->addTopLevelItem(item);
        QSpinBox *spin_box = new QSpinBox();
        spin_box->setValue(all_categories.at(i).value("position").toInt());
        tree->setItemWidget(item, 1, spin_box);
    }
}
