/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2012 Matus & Michal Tomlein

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

#include "navigation.h"

#include "global.h"
#include "mainwindowsettings.h"
#include "linkparser.h"

Navigation::Navigation(QWidget * parent):
    QWidget(parent),
    default_view_for_group(GroupCount)
{
    restoreDefaults(false);
    setupUi(this);

    btngrp_view = new QButtonGroup(this);
    btngrp_view->addButton(tbtn_view_service_company, Navigation::ServiceCompany);
    btngrp_view->addButton(tbtn_view_refrigerant_management, Navigation::RefrigerantManagement);
    btngrp_view->addButton(tbtn_view_customers, Navigation::ListOfCustomers);
    btngrp_view->addButton(tbtn_view_circuits, Navigation::ListOfCircuits);
    btngrp_view->addButton(tbtn_view_inspections, Navigation::ListOfInspections);
    btngrp_view->addButton(tbtn_view_inspection, Navigation::Inspection);
    btngrp_view->addButton(tbtn_view_assembly_record, Navigation::AssemblyRecord);
    btngrp_view->addButton(tbtn_view_table, Navigation::TableOfInspections);
    btngrp_view->addButton(tbtn_view_repairs, Navigation::ListOfRepairs);
    btngrp_view->addButton(tbtn_view_inspectors, Navigation::ListOfInspectors);
    btngrp_view->addButton(tbtn_view_operator_report, Navigation::OperatorReport);
    btngrp_view->addButton(tbtn_view_leakages, Navigation::LeakagesByApplication);
    btngrp_view->addButton(tbtn_view_agenda, Navigation::Agenda);
    btngrp_view->addButton(tbtn_view_assembly_record_types, Navigation::ListOfAssemblyRecordTypes);
    btngrp_view->addButton(tbtn_view_assembly_record_item_types, Navigation::ListOfAssemblyRecordItemTypes);
    btngrp_view->addButton(tbtn_view_assembly_record_item_categories, Navigation::ListOfAssemblyRecordItemCategories);
    btngrp_view->addButton(tbtn_view_circuit_unit_types, Navigation::ListOfCircuitUnitTypes);
    btngrp_view->addButton(tbtn_view_assembly_records, Navigation::ListOfAssemblyRecords);
    btngrp_view->addButton(tbtn_view_inspector, Navigation::Inspector);
    btngrp_view->addButton(tbtn_view_inspection_images, Navigation::InspectionImages);
    QObject::connect(btngrp_view, SIGNAL(buttonClicked(int)), this, SLOT(setView(int)));
    QObject::connect(cb_view_table, SIGNAL(currentIndexChanged(int)), this, SLOT(tableChanged(int)));
    QObject::connect(chb_table_all_circuits, SIGNAL(toggled(bool)), this, SLOT(toggleTableForAllCircuits()));
    QObject::connect(spb_filter_since, SIGNAL(valueChanged(int)), this, SIGNAL(filterChanged()));
    QObject::connect(spb_filter_month_from, SIGNAL(valueChanged(int)), this, SLOT(monthFromChanged(int)));
    QObject::connect(spb_filter_month_until, SIGNAL(valueChanged(int)), this, SLOT(monthUntilChanged(int)));
    QObject::connect(le_filter, SIGNAL(returnPressed()), this, SIGNAL(filterChanged()));
    QObject::connect(cb_filter_column, SIGNAL(currentIndexChanged(int)), this, SLOT(emitFilterChanged()));
    QObject::connect(cb_filter_type, SIGNAL(currentIndexChanged(int)), this, SLOT(emitFilterChanged()));
    toggleVisibleGroup(current_group);
    setReportDataGroupBoxVisible(false);
}

