/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2023 Matus & Michal Tomlein

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

#ifndef VIEWTABSETTINGS_H
#define VIEWTABSETTINGS_H

#include "view.h"
#include "linkparser.h"

#include <QString>

class ToolBarStack;
class MainWindowSettings;

class QObject;
class QSettings;
class QWebEngineView;

class ViewTabSettings
{
public:
    ViewTabSettings();
    virtual QObject *object() = 0;

    virtual void enableAllTools() = 0;
    virtual void enableTools() = 0;

    virtual MainWindowSettings &mainWindowSettings() = 0;

    virtual ToolBarStack *toolBarStack() const = 0;
    virtual QWebEngineView *webView() const = 0;

    virtual void setView(View::ViewID view, const QString &table = QString()) = 0;
    virtual void refreshView() = 0;

    virtual View *view(View::ViewID view) = 0;
    virtual View::ViewID currentView() const = 0;
    virtual QString currentViewTitle() const = 0;
    virtual QString currentTableUUID() const = 0;

    virtual bool isShowDateUpdatedChecked() const = 0;
    virtual bool isShowOwnerChecked() const = 0;
    virtual bool isShowNotesChecked() const = 0;
    virtual bool isShowLeakedChecked() const = 0;
    virtual bool isShowPlaceOfOperationChecked() const = 0;
    virtual bool isShowBuildingChecked() const = 0;
    virtual bool isShowDeviceChecked() const = 0;
    virtual bool isShowManufacturerChecked() const = 0;
    virtual bool isShowTypeChecked() const = 0;
    virtual bool isShowSerialNumberChecked() const = 0;
    virtual bool isShowYearOfPurchaseChecked() const = 0;
    virtual bool isShowDateOfCommissioningChecked() const = 0;
    virtual bool isShowFieldOfApplicationChecked() const = 0;
    virtual bool isShowOilChecked() const = 0;
    virtual bool isShowMostRecentFirstChecked() const = 0;
    virtual bool isCompareValuesChecked() const = 0;
    virtual bool isPrinterFriendlyVersionChecked() const = 0;

    virtual QString appendDefaultOrderToColumn(const QString &) const = 0;

    void saveSettings(QSettings &settings) const;
    void restoreSettings(QSettings &settings);
    void validateSelection();

    virtual QString filterServiceCompanyUUID() const = 0;
    virtual bool isFilterServiceCompanyChecked() const = 0;
    virtual void setFilterServiceCompanyChecked(bool checked) = 0;
    virtual bool isServiceCompanySelected() const = 0;
    virtual QString selectedServiceCompanyUUID() const = 0;
    virtual void setSelectedServiceCompanyUUID(const QString &service_company_uuid) = 0;

    inline bool isCustomerSelected() const { return !_customer_uuid.isEmpty(); }
    inline QString selectedCustomerUUID() const { return _customer_uuid; }
    void setSelectedCustomerUUID(const QString &customer_uuid) { clearSelectedCircuit(); _customer_uuid = customer_uuid; }
    void clearSelectedCustomer() { _customer_uuid.clear(); clearSelectedCircuit(); }

    inline bool isCircuitSelected() const { return !_circuit_uuid.isEmpty(); }
    inline QString selectedCircuitUUID() const { return _circuit_uuid; }
    void setSelectedCircuitUUID(const QString &circuit_uuid) { clearSelectedCircuit(); _circuit_uuid = circuit_uuid; }
    void clearSelectedCircuit() { _circuit_uuid.clear(); clearSelectedCompressor(); clearSelectedInspection(); clearSelectedRepair(); }

    inline bool isCompressorSelected() const { return !_compressor_uuid.isEmpty(); }
    inline QString selectedCompressorUUID() const { return _compressor_uuid; }
    void setSelectedCompressorUUID(const QString &compressor_uuid) { _compressor_uuid = compressor_uuid; }
    void clearSelectedCompressor() { _compressor_uuid.clear(); }

    inline bool isInspectionSelected() const { return !_inspection_uuid.isEmpty(); }
    inline QString selectedInspectionUUID() const { return _inspection_uuid; }
    void setSelectedInspectionUUID(const QString &inspection_uuid) { _inspection_uuid = inspection_uuid; }
    void clearSelectedInspection() { _inspection_uuid.clear(); }

