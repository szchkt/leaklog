#include "modify_inspection_dialogue.h"

#include "global.h"
#include "records.h"
#include "input_widgets.h"
#include "modify_dialogue_table.h"
#include "modify_dialogue_table_groups.h"

#include <QSqlQuery>
#include <QMessageBox>

ModifyInspectionDialogue::ModifyInspectionDialogue(DBRecord * record, QWidget * parent)
    : TabbedModifyDialogue(record, parent)
{
    main_tabw->setTabText(0, tr("Inspection"));

    MDInputWidget * rmds = inputWidget("rmds");
    md_grid_main->addWidget(rmds->label(), md_grid_main->rowCount(), 0);
    md_grid_main->addWidget(rmds->widget(), md_grid_main->rowCount() - 1, 1, -1, md_grid_main->columnCount() - 1);

    addTab(new ModifyInspectionDialogueTab(0, (MDLineEdit *) inputWidget("arno"), (MDComboBox *) inputWidget("ar_type"), md_record->parent("customer"), md_record->parent("circuit")));
}

ModifyInspectionDialogueTab::ModifyInspectionDialogueTab(int, MDLineEdit * arno_w, MDComboBox * ar_type_w, const QString & customer_id, const QString & circuit_id, QWidget * parent)
    : ModifyDialogueTab(parent)
{
    this->ar_type_w = ar_type_w;
    this->arno_w = arno_w;
    this->customer_id = customer_id;
    this->circuit_id = circuit_id;
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
    groups_layout->addHeaderItem(-1, "name", tr("Name"), Global::String);
    groups_layout->addHeaderItem(AssemblyRecordItemCategory::ShowValue, "value", tr("Value"), Global::String);
    groups_layout->addHeaderItem(AssemblyRecordItemCategory::ShowAcquisitionPrice, "acquisition_price", tr("Acquisition price"), Global::Numeric);
    groups_layout->addHeaderItem(AssemblyRecordItemCategory::ShowListPrice, "list_price", tr("List price"), Global::Numeric);
    groups_layout->addHeaderItem(AssemblyRecordItemCategory::ShowDiscount, "discount", tr("Discount"), Global::Numeric);
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
    {
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
            VALUE_DATA_TYPE = 11,
            ITEM_NAME = 12,
            ITEM_UNIT = 13,
            UNIT = 14,
            ITEM_DISCOUNT = 15,
            DISCOUNT = 16
                    };
        QSqlQuery items_query(QString("SELECT assembly_record_item_types.id, assembly_record_item_types.name, assembly_record_item_types.acquisition_price, assembly_record_item_types.list_price,"
                                      " assembly_record_items.value, assembly_record_item_categories.name, assembly_record_item_categories.id, assembly_record_item_categories.display_options,"
                                      " assembly_record_items.acquisition_price, assembly_record_items.list_price, assembly_record_item_types.inspection_variable_id,"
                                      " assembly_record_item_types.value_data_type, assembly_record_items.name, assembly_record_items.unit, assembly_record_item_types.unit,"
                                      " assembly_record_items.discount, assembly_record_item_types.discount"
                                      " FROM assembly_record_item_types"
                                      " LEFT JOIN assembly_record_items ON assembly_record_items.item_type_id = assembly_record_item_types.id"
                                      " AND assembly_record_items.arno = '%1' AND assembly_record_items.source = %2"
                                      " LEFT JOIN assembly_record_item_categories ON assembly_record_item_types.category_id = assembly_record_item_categories.id"
                                      " WHERE assembly_record_item_types.category_id IN"
                                      " (SELECT DISTINCT record_category_id FROM assembly_record_type_categories WHERE record_type_id = %3)")
                              .arg(assemblyRecordId().toString())
                              .arg(AssemblyRecordItem::AssemblyRecordItemTypes)
                              .arg(assemblyRecordType().toString()));
        while (items_query.next()) {
            cells_map.insert("name", new ModifyDialogueTableCell(items_query.value(ITEM_NAME).isNull() ? items_query.value(NAME) : items_query.value(ITEM_NAME), "name"));
            cells_map.insert("value", new ModifyDialogueTableCell(items_query.value(VALUE), "value", items_query.value(INSPECTION_VAR).toString().isEmpty() ? items_query.value(VALUE_DATA_TYPE).toInt() : -1));
            cells_map.insert("item_type_id", new ModifyDialogueTableCell(items_query.value(TYPE_ID), "item_type_id"));
            cells_map.insert("acquisition_price", new ModifyDialogueTableCell(items_query.value(ITEM_ACQUISITION_PRICE).isNull() ? items_query.value(ACQUISITION_PRICE) : items_query.value(ITEM_ACQUISITION_PRICE), "acquisition_price", Global::Numeric));
            cells_map.insert("list_price", new ModifyDialogueTableCell(items_query.value(ITEM_LIST_PRICE).isNull() ? items_query.value(ACQUISITION_PRICE) : items_query.value(ITEM_LIST_PRICE), "list_price", Global::Numeric));
            cells_map.insert("discount", new ModifyDialogueTableCell(items_query.value(ITEM_DISCOUNT).isNull() ? items_query.value(DISCOUNT) : items_query.value(ITEM_DISCOUNT), "discount", Global::Numeric, "%"));
            cells_map.insert("source", new ModifyDialogueTableCell(AssemblyRecordItem::AssemblyRecordItemTypes, "source"));
            cells_map.insert("category_id", new ModifyDialogueTableCell(items_query.value(CATEGORY_ID), "category_id"));
            cells_map.insert("unit", new ModifyDialogueTableCell(items_query.value(ITEM_UNIT).isNull() ? items_query.value(UNIT) : items_query.value(ITEM_UNIT), "unit"));
            groups_layout->addItem(items_query.value(CATEGORY_NAME).toString(),
                                   items_query.value(CATEGORY_ID).toInt(),
                                   cells_map,
                                   items_query.value(DISPLAY_OPTIONS).toInt(),
                                   !items_query.value(VALUE).isNull() || !items_query.value(ITEM_LIST_PRICE).isNull());
        }
    }
    {
        enum QUERY_RESULTS
        {
            TYPE_ID = 0,
            MANUFACTURER = 1,
            TYPE = 2,
            ACQUISITION_PRICE = 3,
            LIST_PRICE = 4,
            VALUE = 5,
            CATEGORY_NAME = 6,
            CATEGORY_ID = 7,
            DISPLAY_OPTIONS = 8,
            ITEM_ACQUISITION_PRICE = 9,
            ITEM_LIST_PRICE = 10,
            ITEM_NAME = 11,
            ITEM_UNIT = 12,
            UNIT = 13,
            ITEM_DISCOUNT = 14,
            DISCOUNT = 15
                    };
        QSqlQuery units_query(QString("SELECT circuit_unit_types.id, circuit_unit_types.manufacturer, circuit_unit_types.type, circuit_unit_types.acquisition_price, circuit_unit_types.list_price,"
                                      " assembly_record_items.value, assembly_record_item_categories.name, assembly_record_item_categories.id, assembly_record_item_categories.display_options,"
                                      " assembly_record_items.acquisition_price, assembly_record_items.list_price, assembly_record_items.name, assembly_record_items.unit, circuit_unit_types.unit,"
                                      " assembly_record_items.discount, circuit_unit_types.discount"
                                      " FROM circuit_units"
                                      " LEFT JOIN circuit_unit_types ON circuit_units.unit_type_id = circuit_unit_types.id"
                                      " LEFT JOIN assembly_record_items ON assembly_record_items.item_type_id = circuit_unit_types.id"
                                      " AND assembly_record_items.arno = '%1' AND assembly_record_items.source = %2"
                                      " LEFT JOIN assembly_record_item_categories ON circuit_unit_types.category_id = assembly_record_item_categories.id"
                                      " WHERE circuit_units.company_id = %3 AND circuit_units.circuit_id = %4 AND assembly_record_item_categories.id IN"
                                      " (SELECT DISTINCT record_category_id FROM assembly_record_type_categories WHERE record_type_id = %5)")
                              .arg(assemblyRecordId().toString())
                              .arg(AssemblyRecordItem::CircuitUnitTypes)
                              .arg(customer_id)
                              .arg(circuit_id)
                              .arg(assemblyRecordType().toString()));
        while (units_query.next()) {
            QString name = units_query.value(ITEM_NAME).isNull() ?
                           QString("%1 - %2").arg(units_query.value(MANUFACTURER).toString()).arg(units_query.value(TYPE).toString())
                               : units_query.value(ITEM_NAME).toString();
            cells_map.insert("name", new ModifyDialogueTableCell(name, "name", Global::String));
            cells_map.insert("value", new ModifyDialogueTableCell(units_query.value(VALUE).toInt() ? units_query.value(VALUE) : 1, "value", Global::Integer));
            cells_map.insert("item_type_id", new ModifyDialogueTableCell(units_query.value(TYPE_ID), "item_type_id"));
            cells_map.insert("acquisition_price", new ModifyDialogueTableCell(units_query.value(ITEM_ACQUISITION_PRICE).isNull() ? units_query.value(ACQUISITION_PRICE) : units_query.value(ITEM_ACQUISITION_PRICE), "acquisition_price", Global::Numeric));
            cells_map.insert("list_price", new ModifyDialogueTableCell(units_query.value(ITEM_LIST_PRICE).isNull() ? units_query.value(ACQUISITION_PRICE) : units_query.value(ITEM_LIST_PRICE), "list_price", Global::Numeric));
            cells_map.insert("discount", new ModifyDialogueTableCell(units_query.value(ITEM_DISCOUNT).isNull() ? units_query.value(DISCOUNT) : units_query.value(ITEM_DISCOUNT), "discount", Global::Numeric, "%"));
            cells_map.insert("source", new ModifyDialogueTableCell(AssemblyRecordItem::CircuitUnitTypes, "source"));
            cells_map.insert("category_id", new ModifyDialogueTableCell(units_query.value(CATEGORY_ID), "category_id"));
            cells_map.insert("unit", new ModifyDialogueTableCell(units_query.value(ITEM_UNIT).isNull() ? units_query.value(UNIT) : units_query.value(ITEM_UNIT), "unit"));
            groups_layout->addItem(units_query.value(CATEGORY_NAME).toString(),
                                   units_query.value(CATEGORY_ID).toInt(),
                                   cells_map,
                                   units_query.value(DISPLAY_OPTIONS).toInt(),
                                   !units_query.value(VALUE).isNull() || !units_query.value(ITEM_LIST_PRICE).isNull());
        }
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
        map.insert("name", record_dicts.at(i).value("name"));
        map.insert("source", record_dicts.at(i).value("source"));
        map.insert("category_id", record_dicts.at(i).value("category_id"));
        map.insert("unit", record_dicts.at(i).value("unit"));
        map.insert("discount", record_dicts.at(i).value("discount"));
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