void Navigation::restoreDefaults(bool apply)
{
    current_group = ServiceCompanyGroup;
    current_view = Navigation::ServiceCompany;
    if (apply) btngrp_view->button(current_view)->setChecked(true);
    default_view_for_group[ServiceCompanyGroup] = Navigation::ServiceCompany;
    default_view_for_group[BasicLogbookGroup] = Navigation::ListOfCustomers;
    default_view_for_group[DetailedLogbookGroup] = Navigation::ListOfCustomers;
    default_view_for_group[AssemblyRecordsGroup] = Navigation::ListOfAssemblyRecordTypes;
}

void Navigation::connectSlots(QObject * receiver)
{
    QObject::connect(this, SIGNAL(viewChanged(int)), receiver, SLOT(viewChanged(int)));
    QObject::connect(this, SIGNAL(groupChanged(int)), receiver, SLOT(groupChanged(int)));
    QObject::connect(this, SIGNAL(filterChanged()), receiver, SLOT(refreshView()));
    QObject::connect(tbtn_edit_service_company, SIGNAL(clicked()), receiver, SLOT(editServiceCompany()));
    QObject::connect(tbtn_add_record_of_refrigerant_management, SIGNAL(clicked()), receiver, SLOT(addRecordOfRefrigerantManagement()));
    QObject::connect(tbtn_report_data, SIGNAL(clicked()), receiver, SLOT(reportData()));
    QObject::connect(tbtn_add_inspector, SIGNAL(clicked()), receiver, SLOT(addInspector()));
    QObject::connect(tbtn_edit_inspector, SIGNAL(clicked()), receiver, SLOT(editInspector()));
    QObject::connect(tbtn_remove_inspector, SIGNAL(clicked()), receiver, SLOT(removeInspector()));
    QObject::connect(tbtn_add_customer, SIGNAL(clicked()), receiver, SLOT(addCustomer()));
    QObject::connect(tbtn_edit_customer, SIGNAL(clicked()), receiver, SLOT(editCustomer()));
    QObject::connect(tbtn_remove_customer, SIGNAL(clicked()), receiver, SLOT(removeCustomer()));
    QObject::connect(tbtn_add_repair, SIGNAL(clicked()), receiver, SLOT(addRepair()));
    QObject::connect(tbtn_edit_repair, SIGNAL(clicked()), receiver, SLOT(editRepair()));
    QObject::connect(tbtn_remove_repair, SIGNAL(clicked()), receiver, SLOT(removeRepair()));
    QObject::connect(tbtn_add_circuit, SIGNAL(clicked()), receiver, SLOT(addCircuit()));
    QObject::connect(tbtn_edit_circuit, SIGNAL(clicked()), receiver, SLOT(editCircuit()));
    QObject::connect(tbtn_remove_circuit, SIGNAL(clicked()), receiver, SLOT(removeCircuit()));
    QObject::connect(tbtn_add_inspection, SIGNAL(clicked()), receiver, SLOT(addInspection()));
    QObject::connect(tbtn_edit_inspection, SIGNAL(clicked()), receiver, SLOT(editInspection()));
    QObject::connect(tbtn_remove_inspection, SIGNAL(clicked()), receiver, SLOT(removeInspection()));
    QObject::connect(tbtn_add_assembly_record_type, SIGNAL(clicked()), receiver, SLOT(addAssemblyRecordType()));
    QObject::connect(tbtn_edit_assembly_record_type, SIGNAL(clicked()), receiver, SLOT(editAssemblyRecordType()));
    QObject::connect(tbtn_remove_assembly_record_type, SIGNAL(clicked()), receiver, SLOT(removeAssemblyRecordType()));
    QObject::connect(tbtn_edit_assembly_record_item_type, SIGNAL(clicked()), receiver, SLOT(editAssemblyRecordItemType()));
    QObject::connect(tbtn_add_assembly_record_item_type, SIGNAL(clicked()), receiver, SLOT(addAssemblyRecordItemType()));
    QObject::connect(tbtn_remove_assembly_record_item_type, SIGNAL(clicked()), receiver, SLOT(removeAssemblyRecordItemType()));
    QObject::connect(tbtn_edit_assembly_record_item_category, SIGNAL(clicked()), receiver, SLOT(editAssemblyRecordItemCategory()));
    QObject::connect(tbtn_add_assembly_record_item_category, SIGNAL(clicked()), receiver, SLOT(addAssemblyRecordItemCategory()));
    QObject::connect(tbtn_remove_assembly_record_item_category, SIGNAL(clicked()), receiver, SLOT(removeAssemblyRecordItemCategory()));
    QObject::connect(tbtn_add_circuit_unit_type, SIGNAL(clicked()), receiver, SLOT(addCircuitUnitType()));
    QObject::connect(tbtn_remove_circuit_unit_type, SIGNAL(clicked()), receiver, SLOT(removeCircuitUnitType()));
    QObject::connect(tbtn_edit_circuit_unit_type, SIGNAL(clicked()), receiver, SLOT(editCircuitUnitType()));
    QObject::connect(chb_by_field, SIGNAL(clicked()), receiver, SLOT(refreshView()));
    emit viewChanged(view());
}

