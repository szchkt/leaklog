#include "tabbed_modify_dialogue.h"

#include "records.h"
#include "input_widgets.h"

#include <QTreeWidget>
#include <QHeaderView>
#include <QTabWidget>
#include <QLabel>
#include <QVariantMap>
#include <QGroupBox>
#include <QSqlQuery>

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
    layout->setContentsMargins(9, 9, 9, 9);
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

void AssemblyRecordModifyDialogueTab::init()
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

    groups_layout = new ModifyDialogueGroupsLayout(this);
    layout->addWidget(groups_layout);

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
    /*for (int i = 0; i < item_inputwidgets.count(); ++i) {
        MDInputWidget * w = item_inputwidgets.takeAt(i);
        delete w->label();
        delete w;
    }*/

    enum QUERY_RESULTS
    {
        ID = 0, NAME = 1, ACQUISITION_PRICE = 2, VALUE = 3, CATEGORY_NAME = 4, CATEGORY_ID = 5
    };
    QSqlQuery items_query(QString("SELECT assembly_record_item_types.id, assembly_record_item_types.name, assembly_record_item_types.acquisition_price,"
                                  " assembly_record_items.value, assembly_record_item_categories.name, assembly_record_item_categories.id"
                                  " FROM assembly_record_item_types"
                                  " LEFT JOIN assembly_record_items ON assembly_record_items.item_type_id = assembly_record_item_types.id"
                                  " AND assembly_record_items.arno = '%1'"
                                  " LEFT JOIN assembly_record_item_categories ON assembly_record_item_types.category_id = assembly_record_item_categories.id"
                                  " WHERE assembly_record_item_types.category_id IN"
                                  " (SELECT DISTINCT record_category_id FROM assembly_record_type_categories WHERE record_type_id = %2)")
                          .arg(assemblyRecordId().toString())
                          .arg(assemblyRecordType().toString()));
    while (items_query.next()) {
        groups_layout->addWidget(items_query.value(CATEGORY_NAME).toString(), new MDLineEdit(items_query.value(ID).toString(), items_query.value(NAME).toString(), this, items_query.value(VALUE).toString()));
    }
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

    /*for (int i = 0; i < item_inputwidgets.count(); ++i) {
        map.insert("item_type_id", item_inputwidgets.at(i)->id());
        map.insert("value", item_inputwidgets.at(i)->variantValue());
        record_item.update(map);
    }*/
}

ModifyDialogueGroupsLayout::ModifyDialogueGroupsLayout(QWidget * parent):
QWidget(parent)
{
    layout = new QVBoxLayout(this);
    groups = new QMap<QString, QGroupBox *>;
    this->setLayout(layout);
}

void ModifyDialogueGroupsLayout::addWidget(const QString & group_name, MDInputWidget * widget)
{
    QGroupBox * group_box;
    if (!groups->contains(group_name)) {
        group_box = new QGroupBox(group_name, this);
        group_box->setLayout(new QVBoxLayout(group_box));
        layout->addWidget(group_box);
        groups->insert(group_name, group_box);
    } else {
        group_box = groups->value(group_name);
    }

    item_inputwidgets << widget;
    group_box->layout()->addWidget(widget->widget());
}
