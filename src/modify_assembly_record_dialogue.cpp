#include "modify_assembly_record_dialogue.h"

#include "records.h"

#include <QTreeWidgetItem>
#include <QLabel>
#include <QHeaderView>

ModifyAssemblyRecordDialogue::ModifyAssemblyRecordDialogue(DBRecord * record, QWidget * parent)
    : TabbedModifyDialogue(record, parent)
{
    addTab(new ModifyAssemblyRecordDialogueTab(idFieldValue().toInt()));
}

ModifyAssemblyRecordDialogueTab::ModifyAssemblyRecordDialogueTab(int record_id, QWidget * parent)
    : ModifyDialogueTab(parent)
{
    this->record_id = record_id;
    setName(tr("Assembly record items"));

    init();
}

void ModifyAssemblyRecordDialogueTab::save(int record_id)
{
    this->record_id = record_id;

    AssemblyRecordTypeCategory used_categories(QString("%1").arg(record_id));
    used_categories.remove();

    QVariantMap map;
    map.insert("record_type_id", record_id);

    QTreeWidgetItem * item;
    for (int i = 0; i < tree->topLevelItemCount(); ++i) {
        item = tree->topLevelItem(i);

        if (item->checkState(0) == Qt::Checked) {
            map.insert("record_category_id", item->data(0, Qt::UserRole).toInt());
            used_categories.update(map);
        }
    }
}

void ModifyAssemblyRecordDialogueTab::init()
{
    QVBoxLayout * layout = new QVBoxLayout(this);
    setLayout(layout);

    layout->addWidget(new QLabel(tr("Assembly record categories:")));

    tree = new QTreeWidget;
    tree->header()->hide();
    layout->addWidget(tree);

    AssemblyRecordItemCategory categories_record("");
    ListOfVariantMaps all_categories(categories_record.listAll());

    AssemblyRecordTypeCategory used_categories_record(QString("%1").arg(record_id));
    ListOfVariantMaps used_categories(used_categories_record.listAll());

    QTreeWidgetItem * item;
    for (int i = 0; i < all_categories.count(); ++i) {
        item = new QTreeWidgetItem;
        item->setText(0, all_categories.at(i).value("name").toString());
        item->setCheckState(0, Qt::Unchecked);
        item->setData(0, Qt::UserRole, all_categories.at(i).value("id"));

        for (int n = 0; n < used_categories.count(); ++n) {
            if (used_categories.at(n).value("record_category_id").toInt() == all_categories.at(i).value("id").toInt()) {
                item->setCheckState(0, Qt::Checked);
                break;
            }
        }
        tree->addTopLevelItem(item);
    }
}