Navigation::View Navigation::view()
{
    if (btngrp_view->checkedId() != -1)
        return (Navigation::View)btngrp_view->checkedId();

    tbtn_view_service_company->setChecked(true);
    emit viewChanged(Navigation::ServiceCompany);
    return Navigation::ServiceCompany;
}

void Navigation::addFilterItems(const QString & column, const MTDictionary & items)
{
    cb_filter_column->insertSeparator(cb_filter_column->count());

    QSet<QString> used;
    for (int i = 0; i < items.count(); ++i) {
        if (!used.contains(items.value(i))) {
            used << items.value(i);
            cb_filter_column->addItem(items.value(i), QString("%1 = '%2' AND ? IS NOT NULL").arg(column).arg(items.key(i)));
        }
    }
}

void Navigation::updateView()
{
    lbl_filter_since->setText(tr("Since:"));
    spb_filter_since->setSpecialValueText(tr("All"));
    cb_filter_column->clear();
    le_filter->clear();
    bool filter_by_field_visible = false;
    bool filter_keyword_visible = true;
    bool filter_since_visible = true;
    bool filter_month_visible = false;
    bool filter_visible = true;
    Group group = NoGroup;
    switch (btngrp_view->checkedId()) {
        case Navigation::ServiceCompany:
            group = ServiceCompanyGroup;
            filter_by_field_visible = true;
            filter_keyword_visible = false;
            break;
        case Navigation::RefrigerantManagement:
            group = ServiceCompanyGroup;
            cb_filter_column->addItem(QApplication::translate("RecordOfRefrigerantManagement", "Date"), "date");
            cb_filter_column->addItem(QApplication::translate("RecordOfRefrigerantManagement", "Business partner"), "partner");
            cb_filter_column->addItem(QApplication::translate("RecordOfRefrigerantManagement", "Business partner (ID)"), "partner_id");
            cb_filter_column->addItem(QApplication::translate("RecordOfRefrigerantManagement", "Refrigerant"), "refrigerant");
            break;
        case Navigation::ListOfCustomers:
            filter_since_visible = false;
            cb_filter_column->addItem(QApplication::translate("Customer", "ID"), "id");
            cb_filter_column->addItem(QApplication::translate("Customer", "Company"), "company");
            cb_filter_column->addItem(QApplication::translate("Customer", "Address"), "address");
            cb_filter_column->addItem(QApplication::translate("Customer", "E-mail"), "mail");
            cb_filter_column->addItem(QApplication::translate("Customer", "Phone"), "phone");
            break;
        case Navigation::ListOfCircuits:
            group = DetailedLogbookGroup;
            filter_since_visible = false;
            updateView_ListOfCircuits_CircuitAttributes:
            cb_filter_column->addItem(QApplication::translate("Circuit", "ID"), "id");
            cb_filter_column->addItem(QApplication::translate("Circuit", "Circuit name"), "name");
            cb_filter_column->addItem(QApplication::translate("Circuit", "Place of operation"), "operation");
            cb_filter_column->addItem(QApplication::translate("Circuit", "Building"), "building");
            cb_filter_column->addItem(QApplication::translate("Circuit", "Device"), "device");
            cb_filter_column->addItem(QApplication::translate("Circuit", "Manufacturer"), "manufacturer");
            cb_filter_column->addItem(QApplication::translate("Circuit", "Type"), "type");
            cb_filter_column->addItem(QApplication::translate("Circuit", "Serial number"), "sn");
            cb_filter_column->addItem(QApplication::translate("Circuit", "Year of purchase"), "year");
            cb_filter_column->addItem(QApplication::translate("Circuit", "Date of commissioning"), "commissioning");
            cb_filter_column->addItem(QApplication::translate("Circuit", "Refrigerant"), "refrigerant");
            cb_filter_column->addItem(QApplication::translate("Circuit", "Oil"), "oil");
            addFilterItems("field", Global::fieldsOfApplication());
            break;
        case Navigation::ListOfInspections:
            group = DetailedLogbookGroup;
            cb_filter_column->addItem(QApplication::translate("Inspection", "Date"), "date");
            cb_filter_column->addItem(QApplication::translate("Inspection", "Operator"), "operator");
            cb_filter_column->addItem(QApplication::translate("Inspection", "Remedies"), "rmds");
            cb_filter_column->addItem(QApplication::translate("Inspection", "Assembly record No."), "arno");
            cb_filter_column->addItem(QApplication::translate("Inspection", "Assembly record type"), "ar_type");
            break;
        case Navigation::ListOfRepairs:
            group = BasicLogbookGroup;
            cb_filter_column->addItem(QApplication::translate("Repair", "Date"), "date");
            cb_filter_column->addItem(QApplication::translate("Repair", "Customer"), "customer");
            cb_filter_column->addItem(QApplication::translate("Repair", "Device"), "device");
            cb_filter_column->addItem(QApplication::translate("Repair", "Refrigerant"), "refrigerant");
            cb_filter_column->addItem(QApplication::translate("Repair", "Assembly record No."), "arno");
            break;
        case Navigation::ListOfInspectors:
            group = ServiceCompanyGroup;
            filter_since_visible = false;
            cb_filter_column->addItem(QApplication::translate("Inspector", "ID"), "id");
            cb_filter_column->addItem(QApplication::translate("Inspector", "Name"), "person");
            cb_filter_column->addItem(QApplication::translate("Inspector", "E-mail"), "mail");
            cb_filter_column->addItem(QApplication::translate("Inspector", "Phone"), "phone");
            break;
        case Navigation::Inspector:
            group = ServiceCompanyGroup;
            cb_filter_column->addItem(QApplication::translate("Inspection", "Date"), "date");
            cb_filter_column->addItem(QApplication::translate("Inspector", "Customer ID"), "customer");
            cb_filter_column->addItem(QApplication::translate("Circuit", "Circuit ID"), "circuit");
            break;
        case Navigation::TableOfInspections:
            group = DetailedLogbookGroup;
            filter_keyword_visible = false;
            break;
        case Navigation::Inspection:
            group = DetailedLogbookGroup;
            filter_visible = false;
            break;
        case Navigation::Agenda:
            group = ServiceCompanyGroup;
            filter_since_visible = false;
            cb_filter_column->addItem(QApplication::translate("Customer", "ID"), "parent");
            goto updateView_ListOfCircuits_CircuitAttributes;
            break;
        case Navigation::OperatorReport:
            group = ServiceCompanyGroup;
            filter_month_visible = true;
            lbl_filter_since->setText(tr("Year:"));
            spb_filter_since->setSpecialValueText(tr("Last"));
            cb_filter_column->addItem(QApplication::translate("Circuit", "ID"), "id");
            cb_filter_column->addItem(QApplication::translate("Circuit", "Refrigerant"), "refrigerant");
            cb_filter_column->addItem(QApplication::translate("Circuit", "Place of operation"), "operation");
            addFilterItems("field", Global::fieldsOfApplication());
            break;
        case Navigation::ListOfAssemblyRecordTypes:
            filter_since_visible = false;
            cb_filter_column->addItem(QApplication::translate("AssemblyRecord", "ID"), "id");
            cb_filter_column->addItem(QApplication::translate("AssemblyRecord", "Name"), "name");
            cb_filter_column->addItem(QApplication::translate("AssemblyRecord", "Description"), "description");
            group = AssemblyRecordsGroup;
            break;
        case Navigation::ListOfAssemblyRecordItemTypes:
            filter_since_visible = false;
            cb_filter_column->addItem(QApplication::translate("AssemblyRecord", "ID"), "id");
            cb_filter_column->addItem(QApplication::translate("AssemblyRecord", "Name"), "name");
            cb_filter_column->addItem(QApplication::translate("AssemblyRecord", "Category ID"), "category_id");
            group = AssemblyRecordsGroup;
            break;
        case Navigation::ListOfAssemblyRecordItemCategories:
            filter_since_visible = false;
            cb_filter_column->addItem(QApplication::translate("AssemblyRecord", "ID"), "id");
            cb_filter_column->addItem(QApplication::translate("AssemblyRecord", "Name"), "name");
            group = AssemblyRecordsGroup;
            break;
        case Navigation::AssemblyRecord:
            group = DetailedLogbookGroup;
            filter_visible = false;
            break;
        case Navigation::ListOfCircuitUnitTypes:
            filter_since_visible = false;
            cb_filter_column->addItem(QApplication::translate("AssemblyRecord", "ID"), "id");
            cb_filter_column->addItem(QApplication::translate("AssemblyRecord", "Manufacturer"), "manufacturer");
            cb_filter_column->addItem(QApplication::translate("AssemblyRecord", "Type"), "type");
            cb_filter_column->addItem(QApplication::translate("AssemblyRecord", "Refrigerant"), "refrigerant");
            cb_filter_column->addItem(QApplication::translate("AssemblyRecord", "Oil"), "oil");
            cb_filter_column->addItem(QApplication::translate("AssemblyRecord", "Notes"), "notes");
            group = AssemblyRecordsGroup;
            break;
        case Navigation::ListOfAssemblyRecords:
            cb_filter_column->addItem(QApplication::translate("AssemblyRecord", "Date"), "date");
            cb_filter_column->addItem(QApplication::translate("AssemblyRecord", "Inspector ID"), "inspector");
            cb_filter_column->addItem(QApplication::translate("AssemblyRecord", "Assembly record No."), "arno");
            cb_filter_column->addItem(QApplication::translate("AssemblyRecord", "Assembly record type ID"), "ar_type");
            group = DetailedLogbookGroup;
            break;
        case Navigation::InspectionImages:
            group = DetailedLogbookGroup;
            filter_visible = false;
            break;
        case Navigation::LeakagesByApplication:
        default:
            group = ServiceCompanyGroup;
            filter_visible = false;
            break;
    }
    if (group > NoGroup && current_group != group)
        toggleVisibleGroup(group, false);
    chb_by_field->setVisible(filter_by_field_visible);
    cb_filter_column->setVisible(filter_keyword_visible);
    cb_filter_type->setVisible(filter_keyword_visible);
    le_filter->setVisible(filter_keyword_visible);
    lbl_filter_since->setVisible(filter_since_visible);
    spb_filter_since->setVisible(filter_since_visible);
    lbl_filter_month->setVisible(filter_month_visible);
    spb_filter_month_from->setVisible(filter_month_visible);
    lbl_filter_month_dash->setVisible(filter_month_visible);
    spb_filter_month_until->setVisible(filter_month_visible);
    gb_filter->setVisible(filter_visible);
}

