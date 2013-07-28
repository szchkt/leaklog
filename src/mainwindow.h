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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "defs.h"
#include "ui_mainwindow.h"
#include "mtdictionary.h"
#include "mainwindowsettings.h"
#include "viewtab.h"
#include "navigation.h"

#include <QSqlDatabase>

class UndoStack;
class Warnings;
class MTTextStream;
class HTMLTable;
class HTMLTableCell;
class HTMLTableRow;
class HTMLDiv;
class QPushButton;
class QCloseEvent;
class QPainter;
class QNetworkReply;
class QNetworkAccessManager;
class QWebView;
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

    inline MainWindowSettings & settings() { return m_settings; }
    inline UndoStack * undoStack() const { return m_undo_stack; }

    inline bool isShowDateUpdatedChecked() const { return actionShow_date_updated->isChecked(); }
    inline bool isShowOwnerChecked() const { return actionShow_owner->isChecked(); }
    inline bool isCompareValuesChecked() const { return actionCompare_values->isChecked(); }
    inline bool isPrinterFriendlyVersionChecked() const { return actionPrinter_friendly_version->isChecked(); }

    QString appendDefaultOrderToColumn(const QString &);

signals:
    void tablesChanged(const QStringList &);
    void tableAdded(int index, const QString &);
    void tableRemoved(const QString &);

public slots:
    void reportData(bool);
    void reportDataFinished();

    void editServiceCompany();
    void addRecordOfRefrigerantManagement();
    void editRecordOfRefrigerantManagement(const QString &);

    void addCustomer();
    void editCustomer();
    void duplicateCustomer();
    void removeCustomer();

    void decommissionAllCircuits();
    void addCircuit();
    void editCircuit();
    void duplicateCircuit();
    void duplicateAndDecommissionCircuit();
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

private slots:
    // UI
    void showRecentDatabaseContextMenu(const QPoint &);
    void showIconsOnly(bool);
    void about();
    void printPreview();
    void print();
    void exportPDFPortrait();
    void exportPDFLandscape();
    void exportHTML();
    void printDetailedLabel();
    void printLabel(bool = false);
    void dateFormatChanged(MainWindowSettings::DateFormat);
    void dateFormatChanged(QAction *);
    void timeFormatChanged(MainWindowSettings::TimeFormat);
    void timeFormatChanged(QAction *);
    void enableTools();
    void toggleLocked();
    void showOperationNotPermittedMessage();
    void configurePermissions();
    void goBack();
    void goForward();
    void find();
    void findNext();
    void findPrevious();
    void clearSelection(bool = true);
    void refreshView();
    void changeLanguage();
    void languageChanged();
    void checkForUpdates(bool silent = false);
    void httpRequestFinished(QNetworkReply * reply);
    void httpRequestFailed(bool silent);
    // DATABASE
    void openRecent(QListWidgetItem *);
    void newDatabase();
    void open();
    void openRemote();
    void loadDatabase(bool reload = true);
    void save();
    void saveAndCompact();
    void closeDatabase(bool = true);

private:
    // UI
#ifdef Q_OS_MAC
    void macInitUI();
    bool isFullScreen() const;
    void showFullScreen();
#endif
    void clearWindowTitle();
    void setWindowTitleWithRepresentedFilename(const QString & path);
    QMenu * createPopupMenu();
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
    void initDatabase(QSqlDatabase & database, bool transaction = true, bool save_on_upgrade = true);
    void initTables(bool = true);
    void openDatabase(QString);
    void saveDatabase(bool compact = false, bool update_ui = true);
    bool isOperationPermitted(const QString &, const QString & = QString());
    bool isRecordLocked(const QString &);
    void loadVariables(QTreeWidget *, QSqlDatabase = QSqlDatabase::database());
    void addVariable(bool);
    void moveTableVariable(bool);
    void exportData(const QString &);
    inline bool isCustomerSelected() { return current_tab->isCustomerSelected(); }
    inline QString selectedCustomer() { return current_tab->selectedCustomer(); }
    inline bool isCircuitSelected() { return current_tab->isCircuitSelected(); }
    inline QString selectedCircuit() { return current_tab->selectedCircuit(); }
    inline bool isCompressorSelected() { return current_tab->isCompressorSelected(); }
    inline QString selectedCompressor() { return current_tab->selectedCompressor(); }
    inline bool isInspectionSelected() { return current_tab->isInspectionSelected(); }
    inline QString selectedInspection() { return current_tab->selectedInspection(); }
    inline bool isRepairSelected() { return current_tab->isRepairSelected(); }
    inline QString selectedRepair() { return current_tab->selectedRepair(); }
    inline bool isInspectorSelected() { return current_tab->isInspectorSelected(); }
    inline QString selectedInspector() { return current_tab->selectedInspector(); }
    inline QString selectedAssemblyRecordType() { return current_tab->selectedAssemblyRecordType(); }
    inline bool isAssemblyRecordTypeSelected() { return current_tab->isAssemblyRecordTypeSelected(); }
    inline bool isAssemblyRecordItemTypeSelected() { return current_tab->isAssemblyRecordItemTypeSelected(); }
    inline bool isAssemblyRecordItemCategorySelected() { return current_tab->isAssemblyRecordItemCategorySelected(); }
    inline QString selectedAssemblyRecordItemType() { return current_tab->selectedAssemblyRecordItemType(); }
    inline QString selectedAssemblyRecordItemCategory() { return current_tab->selectedAssemblyRecordItemCategory(); }
    inline bool isCircuitUnitTypeSelected() { return current_tab->isCircuitUnitTypeSelected(); }
    inline QString selectedCircuitUnitType() { return current_tab->selectedCircuitUnitType(); }
    // VIEW
    QString fileNameForCurrentView();

    QAction * actionShow_icons_only;
    QLabel * lbl_current_selection;
    QLabel * lbl_selected_repair;
    QLabel * lbl_selected_inspector;
    QPushButton * btn_clear_selection;
    QToolButton * tbtn_open;
    QToolButton * tbtn_undo;
    QToolButton * tbtn_export;
    QString last_search_keyword;
    QComboBox * cb_lang;
    QMap<QAction *, MainWindowSettings::DateFormat> dict_action_date_format;
    QActionGroup * actgrp_date_format;
    QMap<QAction *, MainWindowSettings::TimeFormat> dict_action_time_format;
    QActionGroup * actgrp_time_format;
    QMap<QString, QString> leaklog_i18n;
    QNetworkAccessManager * network_access_manager;
    MainWindowSettings m_settings;
    UndoStack * m_undo_stack;
    Navigation * navigation;
    ViewTab * current_tab;
};

#endif // MAINWINDOW_H
