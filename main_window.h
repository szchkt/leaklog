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
#include "i18n.h"

#include <QCloseEvent>
#include <QSettings>
#include <QTranslator>
#include <QLocale>
#include <QFileDialog>
#include <QInputDialog>
#include <QTextStream>
#include <QBuffer>
#include <QXmlQuery>
#include <QXmlFormatter>
#include <QUrl>
#include <QPrintPreviewDialog>
#include <QPrintDialog>
#include <QPrinter>
#include <QHeaderView>
#include <QSqlDatabase>
#include <QSqlError>

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
    // DOCUMENT
    void openRecent(QListWidgetItem *);
    void newDocument();
    void open();
    void save();
    void saveAs();
    void closeDocument(bool = true);
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
    void addTableVariable();
    void removeTableVariable();
    void addWarning();
    void modifyWarning();
    void removeWarning();
    void exportCustomerData();
    void exportCircuitData();
    void exportInspectionData();
    void importData();
    // VIEW
    void viewChanged(const QString &);

private:
    // UI
    void setView(const QString &);
    void addRecent(QString);
    void clearAll();
    void setAllEnabled(bool);
    void closeEvent(QCloseEvent *);
    void loadSettings();
    void saveSettings();
    // DOCUMENT
    bool saveChangesBeforeProceeding(QString, bool);
    void openDocument(QString);
    void saveDocument(QString);
    void loadCustomer(QListWidgetItem *, bool);
    QDomElement selectedCustomerElement(QStringList * = NULL);
    void loadCircuit(QListWidgetItem *, bool);
    QDomElement selectedCircuitElement(QStringList * = NULL);
    void loadInspection(QListWidgetItem *, bool);
    QDomElement selectedInspectionElement();
    QDomElement selectedInspectionElement(QStringList *, bool &);
    void addVariable(bool);
    QDomElement selectedVariableElement(QStringList * = NULL);
    QDomElement selectedTableElement(QStringList * = NULL);
    QDomElement selectedWarningElement();
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
    QString expressionToHtml(QSqlQuery &, const MTDictionary &, const QString &, const QString &, const QString &);

    MTDictionary dict_vartypes;
    MTDictionary dict_queries;
    MTDictionary dict_html;
    i18n dict_i18n;
    QString dict_i18n_javascript;
    QMap<QString, int> view_indices;
    QActionGroup * actgrp_view;
    QString last_search_keyword;
    QSqlDatabase db;
    QDomDocument document;
    bool document_open;
    QString document_path;
    QString leaklog_version; float f_leaklog_version;

    friend class ModifyDialogue;
    friend class ModifyWarningDialogue;
};
