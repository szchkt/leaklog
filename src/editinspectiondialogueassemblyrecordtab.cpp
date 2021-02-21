/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2021 Matus & Michal Tomlein

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

#include "editinspectiondialogueassemblyrecordtab.h"

#include "editdialoguetable.h"
#include "editdialoguetablegroups.h"
#include "inputwidgets.h"
#include "editdialoguelayout.h"
#include "global.h"
#include "mtsqlquery.h"
#include "editinspectiondialogueaccess.h"

#include <QMessageBox>
#include <QApplication>

EditInspectionDialogueAssemblyRecordTab::EditInspectionDialogueAssemblyRecordTab(int, MDLineEdit *arno_w, MDComboBox *ar_type_w, EditInspectionDialogueAccess *inspection_dialogue_access, const QString &customer_uuid, const QString &circuit_uuid, QWidget *parent)
    : EditDialogueTab(parent),
      arno_being_changed(false),
      inspection_dialogue_access(inspection_dialogue_access)
{
    this->ar_type_w = ar_type_w;
    this->arno_w = arno_w;
    this->customer_uuid = customer_uuid;
    this->circuit_uuid = circuit_uuid;
    original_arno = arno_w->text();
    current_arno = original_arno;

    setName(tr("Assembly record"));

    init();

    QObject::connect(ar_type_w, SIGNAL(activated(int)), this, SLOT(recordTypeChanged()));
    QObject::connect(arno_w, SIGNAL(editingFinished()), this, SLOT(assemblyRecordNumberChanged()));
}

void EditInspectionDialogueAssemblyRecordTab::init()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    setLayout(layout);

    QGridLayout *form_grid = new QGridLayout;
    form_grid->setContentsMargins(0, 0, 0, 0);
    QList<MDAbstractInputWidget *> inputwidgets;
    arno_w->setRowSpan(1);
    inputwidgets.append(arno_w);
    ar_type_w->setRowSpan(1);
    inputwidgets.append(ar_type_w);
    EditDialogueColumnLayout(&inputwidgets, form_grid, 1).layout();

    layout->addLayout(form_grid);

    groups_layout = new EditDialogueGroupsLayout(this);
    groups_layout->addHeaderItem(-1, "name", QApplication::translate("AssemblyRecordItemCategory", "Name"), Global::String);
    groups_layout->addHeaderItem(AssemblyRecordItemCategory::ShowValue, "value", QApplication::translate("AssemblyRecordItem", "Value"), Global::String);
    if (DBInfo::isOperationPermitted("access_assembly_record_acquisition_price") > 0)
        groups_layout->addHeaderItem(AssemblyRecordItemCategory::ShowAcquisitionPrice, "acquisition_price", QApplication::translate("AssemblyRecordItem", "Acquisition price"), Global::Numeric);
    if (DBInfo::isOperationPermitted("access_assembly_record_list_price") > 0)
        groups_layout->addHeaderItem(AssemblyRecordItemCategory::ShowListPrice, "list_price", QApplication::translate("AssemblyRecordItem", "List price"), Global::Numeric);
    if (DBInfo::isOperationPermitted("access_assembly_record_list_price") > 0)
        groups_layout->addHeaderItem(AssemblyRecordItemCategory::ShowDiscount, "discount", QApplication::translate("AssemblyRecordItemType", "Discount"), Global::Numeric);
    layout->addWidget(groups_layout);

    loadItemInputWidgets(true);
}

const QVariant EditInspectionDialogueAssemblyRecordTab::assemblyRecordType()
{
    return ar_type_w->variantValue();
}

const QVariant EditInspectionDialogueAssemblyRecordTab::assemblyRecordId()
{
    return arno_w->variantValue();
}

void EditInspectionDialogueAssemblyRecordTab::recordTypeChanged()
{
    AssemblyRecordType type(assemblyRecordType().toString());
    QString name_format = type.nameFormat();

    if (!name_format.isEmpty()) {
        QDateTime current_date = QDateTime::fromString(inspection_dialogue_access->getVariableValue("date").toString(), DATE_TIME_FORMAT);
        name_format.replace("year", QString("%1").arg(current_date.date().year(), 4, 10, QChar('0')));
        name_format.replace("month", QString("%1").arg(current_date.date().month(), 2, 10, QChar('0')));
        name_format.replace("day", QString("%1").arg(current_date.date().day(), 2, 10, QChar('0')));
        name_format.replace("hour", QString("%1").arg(current_date.time().hour(), 2, 10, QChar('0')));
        name_format.replace("minute", QString("%1").arg(current_date.time().minute(), 2, 10, QChar('0')));

        name_format.replace("customer_id", customer_uuid);
        name_format.replace("circuit_id", circuit_uuid);

        if (name_format.contains("customer_name")) {
            name_format.replace("customer_name", Customer(customer_uuid).companyName());
        }
        if (name_format.contains("circuit_name")) {
            name_format.replace("circuit_name", Circuit(circuit_uuid).circuitName());
        }
        arno_w->setText(name_format);
    }

    loadItemInputWidgets();
}

