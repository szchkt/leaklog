/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2010 Matus & Michal Tomlein

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

Navigation::Navigation(QWidget * parent):
QWidget(parent)
{
    default_view_for_group.resize(3);
    restoreDefaults();
    setupUi(this);
    btngrp_view = new QButtonGroup(this);
    btngrp_view->addButton(tbtn_view_service_company);
    btngrp_view->setId(tbtn_view_service_company, Navigation::ServiceCompany);
    btngrp_view->addButton(tbtn_view_customers);
    btngrp_view->setId(tbtn_view_customers, Navigation::ListOfCustomers);
    btngrp_view->addButton(tbtn_view_circuits);
    btngrp_view->setId(tbtn_view_circuits, Navigation::ListOfCircuits);
    btngrp_view->addButton(tbtn_view_inspections);
    btngrp_view->setId(tbtn_view_inspections, Navigation::ListOfInspections);
    btngrp_view->addButton(tbtn_view_inspection);
    btngrp_view->setId(tbtn_view_inspection, Navigation::Inspection);
    btngrp_view->addButton(tbtn_view_table);
    btngrp_view->setId(tbtn_view_table, Navigation::TableOfInspections);
    btngrp_view->addButton(tbtn_view_repairs);
    btngrp_view->setId(tbtn_view_repairs, Navigation::ListOfRepairs);
    btngrp_view->addButton(tbtn_view_inspectors);
    btngrp_view->setId(tbtn_view_inspectors, Navigation::ListOfInspectors);
    btngrp_view->addButton(tbtn_view_leakages);
    btngrp_view->setId(tbtn_view_leakages, Navigation::LeakagesByApplication);
    btngrp_view->addButton(tbtn_view_agenda);
    btngrp_view->setId(tbtn_view_agenda, Navigation::Agenda);
    QObject::connect(btngrp_view, SIGNAL(buttonClicked(int)), this, SLOT(setView(int)));
    QObject::connect(cb_view_table, SIGNAL(currentIndexChanged(int)), this, SLOT(tableChanged(int)));
    toggleVisibleGroup(current_group);
    setReportDataGroupBoxVisible(false);
}

void Navigation::restoreDefaults()
{
    current_group = 0;
    current_view = Navigation::ServiceCompany;
    default_view_for_group[0] = Navigation::ServiceCompany;
    default_view_for_group[1] = Navigation::ListOfCustomers;
    default_view_for_group[2] = Navigation::ListOfCustomers;
}

void Navigation::connectSlots(QObject * receiver)
{
    QObject::connect(this, SIGNAL(viewChanged(int)), receiver, SLOT(viewChanged(int)));
    QObject::connect(this, SIGNAL(groupChanged(int)), receiver, SLOT(groupChanged(int)));
    QObject::connect(tbtn_modify_service_company, SIGNAL(clicked()), receiver, SLOT(modifyServiceCompany()));
    QObject::connect(tbtn_add_record_of_refrigerant_management, SIGNAL(clicked()), receiver, SLOT(addRecordOfRefrigerantManagement()));
    QObject::connect(tbtn_report_data, SIGNAL(clicked()), receiver, SLOT(reportData()));
    QObject::connect(tbtn_add_inspector, SIGNAL(clicked()), receiver, SLOT(addInspector()));
    QObject::connect(tbtn_modify_inspector, SIGNAL(clicked()), receiver, SLOT(modifyInspector()));
    QObject::connect(tbtn_remove_inspector, SIGNAL(clicked()), receiver, SLOT(removeInspector()));
    QObject::connect(tbtn_add_customer, SIGNAL(clicked()), receiver, SLOT(addCustomer()));
    QObject::connect(tbtn_modify_customer, SIGNAL(clicked()), receiver, SLOT(modifyCustomer()));
    QObject::connect(tbtn_remove_customer, SIGNAL(clicked()), receiver, SLOT(removeCustomer()));
    QObject::connect(tbtn_add_repair, SIGNAL(clicked()), receiver, SLOT(addRepair()));
    QObject::connect(tbtn_modify_repair, SIGNAL(clicked()), receiver, SLOT(modifyRepair()));
    QObject::connect(tbtn_remove_repair, SIGNAL(clicked()), receiver, SLOT(removeRepair()));
    QObject::connect(tbtn_add_circuit, SIGNAL(clicked()), receiver, SLOT(addCircuit()));
    QObject::connect(tbtn_modify_circuit, SIGNAL(clicked()), receiver, SLOT(modifyCircuit()));
    QObject::connect(tbtn_remove_circuit, SIGNAL(clicked()), receiver, SLOT(removeCircuit()));
    QObject::connect(tbtn_add_inspection, SIGNAL(clicked()), receiver, SLOT(addInspection()));
    QObject::connect(tbtn_modify_inspection, SIGNAL(clicked()), receiver, SLOT(modifyInspection()));
    QObject::connect(tbtn_remove_inspection, SIGNAL(clicked()), receiver, SLOT(removeInspection()));
    QObject::connect(spb_filter_since, SIGNAL(valueChanged(int)), receiver, SLOT(refreshView()));
    QObject::connect(le_filter, SIGNAL(returnPressed()), receiver, SLOT(refreshView()));
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

void Navigation::updateView()
{
    cb_filter_column->clear();
    le_filter->clear();
    bool filter_keyword_visible = true;
    bool filter_since_visible = true;
    bool filter_visible = true;
    int group = -1;
    switch (btngrp_view->checkedId()) {
        case Navigation::ServiceCompany:
            group = 0;
            filter_keyword_visible = false;
            break;
        case Navigation::ListOfCustomers:
            filter_since_visible = false;
            cb_filter_column->addItem(QApplication::translate("Customer", "ID"), "id");
            cb_filter_column->addItem(QApplication::translate("Customer", "Company"), "company");
            cb_filter_column->addItem(QApplication::translate("Customer", "Contact person"), "contact_person");
            cb_filter_column->addItem(QApplication::translate("Customer", "Address"), "address");
            cb_filter_column->addItem(QApplication::translate("Customer", "E-mail"), "mail");
            cb_filter_column->addItem(QApplication::translate("Customer", "Phone"), "phone");
            break;
        case Navigation::ListOfCircuits:
            if (current_group == 1) goto updateView_ListOfRepairs;
            group = 2;
            filter_since_visible = false;
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
            break;
        case Navigation::ListOfInspections:
            group = 2;
            cb_filter_column->addItem(QApplication::translate("Inspection", "Date"), "date");
            cb_filter_column->addItem(QApplication::translate("Inspection", "Operator"), "operator");
            cb_filter_column->addItem(QApplication::translate("Inspection", "Remedies"), "rmds");
            break;
        case Navigation::ListOfRepairs:
            group = 1;
            updateView_ListOfRepairs:
            cb_filter_column->addItem(QApplication::translate("Repair", "Date"), "date");
            cb_filter_column->addItem(QApplication::translate("Repair", "Customer"), "customer");
            cb_filter_column->addItem(QApplication::translate("Repair", "Refrigerant"), "refrigerant");
            break;
        case Navigation::ListOfInspectors:
            group = 0;
            filter_since_visible = false;
            cb_filter_column->addItem(QApplication::translate("Inspector", "ID"), "id");
            cb_filter_column->addItem(QApplication::translate("Inspector", "Name"), "person");
            cb_filter_column->addItem(QApplication::translate("Inspector", "Person registry number"), "person_reg_num");
            cb_filter_column->addItem(QApplication::translate("Inspector", "Phone"), "phone");
            break;
        case Navigation::TableOfInspections:
            group = 2;
            filter_keyword_visible = false;
            break;
        case Navigation::Inspection:
            group = 2;
            filter_visible = false;
            break;
        case Navigation::LeakagesByApplication:
        case Navigation::Agenda:
        default:
            group = 0;
            filter_visible = false;
            break;
    }
    if (group >= 0 && current_group != group)
        toggleVisibleGroup(group, false);
    cb_filter_column->setVisible(filter_keyword_visible);
    le_filter->setVisible(filter_keyword_visible);
    lbl_filter_since->setVisible(filter_since_visible);
    spb_filter_since->setVisible(filter_since_visible);
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
        btngrp_view->button(v)->setChecked(true);
        updateView();
        current_view = (Navigation::View)v;
    }
    if (emit_signal) emit viewChanged(v);
}

