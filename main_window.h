/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2009 Matus & Michal Tomlein

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

#include "ui_main_window.h"
#include "about_widget.h"
#include "modify_warning_dialogue.h"
#include "import_dialogue.h"
#include "report_data_controller.h"
#include "sha256.h"

#include <QCloseEvent>
#include <QSettings>
#include <QTranslator>
#include <QLocale>
#include <QFileDialog>
#include <QInputDialog>
#include <QBuffer>
#include <QUrl>
#include <QPrintPreviewDialog>
#include <QPrintDialog>
#include <QPrinter>
#include <QHttp>
#include <QBuffer>
#include <QPainter>

class MainWindow : public QMainWindow, private Ui::MainWindow
{
    Q_OBJECT

public:
    MainWindow();

    void openFile(const QString &);

private slots:
    // UI
    void showIconsOnly(bool);
    void about();
    void executeLink(const QUrl &);
    void printPreview();
    void print();
    void exportPDF();
    void exportHTML();
    void printDetailedLabel();
    void printLabel(bool = false);
    void reportData();
    void reportDataFinished();
    void enableTools();
    void toggleLocked();
    void find();
    void findNext();
    void findPrevious();
    void refreshView();
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
    void modifyServiceCompany();
    void addRecordOfRefrigerantManagement();
    void addCustomer();
    void modifyCustomer();
    void removeCustomer();
    void addCircuit();
    void modifyCircuit();
    void removeCircuit();
    void addInspection();
    void modifyInspection();
    void removeInspection();
    void addRepair();
    void modifyRepair();
    void removeRepair();
    void addVariable();
    void addSubvariable();
    void modifyVariable();
    void removeVariable();
    void addTable();
    void modifyTable();
    void removeTable();
    void loadTable(const QString &);
    void saveTable();
    void addTableVariable();
    void removeTableVariable();
    void moveTableVariableUp();
    void moveTableVariableDown();
    void addWarning();
    void modifyWarning();
    void removeWarning();
    void addInspector();
    void modifyInspector();
    void removeInspector();
    void exportCustomerData();
    void exportCircuitData();
    void exportInspectionData();
    void importData();
    // VIEW
    QString viewChanged(int);

private:
    // UI
    QMenu * createPopupMenu();
    void paintLabel(const StringVariantMap &, QPainter &, int, int, int, int);
    void addRecent(const QString &);
    void clearAll();
    void setAllEnabled(bool, bool = false);
    void updateLockButton();
    bool isRecordLocked(const QString &);
    void closeEvent(QCloseEvent *);
    void loadSettings();
    void saveSettings();
    // DATABASE
    bool saveChangesBeforeProceeding(const QString &, bool);
    void initDatabase(QSqlDatabase *, bool = true);
    void initTables(bool = true);
    void openDatabase(QString);
    void saveDatabase(bool = false);
    QString DBInfoValueForKey(const QString &);
    QSqlError setDBInfoValueForKey(const QString &, const QString &);
    void modifyRecordOfRefrigerantManagement(const QString &);
    void loadCustomer(int, bool);
    void loadCircuit(int, bool);
    void loadInspection(const QString &, bool);
    void loadRepair(const QString &, bool);
    void addVariable(bool);
    void moveTableVariable(bool);
    void loadInspector(int, bool);
    void exportData(const QString &);
    inline int selectedCustomer() { return selected_customer; }
    inline int selectedCircuit() { return selected_circuit; }
    inline QString selectedInspection() { return selected_inspection; }
    inline QString selectedRepair() { return selected_repair; }
    inline int selectedInspector() { return selected_inspector; }
    // VIEW
    QString viewServiceCompany(int);
    QString viewAllCustomers();
    QString viewCustomer(const QString &);
    QString viewCircuit(const QString &, const QString &, int);
    QString viewInspection(const QString &, const QString &, const QString &);
    QString viewTable(const QString &, const QString &, const QString &, int);
    QString viewRepairs(const QString &, int, const QString & = QString());
    QString viewAllInspectors(const QString &);
    QString viewLeakagesByApplication();
    QString viewAgenda();
    QStringList listWarnings(Warnings &, const QString &, const QString &, StringVariantMap &, StringVariantMap &);
    QStringList listDelayedWarnings(Warnings &, const QString &, const QString &, StringVariantMap &, const QString &, const QString &, int * = NULL);
    void writeTableVarCell(MTTextStream &, const QString &, const QString &, const QString &, const QString &, bool, int, double);
    void writeCustomersTable(MTTextStream &, const QString & = QString());
    void writeCircuitsTable(MTTextStream &, const QString &, const QString & = QString());

    int selected_customer;
    QString selected_customer_company;
    int selected_circuit;
    QString selected_inspection;
    bool selected_inspection_is_repair;
    QString selected_repair;
    int selected_inspector;
    QString selected_inspector_name;
    bool database_locked;
    QString database_lock_date;
    QSet<int> years_expanded_in_service_company_view;
    bool show_leaked_in_store_in_service_company_view;
    MTDictionary dict_dbtables;
    MTDictionary dict_vartypes;
    MTDictionary dict_varnames;
    MTDictionary dict_attrvalues;
    MTDictionary dict_attrnames;
    QMap<Navigation::View, QString> dict_html;
    QMap<QString, MTVariant::Type> dict_fieldtypes;
    QActionGroup * actgrp_view;
    QAction * actionShow_icons_only;
    QLabel * lbl_current_selection;
    QLabel * lbl_selected_repair;
    QLabel * lbl_selected_inspector;
    QToolButton * tbtn_open;
    QToolButton * tbtn_add;
    QToolButton * tbtn_modify;
    QToolButton * tbtn_export;
    QString last_search_keyword;
    QSqlDatabase db;
    QComboBox * cb_lang;
    QMap<QString, QString> leaklog_i18n;
    QHttp * http; QBuffer * http_buffer;
};
