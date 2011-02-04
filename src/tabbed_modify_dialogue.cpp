#include "tabbed_modify_dialogue.h"

#include "records.h"
#include "input_widgets.h"

#include <QTreeWidget>
#include <QHeaderView>
#include <QTabWidget>
#include <QLabel>
#include <QVariantMap>
#include <QGroupBox>

TabbedModifyDialogue::TabbedModifyDialogue(DBRecord * record, QWidget * parent)
    : ModifyDialogue(parent)
{
    main_tabw = new QTabWidget;

    init(record);

    md_record->initModifyDialogue(this);

    ModifyDialogueColumnLayout(&md_inputwidgets, md_grid_main).layout();
}

TabbedModifyDialogue::~TabbedModifyDialogue()
{
    delete main_tabw;
}

void TabbedModifyDialogue::addMainGridLayout(QVBoxLayout * md_vlayout_main)
{
    QWidget * main_form_tab = new QWidget(main_tabw);
    main_form_tab->setLayout(md_grid_main);

    main_tabw->addTab(main_form_tab, tr("Basic information"));
    md_vlayout_main->addWidget(main_tabw);
}

void TabbedModifyDialogue::addTab(ModifyDialogueTab * tab)
{
    main_tabw->addTab(tab, tab->name());
}

void TabbedModifyDialogue::save()
{
    if (!ModifyDialogue::save(false)) return;

    for (int i = 1; i < main_tabw->count(); ++i) {
        ((ModifyDialogueTab *) main_tabw->widget(i))->save(idFieldValue().toInt());
    }

    accept();
}

ModifyDialogueTab::ModifyDialogueTab(QWidget * parent)
    : QWidget(parent)
{
}

void ModifyDialogueTab::setLayout(QLayout * layout)
{
    layout->setContentsMargins(0, 0, 0, 0);
    QWidget::setLayout(layout);
}

AssemblyRecordModifyDialogue::AssemblyRecordModifyDialogue(DBRecord * record, QWidget * parent)
    : TabbedModifyDialogue(record, parent)
{
    addTab(new AssemblyRecordModifyDialogueTab(idFieldValue().toInt()));
}

AssemblyRecordModifyDialogueTab::AssemblyRecordModifyDialogueTab(int record_id, QWidget * parent)
    : ModifyDialogueTab(parent)
{
    this->record_id = record_id;
    setName(tr("Assembly record items"));

    init();
}

void AssemblyRecordModifyDialogueTab::save(int record_id)
{
    this->record_id = record_id;

    AssemblyRecordTypeItem used_item_records(QString("%1").arg(record_id));
    used_item_records.remove();

    QVariantMap map;
    map.insert("record_type_id", record_id);

    QTreeWidgetItem * item;
    for (int i = 0; i < tree->topLevelItemCount(); ++i) {
        item = tree->topLevelItem(i);

        if (item->checkState(0) == Qt::Checked) {
            map.insert("record_item_id", item->data(0, Qt::UserRole).toInt());
            used_item_records.update(map);
        }
    }
}

void AssemblyRecordModifyDialogueTab::init()
{
    QVBoxLayout * layout = new QVBoxLayout(this);
    setLayout(layout);

    layout->addWidget(new QLabel(tr("Assembly record items:")));

    tree = new QTreeWidget;
    tree->header()->hide();
    layout->addWidget(tree);

    AssemblyRecordItemType all_items("");
    ListOfVariantMaps items(all_items.listAll());

    AssemblyRecordTypeItem used_item_records(QString("%1").arg(record_id));
    ListOfVariantMaps used_items(used_item_records.listAll());

    QTreeWidgetItem * item;
    for (int i = 0; i < items.count(); ++i) {
        item = new QTreeWidgetItem;
        item->setText(0, items.at(i).value("name").toString());
        item->setCheckState(0, Qt::Unchecked);
        item->setData(0, Qt::UserRole, items.at(i).value("id"));

        for (int n = 0; n < used_items.count(); ++n) {
            if (used_items.at(n).value("record_item_id").toInt() == items.at(i).value("id").toInt()) {
                item->setCheckState(0, Qt::Checked);
                break;
            }
        }
        tree->addTopLevelItem(item);
    }
}