void EditInspectionDialogueAssemblyRecordTab::loadItemInputWidgets(bool initial)
{
    QString currency = DBInfo::valueForKey("currency", "EUR");

    groups_layout->clear();
    QMap<QString, EditDialogueTableCell *> cells_map;

    {
        enum QUERY_RESULTS {
            TYPE_UUID,
            NAME,
            ACQUISITION_PRICE,
            LIST_PRICE,
            VALUE,
            CATEGORY_NAME,
            CATEGORY_UUID,
            DISPLAY_OPTIONS,
            ITEM_ACQUISITION_PRICE,
            ITEM_LIST_PRICE,
            INSPECTION_VAR,
            VALUE_DATA_TYPE,
            ITEM_NAME,
            ITEM_UNIT,
            UNIT,
            ITEM_DISCOUNT,
            DISCOUNT,
            AUTO_SHOW
        };

        MTSqlQuery items_query;
        items_query.prepare("SELECT assembly_record_item_types.uuid, assembly_record_item_types.name, assembly_record_item_types.acquisition_price, assembly_record_item_types.list_price,"
                            " assembly_record_items.value, assembly_record_item_categories.name, assembly_record_item_categories.uuid, assembly_record_item_categories.display_options,"
                            " assembly_record_items.acquisition_price, assembly_record_items.list_price, assembly_record_item_types.inspection_variable_id,"
                            " assembly_record_item_types.value_data_type, assembly_record_items.name, assembly_record_items.unit, assembly_record_item_types.unit,"
                            " assembly_record_items.discount, assembly_record_item_types.discount, assembly_record_item_types.auto_show"
                            " FROM assembly_record_item_types"
                            " LEFT JOIN assembly_record_items ON assembly_record_items.ar_item_type_uuid = assembly_record_item_types.uuid"
                            " AND assembly_record_items.arno = :arno AND assembly_record_items.source = :source"
                            " LEFT JOIN assembly_record_item_categories ON assembly_record_item_types.ar_item_category_uuid = assembly_record_item_categories.uuid"
                            " WHERE assembly_record_item_types.ar_item_category_uuid IN"
                            " (SELECT DISTINCT ar_item_category_uuid FROM assembly_record_type_categories WHERE ar_type_uuid = :ar_type_uuid)");
        items_query.bindValue(":arno", assemblyRecordId().toString());
        items_query.bindValue(":source", AssemblyRecordItem::AssemblyRecordItemTypes);
        items_query.bindValue(":ar_type_uuid", assemblyRecordType());
        items_query.exec();

        while (items_query.next()) {
            cells_map.insert("name", new EditDialogueTableCell(items_query.value(ITEM_NAME).isNull() ? items_query.value(NAME) : items_query.value(ITEM_NAME), "name"));
            cells_map.insert("value", new EditDialogueTableCell(items_query.value(VALUE), "value", items_query.value(INSPECTION_VAR).toString().isEmpty() ? items_query.value(VALUE_DATA_TYPE).toInt() : -1));
            cells_map.insert("ar_item_type_uuid", new EditDialogueTableCell(items_query.value(TYPE_UUID), "ar_item_type_uuid"));
            cells_map.insert("acquisition_price", new EditDialogueTableCell(items_query.value(ITEM_ACQUISITION_PRICE).isNull() ? items_query.value(ACQUISITION_PRICE) : items_query.value(ITEM_ACQUISITION_PRICE), "acquisition_price", Global::Numeric, currency));
            cells_map.insert("list_price", new EditDialogueTableCell(items_query.value(ITEM_LIST_PRICE).isNull() ? items_query.value(LIST_PRICE) : items_query.value(ITEM_LIST_PRICE), "list_price", Global::Numeric, currency));
            cells_map.insert("discount", new EditDialogueTableCell(items_query.value(ITEM_DISCOUNT).isNull() ? items_query.value(DISCOUNT) : items_query.value(ITEM_DISCOUNT), "discount", Global::Numeric, "%"));
            cells_map.insert("source", new EditDialogueTableCell(AssemblyRecordItem::AssemblyRecordItemTypes, "source"));
            cells_map.insert("ar_item_category_uuid", new EditDialogueTableCell(items_query.value(CATEGORY_UUID), "ar_item_category_uuid"));
            cells_map.insert("unit", new EditDialogueTableCell(items_query.value(ITEM_UNIT).isNull() ? items_query.value(UNIT) : items_query.value(ITEM_UNIT), "unit"));
            groups_layout->addItem(items_query.value(CATEGORY_NAME).toString(),
                                   items_query.value(CATEGORY_UUID).toString(),
                                   cells_map,
                                   items_query.value(DISPLAY_OPTIONS).toInt(),
                                   !items_query.value(VALUE).isNull() || !items_query.value(ITEM_LIST_PRICE).isNull() || (items_query.value(AUTO_SHOW).toBool() && !initial));
        }
    }

    {
        enum QUERY_RESULTS {
            TYPE_UUID,
            MANUFACTURER,
            TYPE,
            ACQUISITION_PRICE,
            LIST_PRICE,
            VALUE,
            CATEGORY_NAME,
            CATEGORY_UUID,
            DISPLAY_OPTIONS,
            ITEM_ACQUISITION_PRICE,
            ITEM_LIST_PRICE,
            ITEM_NAME,
            ITEM_UNIT,
            UNIT,
            ITEM_DISCOUNT,
            DISCOUNT,
            UNIT_ID
        };

        MTSqlQuery units_query;
        units_query.prepare(QString("SELECT circuit_unit_types.uuid, circuit_unit_types.manufacturer, circuit_unit_types.type, circuit_unit_types.acquisition_price, circuit_unit_types.list_price,"
                                    " assembly_record_items.value, assembly_record_item_categories.name, assembly_record_item_categories.uuid, assembly_record_item_categories.display_options,"
                                    " assembly_record_items.acquisition_price, assembly_record_items.list_price, assembly_record_items.name, assembly_record_items.unit, circuit_unit_types.unit,"
                                    " assembly_record_items.discount, circuit_unit_types.discount, circuit_units.uuid"
                                    " FROM circuit_units"
                                    " LEFT JOIN circuit_unit_types ON circuit_units.unit_type_uuid = circuit_unit_types.uuid"
                                    " LEFT JOIN assembly_record_items ON assembly_record_items.ar_item_type_uuid = circuit_units.uuid"
                                    " AND assembly_record_items.arno = :arno AND assembly_record_items.source = :source"
                                    " LEFT JOIN assembly_record_item_categories ON '%1' = assembly_record_item_categories.uuid"
                                    " WHERE circuit_units.circuit_uuid = :circuit_uuid AND '%1' IN"
                                    " (SELECT DISTINCT ar_item_category_uuid FROM assembly_record_type_categories WHERE ar_type_uuid = :ar_type_uuid)")
                                    .arg(CIRCUIT_UNITS_CATEGORY_UUID));
        units_query.bindValue(":arno", assemblyRecordId().toString());
        units_query.bindValue(":source", AssemblyRecordItem::CircuitUnitTypes);
        units_query.bindValue(":circuit_uuid", circuit_uuid);
        units_query.bindValue(":ar_type_uuid", assemblyRecordType());
        units_query.exec();

        while (units_query.next()) {
            QString name = units_query.value(ITEM_NAME).isNull() ?
                           QString("%1 - %2").arg(units_query.value(MANUFACTURER).toString()).arg(units_query.value(TYPE).toString())
                               : units_query.value(ITEM_NAME).toString();
            cells_map.insert("name", new EditDialogueTableCell(name, "name"));
            cells_map.insert("value", new EditDialogueTableCell(units_query.value(VALUE).toInt() ? units_query.value(VALUE) : 1, "value", Global::Integer));
            cells_map.insert("ar_item_type_uuid", new EditDialogueTableCell(units_query.value(UNIT_ID), "ar_item_type_uuid"));
            cells_map.insert("acquisition_price", new EditDialogueTableCell(units_query.value(ITEM_ACQUISITION_PRICE).isNull() ? units_query.value(ACQUISITION_PRICE) : units_query.value(ITEM_ACQUISITION_PRICE), "acquisition_price", Global::Numeric, currency));
            cells_map.insert("list_price", new EditDialogueTableCell(units_query.value(ITEM_LIST_PRICE).isNull() ? units_query.value(LIST_PRICE) : units_query.value(ITEM_LIST_PRICE), "list_price", Global::Numeric, currency));
            cells_map.insert("discount", new EditDialogueTableCell(units_query.value(ITEM_DISCOUNT).isNull() ? units_query.value(DISCOUNT) : units_query.value(ITEM_DISCOUNT), "discount", Global::Numeric, "%"));
            cells_map.insert("source", new EditDialogueTableCell(AssemblyRecordItem::CircuitUnitTypes, "source"));
            cells_map.insert("ar_item_category_uuid", new EditDialogueTableCell(units_query.value(CATEGORY_UUID), "ar_item_category_uuid"));
            cells_map.insert("unit", new EditDialogueTableCell(units_query.value(ITEM_UNIT).isNull() ? units_query.value(UNIT) : units_query.value(ITEM_UNIT), "unit"));
            groups_layout->addItem(units_query.value(CATEGORY_NAME).toString(),
                                   units_query.value(CATEGORY_UUID).toString(),
                                   cells_map,
                                   units_query.value(DISPLAY_OPTIONS).toInt(),
                                   !units_query.value(VALUE).isNull() || !units_query.value(ITEM_LIST_PRICE).isNull());
        }
    }

    {
        enum QUERY_RESULTS {
            INSPECTOR_UUID,
            NAME,
            ACQUISITION_PRICE,
            LIST_PRICE,
            VALUE,
            CATEGORY_NAME,
            CATEGORY_UUID,
            DISPLAY_OPTIONS,
            ITEM_ACQUISITION_PRICE,
            ITEM_LIST_PRICE,
            ITEM_NAME,
            ITEM_UNIT,
            ITEM_DISCOUNT
        };

        MTSqlQuery inspectors_query;
        inspectors_query.prepare(QString("SELECT inspectors.uuid, inspectors.person, inspectors.acquisition_price, inspectors.list_price,"
                                         " assembly_record_items.value, assembly_record_item_categories.name, assembly_record_item_categories.uuid, assembly_record_item_categories.display_options,"
                                         " assembly_record_items.acquisition_price, assembly_record_items.list_price, assembly_record_items.name, assembly_record_items.unit,"
                                         " assembly_record_items.discount"
                                         " FROM inspectors"
                                         " LEFT JOIN assembly_record_items ON assembly_record_items.ar_item_type_uuid = inspectors.uuid"
                                         " AND assembly_record_items.arno = :arno AND assembly_record_items.source = :source"
                                         " LEFT JOIN assembly_record_item_categories ON '%1' = assembly_record_item_categories.uuid"
                                         " WHERE '%1' IN"
                                         " (SELECT DISTINCT ar_item_category_uuid FROM assembly_record_type_categories WHERE ar_type_uuid = :record_type_id)")
                                         .arg(INSPECTORS_CATEGORY_UUID));
        inspectors_query.bindValue(":arno", assemblyRecordId().toString());
        inspectors_query.bindValue(":source", AssemblyRecordItem::Inspectors);
        inspectors_query.bindValue(":record_type_id", assemblyRecordType());
        inspectors_query.exec();

        while (inspectors_query.next()) {
            QString name = inspectors_query.value(ITEM_NAME).isNull() ? inspectors_query.value(NAME).toString() : inspectors_query.value(ITEM_NAME).toString();
            cells_map.insert("name", new EditDialogueTableCell(name, "name"));
            cells_map.insert("value", new EditDialogueTableCell(inspectors_query.value(VALUE).toInt() ? inspectors_query.value(VALUE) : 1, "value", Global::Numeric));
            cells_map.insert("ar_item_type_uuid", new EditDialogueTableCell(inspectors_query.value(INSPECTOR_UUID), "ar_item_type_uuid"));
            cells_map.insert("acquisition_price", new EditDialogueTableCell(inspectors_query.value(ITEM_ACQUISITION_PRICE).isNull() ? inspectors_query.value(ACQUISITION_PRICE) : inspectors_query.value(ITEM_ACQUISITION_PRICE), "acquisition_price", Global::Numeric, currency));
            cells_map.insert("list_price", new EditDialogueTableCell(inspectors_query.value(ITEM_LIST_PRICE).isNull() ? inspectors_query.value(LIST_PRICE) : inspectors_query.value(ITEM_LIST_PRICE), "list_price", Global::Numeric, currency));
            cells_map.insert("discount", new EditDialogueTableCell(inspectors_query.value(ITEM_DISCOUNT), "discount", Global::Numeric, "%"));
            cells_map.insert("source", new EditDialogueTableCell(AssemblyRecordItem::Inspectors, "source"));
            cells_map.insert("ar_item_category_uuid", new EditDialogueTableCell(inspectors_query.value(CATEGORY_UUID), "ar_item_category_uuid"));
            cells_map.insert("unit", new EditDialogueTableCell(inspectors_query.value(ITEM_UNIT), "unit"));
            groups_layout->addItem(inspectors_query.value(CATEGORY_NAME).toString(),
                                   inspectors_query.value(CATEGORY_UUID).toString(),
                                   cells_map,
                                   inspectors_query.value(DISPLAY_OPTIONS).toInt(),
                                   !inspectors_query.value(VALUE).isNull() || !inspectors_query.value(ITEM_LIST_PRICE).isNull());
        }
    }
}