    inline bool isRepairSelected() const { return !_repair_uuid.isEmpty(); }
    inline QString selectedRepairUUID() const { return _repair_uuid; }
    void setSelectedRepairUUID(const QString &repair_uuid) { _repair_uuid = repair_uuid; }
    inline void clearSelectedRepair() { _repair_uuid.clear(); }

    inline bool isInspectorSelected() const { return !_inspector_uuid.isEmpty(); }
    inline QString selectedInspectorUUID() const { return _inspector_uuid; }
    void setSelectedInspectorUUID(const QString &inspector_uuid) { _inspector_uuid = inspector_uuid; }
    inline void clearSelectedInspector() { _inspector_uuid.clear(); }

    inline bool isAssemblyRecordTypeSelected() const { return !_ar_type_uuid.isEmpty(); }
    inline QString selectedAssemblyRecordTypeUUID() const { return _ar_type_uuid; }
    void setSelectedAssemblyRecordTypeUUID(const QString &ar_type_uuid) { _ar_type_uuid = ar_type_uuid; }
    inline void clearSelectedAssemblyRecordType() { _ar_type_uuid.clear(); }

    inline bool isAssemblyRecordItemTypeSelected() const { return !_ar_item_type_uuid.isEmpty(); }
    inline QString selectedAssemblyRecordItemTypeUUID() const { return _ar_item_type_uuid; }
    void setSelectedAssemblyRecordItemTypeUUID(const QString &ar_item_type_uuid) { _ar_item_type_uuid = ar_item_type_uuid; }
    inline void clearSelectedAssemblyRecordItemType() { _ar_item_type_uuid.clear(); }

    inline bool isAssemblyRecordItemCategorySelected() const { return !_ar_item_category_uuid.isEmpty(); }
    inline QString selectedAssemblyRecordItemCategoryUUID() const { return _ar_item_category_uuid; }
    void setSelectedAssemblyRecordItemCategoryUUID(const QString &ar_item_category_uuid) { _ar_item_category_uuid = ar_item_category_uuid; }
    inline void clearSelectedAssemblyRecordItemCategory() { _ar_item_category_uuid.clear(); }

    inline bool isCircuitUnitTypeSelected() const { return !_circuit_unit_type_uuid.isEmpty(); }
    inline QString selectedCircuitUnitTypeUUID() const { return _circuit_unit_type_uuid; }
    void setSelectedCircuitUnitTypeUUID(const QString &circuit_unit_type_uuid) { _circuit_unit_type_uuid = circuit_unit_type_uuid; }
    inline void clearSelectedCircuitUnitType() { _circuit_unit_type_uuid.clear(); }

    void loadCustomer(const QString &customer_uuid, bool refresh);
    void loadCircuit(const QString &circuit_uuid, bool refresh);
    void loadInspection(const QString &inspection_uuid, bool refresh);
    void loadRepair(const QString &repair_uuid, bool refresh);
    void loadInspector(const QString &inspector_uuid, bool refresh);
    void loadInspectorReport(const QString &inspector_uuid, bool refresh);
    void loadAssemblyRecordType(const QString &ar_type_uuid, bool refresh);
    void loadAssemblyRecordItemType(const QString &ar_item_type_uuid, bool refresh);
    void loadAssemblyRecordItemCategory(const QString &ar_item_category_uuid, bool refresh);
    void loadAssemblyRecord(const QString &inspection_uuid, bool refresh);
    void loadCircuitUnitType(const QString &circuit_unit_type_uuid, bool refresh);

    LinkParser &linkParser() { return link_parser; }

private:
    QString _customer_uuid;
    QString _circuit_uuid;
    QString _compressor_uuid;
    QString _inspection_uuid;
    QString _repair_uuid;
    QString _inspector_uuid;
    QString _ar_type_uuid;
    QString _ar_item_type_uuid;
    QString _ar_item_category_uuid;
    QString _circuit_unit_type_uuid;

    LinkParser link_parser;
};

#endif // VIEWTABSETTINGS_H
