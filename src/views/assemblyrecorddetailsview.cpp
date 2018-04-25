/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2018 Matus & Michal Tomlein

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

#include "assemblyrecorddetailsview.h"

#include "global.h"
#include "records.h"
#include "viewtabsettings.h"
#include "mainwindowsettings.h"
#include "toolbarstack.h"
#include "variableevaluation.h"
#include "htmlbuilder.h"

using namespace Global;

AssemblyRecordDetailsView::AssemblyRecordDetailsView(ViewTabSettings *settings):
    TableView(settings)
{
}

QString AssemblyRecordDetailsView::renderHTML()
{
    QString customer_id = settings->selectedCustomer();
    QString circuit_id = settings->selectedCircuit();
    QString inspection_date = settings->selectedInspection();

    Inspection inspection_record(customer_id, circuit_id, inspection_date);
    QVariantMap inspection = inspection_record.list();
    bool nominal = inspection.value("nominal").toInt();
    Inspection::Repair repair = (Inspection::Repair)inspection.value("repair").toInt();
    bool locked = DBInfo::isRecordLocked(inspection_date);

    VariableEvaluation::EvaluationContext var_evaluation(customer_id, circuit_id);
    VariableEvaluation::Variable *variable;
    QString nom_value;
    QString currency = DBInfo::valueForKey("currency", "EUR");

    AssemblyRecordType ar_type_record(inspection.value("ar_type").toString());
    QVariantMap ar_type = ar_type_record.list();
    int type_display_options = ar_type.value("display_options").toInt();

    HTMLParent *main = NULL;

    QString custom_style;
    if (ar_type.value("style", -1).toInt() >= 0) {
        QVariantMap style = Style(ar_type.value("style").toString()).list("content, div_tables");
        custom_style = style.value("content").toString();
        if (style.value("div_tables").toBool())
            main = new HTMLDivMain();
    }
    if (!main)
        main = new HTMLMain();

    HTMLTable *table, *top_table;
    HTMLTableRow *_tr;
    HTMLTableCell *_td;
    HTMLParentElement *elem;

    if (type_display_options & AssemblyRecordType::ShowServiceCompany) {
        writeServiceCompany(main->table());
        main->newLine();
    }

    table = main->table("cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\" class=\"no_border\"");
    *(table->addRow()->addHeaderCell()) << tr("Assembly record No. %1").arg(inspection.value("arno").toString());
    *(table->addRow()->addCell()->subHeading()) << ar_type.value("name").toString();
    *(table->addRow()->addCell()->paragraph()) << ar_type.value("description").toString();
    main->newLine();

    QLocale locale;
    QString html;

    if (type_display_options & AssemblyRecordType::ShowCustomer) {
        writeCustomersTable(customer_id, main->table());
        main->newLine();
    }

    if (type_display_options & AssemblyRecordType::ShowCustomerContactPersons) {
        customerContactPersons(customer_id, main->table());
        main->newLine();
    }

    if (type_display_options & AssemblyRecordType::ShowCircuit) {
        writeCircuitsTable(customer_id, circuit_id, 8, main->table("", 8));
        main->newLine();
    }
    *main << html;

    if (type_display_options & AssemblyRecordType::ShowCompressors) {
        circuitCompressorsTable(customer_id, circuit_id, main->table());
        main->newLine();
    }

    if (type_display_options & AssemblyRecordType::ShowCircuitUnits) {
        circuitUnitsTable(customer_id, circuit_id, main->table());
        main->newLine();
    }

    top_table = main->table("cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\"");
    top_table->addClass("items_top_table");
    top_table->addClass("no_border");
    _td = top_table->addRow()->addHeaderCell("colspan=\"6\" style=\"font-size: medium; background-color: lightgoldenrodyellow;\"");
    if (!locked) {
        elem = _td->link("customer:" + customer_id + "/circuit:" + circuit_id
                         + (repair == Inspection::IsRepair ? "/repair:" : "/inspection:") + inspection_date + "/edit");
    } else {
        elem = _td;
    }
    *elem << Inspection::titleForInspection(nominal, repair);
    *elem << "&nbsp;" << settings->mainWindowSettings().formatDateTime(inspection_date);

    enum QUERY_RESULTS
    {
        VALUE = 0,
        NAME = 1,
        CATEGORY_ID = 2,
        CATEGORY_NAME = 3,
        DISPLAY_OPTIONS = 4,
        LIST_PRICE = 5,
        ACQUISITION_PRICE = 6,
        UNIT = 7,
        VARIABLE_ID = 8,
        VALUE_DATA_TYPE = 9,
        CATEGORY_POSITION = 10,
        DISCOUNT = 11,
        ITEM_TYPE_ID = 12
    };

    MTSqlQuery categories_query;
    categories_query.prepare("SELECT assembly_record_items.value, assembly_record_items.name,"
                             " assembly_record_item_categories.id, assembly_record_item_categories.name,"
                             " assembly_record_item_categories.display_options, assembly_record_items.list_price,"
                             " assembly_record_items.acquisition_price, assembly_record_items.unit,"
                             " assembly_record_item_types.inspection_variable_id, assembly_record_item_types.value_data_type,"
                             " assembly_record_item_categories.display_position,"
                             " assembly_record_items.discount, assembly_record_item_types.id"
                             " FROM assembly_record_items"
                             " LEFT JOIN assembly_record_item_types"
                             " ON assembly_record_items.item_type_id = assembly_record_item_types.id"
                             " AND assembly_record_items.source = :source"
                             " LEFT JOIN assembly_record_item_categories"
                             " ON assembly_record_items.category_id = assembly_record_item_categories.id"
                             " LEFT JOIN assembly_record_type_categories"
                             " ON assembly_record_items.category_id = assembly_record_type_categories.record_category_id"
                             " AND assembly_record_type_categories.record_type_id = :ar_type"
                             " WHERE arno = :arno ORDER BY assembly_record_type_categories.position,"
                             " assembly_record_item_types.category_id, assembly_record_item_types.name");
    categories_query.bindValue(":source", AssemblyRecordItem::AssemblyRecordItemTypes);
    categories_query.bindValue(":arno", inspection.value("arno").toString());
    categories_query.bindValue(":ar_type", inspection.value("ar_type").toInt());
    categories_query.exec();

    int last_category = -1;
    int num_columns = 6, i, n;
    int colspans[num_columns];
    bool show_list_price = settings->toolBarStack()->isAssemblyRecordListPriceChecked();
    bool show_acquisition_price = settings->toolBarStack()->isAssemblyRecordAcquisitionPriceChecked();
    bool show_total = settings->toolBarStack()->isAssemblyRecordTotalChecked();
    double absolute_total = 0.0, total, acquisition_total = 0.0;
    QString colspan = "colspan=\"%1\"";
    QString item_value;
    while (categories_query.next()) {
        if (last_category != categories_query.value(CATEGORY_ID).toInt()) {
            if (categories_query.value(CATEGORY_POSITION).toInt() == AssemblyRecordItemCategory::DisplayAtTop) {
                table = top_table;
            } else {
                main->newLine();
                table = main->table("cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\"");
                table->addClass("items_bottom_table");
                table->addClass("no_border");
            }

            int cat_display_options = categories_query.value(DISPLAY_OPTIONS).toInt();

            for (i = 1; i < num_columns; ++i) colspans[i] = 0;
            i = n = 0; colspans[0] = 1;
            if (++n && cat_display_options & AssemblyRecordItemCategory::ShowValue) { i = n; colspans[i] = 1; }
            else colspans[i]++;
            if (++n && (cat_display_options & AssemblyRecordItemCategory::ShowAcquisitionPrice) && show_acquisition_price) { i = n; colspans[i] = 1; }
            else colspans[i]++;
            if (++n && (cat_display_options & AssemblyRecordItemCategory::ShowListPrice) && show_list_price) { i = n; colspans[i] = 1; }
            else colspans[i]++;
            if (++n && (cat_display_options & AssemblyRecordItemCategory::ShowDiscount && show_list_price)) { i = n; colspans[i] = 1; }
            else colspans[i]++;
            if (++n && (cat_display_options & AssemblyRecordItemCategory::ShowTotal && show_total)) { i = n; colspans[i] = 1; }
            else colspans[i]++;

            if (categories_query.value(CATEGORY_POSITION).toInt() == AssemblyRecordItemCategory::DisplayAtTop) {
                i = 0;
                _tr = table->addRow();
                _td = _tr->addHeaderCell(colspan.arg(colspans[i]));
                _td->setId("item_name_label");
                *_td << categories_query.value(CATEGORY_NAME).toString();
                if (colspans[++i]) {
                    _td = _tr->addHeaderCell(colspan.arg(colspans[i]));
                    _td->setId("item_value_label");
                    *_td << tr("Value");
                } if (colspans[++i]) {
                    _td = _tr->addHeaderCell(colspan.arg(colspans[i]));
                    _td->setId("item_acquisition_price_label");
                    *_td << tr("Acquisition price (%1)").arg(currency);
                } if (colspans[++i]) {
                    _td = _tr->addHeaderCell(colspan.arg(colspans[i]));
                    _td->setId("item_list_price_label");
                    *_td << tr("List price (%1)").arg(currency);
                } if (colspans[++i]) {
                    _td = _tr->addHeaderCell(colspan.arg(colspans[i]));
                    _td->setId("item_discount_label");
                    *_td << tr("Discount");
                } if (colspans[++i]) {
                    _td = _tr->addHeaderCell(colspan.arg(colspans[i]));
                    _td->setId("item_total_label");
                    *_td << tr("Total (%1)").arg(currency);
                }
            }
            last_category = categories_query.value(CATEGORY_ID).toInt();
        }
        i = 0; total = 0.0;
        _tr = table->addRow();
        _td = _tr->addCell(colspan.arg(colspans[i]));
        _td->setId(QString("item_%1_name").arg(categories_query.value(ITEM_TYPE_ID).toInt()));
        *_td << categories_query.value(NAME).toString();
        if (colspans[++i]) {
            if (categories_query.value(VARIABLE_ID).toString().isEmpty()) {
                total = categories_query.value(VALUE).toDouble();
                switch (categories_query.value(VALUE_DATA_TYPE).toInt()) {
                    case Global::Boolean:
                        item_value = categories_query.value(VALUE).toInt() ? tr("Yes") : tr("No");
                        break;

                    default:
                        if (!total) {
                            case Global::Integer:
                                item_value = categories_query.value(VALUE).toString();
                                break;
                        }

                    case Global::Numeric:
                        item_value = locale.toString(total);
                        break;
                }
            } else {
                variable = var_evaluation.variable(categories_query.value(VARIABLE_ID).toString());
                item_value = var_evaluation.evaluate(variable, inspection, nom_value);
                total = item_value.toDouble();
                item_value = tableVarValue(variable->type(), item_value, QString(), QString(), false, 0.0, true);
            }
            _td = _tr->addCell(colspan.arg(colspans[i]));
            *_td << item_value << " " << categories_query.value(UNIT).toString();
            _td->setId(QString("item_%1_value").arg(categories_query.value(ITEM_TYPE_ID).toInt()));
        }
        if (colspans[++i]) {
            _td = _tr->addCell(colspan.arg(colspans[i]));
            *_td << categories_query.value(ACQUISITION_PRICE).toDouble();
            _td->setId(QString("item_%1_acquisition_price").arg(categories_query.value(ITEM_TYPE_ID).toInt()));
        }
        if (colspans[++i]) {
            _td = _tr->addCell(colspan.arg(colspans[i]));
            _td->setId(QString("item_%1_list_price").arg(categories_query.value(ITEM_TYPE_ID).toInt()));
            *_td << categories_query.value(LIST_PRICE).toDouble();
            total *= categories_query.value(LIST_PRICE).toDouble();
        }
        if (colspans[++i]) {
            double total_discount = categories_query.value(DISCOUNT).toDouble();
            total *= 1 - total_discount / 100;
            _td = _tr->addCell(colspan.arg(colspans[i]));
            _td->setId(QString("item_%1_discount").arg(categories_query.value(ITEM_TYPE_ID).toInt()));
            *_td << total_discount << " %";
        }
        if (colspans[++i]) {
            _td = _tr->addCell(colspan.arg(colspans[i]));
            _td->setId(QString("item_%1_total").arg(categories_query.value(ITEM_TYPE_ID).toInt()));
            *_td << total;
        }
        absolute_total += total;
        acquisition_total += item_value.toDouble() * categories_query.value(ACQUISITION_PRICE).toDouble();
    }
    if (show_total) {
        table = top_table;
        _tr = table->addRow();
        _td = _tr->addHeaderCell(QString(colspan.arg(num_columns - 4 + !show_acquisition_price + 2 * !show_list_price))
                                 + " rowspan=\"2\"");
        _td->setId("total_label");
        *_td << tr("Total");
        if (show_acquisition_price) {
            _td = _tr->addHeaderCell();
            _td->setId("total_acquisition_price_label");
            *_td << tr("Acquisition price (%1)").arg(currency);
        }
        _td = _tr->addHeaderCell(colspan.arg(3));
        _td->setId("total_list_price_label");
        *_td << tr("List price (%1)").arg(currency);

        _tr = table->addRow();
        if (show_acquisition_price) {
            _td = _tr->addCell();
            _td->setId("total_acquisition_price");
            *_td << acquisition_total;
        }
        _td = _tr->addCell(colspan.arg(3));
        _td->setId("total_list_price");
        *_td << absolute_total;
    }

    QString ret = viewTemplate("assembly_record").arg(main->html()).arg(custom_style);
    delete main;
    return ret;
}

QString AssemblyRecordDetailsView::title() const
{
    return Inspection(settings->selectedCustomer(), settings->selectedCircuit(), settings->selectedInspection()).stringValue("arno");
}