void EditInspectionDialogueAssemblyRecordTab::save(const QString &)
{
    if (!original_arno.isEmpty()) {
        AssemblyRecordItem::query({{"arno", original_arno}}).removeAll();
    }

    QString arno = assemblyRecordId().toString();
    if (arno.isEmpty())
        return;

    if (arno != original_arno) {
        AssemblyRecordItem::query({{"arno", arno}}).removeAll();
    }

    QString ar_type_uuid = ar_type_w->variantValue().toString();
    auto inspections = Inspection::query({{"arno", arno}}).all();
    foreach (auto inspection, inspections) {
        inspection.setValue("ar_type_uuid", ar_type_uuid);
        inspection.save();
    }

    QList<QVariantMap> record_dicts = groups_layout->allValues();

    for (int i = 0; i < record_dicts.count(); ++i) {
        AssemblyRecordItem item;
        item.setArno(arno);
        item.setValue(record_dicts.at(i).value("value").toString());
        item.setItemTypeUUID(record_dicts.at(i).value("ar_item_type_uuid").toString().isEmpty()
                             ? saveNewItemType(record_dicts.at(i)) : record_dicts.at(i).value("ar_item_type_uuid").toString());
        item.setItemCategoryUUID(record_dicts.at(i).value("ar_item_category_uuid").toString());
        item.setValue("acquisition_price", record_dicts.at(i).value("acquisition_price"));
        item.setValue("list_price", record_dicts.at(i).value("list_price"));
        item.setName(record_dicts.at(i).value("name").toString());
        item.setValue("source", record_dicts.at(i).value("source"));
        item.setUnit(record_dicts.at(i).value("unit").toString());
        item.setValue("discount", record_dicts.at(i).value("discount"));
        item.save();
    }
}

