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

#ifndef NAVIGATION_H
#define NAVIGATION_H

#include "ui_navigation.h"

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
        Agenda
    };

    Navigation(QWidget *);
    void restoreDefaults(bool = true);
    void connectSlots(QObject *);

    // View
    View view();
    void setView(int, bool);

    // Widgets
    void enableTools(bool customer_selected, bool circuit_selected, bool inspection_selected, bool inspection_locked, bool repair_selected, bool repair_locked, bool inspector_selected);
    inline QComboBox * tableComboBox() const { return cb_view_table; }
    inline int filterSinceValue() const { return spb_filter_since->value(); }
    inline QString filterColumn() const { return cb_filter_column->itemData(cb_filter_column->currentIndex(), Qt::UserRole).toString(); }
    inline bool isFilterEmpty() const { return le_filter->text().isEmpty(); }
    QString filterKeyword() const;

    // Report data
    void setReportDataGroupBoxVisible(bool visible);
    inline QToolButton * autofillButton() const { return tbtn_autofill; }
    inline QToolButton * doneButton() const { return tbtn_done; }
    inline QLabel * reportYearLabel() const { return lbl_report_year; }
    inline QProgressBar * reportDataProgressBar() const { return progressbar_loadprogress; }

public slots:
    // View
    void viewServiceCompany();
    void viewBasicLogbook();
    void viewDetailedLogbook();
    void setView(int);
    void setView(const QString &);

private slots:
    void tableChanged(int);
    void emitFilterChanged();

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
