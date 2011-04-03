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

    MDAbstractInputWidget * rmds = inputWidget("rmds");
    md_grid_main->addWidget(rmds->label()->widget(), md_grid_main->rowCount(), 0);
    md_grid_main->addWidget(rmds->widget(), md_grid_main->rowCount() - 1, 1, -1, md_grid_main->columnCount() - 1);

    addTab(new ModifyInspectionDialogueTab(0, (MDLineEdit *) inputWidget("arno"), (MDComboBox *) inputWidget("ar_type"), md_record->parent("customer"), md_record->parent("circuit")));
    addTab(new ModifyInspectionDialogueImagesTab(md_record->parent("customer"), md_record->parent("circuit"), idFieldValue().toString()));
}

const QVariant ModifyInspectionDialogue::idFieldValue()
{
    MDAbstractInputWidget * iw = inputWidget("date");
    if (iw)
        return iw->variantValue();
    else
        return QVariant();
}

ModifyInspectionDialogueTab::ModifyInspectionDialogueTab(int, MDLineEdit * arno_w, MDComboBox * ar_type_w, const QString & customer_id, const QString & circuit_id, QWidget * parent)
    : ModifyDialogueTab(parent)
{
    this->ar_type_w = ar_type_w;
    this->arno_w = arno_w;
    this->customer_id = customer_id;
    this->circuit_id = circuit_id;
    original_arno = arno_w->text();

    setName(tr("Assembly record"));

    init();

    QObject::connect(ar_type_w, SIGNAL(currentIndexChanged(int)), this, SLOT(recordTypeChanged()));
    QObject::connect(arno_w, SIGNAL(editingFinished()), this, SLOT(assemblyRecordNumberChanged()));
}

void ModifyInspectionDialogueTab::init()
{
    QVBoxLayout * layout = new QVBoxLayout(this);
    setLayout(layout);

    QGridLayout * form_grid = new QGridLayout;
    QList<MDAbstractInputWidget *> inputwidgets;
    arno_w->setShowInForm(true);
    inputwidgets.append(arno_w);
    ar_type_w->setShowInForm(true);
    inputwidgets.append(ar_type_w);
    ModifyDialogueColumnLayout(&inputwidgets, form_grid, 1).layout();

    layout->addLayout(form_grid);

    groups_layout = new ModifyDialogueGroupsLayout(this);
    groups_layout->addHeaderItem(-1, "name", tr("Name"), Global::String);
    groups_layout->addHeaderItem(AssemblyRecordItemCategory::ShowValue, "value", tr("Value"), Global::String);
    if (Global::isOperationPermitted("access_assembly_record_acquisition_price") > 0)
        groups_layout->addHeaderItem(AssemblyRecordItemCategory::ShowAcquisitionPrice, "acquisition_price", tr("Acquisition price"), Global::Numeric);
    if (Global::isOperationPermitted("access_assembly_record_list_price") > 0)
        groups_layout->addHeaderItem(AssemblyRecordItemCategory::ShowListPrice, "list_price", tr("List price"), Global::Numeric);
    if (Global::isOperationPermitted("access_assembly_record_list_price") > 0)
        groups_layout->addHeaderItem(AssemblyRecordItemCategory::ShowDiscount, "discount", tr("Discount"), Global::Numeric);
    layout->addWidget(groups_layout);

    layout->addStretch();

    loadItemInputWidgets(true);
}

const QVariant ModifyInspectionDialogueTab::assemblyRecordType()
{
    return ar_type_w->variantValue();
}

const QVariant ModifyInspectionDialogueTab::assemblyRecordId()
{
    return arno_w->variantValue();
}

void ModifyInspectionDialogueTab::recordTypeChanged()
{
    AssemblyRecordType type_rec(assemblyRecordType().toString());
    QVariantMap type = type_rec.list();
    QString name_format = type.value("name_format").toString();

    if (!name_format.isEmpty()) {
        QDate current_date = QDate::currentDate();
        name_format.replace("year", QString::number(current_date.year()));
        name_format.replace("month", QString::number(current_date.month()));
        name_format.replace("day", QString::number(current_date.day()));

        name_format.replace("customer_id", customer_id);
        name_format.replace("circuit_id", circuit_id);

        if (name_format.contains("customer_name")) {
            name_format.replace("customer_name", Customer(customer_id).list().value("company").toString());
        }
        if (name_format.contains("circuit_name")) {
            name_format.replace("circuit_name", Circuit(customer_id, circuit_id).list().value("name").toString());
        }
        arno_w->setText(name_format);
    }

    loadItemInputWidgets();
}