QString EditInspectionDialogueAssemblyRecordTab::saveNewItemType(const QVariantMap &dict)
{
    AssemblyRecordItemType item_type;
    item_type.setValue("name", dict.value("name"));
    item_type.setValue("acquisition_price", dict.contains("acquisition_price") ? dict.value("acquisition_price") : dict.value("list_price"));
    item_type.setValue("list_price", dict.value("list_price"));
    item_type.setValue("ar_item_category_uuid", dict.value("ar_item_category_uuid"));
    item_type.save();
    return item_type.uuid();
}

void EditInspectionDialogueAssemblyRecordTab::assemblyRecordNumberChanged()
{
    if (arno_being_changed)
        return;
    arno_being_changed = true;

    if (current_arno == arno_w->text()) return;

    MTSqlQuery query;
    query.prepare("SELECT date, ar_type_uuid FROM inspections WHERE arno = :arno");
    query.bindValue(":arno", arno_w->text());
    if (query.next()) {
        QMessageBox message(this);
        message.setWindowTitle(tr("Assembly record number already in use"));
        message.setWindowModality(Qt::WindowModal);
        message.setWindowFlags(message.windowFlags() | Qt::Sheet);
        message.setIcon(QMessageBox::Information);
        message.setText(tr("An inspection with the same assembly record number already exists."));
        message.setInformativeText(tr("Use anyway?"));
        message.addButton(tr("&Use"), QMessageBox::AcceptRole);
        message.addButton(tr("Cancel"), QMessageBox::RejectRole);

        switch (message.exec()) {
            case 0: // Use
                current_arno = arno_w->text();
                ar_type_w->setVariantValue(query.value(1).toInt());
                loadItemInputWidgets();
                break;
            default: // Cancel
                arno_w->setText(current_arno);
                break;
        }
    }

    arno_being_changed = false;
    QApplication::processEvents();
}
