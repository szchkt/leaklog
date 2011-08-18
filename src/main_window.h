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

#include "defs.h"
#include "ui_main_window.h"
#include "mtdictionary.h"
#include "link_parser.h"

#include <QSqlDatabase>

class Warnings;
class MTTextStream;
class HTMLTable;
class HTMLTableCell;
class HTMLDiv;
class QPushButton;
class QCloseEvent;
class QPainter;
class QUrl;
class QHttp;
class QBuffer;
namespace VariableEvaluation {
    class Variable;
    class EvaluationContext;
}

class MainWindow : public QMainWindow, private Ui::MainWindow
{
    Q_OBJECT

public:
    MainWindow();

    void openFile(const QString &);

private slots:
    // UI
    void showRecentDatabaseContextMenu(const QPoint &);
    void showIconsOnly(bool);
    void about();
    void executeLink(const QUrl &);
    void printPreview();
    void print();
    void exportPDFPortrait();
    void exportPDFLandscape();
    void exportHTML();
    void printDetailedLabel();
    void printLabel(bool = false);
    void reportData();
    void reportDataFinished();
    void enableTools();
    void toggleLocked();
    void showOperationNotPermittedMessage();
    void configurePermissions();
    void find();
    void findNext();
    void findPrevious();
    void clearSelection(bool = true);
    void refreshView();
    void groupChanged(int);
    void changeLanguage();
    void languageChanged();
    void checkForUpdates();
    void httpRequestFinished(bool);
    // DATABASE
    void openRecent(QListWidgetItem *);
    void newDatabase();
    void open();
    void openRemote();
    void save();
    void saveAndCompact();
    void closeDatabase(bool = true);
    void editServiceCompany();
    void addRecordOfRefrigerantManagement();
    void addCustomer();
    void editCustomer();
    void duplicateCustomer();
    void removeCustomer();
    void addCircuit();
    void editCircuit();
    void duplicateCircuit();
    void removeCircuit();
    void addInspection();
    void editInspection();
    void duplicateInspection();
    void removeInspection();
    void addRepair();
    void editRepair();
    void duplicateRepair();
    void removeRepair();
    void addVariable();
    void addSubvariable();
    void editVariable();
    void removeVariable();
    void addTable();
    void editTable();
    void removeTable();
    void loadTable(const QString &);
    void saveTable();
    void addTableVariable();
    void removeTableVariable();
    void moveTableVariableUp();
    void moveTableVariableDown();
    void addWarning();
    void editWarning();
    void removeWarning();
    void addInspector();
    void editInspector();
    void removeInspector();
    void exportCustomerData();
    void exportCircuitData();
    void exportInspectionData();
    void importData();
    void importCSV();
    void addAssemblyRecordType();
    void editAssemblyRecordType();
    void removeAssemblyRecordType();
    void addAssemblyRecordItemType();
    void editAssemblyRecordItemType();
    void removeAssemblyRecordItemType();
    void addAssemblyRecordItemCategory();
    void editAssemblyRecordItemCategory();
    void removeAssemblyRecordItemCategory();
    void addCircuitUnitType();
    void editCircuitUnitType();
    void removeCircuitUnitType();
    void addStyle();
    void editStyle();
    void removeStyle();
    // VIEW
    QString viewChanged(int);

private:
    // UI
    QMenu * createPopupMenu();
    void setDefaultWebPage();
    void paintLabel(const QVariantMap &, QPainter &, int, int, int, int);
    void exportPDF(int);
    void addRecent(const QString &);
    void clearAll();
    void setAllEnabled(bool, bool = false);
    void updateLockButton();
    void closeEvent(QCloseEvent *);
    void loadSettings();
    void saveSettings();
    // DATABASE
    bool saveChangesBeforeProceeding(const QString &, bool);
    void initDatabase(QSqlDatabase &, bool = true);
    void initTables(bool = true);
    void openDatabase(QString);
    void saveDatabase(bool = false);
    bool isOperationPermitted(const QString &, const QString & = QString());
    bool isRecordLocked(const QString &);
    void editRecordOfRefrigerantManagement(const QString &);
    void loadCustomer(int, bool);
    void loadCircuit(int, bool);
    void loadInspection(const QString &, bool);
    void loadRepair(const QString &, bool);
    void loadVariables(QTreeWidget *, QSqlDatabase = QSqlDatabase::database());
    void addVariable(bool);
    void moveTableVariable(bool);
    void loadInspector(int, bool);
    void loadInspectorReport(int, bool);
    void exportData(const QString &);
    void loadAssemblyRecordType(int, bool);
    void loadAssemblyRecordItemType(int, bool);
    void loadAssemblyRecordItemCategory(int, bool);
    void loadAssemblyRecord(const QString &, bool);
    void loadCircuitUnitType(int, bool);
    inline bool isCustomerSelected() { return selected_customer >= 0; }
    inline QString selectedCustomer() { return QString::number(selected_customer); }
    inline bool isCircuitSelected() { return selected_circuit >= 0; }
    inline QString selectedCircuit() { return QString::number(selected_circuit); }
    inline bool isCompressorSelected() { return selected_compressor >= 0; }
    inline QString selectedCompressor() { return QString::number(selected_compressor); }
    inline bool isInspectionSelected() { return !selected_inspection.isEmpty(); }
    inline QString selectedInspection() { return selected_inspection; }
    inline bool isRepairSelected() { return !selected_repair.isEmpty(); }
    inline QString selectedRepair() { return selected_repair; }
    inline bool isInspectorSelected() { return selected_inspector >= 0; }
    inline QString selectedInspector() { return QString::number(selected_inspector); }
    inline QString selectedAssemblyRecordType() { return QString::number(selected_assembly_record_type); }
    inline bool isAssemblyRecordTypeSelected() { return selected_assembly_record_type >= 0; }
    inline bool isAssemblyRecordItemTypeSelected() { return selected_assembly_record_item_type >= 0; }
    inline bool isAssemblyRecordItemCategorySelected() { return selected_assembly_record_item_category >= 0; }
    inline QString selectedAssemblyRecordItemType() { return QString::number(selected_assembly_record_item_type); }
    inline QString selectedAssemblyRecordItemCategory() { return QString::number(selected_assembly_record_item_category); }
    inline bool isCircuitUnitTypeSelected() { return selected_circuit_unit_type >= 0; }
    inline QString selectedCircuitUnitType() { return QString::number(selected_circuit_unit_type); }
    // VIEW
    QString currentView();
    QString fileNameForCurrentView();
    QString viewServiceCompany(int);
    QString viewRefrigerantManagement(int);
    QString viewAllCustomers();
    QString viewCustomer(const QString &);
    QString viewCircuit(const QString &, const QString &, int);
    QString viewInspection(const QString &, const QString &, const QString &);
    QString viewTable(const QString &, const QString &, const QString &, int, const QString & = QString());
    QString viewRepairs(const QString &, int, const QString & = QString());
    QString viewAllInspectors(const QString &);
    QString viewInspector(const QString &);
    QString viewOperatorReport(const QString &, int);
    QString viewLeakagesByApplication();
    QString viewAgenda();
    QString viewAllAssemblyRecordTypes(const QString & = QString());
    QString viewAllAssemblyRecordItemTypes(const QString & = QString());
    QString viewAllAssemblyRecordItemCategories(const QString & = QString());
    QString viewAssemblyRecord(const QString &, const QString &, const QString &);
    QString viewAllCircuitUnitTypes(const QString & = QString());
    QString viewAllAssemblyRecords(const QString &, const QString &, int);
    QString viewInspectionImages(const QString &, const QString &, const QString &);
    QStringList listWarnings(Warnings &, const QString &, const QString &, QVariantMap &, QVariantMap &);
    QStringList listDelayedWarnings(Warnings &, const QString &, const QString &, QVariantMap &, const QString &, const QString &, int * = NULL);
    void writeTableVarCell(MTTextStream &, const QString &, const QString &, const QString &, const QString &, bool, int, double);
    HTMLTableCell * writeTableVarCell(const QString &, const QString &, const QString &, const QString &, bool, int, double);
    void writeCustomersTable(MTTextStream &, const QString & = QString());
    HTMLTable * writeCustomersTable(const QString &, HTMLTable * = NULL);
    HTMLDiv * writeCircuitsTable(const QString &, const QString &, int, HTMLTable * = NULL);
    void writeCircuitsTable(MTTextStream &, const QString &, const QString & = QString(), int = -1);
    QString tableVarValue(const QString &, const QString &, const QString &, const QString &, bool, double, bool = false);
    HTMLTable * writeServiceCompany(HTMLTable * = NULL);
    HTMLTable * circuitUnitsTable(const QString &, const QString &, HTMLTable * = NULL);
    HTMLTable * circuitCompressorsTable(const QString &, const QString &, HTMLTable * = NULL);
    HTMLTable * customerContactPersons(const QString &, HTMLTable * = NULL);
    HTMLTable * writeInspectorsTable(const QString &, const QString & = QString());
    void showVariableInInspectionTable(VariableEvaluation::Variable *, VariableEvaluation::EvaluationContext &, QVariantMap &, HTMLTable *);
    HTMLTable * writeInspectionsTable(const QString &, const QString &, const QVariantMap &, ListOfVariantMaps &, VariableEvaluation::EvaluationContext &);

    int selected_customer;
    QString selected_customer_company;
    int selected_circuit;
    int selected_compressor;
    QString selected_inspection;
    bool selected_inspection_is_repair;
    QString selected_repair;
    int selected_inspector;
    QString selected_inspector_name;
    int selected_assembly_record_type;
    int selected_assembly_record_item_type;
    int selected_assembly_record_item_category;
    int selected_circuit_unit_type;
    QSet<int> years_expanded_in_service_company_view;
    QMap<Navigation::View, QString> dict_html;
    QMap<QString, int> dict_fieldtypes;
    QActionGroup * actgrp_view;
    QAction * actionShow_icons_only;
    QLabel * lbl_current_selection;
    QLabel * lbl_selected_repair;
    QLabel * lbl_selected_inspector;
    QPushButton * btn_clear_selection;
    QToolButton * tbtn_open;
    QToolButton * tbtn_export;
    QString last_search_keyword;
    QComboBox * cb_lang;
    QMap<QString, QString> leaklog_i18n;
    bool check_for_updates;
    QHttp * http; QBuffer * http_buffer;
    LinkParser link_parser;
    Link * last_link;
};