void Navigation::viewServiceCompany()
{
    toggleVisibleGroup(0);
    updateView();
}

void Navigation::viewBasicLogbook()
{
    toggleVisibleGroup(1);
    updateView();
}

void Navigation::viewDetailedLogbook()
{
    toggleVisibleGroup(2);
    updateView();
}

void Navigation::toggleVisibleGroup(int g, bool emit_signal)
{
    if (current_group < 3) {
        default_view_for_group[current_group] = current_view;
    }
    if (g < 3 && current_group != g) {
        emit groupChanged(g);
    }
    current_group = g;
    gb_service_company->setVisible(g == 0);
    gb_statistics->setVisible(g == 0);
    gb_inspectors->setVisible(g == 0);
    gb_customers->setVisible(g && g < 3);
    gb_repairs->setVisible(g == 1);
    gb_circuits->setVisible(g == 2);
    gb_inspections->setVisible(g == 2);
    gb_tables->setVisible(g == 2);
    gb_filter->setVisible(g < 3);
    gb_report_data->setVisible(g == 3);
    if (emit_signal && g < 3 && current_view != default_view_for_group[current_group]) {
        current_view = default_view_for_group[current_group];
        btngrp_view->button(current_view)->setChecked(true);
        emit viewChanged(current_view);
    }
}

void Navigation::tableChanged(int)
{
    if (tbtn_view_table->isChecked()) {
        emit viewChanged(Navigation::TableOfInspections);
    }
}

void Navigation::enableTools(bool customer_selected, bool circuit_selected, bool inspection_selected, bool inspection_locked, bool repair_selected, bool repair_locked, bool inspector_selected)
{
    tbtn_modify_inspector->setEnabled(inspector_selected);
    tbtn_remove_inspector->setEnabled(inspector_selected);
    tbtn_modify_customer->setEnabled(customer_selected);
    tbtn_remove_customer->setEnabled(customer_selected);
    tbtn_modify_repair->setEnabled(repair_selected && !repair_locked);
    tbtn_remove_repair->setEnabled(repair_selected && !repair_locked);
    tbtn_view_circuits->setEnabled(customer_selected);
    gb_circuits->setEnabled(customer_selected);
    tbtn_modify_circuit->setEnabled(circuit_selected);
    tbtn_remove_circuit->setEnabled(circuit_selected);
    gb_inspections->setEnabled(circuit_selected);
    tbtn_modify_inspection->setEnabled(inspection_selected && !inspection_locked);
    tbtn_remove_inspection->setEnabled(inspection_selected && !inspection_locked);
    tbtn_view_inspection->setEnabled(inspection_selected);
    gb_tables->setEnabled(circuit_selected);
}

void Navigation::setReportDataGroupBoxVisible(bool visible)
{
    static int last_group = current_group;
    if (visible) {
        last_group = current_group;
        toggleVisibleGroup(3);
    } else {
        toggleVisibleGroup(last_group);
    }
}
