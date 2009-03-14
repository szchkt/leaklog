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

private slots:
    // UI
    void showIconsOnly(bool);
    void about();
    void executeLink(const QUrl &);
    void printPreview();
    void print();
    void exportPDF();
    void exportHTML();
    void printLabel();
    void enableTools();
    void find();
    void findNext();
    void findPrevious();
    void clearSelection(bool = true);
    void setView(QAction *);
    void setView(const QString &);
    void refreshView();
    void viewLevelUp();
    void viewLevelDown();
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
    void loadCustomer(QListWidgetItem *);
    void addCircuit();
    void modifyCircuit();
    void removeCircuit();
    void loadCircuit(QListWidgetItem *);
    void addInspection();
    void modifyInspection();
    void removeInspection();
    void loadInspection(QListWidgetItem *);
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
    void loadInspector(QListWidgetItem *);
    void exportCustomerData();
    void exportCircuitData();
    void exportInspectionData();
    void importData();
    // VIEW
    QString viewChanged(const QString &);

private:
    // UI
    QMenu * createPopupMenu();
    void paintLabel(const StringVariantMap &, QPainter &, int, int, int, int);
    void addRecent(const QString &);
    void clearAll();
    void setAllEnabled(bool);
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
    void loadCustomer(QListWidgetItem *, bool);
    void loadCircuit(QListWidgetItem *, bool);
    void loadInspection(QListWidgetItem *, bool);
    void loadRepair(const QString &, bool);
    void addVariable(bool);
    void moveTableVariable(bool);
    void loadInspector(QListWidgetItem *, bool);
    void exportData(const QString &);
    inline int selectedCustomer() { return lw_customers->highlightedRow() < 0 ? -1 : lw_customers->highlightedItem()->data(Qt::UserRole).toInt(); };
    inline int selectedCircuit() { return lw_circuits->highlightedRow() < 0 ? -1 : lw_circuits->highlightedItem()->data(Qt::UserRole).toInt(); };
    inline QString selectedInspection() { return lw_inspections->highlightedRow() < 0 ? QString() : lw_inspections->highlightedItem()->data(Qt::UserRole).toString(); };
    inline QString selectedRepair() { return selected_repair; };
    inline int selectedInspector() { return lw_inspectors->highlightedRow() < 0 ? -1 : lw_inspectors->highlightedItem()->data(Qt::UserRole).toInt(); };
    // VIEW
    QString viewServiceCompany(int);
    QString viewAllCustomers();
    QString viewCustomer(const QString &);
    QString viewCircuit(const QString &, const QString &);
    QString viewInspection(const QString &, const QString &, const QString &);
    QString viewTable(const QString &, const QString &, const QString &, int);
    QString viewAllRepairs(const QString &, int);
    QString viewAllInspectors(const QString &);
    QString viewLeakagesByApplication();
    QString viewAgenda();
    QStringList listWarnings(Warnings &, const QString &, const QString &, StringVariantMap &, StringVariantMap &);
    QStringList listDelayedWarnings(Warnings &, const QString &, const QString &, StringVariantMap &, const QString &, const QString &, int * = NULL);
    void writeTableVarCell(MTTextStream &, const QString &, const QString &, const QString &, const QString &, bool, int, double);
    void writeCustomersTable(MTTextStream &, const QString & = QString());
    void writeCircuitsTable(MTTextStream &, const QString &, const QString & = QString());

    QSet<int> years_expanded_in_service_company_view;
    QString selected_repair;
    MTDictionary dict_dbtables;
    MTDictionary dict_vartypes;
    MTDictionary dict_varnames;
    MTDictionary dict_attrvalues;
    MTDictionary dict_attrnames;
    MTDictionary dict_html;
    QMap<QString, QAction *> view_actions;
    QStringList views_list;
    QActionGroup * actgrp_view;
    QAction * actionShow_icons_only;
    QToolButton * tbtn_open;
    QToolButton * tbtn_view;
    QToolButton * tbtn_add;
    QToolButton * tbtn_modify;
    QToolButton * tbtn_export;
    QMenu * menu_view;
    QMenu * menu_add;
    QMenu * menu_modify;
    QString last_search_keyword;
    QSqlDatabase db;
    QComboBox * cb_lang;
    QMap<QString, QString> leaklog_i18n;
    QHttp * http; QBuffer * http_buffer;
};
