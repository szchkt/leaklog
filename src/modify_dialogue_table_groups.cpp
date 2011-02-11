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

    ModifyDialogueAdvancedTable * group_box = new ModifyDialogueAdvancedTable(group_name, category_id, cells, this);
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