void Navigation::setView(int v)
{
    setView(v, true);
}

void Navigation::setView(const QString & v)
{
    setView(v.toInt(), true);
}

void Navigation::setView(int v, bool emit_signal)
{
    if (current_view != v) {
        if (btngrp_view->button(v)) btngrp_view->button(v)->setChecked(true);
        updateView();
        current_view = (Navigation::View)v;
    }
    if (emit_signal) emit viewChanged(v);
}

void Navigation::viewServiceCompany()
{
    toggleVisibleGroup(ServiceCompanyGroup);
}

void Navigation::viewBasicLogbook()
{
    toggleVisibleGroup(BasicLogbookGroup);
}

void Navigation::viewDetailedLogbook()
{
    toggleVisibleGroup(DetailedLogbookGroup);
}

void Navigation::viewAssemblyRecords()
{
    toggleVisibleGroup(AssemblyRecordsGroup);
}

void Navigation::toggleVisibleGroup(Group g, bool emit_signal)
{
    if (current_group < ReportDataGroup) {
        default_view_for_group[current_group] = current_view;
    }
    if (g < ReportDataGroup && current_group != g) {
        emit groupChanged(g);
    }
    current_group = g;
    gb_service_company->setVisible(g == ServiceCompanyGroup);
    gb_statistics->setVisible(g == ServiceCompanyGroup);
    gb_inspectors->setVisible(g == ServiceCompanyGroup);
    gb_customers->setVisible(g < AssemblyRecordsGroup);
    gb_repairs->setVisible(g == BasicLogbookGroup);
    gb_circuits->setVisible(g == DetailedLogbookGroup);
    gb_inspections->setVisible(g == DetailedLogbookGroup);
    gb_tables->setVisible(g == DetailedLogbookGroup);
    gb_assembly_records->setVisible(g == DetailedLogbookGroup);
    gb_assembly_record_types->setVisible(g == AssemblyRecordsGroup);
    gb_assembly_record_item_types->setVisible(g == AssemblyRecordsGroup);
    gb_assembly_record_item_categories->setVisible(g == AssemblyRecordsGroup);
    gb_filter->setVisible(g < ReportDataGroup);
    gb_report_data->setVisible(g == ReportDataGroup);
    gb_circuit_unit_types->setVisible(g == AssemblyRecordsGroup);
    if (g < ReportDataGroup) {
        if (emit_signal && current_view != default_view_for_group[current_group]) {
            current_view = default_view_for_group[current_group];
            if (btngrp_view->button(current_view)) btngrp_view->button(current_view)->setChecked(true);
            updateView();
            emit viewChanged(current_view);
        } else {
            updateView();
        }
    }
}