void ModifyInspectionDialogueTab::loadItemInputWidgets(bool initial)
{
    QString currency = Global::DBInfoValueForKey("currency", "EUR");

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
            DISCOUNT = 16,
            AUTO_SHOW = 17
                    };
        QSqlQuery items_query(QString("SELECT assembly_record_item_types.id, assembly_record_item_types.name, assembly_record_item_types.acquisition_price, assembly_record_item_types.list_price,"
                                      " assembly_record_items.value, assembly_record_item_categories.name, assembly_record_item_categories.id, assembly_record_item_categories.display_options,"
                                      " assembly_record_items.acquisition_price, assembly_record_items.list_price, assembly_record_item_types.inspection_variable_id,"
                                      " assembly_record_item_types.value_data_type, assembly_record_items.name, assembly_record_items.unit, assembly_record_item_types.unit,"
                                      " assembly_record_items.discount, assembly_record_item_types.discount, assembly_record_item_types.auto_show"
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
            cells_map.insert("acquisition_price", new ModifyDialogueTableCell(items_query.value(ITEM_ACQUISITION_PRICE).isNull() ? items_query.value(ACQUISITION_PRICE) : items_query.value(ITEM_ACQUISITION_PRICE), "acquisition_price", Global::Numeric, currency));
            cells_map.insert("list_price", new ModifyDialogueTableCell(items_query.value(ITEM_LIST_PRICE).isNull() ? items_query.value(LIST_PRICE) : items_query.value(ITEM_LIST_PRICE), "list_price", Global::Numeric, currency));
            cells_map.insert("discount", new ModifyDialogueTableCell(items_query.value(ITEM_DISCOUNT).isNull() ? items_query.value(DISCOUNT) : items_query.value(ITEM_DISCOUNT), "discount", Global::Numeric, "%"));
            cells_map.insert("source", new ModifyDialogueTableCell(AssemblyRecordItem::AssemblyRecordItemTypes, "source"));
            cells_map.insert("category_id", new ModifyDialogueTableCell(items_query.value(CATEGORY_ID), "category_id"));
            cells_map.insert("unit", new ModifyDialogueTableCell(items_query.value(ITEM_UNIT).isNull() ? items_query.value(UNIT) : items_query.value(ITEM_UNIT), "unit"));
            groups_layout->addItem(items_query.value(CATEGORY_NAME).toString(),
                                   items_query.value(CATEGORY_ID).toInt(),
                                   cells_map,
                                   items_query.value(DISPLAY_OPTIONS).toInt(),
                                   !items_query.value(VALUE).isNull() || !items_query.value(ITEM_LIST_PRICE).isNull() || (items_query.value(AUTO_SHOW).toBool() && !initial));
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
            DISCOUNT = 15,
            UNIT_ID = 16
                    };
        QSqlQuery units_query(QString("SELECT circuit_unit_types.id, circuit_unit_types.manufacturer, circuit_unit_types.type, circuit_unit_types.acquisition_price, circuit_unit_types.list_price,"
                                      " assembly_record_items.value, assembly_record_item_categories.name, assembly_record_item_categories.id, assembly_record_item_categories.display_options,"
                                      " assembly_record_items.acquisition_price, assembly_record_items.list_price, assembly_record_items.name, assembly_record_items.unit, circuit_unit_types.unit,"
                                      " assembly_record_items.discount, circuit_unit_types.discount, circuit_units.id"
                                      " FROM circuit_units"
                                      " LEFT JOIN circuit_unit_types ON circuit_units.unit_type_id = circuit_unit_types.id"
                                      " LEFT JOIN assembly_record_items ON assembly_record_items.item_type_id = circuit_units.id"
                                      " AND assembly_record_items.arno = '%1' AND assembly_record_items.source = %2"
                                      " LEFT JOIN assembly_record_item_categories ON %5 = assembly_record_item_categories.id"
                                      " WHERE circuit_units.company_id = %3 AND circuit_units.circuit_id = %4 AND %5 IN"
                                      " (SELECT DISTINCT record_category_id FROM assembly_record_type_categories WHERE record_type_id = %6)")
                              .arg(assemblyRecordId().toString())
                              .arg(AssemblyRecordItem::CircuitUnitTypes)
                              .arg(customer_id)
                              .arg(circuit_id)
                              .arg(CIRCUIT_UNITS_CATEGORY_ID)
                              .arg(assemblyRecordType().toString()));
        while (units_query.next()) {
            QString name = units_query.value(ITEM_NAME).isNull() ?
                           QString("%1 - %2").arg(units_query.value(MANUFACTURER).toString()).arg(units_query.value(TYPE).toString())
                               : units_query.value(ITEM_NAME).toString();
            cells_map.insert("name", new ModifyDialogueTableCell(name, "name"));
            cells_map.insert("value", new ModifyDialogueTableCell(units_query.value(VALUE).toInt() ? units_query.value(VALUE) : 1, "value", Global::Integer));
            cells_map.insert("item_type_id", new ModifyDialogueTableCell(units_query.value(UNIT_ID), "item_type_id"));
            cells_map.insert("acquisition_price", new ModifyDialogueTableCell(units_query.value(ITEM_ACQUISITION_PRICE).isNull() ? units_query.value(ACQUISITION_PRICE) : units_query.value(ITEM_ACQUISITION_PRICE), "acquisition_price", Global::Numeric, currency));
            cells_map.insert("list_price", new ModifyDialogueTableCell(units_query.value(ITEM_LIST_PRICE).isNull() ? units_query.value(LIST_PRICE) : units_query.value(ITEM_LIST_PRICE), "list_price", Global::Numeric, currency));
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
    {
        enum QUERY_RESULTS
        {
            INSPECTOR_ID = 0,
            NAME = 1,
            ACQUISITION_PRICE = 2,
            LIST_PRICE = 3,
            VALUE = 4,
            CATEGORY_NAME = 5,
            CATEGORY_ID = 6,
            DISPLAY_OPTIONS = 7,
            ITEM_ACQUISITION_PRICE = 8,
            ITEM_LIST_PRICE = 9,
            ITEM_NAME = 10,
            ITEM_UNIT = 11,
            ITEM_DISCOUNT = 12
        };
        QSqlQuery inspectors_query(QString("SELECT inspectors.id, inspectors.person, inspectors.acquisition_price, inspectors.list_price,"
                                      " assembly_record_items.value, assembly_record_item_categories.name, assembly_record_item_categories.id, assembly_record_item_categories.display_options,"
                                      " assembly_record_items.acquisition_price, assembly_record_items.list_price, assembly_record_items.name, assembly_record_items.unit,"
                                      " assembly_record_items.discount"
                                      " FROM inspectors"
                                      " LEFT JOIN assembly_record_items ON assembly_record_items.item_type_id = inspectors.id"
                                      " AND assembly_record_items.arno = '%1' AND assembly_record_items.source = %2"
                                      " LEFT JOIN assembly_record_item_categories ON %3 = assembly_record_item_categories.id"
                                      " WHERE %3 IN"
                                      " (SELECT DISTINCT record_category_id FROM assembly_record_type_categories WHERE record_type_id = %4)")
                              .arg(assemblyRecordId().toString())
                              .arg(AssemblyRecordItem::Inspectors)
                              .arg(INSPECTORS_CATEGORY_ID)
                              .arg(assemblyRecordType().toString()));
        while (inspectors_query.next()) {
            QString name = inspectors_query.value(ITEM_NAME).isNull() ? inspectors_query.value(NAME).toString() : inspectors_query.value(ITEM_NAME).toString();
            cells_map.insert("name", new ModifyDialogueTableCell(name, "name"));
            cells_map.insert("value", new ModifyDialogueTableCell(inspectors_query.value(VALUE).toInt() ? inspectors_query.value(VALUE) : 1, "value", Global::Numeric));
            cells_map.insert("item_type_id", new ModifyDialogueTableCell(inspectors_query.value(INSPECTOR_ID), "item_type_id"));
            cells_map.insert("acquisition_price", new ModifyDialogueTableCell(inspectors_query.value(ITEM_ACQUISITION_PRICE).isNull() ? inspectors_query.value(ACQUISITION_PRICE) : inspectors_query.value(ITEM_ACQUISITION_PRICE), "acquisition_price", Global::Numeric, currency));
            cells_map.insert("list_price", new ModifyDialogueTableCell(inspectors_query.value(ITEM_LIST_PRICE).isNull() ? inspectors_query.value(LIST_PRICE) : inspectors_query.value(ITEM_LIST_PRICE), "list_price", Global::Numeric, currency));
            cells_map.insert("discount", new ModifyDialogueTableCell(inspectors_query.value(ITEM_DISCOUNT), "discount", Global::Numeric, "%"));
            cells_map.insert("source", new ModifyDialogueTableCell(AssemblyRecordItem::Inspectors, "source"));
            cells_map.insert("category_id", new ModifyDialogueTableCell(inspectors_query.value(CATEGORY_ID), "category_id"));
            cells_map.insert("unit", new ModifyDialogueTableCell(inspectors_query.value(ITEM_UNIT), "unit"));
            groups_layout->addItem(inspectors_query.value(CATEGORY_NAME).toString(),
                                   inspectors_query.value(CATEGORY_ID).toInt(),
                                   cells_map,
                                   inspectors_query.value(DISPLAY_OPTIONS).toInt(),
                                   !inspectors_query.value(VALUE).isNull() || !inspectors_query.value(ITEM_LIST_PRICE).isNull());
        }
    }
}

void ModifyInspectionDialogueTab::save(const QVariant &)
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

ModifyInspectionDialogueImagesTab::ModifyInspectionDialogueImagesTab(const QString & customer_id, const QString & circuit_id, const QString & inspection_id)
{
    this->customer_id = customer_id;
    this->circuit_id = circuit_id;

    setName(tr("Images"));

    init(inspection_id);
}

void ModifyInspectionDialogueImagesTab::init(const QString & inspection_id)
{
    QVBoxLayout * layout = new QVBoxLayout(this);

    QList<ModifyDialogueTableCell *> cells;
    ModifyDialogueTableCell * cell = new ModifyDialogueTableCell(tr("Description"), Global::Text);
    cell->setId("description");
    cells.append(cell);
    cell = new ModifyDialogueTableCell(tr("Image"), Global::File);
    cell->setId("file_id");
    cells.append(cell);
    table = new ModifyDialogueBasicTable(tr("Images"), cells, this);
    layout->addWidget(table);

    loadItemInputWidgets(inspection_id);
}

void ModifyInspectionDialogueImagesTab::loadItemInputWidgets(const QString & inspection_id)
{
    InspectionImage images_record(customer_id, circuit_id, inspection_id);
    ListOfVariantMaps images = images_record.listAll();

    QMap<QString, ModifyDialogueTableCell *> image_data;
    ModifyDialogueTableCell * cell;

    for (int i = 0; i < images.count(); ++i) {
        cell = new ModifyDialogueTableCell(images.at(i).value("description"), Global::Text);
        cell->setId("description");
        image_data.insert("description", cell);
        cell = new ModifyDialogueTableCell(images.at(i).value("file_id"), Global::File);
        cell->setId("file_id");
        image_data.insert("file_id", cell);
        table->addRow(image_data);
    }

    if (!images.count()) table->addNewRow();
}

void ModifyInspectionDialogueImagesTab::save(const QVariant & inspection_id)
{
    QVariantMap map;

    QList<MTDictionary> dicts = table->allValues();
    QList<int> undeleted_files;

    InspectionImage images_record(customer_id, circuit_id, inspection_id.toString());

    ListOfVariantMaps images = images_record.listAll("file_id");

    for (int i = 0; i < images.count(); ++i) {
        int file_id = images.at(i).value("file_id").toInt();
        if (!undeleted_files.contains(file_id))
            undeleted_files.append(file_id);
    }

    images_record.remove();

    for (int i = 0; i < dicts.count(); ++i) {
        int file_id = dicts.at(i).value("file_id").toInt();
        if (file_id <= 0) continue;
        undeleted_files.removeAll(file_id);

        map.insert("description", dicts.at(i).value("description"));
        map.insert("file_id", file_id);
        images_record.update(map);
    }

    for (int i = 0; i < undeleted_files.count(); ++i) {
        DBFile file(undeleted_files.at(i));
        file.remove();
    }
}
