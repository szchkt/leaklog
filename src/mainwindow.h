/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2014 Matus & Michal Tomlein

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
#include "toolbarstack.h"

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

    void scaleFactorChanged();

    inline MainWindowSettings &settings() { return m_settings; }
    inline UndoStack *undoStack() const { return m_undo_stack; }
    inline ViewTab *currentTab() const { return m_tab; }

    inline bool isShowDateUpdatedChecked() const { return actionShow_date_updated->isChecked(); }
    inline bool isShowOwnerChecked() const { return actionShow_owner->isChecked(); }
    inline bool isCompareValuesChecked() const { return actionCompare_values->isChecked(); }
    inline bool isPrinterFriendlyVersionChecked() const { return actionPrinter_friendly_version->isChecked(); }

    QString appendDefaultOrderToColumn(const QString &);

signals:
    void databaseModified();
    void tablesChanged(const QStringList &);
    void tableAdded(int index, const QString &);
    void tableRemoved(const QString &);

public slots:
    void enableTools();

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
    void moveCircuit();
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
    void openDocumentation();
    void openReleaseNotes();
    void printPreview();
    void print();
    void exportPDFPortrait();
    void exportPDFLandscape();
    void exportHTML();
    void printDetailedLabel();
    void printLabel(bool = false);
    void serviceCompanyInformationVisibilityChanged(bool visible);
    void dateFormatChanged(MainWindowSettings::DateFormat);
    void dateFormatChanged(QAction *);
    void timeFormatChanged(MainWindowSettings::TimeFormat);
    void timeFormatChanged(QAction *);
    void toggleLocked();
    void showOperationNotPermittedMessage();
    void configurePermissions();
    void configureAutosave();
    void find();
    void findAll();
    void findNext();
    void findPrevious();
    void refreshView();
    void changeLanguage();
    void languageChanged();
    void checkForUpdates(bool silent = false);
    void httpRequestFinished(QNetworkReply *reply);
    void httpRequestFailed(bool silent);
    void customerChangedInMoveCircuitDialogue(int customer_index);
    // DATABASE
    void openRecent(QListWidgetItem *);
    void newDatabase();
    void open();
    void openRemote();
    void loadDatabase(bool reload = true);
    void save();
    void saveAndCompact();
    void autosave();
    void closeDatabase(bool = true);
    void setDatabaseModified(bool modified);
    // TABS
    void newTab(bool init = true);
    void closeTab();
    void closeTab(int);
    void tabChanged(int index);
    void tabTextChanged(QWidget *tab, const QString &text);

private:
    // UI
#ifdef Q_OS_MAC
    void macInitUI();
    bool isFullScreen() const;
    void showFullScreen();
#endif
    bool hasActiveModalWidget();
    void clearWindowTitle();
    void setWindowTitleWithRepresentedFilename(const QString &path);
    QMenu *createPopupMenu();
    void paintLabel(const QVariantMap &, QPainter &, int, int, int, int);
    QString fileNameForCurrentView();
    void exportPDF(int);
    void addRecent(const QString &);
    void setAllEnabled(bool, bool = false);
    void updateLockButton();
    void closeEvent(QCloseEvent *);
    void loadSettings();
    void saveSettings();
    // DATABASE
    bool saveChangesBeforeProceeding(const QString &, bool);
    void initDatabase(QSqlDatabase &database, bool transaction = true, bool save_on_upgrade = true);
    void initTables(bool = true);
    void openDatabase(QString path, const QString &connection_string);
    void saveDatabase(bool compact = false, bool update_ui = true);
    bool isOperationPermitted(const QString &, const QString & = QString());
    bool isRecordLocked(const QString &);
    void loadVariables(QTreeWidget *, QSqlDatabase = QSqlDatabase::database());
    void addVariable(bool);
    void moveTableVariable(bool);
    void exportData(const QString &);

    QAction *actionShow_icons_only;
    QToolButton *tbtn_open;
    QToolButton *tbtn_undo;
    QAction *tbtn_undo_action;
    QToolButton *tbtn_export;
    QLineEdit *le_search;
    QComboBox *cb_lang;
    QMap<QAction *, MainWindowSettings::DateFormat> dict_action_date_format;
    QActionGroup *actgrp_date_format;
    QMap<QAction *, MainWindowSettings::TimeFormat> dict_action_time_format;
    QActionGroup *actgrp_time_format;
    QMap<QString, QString> leaklog_i18n;
    QNetworkAccessManager *network_access_manager;
    MainWindowSettings m_settings;
    UndoStack *m_undo_stack;
    ViewTab *m_tab;
    QString m_connection_string;
    double m_current_scale;
};

#endif // MAINWINDOW_H
