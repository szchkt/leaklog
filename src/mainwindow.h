/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2020 Matus & Michal Tomlein

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
class Authenticator;
class SyncEngine;
class Warnings;
class MTTextStream;
class HTMLTable;
class HTMLTableCell;
class HTMLTableRow;
class HTMLDiv;
class QPushButton;
class QCloseEvent;
class QPainter;
class QPrinter;
class QNetworkReply;
class QNetworkAccessManager;
class QWebEngineView;
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
    inline bool isShowNotesChecked() const { return actionShow_Notes->isChecked(); }
    inline bool isShowLeakedChecked() const { return actionShow_Leaked->isChecked(); }
    inline bool isShowPlaceOfOperationChecked() const { return actionShow_Place_of_operation->isChecked(); }
    inline bool isShowBuildingChecked() const { return actionShow_Building->isChecked(); }
    inline bool isShowDeviceChecked() const { return actionShow_Device->isChecked(); }
    inline bool isShowManufacturerChecked() const { return actionShow_Manufacturer->isChecked(); }
    inline bool isShowTypeChecked() const { return actionShow_Type->isChecked(); }
    inline bool isShowSerialNumberChecked() const { return actionShow_Serial_number->isChecked(); }
    inline bool isShowYearOfPurchaseChecked() const { return actionShow_Year_of_purchase->isChecked(); }
    inline bool isShowDateOfCommissioningChecked() const { return actionShow_Date_of_commissioning->isChecked(); }
    inline bool isShowFieldOfApplicationChecked() const { return actionShow_Field_of_application->isChecked(); }
    inline bool isShowOilChecked() const { return actionShow_Oil->isChecked(); }
    inline bool isShowMostRecentFirstChecked() const { return actionMost_recent_first->isChecked(); }
    inline bool isCompareValuesChecked() const { return actionCompare_values->isChecked(); }
    inline bool isPrinterFriendlyVersionChecked() const { return actionPrinter_friendly_version->isChecked(); }

    QString appendDefaultOrderToColumn(const QString &);

signals:
    void databaseModified();
    void tablesChanged(const MTDictionary &);
    void tableAdded(int index, const QString &uuid, const QString &name);
    void tableRemoved(const QString &uuid);

public slots:
    void enableTools();

    void reportData(bool);
    void reportDataFinished();

    void editRefrigerants();

    void editServiceCompany();
    void addRefrigerantRecord();
    void editRefrigerantRecord(const QString &uuid);

    void addCustomer();
    void editCustomer();
    void duplicateCustomer();
    void removeCustomer();
    void starCustomer(const QString &uuid);

    void decommissionAllCircuits();
    void addCircuit();
    void editCircuit();
    void duplicateCircuit();
    void duplicateAndDecommissionCircuit();
    void moveCircuit();
    void removeCircuit();
    void starCircuit(const QString &customer_uuid, const QString &uuid);

    void addInspection();
    void editInspection();
    void duplicateInspection();
    void removeInspection(const QString &uuid = QString());
    void skipInspection();

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
    void printPreview(QPrinter *printer);
    void print();
    void print(QPrinter *printer);
    void exportPDFPortrait();
    void exportPDFLandscape();
    void exportHTML();
    void printDetailedLabel();
    void printLabel(bool = false);
    void serviceCompanyInformationVisibilityChanged();
    void dateFormatChanged(MainWindowSettings::DateFormat);
    void dateFormatChanged(QAction *);
    void timeFormatChanged(MainWindowSettings::TimeFormat);
    void timeFormatChanged(QAction *);
    void toggleLocked();
    void showOperationNotPermittedMessage();
    void configurePermissions();
    void configureAutosave();
    void openBackupDirectory();
    void find();
    void findNext();
    void findPrevious();
    void refreshView();
    void changeLanguage();
    void languageChanged();
    void logIn();
    void doLogIn();
    void doLogOut(QAbstractButton *);
    void doSignUp(QAbstractButton *);
    void loginFinished(bool);
    void loginFinished(QAbstractButton *button);
    void logoutFinished();
    void checkForUpdates(bool silent = false);
    void httpRequestFinished(QNetworkReply *reply);
    void httpRequestFailed(bool silent);
    void customerChangedInMoveCircuitDialogue(int customer_index);
    // DATABASE
    void openRecent(QListWidgetItem *);
    void newDatabase(const QString &uuid = QString(), const QString &name = QString());
    void downloadDatabase();
    void open();
    void openRemote();
    void loadDatabase(bool reload = true);
    void save();
    void saveAndCompact();
    void autosave();
    void closeDatabase(bool = true);
    void autosync();
    void sync(bool force = true, bool save = true);
    void syncStarted();
    void syncProgress(double progress);
    void syncFinished(bool success, bool changed);
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
    bool initDatabase(QSqlDatabase &database, bool transaction = true, bool save_on_upgrade = true);
    void initTables(bool = true);
    void backupDatabase(const QString &path);
    void openDatabase(QSqlDatabase &database, const QString &connection_string, bool show_warnings = true);
    void saveDatabase(bool compact = false, bool update_ui = true);
    bool isOperationPermitted(const QString &, const QString & = QString());
    bool canRemoveCircuit(const QString &customer_uuid, const QString &circuit_uuid = QString());
    bool isRecordLocked(const QString &);
    void loadVariables(QTreeWidget *, QSqlDatabase = QSqlDatabase::database());
    void addVariable(bool);
    void moveTableVariable(bool);

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
    Authenticator *authenticator;
    SyncEngine *sync_engine;
    QProgressBar *progress_bar;
};

#endif // MAINWINDOW_H
