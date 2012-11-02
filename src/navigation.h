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

#ifndef NAVIGATION_H
#define NAVIGATION_H

#include "ui_navigation.h"

class MainWindowSettings;

class Navigation : public QWidget, private Ui::Navigation
{
    Q_OBJECT

public:
    enum View {
        ServiceCompany,
        RefrigerantManagement,
        ListOfCustomers,
        ListOfCircuits,
        ListOfInspections,
        Inspection,
        TableOfInspections,
        ListOfRepairs,
        ListOfInspectors,
        OperatorReport,
        LeakagesByApplication,
        Agenda,
        ListOfAssemblyRecordTypes,
        ListOfAssemblyRecordItemTypes,
        ListOfAssemblyRecordItemCategories,
        AssemblyRecord,
        ListOfCircuitUnitTypes,
        ListOfAssemblyRecords,
        Inspector,
        InspectionImages
    };

    Navigation(QWidget *);
    void restoreDefaults(bool = true);
    void connectSlots(QObject *);

    // View
    View view();
    void setView(int, bool);

    // Widgets
    void enableTools(const MainWindowSettings &);
    inline QComboBox * tableComboBox() const { return cb_view_table; }
    inline bool isTableForAllCircuitsChecked() const { return chb_table_all_circuits->isChecked(); }
    inline int filterSinceValue() const { return spb_filter_since->value(); }
    inline int filterMonthFromValue() const { return spb_filter_month_from->value(); }
    inline int filterMonthUntilValue() const { return spb_filter_month_until->value(); }
    inline QString filterColumn() const { return cb_filter_column->itemData(cb_filter_column->currentIndex(), Qt::UserRole).toString(); }
    inline bool isFilterEmpty() const { return le_filter->text().isEmpty(); }
    QString filterKeyword() const;

    // Report data
    void setReportDataGroupBoxVisible(bool visible);
    inline QToolButton * autofillButton() const { return tbtn_autofill; }
    inline QToolButton * doneButton() const { return tbtn_done; }
    inline QLabel * reportYearLabel() const { return lbl_report_year; }
    inline QProgressBar * reportDataProgressBar() const { return progressbar_loadprogress; }

    bool isAssemblyRecordListPriceChecked() { return chb_assembly_record_list_price->isChecked(); }
    bool isAssemblyRecordAcquisitionPriceChecked() { return chb_assembly_record_acquisition_price->isChecked(); }
    bool isAssemblyRecordTotalChecked() { return chb_assembly_record_total->isChecked(); }

    bool isByFieldOfApplicationChecked() { return chb_by_field->isChecked(); }

public slots:
    // View
    void viewServiceCompany();
    void viewBasicLogbook();
    void viewDetailedLogbook();
    void viewAssemblyRecords();
    void setView(int);
    void setView(const QString &);

private slots:
    void tableChanged(int);
    void toggleTableForAllCircuits();
    void emitFilterChanged();
    void monthFromChanged(int);
    void monthUntilChanged(int);

signals:
    // Groups
    void groupChanged(int);
    // View
    void viewChanged(int);
    // Widgets
    void filterChanged();

protected:
    // Groups
    void toggleVisibleGroup(int, bool = true);
    // View
    void updateView();

private:
    QButtonGroup * btngrp_view;
    int current_group;
    View current_view;
    QVector<View> default_view_for_group;
};

#endif // NAVIGATION_H
