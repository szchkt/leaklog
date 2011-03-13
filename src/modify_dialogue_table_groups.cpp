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

#include "modify_dialogue_table_groups.h"

#include "modify_dialogue_table.h"

#include <QMap>
#include <QVBoxLayout>

ModifyDialogueGroupsLayout::ModifyDialogueGroupsLayout(QWidget * parent):
QWidget(parent)
{
    layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    groups = new QMap<QString, ModifyDialogueAdvancedTable *>;
}

ModifyDialogueGroupsLayout::~ModifyDialogueGroupsLayout()
{
    clear();
    delete groups;
    for (int i = header_items.count() - 1; i >= 0; --i) { delete header_items.takeAt(i); }
}

void ModifyDialogueGroupsLayout::addHeaderItem(int id, const QString & name, const QString & full_name, int data_type)
{
    header_items.append(new ModifyDialogueGroupHeaderItem(id, name, full_name, data_type));
}

void ModifyDialogueGroupsLayout::addItem(const QString & group_name, int category_id, QMap<QString, ModifyDialogueTableCell *> & values, int category_display, bool display)
{
    ModifyDialogueAdvancedTable * group_box;
    if (!groups->contains(group_name)) {
        group_box = createGroup(group_name, category_id, category_display);
    } else {
        group_box = groups->value(group_name);
    }

    group_box->addRow(values, display);
}

ModifyDialogueAdvancedTable * ModifyDialogueGroupsLayout::createGroup(const QString & group_name, int category_id, int display_options)
{
    QList<ModifyDialogueTableCell *> cells;
    for (int i = 0; i < header_items.count(); ++i) {
        if ((display_options & header_items.at(i)->id()) || header_items.at(i)->id() < 0) {
            cells.append(header_items.at(i)->tableCell());
        }
    }

    ModifyDialogueAdvancedTable * group_box;
    if (category_id == INSPECTORS_CATEGORY_ID)
        group_box = new ModifyDialogueTableWithAdjustableTotal(group_name, category_id, cells, this);
    else
        group_box = new ModifyDialogueAdvancedTable(group_name, category_id, cells, this);
    groups->insert(group_name, group_box);
    layout->addWidget(group_box);
    return group_box;
}

QList<MTDictionary> ModifyDialogueGroupsLayout::allValues()
{
    QList<MTDictionary> values;

    foreach (ModifyDialogueAdvancedTable * group_box, *groups) {
        values.append(group_box->allValues());
    }

    return values;
}

void ModifyDialogueGroupsLayout::clear()
{
    QMapIterator<QString, ModifyDialogueAdvancedTable *> i(*groups);
    while (i.hasNext()) {
        i.next();
        delete groups->take(i.key());
    }
}

ModifyDialogueGroupHeaderItem::ModifyDialogueGroupHeaderItem(int id, const QString & name, const QString & full_name, int data_type)
{
    this->item_id = id;
    this->item_name = name;
    this->item_full_name = full_name;
    this->data_type = data_type;
}

ModifyDialogueTableCell * ModifyDialogueGroupHeaderItem::tableCell()
{
    return new ModifyDialogueTableCell(item_full_name, item_name, data_type);
}
