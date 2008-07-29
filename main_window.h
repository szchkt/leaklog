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
#include "modify_dialogue.h"
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
    // DOCUMENT
    void openRecent(QListWidgetItem *);
    void newDocument();
    void open();
    void save();
    void saveAs();
    void closeDocument();
    void viewChanged(const QString &);
    void addCustomer();
    void modifyCustomer();
    void removeCustomer();
    void loadCustomer(QListWidgetItem *);
    void addCircuit();
    void modifyCircuit();
    void removeCircuit();
    void loadCircuit(const QString &);
    void addInspection();
    void modifyInspection();
    void loadInspection(const QString &);
    void addVariable();
    void addSubvariable();
    void modifyVariable();

private:
    // UI
    void addRecent(QString);
    void clearAll();
    void setAllEnabled(bool);
    void enableTools();
    void closeEvent(QCloseEvent *);
    void loadSettings();
    void saveSettings();
    // DOCUMENT
    bool saveChangesBeforeProceeding(QString, bool);
    void openDocument(QString);
    void saveDocument(QString);
    void loadCustomer(QListWidgetItem *, bool);
    void loadCustomer(const QDomElement &, bool = true);
    QDomElement selectedCustomerElement(QStringList * = NULL);
    void loadCircuit(const QString &, bool);
    QDomElement selectedCircuitElement(QStringList * = NULL);
    void loadInspection(const QString &, bool);
    QDomElement selectedInspectionElement(QStringList * = NULL, bool & = false);
    void addVariable(bool);
    QDomElement selectedVariableElement(QStringList * = NULL);

    MTDictionary dict_vartypes;
    MTDictionary dict_queries;
    i18n dict_i18n;
    QString dict_i18n_javascript;
    QMap<QString, int> view_indices;

    QDomDocument document;
    bool document_open;
    QString document_path;
    QString leaklog_version; float f_leaklog_version;

    friend class ModifyDialogue;
};
