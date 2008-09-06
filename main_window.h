/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008 Matus & Michal Tomlein

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
#include "fparser/fparser.hh"

#include <QCloseEvent>
#include <QSettings>
#include <QTranslator>
#include <QLocale>
#include <QFileDialog>
#include <QInputDialog>
#include <QTextStream>
#include <QBuffer>
#include <QUrl>
#include <QPrintPreviewDialog>
#include <QPrintDialog>
#include <QPrinter>
#include <QHeaderView>
#include <QSqlDatabase>
#include <QSqlError>

#include <cmath>

class MainWindow : public QMainWindow, private Ui::MainWindow
{
    Q_OBJECT

public:
    MainWindow();

private slots:
    // UI
    void about();
    void executeLink(const QUrl &);
    void printPreview();
    void print();
    void enableTools();
    void find();
    void findNext();
    void findPrevious();
    void setView(QAction *);
    void refreshView();
    // DATABASE
    void openRecent(QListWidgetItem *);
    void initDatabase(QSqlDatabase *);
    void newDatabase();
    void open();
    void save();
    void saveAs();
    void closeDatabase(bool = true);
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
    void copyTable(const QString &, QSqlDatabase *, QSqlDatabase *, const QString & = QString());
    void exportCustomerData();
    void exportCircuitData();
    void exportInspectionData();
    void importData();
    // VIEW
    void viewChanged(const QString &);

private:
    // UI
    inline QString upArrow() { return QApplication::translate("MainWindow", "\342\206\221", 0, QApplication::UnicodeUTF8); };
    inline QString downArrow() { return QApplication::translate("MainWindow", "\342\206\223", 0, QApplication::UnicodeUTF8); };
    void setView(const QString &);
    void addRecent(QString);
    void clearAll();
    void setAllEnabled(bool);
    void closeEvent(QCloseEvent *);
    void loadSettings();
    void saveSettings();
    // DATABASE
    bool saveChangesBeforeProceeding(QString, bool);
    void openDatabase(QString);
    void saveDatabase(QString);
    void loadCustomer(QListWidgetItem *, bool);
    void loadCircuit(QListWidgetItem *, bool);
    void loadInspection(QListWidgetItem *, bool);
    void addVariable(bool);
    void moveTableVariable(bool);
    void exportData(const QString &);
    QStringList listVariableIds(bool = false);
    MTDictionary parseExpression(const QString &, QStringList *);
    inline int selectedCustomer() { return lw_customers->highlightedRow() < 0 ? -1 : lw_customers->highlightedItem()->data(Qt::UserRole).toInt(); };
    inline int selectedCircuit() { return lw_circuits->highlightedRow() < 0 ? -1 : lw_circuits->highlightedItem()->data(Qt::UserRole).toInt(); };
    inline QString selectedInspection() { return lw_inspections->highlightedRow() < 0 ? QString() : lw_inspections->highlightedItem()->data(Qt::UserRole).toString(); };
    // VIEW
    void viewAllCustomers();
    void viewCustomer(const QString &);
    void viewCircuit(const QString &, const QString &);
    void viewInspection(const QString &, const QString &, const QString &);
    void viewTable(const QString &, const QString &, const QString &, int);
    double evaluateExpression(QMap<QString, QVariant> &, const MTDictionary &, const QString &, const QString &, bool * = NULL);
    QString compareValues(double, double);
    QStringList listWarnings(QMap<QString, QVariant> &, QMap<QString, QVariant> &, const QString &, const QString &, QStringList &);
    void writeTableVarCell(QTextStream &, const QString &, const QString &, const QString &, bool, int);

    MTDictionary dict_vartypes;
    MTDictionary dict_varnames;
    MTDictionary dict_html;
    QMap<QString, int> view_indices;
    QActionGroup * actgrp_view;
    QString last_search_keyword;
    QSqlDatabase db;
    QString leaklog_version; float f_leaklog_version;

    friend class ModifyDialogue;
    friend class ModifyWarningDialogue;
};
