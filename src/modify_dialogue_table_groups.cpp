#include "modify_dialogue_table_groups.h"

#include "modify_dialogue_table.h"

#include <QMap>
#include <QVBoxLayout>

ModifyDialogueGroupsLayout::ModifyDialogueGroupsLayout(QWidget * parent):
QWidget(parent)
{
    layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    groups = new QMap<QString, ModifyDialogueTableGroupBox *>;
}

ModifyDialogueGroupsLayout::~ModifyDialogueGroupsLayout()
{
    clear();
    delete groups;
    for (int i = header_items.count() - 1; i >= 0; --i) { delete header_items.takeAt(i); }
}

void ModifyDialogueGroupsLayout::addHeaderItem(int id, const QString & name, const QString & full_name)
{
    header_items.append(new ModifyDialogueGroupHeaderItem(id, name, full_name));
}

void ModifyDialogueGroupsLayout::addItem(const QString & group_name, int category_id, const QString & row_name, const QMap<QString, ModifyDialogueTableCell *> & values, int category_display, bool display)
{
    ModifyDialogueTableGroupBox * group_box;
    if (!groups->contains(group_name)) {
        group_box = createGroup(group_name, category_id, category_display);
    } else {
        group_box = groups->value(group_name);
    }

    group_box->addRow(row_name, values, display);
}

ModifyDialogueTableGroupBox * ModifyDialogueGroupsLayout::createGroup(const QString & group_name, int category_id, int display_options)
{
    MTDictionary dict;
    for (int i = 0; i < header_items.count(); ++i) {
        if (display_options & header_items.at(i)->id())
            dict.insert(header_items.at(i)->name(), header_items.at(i)->fullName());
    }

    ModifyDialogueTableGroupBox * group_box = new ModifyDialogueTableGroupBox(group_name, category_id, dict, this);
    groups->insert(group_name, group_box);
    layout->addWidget(group_box);
    return group_box;
}

QList<MTDictionary> ModifyDialogueGroupsLayout::allValues()
{
    QList<MTDictionary> values;

    foreach (ModifyDialogueTableGroupBox * group_box, *groups) {
        values.append(group_box->allValues());
    }

    return values;
}

void ModifyDialogueGroupsLayout::clear()
{
    QMapIterator<QString, ModifyDialogueTableGroupBox *> i(*groups);
    while (i.hasNext()) {
        i.next();
        delete groups->take(i.key());
    }
}

ModifyDialogueGroupHeaderItem::ModifyDialogueGroupHeaderItem(int id, const QString & name, const QString & full_name)
{
    this->item_id = id;
    this->item_name = name;
    this->item_full_name = full_name;
}