ModifyInspectionDialogue::ModifyInspectionDialogue(DBRecord * record, QWidget * parent)
    : TabbedModifyDialogue(record, parent)
{
    addTab(new ModifyInspectionDialogueTab(0, (MDLineEdit *) inputWidget("arno"), (MDComboBox *) inputWidget("ar_type")));
}

ModifyInspectionDialogueTab::ModifyInspectionDialogueTab(int, MDLineEdit * arno_pw, MDComboBox * ar_type_pw, QWidget * parent)
    : ModifyDialogueTab(parent)
{
    this->ar_type_w = ar_type_pw;
    this->arno_w = arno_pw;

    QObject::connect(ar_type_w, SIGNAL(currentIndexChanged(int)), this, SLOT(loadItemInputWidgets()));

    setName(tr("Assembly record"));

    init();
}

void ModifyInspectionDialogueTab::init()
{
    QVBoxLayout * layout = new QVBoxLayout(this);
    setLayout(layout);

    QGridLayout * form_grid = new QGridLayout;
    QList<MDInputWidget *> inputwidgets;
    MDLineEdit * arno_le = new MDLineEdit("arno", tr("Assembly record number:"), this, arno_w->text());
    arno_le->setEnabled(false);
    QObject::connect(arno_w, SIGNAL(textChanged(QString)), arno_le, SLOT(setText(QString)));
    inputwidgets.append(arno_le);
    MDLineEdit * type_le = new MDLineEdit("ar_type", tr("Assembly record type:"), this, ar_type_w->currentText());
    type_le->setEnabled(false);
    QObject::connect(ar_type_w, SIGNAL(currentIndexChanged(QString)), type_le, SLOT(setText(QString)));
    inputwidgets.append(type_le);
    ModifyDialogueColumnLayout(&inputwidgets, form_grid, 1).layout();

    layout->addLayout(form_grid);

    QGroupBox * items_gb = new QGroupBox(tr("Items"));
    items_grid = new QGridLayout;
    items_gb->setLayout(items_grid);
    layout->addWidget(items_gb);

    loadItemInputWidgets();
}

const QVariant ModifyInspectionDialogueTab::assemblyRecordType()
{
    return ar_type_w->variantValue();
}

const QVariant ModifyInspectionDialogueTab::assemblyRecordId()
{
    return arno_w->variantValue();
}

void ModifyInspectionDialogueTab::loadItemInputWidgets()
{
    for (int i = 0; i < item_inputwidgets.count(); ++i) {
        MDInputWidget * w = item_inputwidgets.takeAt(i);
        delete w->label();
        delete w;
    }

    AssemblyRecordTypeItem type_items(assemblyRecordType().toString());
    ListOfVariantMaps items_list(type_items.listAll());

    AssemblyRecordItem assembly_record_item(assemblyRecordId().toString());
    ListOfVariantMaps record_items_list(assembly_record_item.listAll());

    for (int i = 0; i < items_list.count(); ++i) {
        AssemblyRecordItemType item(items_list.at(i).value("record_item_id").toString());
        QVariantMap item_map(item.list());
        QString value;
        for (int n = 0; n < record_items_list.count(); ++n) {
            if (record_items_list.at(n).value("item_type_id").toInt() == items_list.at(i).value("record_item_id").toInt()) {
                value = record_items_list.at(n).value("value").toString(); break;
            }
        }
        item_inputwidgets << new MDLineEdit(item_map.value("id").toString(), item_map.value("name").toString(), this, value);
    }

    ModifyDialogueColumnLayout(&item_inputwidgets, items_grid, 18).layout();
}

void ModifyInspectionDialogueTab::save(int)
{
    QString arno = assemblyRecordId().toString();
    if (arno.isEmpty())
        return;

    AssemblyRecordItem record_item(arno);
    record_item.remove();

    QVariantMap map;
    map.insert("arno", arno);

    for (int i = 0; i < item_inputwidgets.count(); ++i) {
        map.insert("item_type_id", item_inputwidgets.at(i)->id());
        map.insert("value", item_inputwidgets.at(i)->variantValue());
        record_item.update(map);
    }
}