void Navigation::tableChanged(int)
{
    if (tbtn_view_table->isChecked())
        emit viewChanged(Navigation::TableOfInspections);
}

void Navigation::toggleTableForAllCircuits()
{
    if (current_view == Navigation::TableOfInspections)
        emit viewChanged(current_view);
}

void Navigation::emitFilterChanged()
{
    bool enabled = !filterColumn().contains('?');
    bool changed = !isFilterEmpty() || enabled != le_filter->isEnabled();
    cb_filter_type->setEnabled(enabled);
    le_filter->setEnabled(enabled);

    if (changed && cb_filter_column->count())
        emit filterChanged();
}

void Navigation::monthFromChanged(int value)
{
    if (spb_filter_month_until->value() < value)
        spb_filter_month_until->setValue(value);
    emit filterChanged();
}

void Navigation::monthUntilChanged(int value)
{
    if (spb_filter_month_from->value() > value)
        spb_filter_month_from->setValue(value);
    emit filterChanged();
}

void Navigation::enableTools(const MainWindowSettings & settings)
{
    tbtn_edit_inspector->setEnabled(settings.isInspectorSelected());
    tbtn_remove_inspector->setEnabled(settings.isInspectorSelected());
    tbtn_view_inspector->setEnabled(settings.isInspectorSelected());
    tbtn_edit_customer->setEnabled(settings.isCustomerSelected());
    tbtn_remove_customer->setEnabled(settings.isCustomerSelected());
    tbtn_edit_repair->setEnabled(settings.isRepairSelected());
    tbtn_remove_repair->setEnabled(settings.isRepairSelected());
    tbtn_view_operator_report->setEnabled(settings.isCustomerSelected());
    tbtn_view_circuits->setEnabled(settings.isCustomerSelected());
    gb_circuits->setEnabled(settings.isCustomerSelected());
    tbtn_edit_circuit->setEnabled(settings.isCircuitSelected());
    tbtn_remove_circuit->setEnabled(settings.isCircuitSelected());
    gb_inspections->setEnabled(settings.isCircuitSelected());
    tbtn_edit_inspection->setEnabled(settings.isInspectionSelected());
    tbtn_remove_inspection->setEnabled(settings.isInspectionSelected());
    tbtn_view_inspection->setEnabled(settings.isInspectionSelected());
    tbtn_view_assembly_record->setEnabled(settings.isInspectionSelected() && settings.hasAssemblyRecord());
    tbtn_edit_assembly_record_type->setEnabled(settings.isAssemblyRecordTypeSelected());
    tbtn_remove_assembly_record_type->setEnabled(settings.isAssemblyRecordTypeSelected());
    tbtn_edit_assembly_record_item_type->setEnabled(settings.isAssemblyRecordItemTypeSelected());
    tbtn_remove_assembly_record_item_type->setEnabled(settings.isAssemblyRecordItemTypeSelected());
    tbtn_edit_assembly_record_item_category->setEnabled(settings.isAssemblyRecordItemCategorySelected());
    tbtn_remove_assembly_record_item_category->setEnabled(settings.isAssemblyRecordItemCategorySelected());
    tbtn_edit_circuit_unit_type->setEnabled(settings.isCircuitUnitTypeSelected());
    tbtn_remove_circuit_unit_type->setEnabled(settings.isCircuitUnitTypeSelected());
    tbtn_view_inspection_images->setEnabled(settings.isInspectionSelected());
    gb_tables->setEnabled(settings.isCustomerSelected());
    chb_table_all_circuits->setEnabled(settings.isCircuitSelected());
    chb_table_all_circuits->setChecked(!settings.isCircuitSelected());
    ar_show_options_widget->setVisible(settings.isInspectionSelected());

    bool enabled = Global::isOperationPermitted("access_assembly_record_acquisition_price") > 0;
    chb_assembly_record_acquisition_price->setEnabled(enabled);
    chb_assembly_record_acquisition_price->setChecked(enabled);

    enabled = Global::isOperationPermitted("access_assembly_record_list_price") > 0;
    chb_assembly_record_list_price->setEnabled(enabled);
    chb_assembly_record_list_price->setChecked(enabled);

    chb_assembly_record_total->setEnabled(enabled);
    chb_assembly_record_total->setChecked(enabled);
}

void Navigation::setReportDataGroupBoxVisible(bool visible)
{
    static Group last_group = current_group;
    if (visible) {
        last_group = current_group;
        toggleVisibleGroup(ReportDataGroup);
    } else {
        toggleVisibleGroup(last_group);
    }
}

QString Navigation::filterKeyword() const
{
    if (filterColumn().contains('?'))
        return le_filter->text();
    switch (cb_filter_type->currentIndex()) {
        // contains
        case 0: return "%" + le_filter->text() + "%"; break;
        // is
        case 1: return le_filter->text(); break;
        // starts with
        case 2: return le_filter->text() + "%"; break;
        // ends with
        case 3: return "%" + le_filter->text(); break;
    }
    return le_filter->text();
}
