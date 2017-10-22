/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2017 Matus & Michal Tomlein

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

#ifndef TOOLBARSTACK_H
#define TOOLBARSTACK_H

#include "ui_toolbarstack.h"
#include "view.h"

class ViewTabSettings;
class MTDictionary;

class ToolBarStack : public QWidget, private Ui::ToolBarStack
{
    Q_OBJECT

public:
    ToolBarStack(QWidget *parent);
    void setSettings(ViewTabSettings *settings);
    void connectSlots(QObject *);

    void scaleFactorChanged();

    // Widgets
    void enableTools();

    inline bool isTableForAllCircuitsChecked() const { return chb_table_all_circuits->isChecked(); }
    inline bool isTableForAllCircuitsExceptDecommissionedChecked() const { return isTableForAllCircuitsChecked() && chb_table_except_decommissioned_before->isChecked(); }
    inline QDate minimumDecommissioningDateForTableOfAllCircuits() const { return de_table_except_decommissioned_before->date(); }
    inline int filterSinceValue() const { return spb_filter_since->value() == 1999 ? 0 : spb_filter_since->value(); }
    inline int filterMonthFromValue() const { return spb_filter_month_from->value(); }
    inline int filterMonthUntilValue() const { return spb_filter_month_until->value(); }
    inline QString filterColumn() const { return cb_filter_column->itemData(cb_filter_column->currentIndex(), Qt::UserRole).toString(); }
    inline bool isFilterEmpty() const { return le_filter->text().isEmpty() && !filterColumn().contains('?'); }
    QString filterKeyword() const;

    bool isAssemblyRecordListPriceChecked() const { return chb_assembly_record_list_price->isChecked(); }
    bool isAssemblyRecordAcquisitionPriceChecked() const { return chb_assembly_record_acquisition_price->isChecked(); }
    bool isAssemblyRecordTotalChecked() const { return chb_assembly_record_total->isChecked(); }

    bool isByFieldOfApplicationChecked() const { return chb_by_field->isChecked(); }
    bool isShowBusinessPartnerChecked() const { return chb_show_partner->isChecked(); }
    bool isShowCircuitNameChecked() const { return chb_show_circuit_name->isChecked(); }

    bool isCO2EquivalentChecked() const { return chb_CO2_equivalent->isChecked(); }
    bool isMin5tCO2EquivalentChecked() const { return chb_CO2_equivalent->isChecked() ? chb_min_5tCO2->isChecked() : chb_min_3kg->isChecked(); }

    // Reporting
    void setReportDataGroupBoxVisible(bool visible);
    inline QToolButton *autofillButton() const { return tbtn_autofill; }
    inline QToolButton *doneButton() const { return tbtn_done; }
    inline QLabel *reportYearLabel() const { return lbl_report_year; }
    inline QProgressBar *reportDataProgressBar() const { return progressbar_loadprogress; }

public slots:
    void clearInspector();
    void clearCustomer();
    void clearCircuit();
    void clearInspection();
    void clearRepair();
    void clearAssemblyRecordType();
    void clearAssemblyRecordItemType();
    void clearAssemblyRecordItemCategory();
    void clearCircuitUnitType();

    void viewChanged(View::ViewID view);

private slots:
    void toggleCO2Equivalent();
    void toggleTableForAllCircuits();
    void emitFilterChanged();
    void monthFromChanged(int);
    void monthUntilChanged(int);

signals:
    void filterChanged();

protected:
    void addFilterItems(const QString &column, const MTDictionary &items);

    View::ViewID _view;
    ViewTabSettings *_settings;
};

#endif // TOOLBARSTACK_H
