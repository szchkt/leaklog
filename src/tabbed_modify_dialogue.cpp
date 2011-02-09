#include "tabbed_modify_dialogue.h"

#include "records.h"
#include "input_widgets.h"
#include "modifydialoguetable.h"
#include "global.h"

#include <QTreeWidget>
#include <QHeaderView>
#include <QTabWidget>
#include <QLabel>
#include <QVariantMap>
#include <QGroupBox>
#include <QSqlQuery>
#include <QMessageBox>
#include <QScrollArea>

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
    QScrollArea * scroll_area = new QScrollArea;
    scroll_area->setWidgetResizable(1);
    scroll_area->setWidget(tab);
    scroll_area->setFrameStyle(QFrame::NoFrame);
    scroll_area->setAutoFillBackground(true);
    scroll_area->setBackgroundRole(QPalette::NoRole);
    main_tabw->addTab(scroll_area, tab->name());
}

void TabbedModifyDialogue::save()
{
    if (!ModifyDialogue::save(false)) return;

    for (int i = 1; i < main_tabw->count(); ++i) {
        ((ModifyDialogueTab *) ((QScrollArea *) main_tabw->widget(i))->widget())
                ->save(idFieldValue().toInt());
    }

    accept();
}

ModifyDialogueTab::ModifyDialogueTab(QWidget * parent)
    : QWidget(parent)
{
    setAutoFillBackground(false);
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

ModifyInspectionDialogueTab::ModifyInspectionDialogueTab(int, MDLineEdit * arno_w, MDComboBox * ar_type_w, QWidget * parent)
    : ModifyDialogueTab(parent)
{
    this->ar_type_w = ar_type_w;
    this->arno_w = arno_w;
    original_arno = arno_w->text();

    QObject::connect(ar_type_w, SIGNAL(currentIndexChanged(int)), this, SLOT(loadItemInputWidgets()));
    QObject::connect(arno_w, SIGNAL(editingFinished()), this, SLOT(assemblyRecordNumberChanged()));

    setName(tr("Assembly record"));

    init();
}

void ModifyInspectionDialogueTab::init()
{
    QVBoxLayout * layout = new QVBoxLayout(this);
    setLayout(layout);

    QGridLayout * form_grid = new QGridLayout;
    QList<MDInputWidget *> inputwidgets;
    arno_w->setShowInForm(true);
    inputwidgets.append(arno_w);
    ar_type_w->setShowInForm(true);
    inputwidgets.append(ar_type_w);
    ModifyDialogueColumnLayout(&inputwidgets, form_grid, 1).layout();

    layout->addLayout(form_grid);

    groups_layout = new ModifyDialogueGroupsLayout(this);
    groups_layout->addHeaderItem(AssemblyRecordItemCategory::ShowValue, "value", tr("Value"));
    groups_layout->addHeaderItem(AssemblyRecordItemCategory::ShowAcquisitionPrice, "acquisition_price", tr("Acquisition price"));
    groups_layout->addHeaderItem(AssemblyRecordItemCategory::ShowListPrice, "list_price", tr("List price"));
    layout->addWidget(groups_layout);

    layout->addStretch();

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
    groups_layout->clear();
    QMap<QString, ModifyDialogueTableCell *> cells_map;

    enum QUERY_RESULTS
    {
        TYPE_ID = 0,
        NAME = 1,
        ACQUISITION_PRICE = 2,
        LIST_PRICE = 3,
        VALUE = 4,
        CATEGORY_NAME = 5,
        CATEGORY_ID = 6,
        DISPLAY_OPTIONS = 7,
        ITEM_ACQUISITION_PRICE = 8,
        ITEM_LIST_PRICE = 9,
        INSPECTION_VAR = 10,
        VALUE_DATA_TYPE = 11
    };
    QSqlQuery items_query(QString("SELECT assembly_record_item_types.id, assembly_record_item_types.name, assembly_record_item_types.acquisition_price, assembly_record_item_types.list_price,"
                                  " assembly_record_items.value, assembly_record_item_categories.name, assembly_record_item_categories.id, assembly_record_item_categories.display_options,"
                                  " assembly_record_items.acquisition_price, assembly_record_items.list_price, assembly_record_item_types.inspection_variable_id, assembly_record_item_types.value_data_type"
                                  " FROM assembly_record_item_types"
                                  " LEFT JOIN assembly_record_items ON assembly_record_items.item_type_id = assembly_record_item_types.id"
                                  " AND assembly_record_items.arno = '%1'"
                                  " LEFT JOIN assembly_record_item_categories ON assembly_record_item_types.category_id = assembly_record_item_categories.id"
                                  " WHERE assembly_record_item_types.category_id IN"
                                  " (SELECT DISTINCT record_category_id FROM assembly_record_type_categories WHERE record_type_id = %2)")
                          .arg(assemblyRecordId().toString())
                          .arg(assemblyRecordType().toString()));
    while (items_query.next()) {
        cells_map.insert("value", new ModifyDialogueTableCell(items_query.value(VALUE), items_query.value(VALUE_DATA_TYPE).toInt(), items_query.value(INSPECTION_VAR).toString().isEmpty()));
        cells_map.insert("item_type_id", new ModifyDialogueTableCell(items_query.value(TYPE_ID)));
        cells_map.insert("acquisition_price", new ModifyDialogueTableCell(items_query.value(ITEM_ACQUISITION_PRICE).isNull() ? items_query.value(ACQUISITION_PRICE) : items_query.value(ITEM_ACQUISITION_PRICE), Global::Numeric));
        cells_map.insert("list_price", new ModifyDialogueTableCell(items_query.value(ITEM_LIST_PRICE).isNull() ? items_query.value(ACQUISITION_PRICE) : items_query.value(ITEM_LIST_PRICE), Global::Numeric));
        groups_layout->addItem(items_query.value(CATEGORY_NAME).toString(),
                               items_query.value(CATEGORY_ID).toInt(),
                               items_query.value(NAME).toString(),
                               cells_map,
                               items_query.value(DISPLAY_OPTIONS).toInt(),
                               !items_query.value(VALUE).isNull() || !items_query.value(ITEM_LIST_PRICE).isNull());
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

    QList<MTDictionary> record_dicts = groups_layout->allValues();

    for (int i = 0; i < record_dicts.count(); ++i) {
        map.insert("value", record_dicts.at(i).value("value"));
        map.insert("item_type_id", record_dicts.at(i).value("item_type_id").toInt() < 0
                   ? QVariant(saveNewItemType(record_dicts.at(i))) : record_dicts.at(i).value("item_type_id"));
        map.insert("acquisition_price", record_dicts.at(i).value("acquisition_price"));
        map.insert("list_price", record_dicts.at(i).value("list_price"));
        record_item.update(map);
    }
}

int ModifyInspectionDialogueTab::saveNewItemType(const MTDictionary & dict)
{
    int id = -1;
    QSqlQuery query("SELECT MAX(id) FROM assembly_record_item_types");
    if (query.last()) {
        id = query.value(0).toInt() + 1;

        AssemblyRecordItemType item_type(QString::number(id));
        QVariantMap map;
        map.insert("name", dict.value("name"));
        map.insert("acquisition_price", dict.value("acquisition_price"));
        map.insert("list_price", dict.value("list_price"));
        map.insert("category_id", dict.value("category_id"));
        item_type.update(map);
    }
    return id;
}

void ModifyInspectionDialogueTab::assemblyRecordNumberChanged()
{
    if (original_arno == arno_w->text()) return;

    QSqlQuery query(QString("SELECT date FROM inspections WHERE arno = '%1'").arg(arno_w->text()));
    if (query.next()) {
        QMessageBox::warning(this, tr("Conflict"), tr("Inspection with the same assembly record number already exists."));
    }
}

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
