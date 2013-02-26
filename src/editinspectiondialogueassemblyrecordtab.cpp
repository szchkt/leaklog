/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2013 Matus & Michal Tomlein

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

EditInspectionDialogueAssemblyRecordTab::EditInspectionDialogueAssemblyRecordTab(int, MDLineEdit * arno_w, MDComboBox * ar_type_w, EditInspectionDialogueAccess * inspection_dialogue_access, const QString & customer_id, const QString & circuit_id, QWidget * parent)
    : EditDialogueTab(parent),
      arno_being_changed(false),
      inspection_dialogue_access(inspection_dialogue_access)
{
    this->ar_type_w = ar_type_w;
    this->arno_w = arno_w;
    this->customer_id = customer_id;
    this->circuit_id = circuit_id;
    original_arno = arno_w->text();
    current_arno = original_arno;

    setName(tr("Assembly record"));

    init();

    QObject::connect(ar_type_w, SIGNAL(activated(int)), this, SLOT(recordTypeChanged()));
    QObject::connect(arno_w, SIGNAL(editingFinished()), this, SLOT(assemblyRecordNumberChanged()));
}

void EditInspectionDialogueAssemblyRecordTab::init()
{
    QVBoxLayout * layout = new QVBoxLayout(this);
    setLayout(layout);

    QGridLayout * form_grid = new QGridLayout;
    form_grid->setContentsMargins(0, 0, 0, 0);
    QList<MDAbstractInputWidget *> inputwidgets;
    arno_w->setShowInForm(true);
    inputwidgets.append(arno_w);
    ar_type_w->setShowInForm(true);
    inputwidgets.append(ar_type_w);
    EditDialogueColumnLayout(&inputwidgets, form_grid, 1).layout();

    layout->addLayout(form_grid);

    groups_layout = new EditDialogueGroupsLayout(this);
    groups_layout->addHeaderItem(-1, "name", tr("Name"), Global::String);
    groups_layout->addHeaderItem(AssemblyRecordItemCategory::ShowValue, "value", tr("Value"), Global::String);
    if (Global::isOperationPermitted("access_assembly_record_acquisition_price") > 0)
        groups_layout->addHeaderItem(AssemblyRecordItemCategory::ShowAcquisitionPrice, "acquisition_price", tr("Acquisition price"), Global::Numeric);
    if (Global::isOperationPermitted("access_assembly_record_list_price") > 0)
        groups_layout->addHeaderItem(AssemblyRecordItemCategory::ShowListPrice, "list_price", tr("List price"), Global::Numeric);
    if (Global::isOperationPermitted("access_assembly_record_list_price") > 0)
        groups_layout->addHeaderItem(AssemblyRecordItemCategory::ShowDiscount, "discount", tr("Discount"), Global::Numeric);
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
    AssemblyRecordType type_rec(assemblyRecordType().toString());
    QVariantMap type = type_rec.list();
    QString name_format = type.value("name_format").toString();

    if (!name_format.isEmpty()) {
        QDate current_date = QDateTime::fromString(inspection_dialogue_access->getVariableValue("date").toString(), DATE_TIME_FORMAT).date();
        name_format.replace("year", QString("%1").arg(current_date.year(), 4, 10, QChar('0')));
        name_format.replace("month", QString("%1").arg(current_date.month(), 2, 10, QChar('0')));
        name_format.replace("day", QString("%1").arg(current_date.day(), 2, 10, QChar('0')));

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

void EditInspectionDialogueAssemblyRecordTab::loadItemInputWidgets(bool initial)
{
    QString currency = Global::DBInfoValueForKey("currency", "EUR");

    groups_layout->clear();
    QMap<QString, EditDialogueTableCell *> cells_map;
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
        MTSqlQuery items_query;
        items_query.prepare("SELECT assembly_record_item_types.id, assembly_record_item_types.name, assembly_record_item_types.acquisition_price, assembly_record_item_types.list_price,"
                            " assembly_record_items.value, assembly_record_item_categories.name, assembly_record_item_categories.id, assembly_record_item_categories.display_options,"
                            " assembly_record_items.acquisition_price, assembly_record_items.list_price, assembly_record_item_types.inspection_variable_id,"
                            " assembly_record_item_types.value_data_type, assembly_record_items.name, assembly_record_items.unit, assembly_record_item_types.unit,"
                            " assembly_record_items.discount, assembly_record_item_types.discount, assembly_record_item_types.auto_show"
                            " FROM assembly_record_item_types"
                            " LEFT JOIN assembly_record_items ON assembly_record_items.item_type_id = assembly_record_item_types.id"
                            " AND assembly_record_items.arno = :arno AND assembly_record_items.source = :source"
                            " LEFT JOIN assembly_record_item_categories ON assembly_record_item_types.category_id = assembly_record_item_categories.id"
                            " WHERE assembly_record_item_types.category_id IN"
                            " (SELECT DISTINCT record_category_id FROM assembly_record_type_categories WHERE record_type_id = :ar_type)");
        items_query.bindValue(":arno", assemblyRecordId().toString());
        items_query.bindValue(":source", AssemblyRecordItem::AssemblyRecordItemTypes);
        items_query.bindValue(":ar_type", assemblyRecordType().toInt());
        items_query.exec();

        while (items_query.next()) {
            cells_map.insert("name", new EditDialogueTableCell(items_query.value(ITEM_NAME).isNull() ? items_query.value(NAME) : items_query.value(ITEM_NAME), "name"));
            cells_map.insert("value", new EditDialogueTableCell(items_query.value(VALUE), "value", items_query.value(INSPECTION_VAR).toString().isEmpty() ? items_query.value(VALUE_DATA_TYPE).toInt() : -1));
            cells_map.insert("item_type_id", new EditDialogueTableCell(items_query.value(TYPE_ID), "item_type_id"));
            cells_map.insert("acquisition_price", new EditDialogueTableCell(items_query.value(ITEM_ACQUISITION_PRICE).isNull() ? items_query.value(ACQUISITION_PRICE) : items_query.value(ITEM_ACQUISITION_PRICE), "acquisition_price", Global::Numeric, currency));
            cells_map.insert("list_price", new EditDialogueTableCell(items_query.value(ITEM_LIST_PRICE).isNull() ? items_query.value(LIST_PRICE) : items_query.value(ITEM_LIST_PRICE), "list_price", Global::Numeric, currency));
            cells_map.insert("discount", new EditDialogueTableCell(items_query.value(ITEM_DISCOUNT).isNull() ? items_query.value(DISCOUNT) : items_query.value(ITEM_DISCOUNT), "discount", Global::Numeric, "%"));
            cells_map.insert("source", new EditDialogueTableCell(AssemblyRecordItem::AssemblyRecordItemTypes, "source"));
            cells_map.insert("category_id", new EditDialogueTableCell(items_query.value(CATEGORY_ID), "category_id"));
            cells_map.insert("unit", new EditDialogueTableCell(items_query.value(ITEM_UNIT).isNull() ? items_query.value(UNIT) : items_query.value(ITEM_UNIT), "unit"));
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
        MTSqlQuery units_query;
        units_query.prepare(QString("SELECT circuit_unit_types.id, circuit_unit_types.manufacturer, circuit_unit_types.type, circuit_unit_types.acquisition_price, circuit_unit_types.list_price,"
                                    " assembly_record_items.value, assembly_record_item_categories.name, assembly_record_item_categories.id, assembly_record_item_categories.display_options,"
                                    " assembly_record_items.acquisition_price, assembly_record_items.list_price, assembly_record_items.name, assembly_record_items.unit, circuit_unit_types.unit,"
                                    " assembly_record_items.discount, circuit_unit_types.discount, circuit_units.id"
                                    " FROM circuit_units"
                                    " LEFT JOIN circuit_unit_types ON circuit_units.unit_type_id = circuit_unit_types.id"
                                    " LEFT JOIN assembly_record_items ON assembly_record_items.item_type_id = circuit_units.id"
                                    " AND assembly_record_items.arno = :arno AND assembly_record_items.source = :source"
                                    " LEFT JOIN assembly_record_item_categories ON %1 = assembly_record_item_categories.id"
                                    " WHERE circuit_units.company_id = :customer_id AND circuit_units.circuit_id = :circuit_id AND %1 IN"
                                    " (SELECT DISTINCT record_category_id FROM assembly_record_type_categories WHERE record_type_id = :ar_type)")
                                    .arg(CIRCUIT_UNITS_CATEGORY_ID));
        units_query.bindValue(":arno", assemblyRecordId().toString());
        units_query.bindValue(":source", AssemblyRecordItem::CircuitUnitTypes);
        units_query.bindValue(":customer_id", customer_id);
        units_query.bindValue(":circuit_id", circuit_id);
        units_query.bindValue(":ar_type", assemblyRecordType().toInt());
        units_query.exec();

        while (units_query.next()) {
            QString name = units_query.value(ITEM_NAME).isNull() ?
                           QString("%1 - %2").arg(units_query.value(MANUFACTURER).toString()).arg(units_query.value(TYPE).toString())
                               : units_query.value(ITEM_NAME).toString();
            cells_map.insert("name", new EditDialogueTableCell(name, "name"));
            cells_map.insert("value", new EditDialogueTableCell(units_query.value(VALUE).toInt() ? units_query.value(VALUE) : 1, "value", Global::Integer));
            cells_map.insert("item_type_id", new EditDialogueTableCell(units_query.value(UNIT_ID), "item_type_id"));
            cells_map.insert("acquisition_price", new EditDialogueTableCell(units_query.value(ITEM_ACQUISITION_PRICE).isNull() ? units_query.value(ACQUISITION_PRICE) : units_query.value(ITEM_ACQUISITION_PRICE), "acquisition_price", Global::Numeric, currency));
            cells_map.insert("list_price", new EditDialogueTableCell(units_query.value(ITEM_LIST_PRICE).isNull() ? units_query.value(LIST_PRICE) : units_query.value(ITEM_LIST_PRICE), "list_price", Global::Numeric, currency));
            cells_map.insert("discount", new EditDialogueTableCell(units_query.value(ITEM_DISCOUNT).isNull() ? units_query.value(DISCOUNT) : units_query.value(ITEM_DISCOUNT), "discount", Global::Numeric, "%"));
            cells_map.insert("source", new EditDialogueTableCell(AssemblyRecordItem::CircuitUnitTypes, "source"));
            cells_map.insert("category_id", new EditDialogueTableCell(units_query.value(CATEGORY_ID), "category_id"));
            cells_map.insert("unit", new EditDialogueTableCell(units_query.value(ITEM_UNIT).isNull() ? units_query.value(UNIT) : units_query.value(ITEM_UNIT), "unit"));
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
        MTSqlQuery inspectors_query;
        inspectors_query.prepare(QString("SELECT inspectors.id, inspectors.person, inspectors.acquisition_price, inspectors.list_price,"
                                         " assembly_record_items.value, assembly_record_item_categories.name, assembly_record_item_categories.id, assembly_record_item_categories.display_options,"
                                         " assembly_record_items.acquisition_price, assembly_record_items.list_price, assembly_record_items.name, assembly_record_items.unit,"
                                         " assembly_record_items.discount"
                                         " FROM inspectors"
                                         " LEFT JOIN assembly_record_items ON assembly_record_items.item_type_id = inspectors.id"
                                         " AND assembly_record_items.arno = :arno AND assembly_record_items.source = :source"
                                         " LEFT JOIN assembly_record_item_categories ON %1 = assembly_record_item_categories.id"
                                         " WHERE %1 IN"
                                         " (SELECT DISTINCT record_category_id FROM assembly_record_type_categories WHERE record_type_id = :record_type_id)")
                                         .arg(INSPECTORS_CATEGORY_ID));
        inspectors_query.bindValue(":arno", assemblyRecordId().toString());
        inspectors_query.bindValue(":source", AssemblyRecordItem::Inspectors);
        inspectors_query.bindValue(":record_type_id", assemblyRecordType().toInt());
        inspectors_query.exec();

        while (inspectors_query.next()) {
            QString name = inspectors_query.value(ITEM_NAME).isNull() ? inspectors_query.value(NAME).toString() : inspectors_query.value(ITEM_NAME).toString();
            cells_map.insert("name", new EditDialogueTableCell(name, "name"));
            cells_map.insert("value", new EditDialogueTableCell(inspectors_query.value(VALUE).toInt() ? inspectors_query.value(VALUE) : 1, "value", Global::Numeric));
            cells_map.insert("item_type_id", new EditDialogueTableCell(inspectors_query.value(INSPECTOR_ID), "item_type_id"));
            cells_map.insert("acquisition_price", new EditDialogueTableCell(inspectors_query.value(ITEM_ACQUISITION_PRICE).isNull() ? inspectors_query.value(ACQUISITION_PRICE) : inspectors_query.value(ITEM_ACQUISITION_PRICE), "acquisition_price", Global::Numeric, currency));
            cells_map.insert("list_price", new EditDialogueTableCell(inspectors_query.value(ITEM_LIST_PRICE).isNull() ? inspectors_query.value(LIST_PRICE) : inspectors_query.value(ITEM_LIST_PRICE), "list_price", Global::Numeric, currency));
            cells_map.insert("discount", new EditDialogueTableCell(inspectors_query.value(ITEM_DISCOUNT), "discount", Global::Numeric, "%"));
            cells_map.insert("source", new EditDialogueTableCell(AssemblyRecordItem::Inspectors, "source"));
            cells_map.insert("category_id", new EditDialogueTableCell(inspectors_query.value(CATEGORY_ID), "category_id"));
            cells_map.insert("unit", new EditDialogueTableCell(inspectors_query.value(ITEM_UNIT), "unit"));
            groups_layout->addItem(inspectors_query.value(CATEGORY_NAME).toString(),
                                   inspectors_query.value(CATEGORY_ID).toInt(),
                                   cells_map,
                                   inspectors_query.value(DISPLAY_OPTIONS).toInt(),
                                   !inspectors_query.value(VALUE).isNull() || !inspectors_query.value(ITEM_LIST_PRICE).isNull());
        }
    }
}

void EditInspectionDialogueAssemblyRecordTab::save(const QVariant &)
{
    if (!original_arno.isEmpty())
        AssemblyRecordItem(original_arno).remove();

    QString arno = assemblyRecordId().toString();
    if (arno.isEmpty())
        return;

    if (arno != original_arno)
        AssemblyRecordItem(arno).remove();

    MTSqlQuery query;
    query.prepare("UPDATE inspections SET ar_type = :ar_type WHERE arno = :arno");
    query.bindValue(":ar_type", ar_type_w->variantValue().toInt());
    query.bindValue(":arno", arno);
    query.exec();

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
        AssemblyRecordItem("").update(map);
    }
}

int EditInspectionDialogueAssemblyRecordTab::saveNewItemType(const MTDictionary & dict)
{
    int id = -1;
    MTSqlQuery query("SELECT MAX(id) FROM assembly_record_item_types");
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

void EditInspectionDialogueAssemblyRecordTab::assemblyRecordNumberChanged()
{
    if (arno_being_changed)
        return;
    arno_being_changed = true;

    if (current_arno == arno_w->text()) return;

    MTSqlQuery query;
    query.prepare("SELECT date, ar_type FROM inspections WHERE arno = :arno");
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
