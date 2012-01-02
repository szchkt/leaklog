/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2012 Matus & Michal Tomlein

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

#include "edit_assembly_record_dialogue.h"

#include "records.h"

#include <QTreeWidgetItem>
#include <QLabel>
#include <QHeaderView>
#include <QSpinBox>

EditAssemblyRecordDialogue::EditAssemblyRecordDialogue(DBRecord * record, QWidget * parent):
    TabbedEditDialogue(record, parent)
{
    main_tabw->setTabText(0, tr("Assembly record type"));
    addTab(new EditAssemblyRecordDialogueTab(idFieldValue().toInt()));
}

EditAssemblyRecordDialogueTab::EditAssemblyRecordDialogueTab(int record_id, QWidget * parent):
    EditDialogueTab(parent),
    record_id(record_id)
{
    setName(tr("Item categories"));

    init();
}

void EditAssemblyRecordDialogueTab::save(const QVariant & record_id_variant)
{
    AssemblyRecordTypeCategory(QString("%1").arg(record_id)).remove();

    record_id = record_id_variant.toInt();

    QVariantMap map;
    map.insert("record_type_id", record_id);

    QTreeWidgetItem * item;
    for (int i = 0; i < tree->topLevelItemCount(); ++i) {
        item = tree->topLevelItem(i);

        if (item->checkState(0) == Qt::Checked) {
            map.insert("record_category_id", item->data(0, Qt::UserRole).toInt());
            map.insert("position", ((QSpinBox *) tree->itemWidget(item, 1))->value());
            AssemblyRecordTypeCategory().update(map);
        }
    }
}

void EditAssemblyRecordDialogueTab::init()
{
    QVBoxLayout * layout = new QVBoxLayout(this);
    setLayout(layout);

    layout->addWidget(new QLabel(tr("Assembly record categories:")));

    tree = new QTreeWidget;
    tree->setColumnCount(2);
    tree->setHeaderLabels(QStringList() << tr("Category") << tr("Position"));
    tree->header()->setResizeMode(0, QHeaderView::Stretch);
    tree->header()->setResizeMode(1, QHeaderView::ResizeToContents);
    layout->addWidget(tree);

    MTRecord record(QString("assembly_record_item_categories LEFT JOIN assembly_record_type_categories"
                    " ON assembly_record_type_categories.record_category_id = assembly_record_item_categories.id"
                    " AND assembly_record_type_categories.record_type_id = %1").arg(record_id),
                    "", "", MTDictionary());

    ListOfVariantMaps all_categories(record.listAll("assembly_record_item_categories.name, assembly_record_item_categories.id,"
                                                    " assembly_record_type_categories.record_type_id, assembly_record_type_categories.position"));

    QTreeWidgetItem * item;
    for (int i = 0; i < all_categories.count(); ++i) {
        item = new QTreeWidgetItem;
        item->setText(0, all_categories.at(i).value("name").toString());
        item->setData(0, Qt::UserRole, all_categories.at(i).value("id"));

        if (!all_categories.at(i).value("record_type_id").isNull()) item->setCheckState(0, Qt::Checked);
        else item->setCheckState(0, Qt::Unchecked);
        tree->addTopLevelItem(item);
        QSpinBox * spin_box = new QSpinBox();
        spin_box->setValue(all_categories.at(i).value("position").toInt());
        tree->setItemWidget(item, 1, spin_box);
    }
}
