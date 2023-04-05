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

#include "mainwindow.h"
#include "global.h"
#include "expression.h"
#include "records.h"
#include "variables.h"
#include "warnings.h"
#include "mtvariant.h"
#include "mtaddress.h"
#include "aboutwidget.h"
#include "permissionsdialogue.h"
#include "sha256.h"
#include "syncengine.h"
#include "undostack.h"
#include "viewtab.h"

#include <QActionGroup>
#include <QSettings>
#include <QTranslator>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTextStream>
#include <QFile>
#include <QFileInfo>
#include <QFileDialog>
#include <QInputDialog>
#include <QPushButton>
#include <QCheckBox>
#include <QRadioButton>
#include <QDialogButtonBox>
#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QPrinter>
#include <QPainter>
#include <QMessageBox>
#include <QSqlRecord>
#include <QSqlError>
#include <QDate>
#include <QDateEdit>
#include <QCalendarWidget>
#include <QDesktopServices>

#include <cmath>

using namespace Global;

MainWindow::MainWindow():
    m_tab(NULL),
    m_current_scale(1.0),
    sync_engine(NULL)
{
    // i18n
    QTranslator translator; translator.load(":/i18n/Leaklog-i18n.qm");
    leaklog_i18n.insert("English", "English");
    leaklog_i18n.insert(translator.translate("LanguageNames", "Slovak"), "Slovak");
    leaklog_i18n.insert(translator.translate("LanguageNames", "Polish"), "Polish");
    leaklog_i18n.insert(translator.translate("LanguageNames", "Czech"), "Czech");

    // UI
    if (tr("LTR") == "RTL")
        qApp->setLayoutDirection(Qt::RightToLeft);
    setupUi(this);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    setUnifiedTitleAndToolBarOnMac(true);
#endif

    scaleFactorChanged();

    network_access_manager = new QNetworkAccessManager(this);
    authenticator = new Authenticator(this);
    QObject::connect(authenticator, SIGNAL(loginFinished(bool)), this, SLOT(loginFinished(bool)));
    QObject::connect(authenticator, SIGNAL(logoutFinished()), this, SLOT(logoutFinished()));

    loginFinished(!authenticator->token().isEmpty());

    // Dock widgets
    dw_variables->setVisible(false);
    dw_tables->setVisible(false);
    dw_warnings->setVisible(false);
    dw_styles->setVisible(false);

    // Menubar
    actgrp_date_format = new QActionGroup(this);
    actgrp_date_format->addAction(actionDate_yyyyMMdd);
    dict_action_date_format.insert(actionDate_yyyyMMdd, MainWindowSettings::yyyyMMdd);
    actgrp_date_format->addAction(actionDate_ddMMyyyy);
    dict_action_date_format.insert(actionDate_ddMMyyyy, MainWindowSettings::ddMMyyyy);
    actgrp_date_format->addAction(actionDate_dMyyyy);
    dict_action_date_format.insert(actionDate_dMyyyy, MainWindowSettings::dMyyyy);
    actgrp_date_format->addAction(actionDate_ddMMyy);
    dict_action_date_format.insert(actionDate_ddMMyy, MainWindowSettings::ddMMyy);
    actgrp_date_format->addAction(actionDate_dMyy);
    dict_action_date_format.insert(actionDate_dMyy, MainWindowSettings::dMyy);
    actgrp_date_format->addAction(actionDate_dMMMyyyy);
    dict_action_date_format.insert(actionDate_dMMMyyyy, MainWindowSettings::dMMMyyyy);
    actgrp_date_format->addAction(actionDate_dMMMyy);
    dict_action_date_format.insert(actionDate_dMMMyy, MainWindowSettings::dMMMyy);

    actgrp_time_format = new QActionGroup(this);
    actgrp_time_format->addAction(actionTime_hhmm);
    dict_action_time_format.insert(actionTime_hhmm, MainWindowSettings::hhmm);
    actgrp_time_format->addAction(actionTime_hmm);
    dict_action_time_format.insert(actionTime_hmm, MainWindowSettings::hmm);

#ifdef Q_OS_MAC
    // Tab bar

    tabw_main->setDocumentMode(false);
    tabw_main->setStyleSheet(R"(
        QTabWidget::pane { border-top: 1px solid #ACACAC; }
        QTabWidget::tab-bar { alignment: left; }
        QTabBar::tab {
            background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #BDBDBD, stop: 1 #B4B4B4);
            border-right: 1px solid #ACACAC; padding: 3px 10px;
        }
        QTabBar::tab:!active { background-color: #E7E7E7; border-right: 1px solid #DBDBDB; }
        QTabBar::tab:selected { background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #D9D9D9, stop: 1 #D0D0D0); }
        QTabBar::tab:!active:selected { background-color: #F4F4F4; }
        QTabBar::tab:!selected:hover { background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #B4B4B4, stop: 1 #ABABAB); }
        QTabBar::tab:!selected:pressed { background-color: #9C9E9C; border-right: 1px solid #7C7A7C; }
        QTabBar::tab:!active:!selected:hover { background-color: #DEDEDE; border-right: 1px solid #DBDBDB; }
        QTabBar::close-button { image: url(:/images/images/tab_close.svg); }
        QTabBar::close-button:hover { image: url(:/images/images/tab_close_hover.svg); }
    )");

    // Status bar

    statusbar->setStyleSheet(R"(
        QStatusBar {
            background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #ECECEC, stop: 1 #D5D5D5);
            border-top: 1px solid #ACACAC;
        }
    )");
#endif

    progress_bar = new QProgressBar(statusbar);
    progress_bar->setRange(0, 0);
    progress_bar->setVisible(false);
    progress_bar->setMaximumSize(150, 16);
    statusbar->addPermanentWidget(progress_bar);
    statusbar->setMaximumHeight(20);

    // Toolbar
    tbtn_open = new QToolButton(this);
    tbtn_open->setDefaultAction(actionOpen);
    tbtn_open->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    tbtn_open->setPopupMode(QToolButton::InstantPopup);
    toolBar->insertWidget(actionSave, tbtn_open);
    actionOpen->setMenu(menuOpen);

    tbtn_undo = new QToolButton(this);
    tbtn_undo->setDefaultAction(actionUndo);
    tbtn_undo->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    tbtn_undo->setPopupMode(QToolButton::InstantPopup);
    tbtn_undo_action = toolBar->insertWidget(toolBar->insertSeparator(actionReporting), tbtn_undo);

    m_undo_stack = new UndoStack(actionUndo, this);
    QObject::connect(m_undo_stack, SIGNAL(undoTriggered()), this, SLOT(loadDatabase()));

    tbtn_export = new QToolButton(this);
    tbtn_export->setDefaultAction(actionExport);
    tbtn_export->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    tbtn_export->setPopupMode(QToolButton::InstantPopup);
    toolBar->insertWidget(actionPrint, tbtn_export);
    actionExport->setMenu(menuExport);

    QWidget *spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    toolBar->insertWidget(actionLock, spacer);

#ifdef Q_OS_MAC
    QString search_style = "QLineEdit { border-bottom: 1px solid #BABABA; border-radius: 4px; background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #FFFFFF, stop: 1 #F0F0F0); }";
#else
    QString search_style = "QLineEdit { border: 1px solid #9F9F9F; border-radius: 10px; }";
#endif
    le_search = new SearchLineEdit(this, true, search_style);
    le_search->setMaximumWidth(200);
    toolBar->insertWidget(actionLock, le_search);

    QMenu *menuAdd_variable = new QMenu(this);
    menuAdd_variable->addAction(actionNew_variable);
    menuAdd_variable->addAction(actionNew_subvariable);
    tbtn_add_variable->setMenu(menuAdd_variable);

    actionShow_icons_only = new QAction(tr("Show icons only"), this);
    actionShow_icons_only->setCheckable(true);

    // Variables
    trw_variables->header()->setSortIndicator(0, Qt::AscendingOrder);
    trw_variables->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    trw_variables->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    trw_variables->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    trw_variables->header()->setSectionResizeMode(3, QHeaderView::ResizeToContents);

    // Tables
    trw_table_variables->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    trw_table_variables->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    trw_table_variables->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);

    lw_recent_docs->setContextMenuPolicy(Qt::CustomContextMenu);

    setAllEnabled(false);
    setDatabaseModified(false);

    QObject::connect(actionShow_icons_only, SIGNAL(toggled(bool)), this, SLOT(showIconsOnly(bool)));
    QObject::connect(actionAbout_Leaklog, SIGNAL(triggered()), this, SLOT(about()));
    QObject::connect(actionChangelog, SIGNAL(triggered()), this, SLOT(openReleaseNotes()));
    QObject::connect(actionDocumentation, SIGNAL(triggered()), this, SLOT(openDocumentation()));
    QObject::connect(actionNew, SIGNAL(triggered()), this, SLOT(newDatabase()));
    QObject::connect(actionLocal_database, SIGNAL(triggered()), this, SLOT(open()));
    QObject::connect(actionRemote_database, SIGNAL(triggered()), this, SLOT(openRemote()));
    QObject::connect(actionServer_database, SIGNAL(triggered()), this, SLOT(downloadDatabase()));
    QObject::connect(actionSave, SIGNAL(triggered()), this, SLOT(save()));
    QObject::connect(actionSave_and_compact, SIGNAL(triggered()), this, SLOT(saveAndCompact()));
    QObject::connect(actionClose, SIGNAL(triggered()), this, SLOT(closeDatabase()));
    QObject::connect(actionNew_Tab, SIGNAL(triggered()), this, SLOT(newTab()));
    QObject::connect(actionClose_Tab, SIGNAL(triggered()), this, SLOT(closeTab()));
    QObject::connect(tabw_main, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));
    QObject::connect(tabw_main, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));
    QObject::connect(actionPrint_preview, SIGNAL(triggered()), this, SLOT(printPreview()));
    QObject::connect(actionPrint, SIGNAL(triggered()), this, SLOT(print()));
    QObject::connect(actionPDF_Portrait, SIGNAL(triggered()), this, SLOT(exportPDFPortrait()));
    QObject::connect(actionPDF_Landscape, SIGNAL(triggered()), this, SLOT(exportPDFLandscape()));
    QObject::connect(actionHTML, SIGNAL(triggered()), this, SLOT(exportHTML()));
    QObject::connect(le_search, SIGNAL(textChanged(QString)), this, SLOT(find()));
    QObject::connect(actionFind, SIGNAL(triggered()), le_search, SLOT(setFocus()));
    QObject::connect(actionFind, SIGNAL(triggered()), le_search, SLOT(selectAll()));
    QObject::connect(actionFind_next, SIGNAL(triggered()), this, SLOT(findNext()));
    QObject::connect(actionFind_previous, SIGNAL(triggered()), this, SLOT(findPrevious()));
    QObject::connect(actionChange_language, SIGNAL(triggered()), this, SLOT(changeLanguage()));
    QObject::connect(actionLog_In, SIGNAL(triggered()), this, SLOT(logIn()));
    QObject::connect(actionReporting, SIGNAL(triggered(bool)), this, SLOT(reportData(bool)));
    QObject::connect(&m_settings, SIGNAL(serviceCompanyInformationVisibilityChanged()), this, SLOT(serviceCompanyInformationVisibilityChanged()));
    QObject::connect(actionPrint_Service_Company_Information, SIGNAL(triggered(bool)), &m_settings, SLOT(setServiceCompanyInformationPrinted(bool)));
    QObject::connect(actionShow_Service_Company_Information, SIGNAL(triggered(bool)), &m_settings, SLOT(setServiceCompanyInformationVisible(bool)));
    QObject::connect(&m_settings, SIGNAL(dateFormatChanged(MainWindowSettings::DateFormat)), this, SLOT(dateFormatChanged(MainWindowSettings::DateFormat)));
    QObject::connect(actgrp_date_format, SIGNAL(triggered(QAction *)), this, SLOT(dateFormatChanged(QAction *)));
    QObject::connect(&m_settings, SIGNAL(timeFormatChanged(MainWindowSettings::TimeFormat)), this, SLOT(timeFormatChanged(MainWindowSettings::TimeFormat)));
    QObject::connect(actgrp_time_format, SIGNAL(triggered(QAction *)), this, SLOT(timeFormatChanged(QAction *)));
    QObject::connect(actionPrinter_friendly_version, SIGNAL(triggered()), this, SLOT(refreshView()));
    QObject::connect(actionCompare_values, SIGNAL(triggered()), this, SLOT(refreshView()));
    QObject::connect(actionShow_date_updated, SIGNAL(triggered()), this, SLOT(refreshView()));
    QObject::connect(actionShow_owner, SIGNAL(triggered()), this, SLOT(refreshView()));
    QObject::connect(actionShow_Notes, SIGNAL(triggered()), this, SLOT(refreshView()));
    QObject::connect(actionShow_Leaked, SIGNAL(triggered()), this, SLOT(refreshView()));
    QObject::connect(actionShow_Place_of_operation, SIGNAL(triggered()), this, SLOT(refreshView()));
    QObject::connect(actionShow_Building, SIGNAL(triggered()), this, SLOT(refreshView()));
    QObject::connect(actionShow_Device, SIGNAL(triggered()), this, SLOT(refreshView()));
    QObject::connect(actionShow_Manufacturer, SIGNAL(triggered()), this, SLOT(refreshView()));
    QObject::connect(actionShow_Type, SIGNAL(triggered()), this, SLOT(refreshView()));
    QObject::connect(actionShow_Serial_number, SIGNAL(triggered()), this, SLOT(refreshView()));
    QObject::connect(actionShow_Year_of_purchase, SIGNAL(triggered()), this, SLOT(refreshView()));
    QObject::connect(actionShow_Date_of_commissioning, SIGNAL(triggered()), this, SLOT(refreshView()));
    QObject::connect(actionShow_Field_of_application, SIGNAL(triggered()), this, SLOT(refreshView()));
    QObject::connect(actionShow_Oil, SIGNAL(triggered()), this, SLOT(refreshView()));
    QObject::connect(actionMost_recent_first, SIGNAL(triggered()), this, SLOT(refreshView()));
    QObject::connect(actionSync, SIGNAL(triggered()), this, SLOT(sync()));
    QObject::connect(actionLock, SIGNAL(triggered()), this, SLOT(toggleLocked()));
    QObject::connect(actionConfigure_permissions, SIGNAL(triggered()), this, SLOT(configurePermissions()));
    QObject::connect(actionAutosave, SIGNAL(triggered()), this, SLOT(configureAutosave()));
    QObject::connect(actionOpen_Backup_Folder, SIGNAL(triggered()), this, SLOT(openBackupDirectory()));
    QObject::connect(actionEdit_Refrigerants, SIGNAL(triggered()), this, SLOT(editRefrigerants()));
    QObject::connect(actionAdd_Service_Company, SIGNAL(triggered()), this, SLOT(addServiceCompany()));
    QObject::connect(actionEdit_Service_Company, SIGNAL(triggered()), this, SLOT(editServiceCompany()));
    QObject::connect(actionRemove_Service_Company, SIGNAL(triggered()), this, SLOT(removeServiceCompany()));
    QObject::connect(actionAdd_record_of_refrigerant_management, SIGNAL(triggered()), this, SLOT(addRefrigerantRecord()));
    QObject::connect(actionAdd_customer, SIGNAL(triggered()), this, SLOT(addCustomer()));
    QObject::connect(actionEdit_customer, SIGNAL(triggered()), this, SLOT(editCustomer()));
    QObject::connect(actionDuplicate_customer, SIGNAL(triggered()), this, SLOT(duplicateCustomer()));
    QObject::connect(actionRemove_customer, SIGNAL(triggered()), this, SLOT(removeCustomer()));
    QObject::connect(actionDecommission_all_circuits, SIGNAL(triggered()), this, SLOT(decommissionAllCircuits()));
    QObject::connect(actionAdd_circuit, SIGNAL(triggered()), this, SLOT(addCircuit()));
    QObject::connect(actionEdit_circuit, SIGNAL(triggered()), this, SLOT(editCircuit()));
    QObject::connect(actionDuplicate_circuit, SIGNAL(triggered()), this, SLOT(duplicateCircuit()));
    QObject::connect(actionDuplicate_and_decommission_circuit, SIGNAL(triggered()), this, SLOT(duplicateAndDecommissionCircuit()));
    QObject::connect(actionMove_Circuit_to_Another_Customer, SIGNAL(triggered()), this, SLOT(moveCircuit()));
    QObject::connect(actionRemove_circuit, SIGNAL(triggered()), this, SLOT(removeCircuit()));
    QObject::connect(actionAdd_inspection, SIGNAL(triggered()), this, SLOT(addInspection()));
    QObject::connect(actionEdit_inspection, SIGNAL(triggered()), this, SLOT(editInspection()));
    QObject::connect(actionDuplicate_inspection, SIGNAL(triggered()), this, SLOT(duplicateInspection()));
    QObject::connect(actionRemove_inspection, SIGNAL(triggered()), this, SLOT(removeInspection()));
    QObject::connect(actionSkip_Inspection, SIGNAL(triggered()), this, SLOT(skipInspection()));
    QObject::connect(actionAdd_repair, SIGNAL(triggered()), this, SLOT(addRepair()));
    QObject::connect(actionEdit_repair, SIGNAL(triggered()), this, SLOT(editRepair()));
    QObject::connect(actionDuplicate_repair, SIGNAL(triggered()), this, SLOT(duplicateRepair()));
    QObject::connect(actionRemove_repair, SIGNAL(triggered()), this, SLOT(removeRepair()));
    QObject::connect(actionPrint_detailed_label, SIGNAL(triggered()), this, SLOT(printDetailedLabel()));
    QObject::connect(actionPrint_label, SIGNAL(triggered()), this, SLOT(printLabel()));
    QObject::connect(actionNew_variable, SIGNAL(triggered()), this, SLOT(addVariable()));
    QObject::connect(actionNew_subvariable, SIGNAL(triggered()), this, SLOT(addSubvariable()));
    QObject::connect(tbtn_edit_variable, SIGNAL(clicked()), this, SLOT(editVariable()));
    QObject::connect(tbtn_remove_variable, SIGNAL(clicked()), this, SLOT(removeVariable()));
    QObject::connect(tbtn_add_table, SIGNAL(clicked()), this, SLOT(addTable()));
    QObject::connect(tbtn_edit_table, SIGNAL(clicked()), this, SLOT(editTable()));
    QObject::connect(tbtn_remove_table, SIGNAL(clicked()), this, SLOT(removeTable()));
    QObject::connect(tbtn_table_add_variable, SIGNAL(clicked()), this, SLOT(addTableVariable()));
    QObject::connect(tbtn_table_remove_variable, SIGNAL(clicked()), this, SLOT(removeTableVariable()));
    QObject::connect(tbtn_table_move_up, SIGNAL(clicked()), this, SLOT(moveTableVariableUp()));
    QObject::connect(tbtn_table_move_down, SIGNAL(clicked()), this, SLOT(moveTableVariableDown()));
    QObject::connect(tbtn_add_warning, SIGNAL(clicked()), this, SLOT(addWarning()));
    QObject::connect(tbtn_edit_warning, SIGNAL(clicked()), this, SLOT(editWarning()));
    QObject::connect(tbtn_remove_warning, SIGNAL(clicked()), this, SLOT(removeWarning()));
    QObject::connect(actionAdd_inspector, SIGNAL(triggered()), this, SLOT(addInspector()));
    QObject::connect(actionEdit_inspector, SIGNAL(triggered()), this, SLOT(editInspector()));
    QObject::connect(actionRemove_inspector, SIGNAL(triggered()), this, SLOT(removeInspector()));
    QObject::connect(actionImport_data, SIGNAL(triggered()), this, SLOT(importData()));
    QObject::connect(actionImport_CSV, SIGNAL(triggered()), this, SLOT(importCSV()));
    QObject::connect(actionCheck_for_updates, SIGNAL(triggered()), this, SLOT(checkForUpdates()));
    QObject::connect(lw_recent_docs, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(openRecent(QListWidgetItem *)));
    QObject::connect(lw_recent_docs, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(showRecentDatabaseContextMenu(const QPoint &)));
    QObject::connect(trw_variables, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(editVariable()));
    QObject::connect(trw_variables, SIGNAL(itemSelectionChanged()), this, SLOT(enableTools()));
    QObject::connect(cb_table_edit, SIGNAL(currentTextChanged(const QString &)), this, SLOT(loadTable(const QString &)));
    QObject::connect(trw_table_variables, SIGNAL(itemSelectionChanged()), this, SLOT(enableTools()));
    QObject::connect(lw_warnings, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(editWarning()));
    QObject::connect(lw_warnings, SIGNAL(itemSelectionChanged()), this, SLOT(enableTools()));
    QObject::connect(network_access_manager, SIGNAL(finished(QNetworkReply *)), this, SLOT(httpRequestFinished(QNetworkReply *)));
    QObject::connect(tbtn_add_style, SIGNAL(clicked()), this, SLOT(addStyle()));
    QObject::connect(tbtn_edit_style, SIGNAL(clicked()), this, SLOT(editStyle()));
    QObject::connect(tbtn_remove_style, SIGNAL(clicked()), this, SLOT(removeStyle()));

#ifdef Q_OS_MAC
    macInitUI();
    show();
#endif
    loadSettings();
    if (qApp->arguments().count() > 1) {
        openFile(qApp->arguments().at(1));
    }
#ifndef Q_OS_MAC
    if (!isVisible())
        show();
#endif
}

void MainWindow::scaleFactorChanged()
{
    double scale = scaleFactor(true);
    double diff = scale / m_current_scale;
    m_current_scale = scale;

    frame_welcome->setMaximumSize(frame_welcome->maximumSize() * diff);
    frame_welcome->setMinimumSize(frame_welcome->minimumSize() * diff);

    for (int i = 0; i < tabw_main->count(); ++i) {
        ViewTab *tab = qobject_cast<ViewTab *>(tabw_main->widget(i));
        tab->scaleFactorChanged();
    }
}

bool MainWindow::hasActiveModalWidget()
{
    QList<QDialog *> list = findChildren<QDialog *>();

    foreach (QDialog *d, list) {
        if (d->isModal() && d->isVisible())
            return true;
    }

    return false;
}

void MainWindow::clearWindowTitle()
{
#ifdef Q_OS_MAC
    setWindowFilePath(QString());
#endif
    setWindowTitle("Leaklog");
}

void MainWindow::setWindowTitleWithRepresentedFilename(const QString &path)
{
#ifdef Q_OS_MAC
    setWindowFilePath(path.startsWith('/') ? path : QString());
    setWindowTitle(QString("%1[*]").arg(QFileInfo(path).completeBaseName()));
#else
    setWindowTitle(QString("%1[*] - Leaklog").arg(QFileInfo(path).completeBaseName()));
#endif
}

void MainWindow::openFile(const QString &file)
{
    QFileInfo file_info(file);
    if (file_info.exists() && !saveChangesBeforeProceeding(tr("Open database - Leaklog"), true)) {
        addRecent(file_info.absoluteFilePath());
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName(file_info.absoluteFilePath());
        openDatabase(db, file_info.absoluteFilePath());
    }
}

QMenu *MainWindow::createPopupMenu()
{
    QMenu *popup_menu = this->QMainWindow::createPopupMenu();
    popup_menu->addSeparator();
    popup_menu->addAction(actionShow_icons_only);
    return popup_menu;
}

void MainWindow::showRecentDatabaseContextMenu(const QPoint &pos)
{
    QListWidgetItem *item = lw_recent_docs->itemAt(pos);
    if (!item) return;
    QAction show(tr("Open containing folder"), this);
    show.setStatusTip(tr("Open the folder which contains this database"));
    QAction separator(this); separator.setSeparator(true);
    QAction remove(tr("Remove from list"), this);
    remove.setStatusTip(tr("Remove the database from the list (the database will not be deleted)"));
    QList<QAction *> actions;
    if (!item->text().startsWith("db:"))
        actions << &show << &separator;
    actions << &remove;
    QAction *clicked = QMenu::exec(actions, lw_recent_docs->mapToGlobal(pos));
    if (clicked == &show) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(item->text()).absolutePath()));
    } else if (clicked == &remove) {
        delete lw_recent_docs->itemAt(pos);
    }
}

void MainWindow::showIconsOnly(bool show)
{
    Qt::ToolButtonStyle tbtn_style = show ? Qt::ToolButtonIconOnly : Qt::ToolButtonTextUnderIcon;
    tbtn_open->setToolButtonStyle(tbtn_style);
    tbtn_export->setToolButtonStyle(tbtn_style);
    tbtn_undo->setToolButtonStyle(tbtn_style);
    toolBar->setToolButtonStyle(tbtn_style);
}

void MainWindow::printPreview()
{
    QPrintPreviewDialog d(this);
    QObject::connect(&d, SIGNAL(paintRequested(QPrinter *)), this, SLOT(printPreview(QPrinter *)));
    d.exec();
}

void MainWindow::printPreview(QPrinter *printer)
{
    bool printing = true;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    m_tab->webView()->page()->print(printer, [&printing](bool) {
#else
    m_tab->webView()->print(printer);
    QObject *context = new QObject(this);
    connect(m_tab->webView(), &QWebEngineView::printFinished, context, [context, &printing](bool) {
        delete context;
#endif
        printing = false;
    });
    while (printing) {
        QApplication::processEvents();
    }
}

void MainWindow::print()
{
#ifdef Q_OS_WIN32
    // QPrinter::HighResolution crashes on Windows
    QPrinter *printer = new QPrinter;
#else
    QPrinter *printer = new QPrinter(QPrinter::HighResolution);
#endif
    QPrintDialog d(printer, this);
    d.setWindowTitle(tr("Print"));
    if (d.exec() != QDialog::Accepted) { return; }
    print(printer);
}

void MainWindow::print(QPrinter *printer)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    m_tab->webView()->page()->print(printer, [printer](bool) {
#else
    m_tab->webView()->print(printer);
    QObject *context = new QObject(this);
    connect(m_tab->webView(), &QWebEngineView::printFinished, context, [context, printer](bool) {
        delete context;
#endif
        delete printer;
    });
}

QString MainWindow::fileNameForCurrentView()
{
    QString title;
    if (m_tab->currentView() != View::ViewCount)
        title = m_tab->view(m_tab->currentView())->title();

    if (m_tab->currentView() == View::AssemblyRecordDetails)
        return title;

    return QString("%1 - %2")
            .arg(QFileInfo(QSqlDatabase::database().databaseName()).completeBaseName())
            .arg(title.replace(':', '.'));
}

void MainWindow::exportPDFPortrait()
{
    exportPDF(QPageLayout::Portrait);
}

void MainWindow::exportPDFLandscape()
{
    exportPDF(QPageLayout::Landscape);
}

void MainWindow::exportPDF(int orientation)
{
    QSettings settings("SZCHKT", "Leaklog");
    QDir dir(settings.value("file_dialog_dir", QDir::homePath()).toString());
    QString path = QFileDialog::getSaveFileName(this, tr("Export PDF - Leaklog"),
                                                dir.absoluteFilePath(QString("%1.pdf").arg(fileNameForCurrentView())),
                                                tr("Adobe PDF (*.pdf)"));
    if (path.isEmpty()) { return; }
    dir.setPath(path);
    dir.cdUp();
    settings.setValue("file_dialog_dir", dir.absolutePath());
    if (!path.endsWith(".pdf", Qt::CaseInsensitive)) { path.append(".pdf"); }
    m_tab->webView()->page()->printToPdf(path, QPageLayout(QPageSize(QPageSize::A4),
                                                           (QPageLayout::Orientation)orientation,
                                                           QMarginsF(10, 10, 10, 10),
                                                           QPageLayout::Millimeter));
}

void MainWindow::exportHTML()
{
    QSettings settings("SZCHKT", "Leaklog");
    QDir dir(settings.value("file_dialog_dir", QDir::homePath()).toString());
    QString path = QFileDialog::getSaveFileName(this, tr("Export HTML - Leaklog"),
                                                dir.absoluteFilePath(QString("%1.html").arg(fileNameForCurrentView())),
                                                tr("Webpage (*.html)"));
    if (path.isEmpty()) { return; }
    dir.setPath(path);
    dir.cdUp();
    settings.setValue("file_dialog_dir", dir.absolutePath());
    if (!path.endsWith(".html", Qt::CaseInsensitive)) { path.append(".html"); }
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, tr("Export HTML - Leaklog"), tr("Cannot write file %1:\n%2.").arg(path).arg(file.errorString()));
        return;
    }

    if (m_tab->currentView() == View::ViewCount)
        return;

    QString html = m_tab->view(m_tab->currentView())->renderHTML(true);
    if (html.contains("<link href=\"default.css\" rel=\"stylesheet\" type=\"text/css\" />")) {
        QFile default_css(":/html/default.css");
        default_css.open(QIODevice::ReadOnly | QIODevice::Text);
        QTextStream in(&default_css);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        in.setEncoding(QStringConverter::Utf8);
#else
        in.setCodec("UTF-8");
#endif
        html.replace("<link href=\"default.css\" rel=\"stylesheet\" type=\"text/css\" />", QString("<style type=\"text/css\">\n<!--\n%1\n-->\n</style>").arg(in.readAll()));
        default_css.close();
    }
    if (html.contains("<link href=\"colours.css\" rel=\"stylesheet\" type=\"text/css\" />")) {
        QFile colours_css(":/html/colours.css");
        colours_css.open(QIODevice::ReadOnly | QIODevice::Text);
        QTextStream in(&colours_css);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        in.setEncoding(QStringConverter::Utf8);
#else
        in.setCodec("UTF-8");
#endif
        html.replace("<link href=\"colours.css\" rel=\"stylesheet\" type=\"text/css\" />", QString("<style type=\"text/css\">\n<!--\n%1\n-->\n</style>").arg(in.readAll()));
        colours_css.close();
    }
    if (html.contains("<script type=\"text/javascript\" src=\"application.js\"></script>")) {
        QFile application_js(":/html/application.js");
        application_js.open(QIODevice::ReadOnly | QIODevice::Text);
        QTextStream in(&application_js);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        in.setEncoding(QStringConverter::Utf8);
#else
        in.setCodec("UTF-8");
#endif
        html.replace("<script type=\"text/javascript\" src=\"application.js\"></script>", QString("<script type=\"text/javascript\">\n<!--\n%1\n-->\n</script>").arg(in.readAll()));
        application_js.close();
    }
    QTextStream out(&file);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    out.setEncoding(QStringConverter::Utf8);
#else
    out.setCodec("UTF-8");
#endif
    out << html;
    file.close();
}

void MainWindow::printDetailedLabel()
{
    printLabel(true);
}

void MainWindow::printLabel(bool detailed)
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (detailed && !m_tab->isCustomerSelected()) { return; }
    if (detailed && !m_tab->isCircuitSelected()) { return; }
    if (!detailed && !m_tab->isInspectorSelected()) { return; }

    QMap<QString, QCheckBox *> label_positions;
    QDialog *d = new QDialog(this);
    d->setWindowTitle(detailed ? tr("Print detailed label - Leaklog") : tr("Print label - Leaklog"));

    QGridLayout *gl = new QGridLayout(d);

    gl->addWidget(new QLabel(tr("Choose the position of the label on the paper:"), d), 0, 0, 1, 2);

    for (int c = 0; c < 2; ++c) {
        for (int r = 0; r < 4; ++r) {
            QCheckBox *chb = new QCheckBox(d);
            chb->setText(tr("Row %1 Column %2").arg(r + 1).arg(c + 1));
            label_positions.insert(QString("%1;%2").arg(r).arg(c), chb);
            gl->addWidget(chb, r + 1, c);
        }
    }

    QCheckBox *chb_CO2_equivalent = NULL;
    if (detailed) {
        chb_CO2_equivalent = new QCheckBox(d);
        chb_CO2_equivalent->setText(QApplication::translate("ToolBarStack", "Convert refrigerant to CO\342\202\202 equivalent"));
        chb_CO2_equivalent->setChecked(true);
        gl->addWidget(chb_CO2_equivalent, 5, 0, 1, 2);
    }

    QDialogButtonBox *bb = new QDialogButtonBox(d);
    bb->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QObject::connect(bb, SIGNAL(accepted()), d, SLOT(accept()));
    QObject::connect(bb, SIGNAL(rejected()), d, SLOT(reject()));
    gl->addWidget(bb, 6, 0, 1, 2);

    if (d->exec() != QDialog::Accepted) { return; }
    bool ok = false;
    QMapIterator<QString, QCheckBox *> iterator(label_positions);
    while (iterator.hasNext()) { iterator.next();
        if (iterator.value()->isChecked()) { ok = true; break; }
    }
    if (!ok) { return; }

    QString selected_inspector_uuid = m_tab->selectedInspectorUUID();
    QVariantMap attributes;
    if (detailed) {
        Customer customer(m_tab->selectedCustomerUUID());
        Circuit circuit(m_tab->selectedCircuitUUID());
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
        attributes.insert(
#else
        attributes.unite(
#endif
            circuit.list("id, refrigerant, " + circuitRefrigerantAmountQuery()
                         + ", hermetic, leak_detector, inspection_interval")
        );
        attributes.insert("circuit_id", customer.companyID() + "." + attributes.value("id").toString().rightJustified(5, '0'));

        MTSqlQuery query;
        query.prepare("SELECT * FROM inspections WHERE circuit_uuid = :circuit_uuid"
                      " AND inspection_type <> 1 AND outside_interval = 0 ORDER BY date DESC");
        query.bindValue(":circuit_uuid", m_tab->selectedCircuitUUID());
        query.exec();
        QSqlRecord record = query.record();
        if (query.next()) {
            QVariantMap inspection;
            for (int i = 0; i < record.count(); ++i)
                inspection.insert(record.fieldName(i), query.value(i));

            attributes.insert("date", inspection.value("date").toString());

            int inspection_interval = Warnings::circuitInspectionInterval(attributes.value("refrigerant").toString(),
                                                                          attributes.value("refrigerant_amount").toDouble(),
                                                                          chb_CO2_equivalent && chb_CO2_equivalent->isChecked(),
                                                                          attributes.value("hermetic").toInt(),
                                                                          attributes.value("leak_detector").toInt(),
                                                                          attributes.value("inspection_interval").toInt());
            if (inspection_interval)
                attributes.insert("next_inspection",
                                  QDate::fromString(inspection.value("date").toString().split("-").first(), DATE_FORMAT)
                                  .addDays(inspection_interval).toString(DATE_FORMAT));

            selected_inspector_uuid = inspection.value("inspector_uuid").toString();

            Variable refr_add_per("refr_add_per");
            refr_add_per.next();
            QString unparsed_expression = refr_add_per.valueExpression();
            if (!unparsed_expression.isEmpty()) {
                attributes.insert("refr_add_per", Expression(unparsed_expression).evaluate(inspection, m_tab->selectedCustomerUUID(), m_tab->selectedCircuitUUID()));
            }
        }
    }

    Inspector inspector(selected_inspector_uuid);
    if (inspector.exists()) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
        attributes.insert(inspector.list("certificate_number, person"));
#else
        attributes.unite(inspector.list("certificate_number, person"));
#endif
    }

    ServiceCompany service_company(m_tab->selectedServiceCompanyUUID());
    if (service_company.exists()) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
        attributes.insert(service_company.list("id, name, address, mail, phone"));
#else
        attributes.unite(service_company.list("id, name, address, mail, phone"));
#endif
        attributes.insert("id", attributes.value("id").toString());
    }

    QApplication::processEvents();

    QPrinter printer(QPrinter::HighResolution);
    QPrintDialog dialogue(&printer, this);
    dialogue.setWindowTitle(tr("Print label"));
    if (dialogue.exec() != QDialog::Accepted) return;
    printer.setPageOrientation(QPageLayout::Portrait);
    printer.setFullPage(true);

    QPainter painter;
    painter.begin(&printer);
    painter.setRenderHint(QPainter::Antialiasing);
    QRect rect = printer.pageLayout().paintRectPixels(printer.resolution());
    int margin = rect.width() / 33;
    int w = (rect.width() - 4 * margin) / 2;
    int h = (rect.height() - 8 * margin) / 4;
    painter.translate(- margin, - margin);
    for (int c = 0; c < 2; ++c) {
        for (int r = 0; r < 4; ++r) {
            if (label_positions.value(QString("%1;%2").arg(r).arg(c))->isChecked()) {
                paintLabel(attributes, painter, (c + 1) * 2 * margin + c * w, (r + 1) * 2 * margin + r * h, w, h);
            }
        }
    }
    painter.end();
}

static void drawText(QPainter &painter, int x, int y, int w, int h, int flags, const QString &str)
{
    QStringList lines = str.split('\n');
    h /= lines.count();
    foreach (const QString &line, lines) {
        painter.drawText(x, y, w, h, flags, line);
        y += h;
    }
}

void MainWindow::paintLabel(const QVariantMap &attributes, QPainter &painter, int x, int y, int w, int h)
{
    bool detailed = attributes.contains("circuit_id");
    painter.save();
    QPen pen;
    QFont font;
#ifdef Q_OS_MAC
    pen.setWidthF(3.0);
    font.setPointSize(font.pointSize() - 7);
#else
    pen.setWidthF(7.0);
    font.setPointSize(font.pointSize() - 2);
#endif
    painter.setPen(pen);
    painter.setFont(font);
    int title_h = 3 * h / 14; int m = w / 75; int dm = m * 2;

    QString refrigerant = attributes.value("refrigerant").toString();

    painter.drawRect(x, y, w, h);
    painter.drawLine(x, y + title_h, x + w, y + title_h);
    painter.drawLine(x + (w / 2), y, x + (w / 2), y + title_h);
    painter.drawText(m + x, m + y, w / 2 - dm, (title_h - dm) / 3, Qt::AlignLeft | Qt::AlignVCenter, tr("Certified company"));
    painter.drawText(m + x, m + y + 2 * (title_h - dm) / 3, w / 2 - dm, (title_h - dm) / 3, Qt::AlignLeft | Qt::AlignVCenter, MTAddress(attributes.value("address").toString()).toPlainText());
    painter.drawText(m + x, m + y + title_h, w / 3 - dm, h / 14 - m, Qt::AlignCenter,
                     detailed ? tr("Circuit ID") : tr("3(6) - <30 kg"));
    painter.drawLine(x + (w / 3), y + title_h, x + (w / 3), y + h);
    painter.drawText(m + x + (w / 3), m + y + title_h, w / 3 - dm, h / 14 - m, Qt::AlignCenter,
                     detailed ? QString("%L1 %2 %3, %4 %L5")
                     .arg(FLOAT_ARG(attributes.value("refrigerant_amount").toDouble()))
                     .arg(QApplication::translate("Units", "kg"))
                     .arg(refrigerant).arg(tr("GWP")).arg(FLOAT_ARG(refrigerantGWP(refrigerant)))
                     : tr("30 - <300 kg"));
    painter.drawLine(x + (2 * w / 3), y + title_h, x + (2 * w / 3), y + h);
    painter.drawText(m + x + (2 * w / 3), m + y + title_h, w / 3 - dm, h / 14 - m, Qt::AlignCenter,
                     detailed ? QString("%1 %L2 %3").arg(tr("Annual leakage")).arg(FLOAT_ARG(attributes.value("refr_add_per").toDouble())).arg(tr("%")) : tr("above 300 kg"));
    painter.drawLine(x, y + title_h + (h / 7), x + w, y + title_h + (h / 7));

    painter.drawText(m + x, m + y + title_h + (h / 7), w / 3 - dm, 9 * h / 14 - dm, Qt::AlignLeft, tr("Date of inspection"));
    painter.drawText(m + x + (2 * w / 3), m + y + title_h + (h / 7), w / 3 - dm, 9 * h / 14 - dm, Qt::AlignLeft, tr("Date of the next inspection"));

    painter.drawLine(x + (w / 3), y + title_h + (2 * h / 7), x + (2 * w / 3), y + title_h + (2 * h / 7));
    painter.drawLine(x + (w / 3), y + title_h + (3 * h / 7), x + (2 * w / 3), y + title_h + (3 * h / 7));
    painter.drawLine(x + (w / 3), y + title_h + (4 * h / 7), x + (2 * w / 3), y + title_h + (4 * h / 7));
    painter.drawLine(x + (w / 3), y + title_h + (5 * h / 7), x + (2 * w / 3), y + title_h + (5 * h / 7));

    painter.drawText(m + x + (w / 3), y + title_h + (5 * h / 7), w / 6 - dm, h / 14, Qt::AlignCenter, attributes.value("certificate_number").toString());
    painter.drawLine(x + (w / 2), y + title_h + (5 * h / 7), x + (w / 2), y + h);
    painter.drawText(m + x + (w / 2), y + title_h + (5 * h / 7), w / 6 - dm, h / 14, Qt::AlignCenter, attributes.value("id").toString());

    font.setBold(true); painter.setFont(font);
    painter.drawText(m + x, m + y + (title_h - dm) / 3, w / 2 - dm, (title_h - dm) / 3, Qt::AlignLeft | Qt::AlignVCenter, attributes.value("name").toString());
    painter.drawText(m + x + (w / 3), y + title_h + (h / 7) + h / 14, w / 3 - dm, h / 14 - m, Qt::AlignCenter, attributes.value("person").toString());
    font.setBold(false); painter.setFont(font);

    drawText(painter, m + x + (w / 2), m + y, w / 2 - dm, title_h - dm, Qt::AlignCenter, tr("Refrigerant leakage inspection\nin accordance with Regulation (EC)\nNo. 842/2006"));

    painter.drawText(m + x, y + title_h + h / 14, w / 3 - dm, h / 14 - m, Qt::AlignCenter,
                     detailed ? attributes.value("circuit_id").toString()
                     : QApplication::translate("MainWindow", "once a year*"));
    painter.drawText(m + x + (w / 3), y + title_h + h / 14, w / 3 - dm, h / 14 - m, Qt::AlignCenter,
                     detailed ? QString("%L1 %2")
                     .arg(FLOAT_ARG(CO2Equivalent(refrigerant, attributes.value("refrigerant_amount").toDouble())))
                     .arg(QApplication::translate("Units", "t of CO\342\202\202 equivalent"))
                     : QApplication::translate("MainWindow", "once in 6 months*"));
    painter.drawText(m + x + (2 * w / 3), y + title_h + h / 14, w / 3 - dm, h / 14 - m, Qt::AlignCenter,
                     detailed ? (attributes.value("leak_detector").toInt() ? tr("Detector installed") : tr("No detector installed"))
                     : QApplication::translate("MainWindow", "once in 3 months"));

    painter.drawText(m + x + (w / 3), m + y + title_h + (h / 7), w / 3 - dm, h / 14 - m, Qt::AlignCenter, tr("Certified person"));
    painter.drawText(m + x + (w / 3), m + y + title_h + (2 * h / 7), w / 3 - dm, h / 14 - m, Qt::AlignCenter, tr("Telephone"));
    painter.drawText(m + x + (w / 3), y + title_h + (2 * h / 7) + h / 14, w / 3 - dm, h / 14 - m, Qt::AlignCenter, attributes.value("phone").toString());
    painter.drawText(m + x + (w / 3), m + y + title_h + (3 * h / 7), w / 3 - dm, h / 14 - m, Qt::AlignCenter, tr("E-mail"));
    painter.drawText(m + x + (w / 3), y + title_h + (3 * h / 7) + h / 14, w / 3 - dm, h / 14 - m, Qt::AlignCenter, attributes.value("mail").toString());
    drawText(painter, m + x + (w / 3), m + y + title_h + (4 * h / 7), w / 3 - dm, h / 7 - dm, Qt::AlignCenter, tr("Registry number of\nperson and company ID"));

    int r = (w / 3 - dm) / 2;
    int year = QDate::currentDate().year(), month = 0;
    int year_next = 0, month_next = 0;

    if (detailed && attributes.contains("date")) {
        year = attributes.value("date").toString().left(4).toInt();
        month = attributes.value("date").toString().mid(5, 2).toInt();
        if (attributes.contains("next_inspection")) {
            year_next = attributes.value("next_inspection").toString().left(4).toInt();
            month_next = attributes.value("next_inspection").toString().mid(5, 2).toInt();
        }
    }

    int c_y = y + title_h + (4 * h / 7) - dm;
    int c_x[] = { m + x, m + x + 2 * w / 3 };

    for (int i = 0; i < 2; ++i) {
        QRect circle_o(c_x[i], c_y - r, 2 * r, 2 * r);
        painter.drawEllipse(circle_o);
        QRect circle_i(c_x[i] + 2 * dm, c_y - r + 2 * dm, 2 * r - 4 * dm, 2 * r - 4 * dm);
        painter.drawEllipse(circle_i);

        if (!detailed) {
            painter.drawText(c_x[i] + 3 * dm, c_y - r + 3 * dm, r - 3 * dm, r - 3 * dm, Qt::AlignCenter, QString::number(year).right(2));
            painter.drawText(c_x[i] + r, c_y - r + 3 * dm, r - 3 * dm, r - 3 * dm, Qt::AlignCenter, QString::number(year + 1).right(2));
            painter.drawText(c_x[i] + 3 * dm, c_y, r - 3 * dm, r - 3 * dm, Qt::AlignCenter, QString::number(year + 2).right(2));
            painter.drawText(c_x[i] + r, c_y, r - 3 * dm, r - 3 * dm, Qt::AlignCenter, QString::number(year + 3).right(2));
        } else if (!i) {
            painter.drawText(c_x[i] + 3 * dm, c_y - r + 3 * dm, 2 * r - 6 * dm, 2 * r - 6 * dm, Qt::AlignCenter, QString::number(year));
        } else if (year_next) {
            painter.drawText(c_x[i] + 3 * dm, c_y - r + 3 * dm, 2 * r - 6 * dm, 2 * r - 6 * dm, Qt::AlignCenter, QString::number(year_next));
        }
    }

    for (int i = 0; i < 2; ++i) {
        painter.restore();
        painter.save();
        painter.setPen(pen);
        painter.setFont(font);
        painter.translate(c_x[i] + r, c_y);

        for (int j = 0; j < 12; ++j) {
            painter.drawLine(0, - r, 0, 2 * dm - r);
            painter.rotate(15.0);
            if ((!i && month == j + 1) || (i && month_next == j + 1)) {
                painter.save();
                painter.setPen(QPen(Qt::NoPen));
                painter.setBrush(QBrush(Qt::black));
                painter.drawEllipse(QPoint(0, dm - r), m, m);
                painter.restore();
            } else {
                painter.drawText(- dm, - r, 2 * dm, 2 * dm, Qt::AlignCenter, QString::number(j + 1));
            }
            painter.rotate(15.0);
        }
    }

    painter.restore();
}

void MainWindow::reportData(bool checked)
{
    if (!checked)
        return;

    setAllEnabled(false, true);
    m_tab->reportData();
}

void MainWindow::reportDataFinished()
{
    actionReporting->setChecked(false);
    setAllEnabled(true, true);
}

void MainWindow::find()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    m_tab->webView()->findText(le_search->text());
}

void MainWindow::findNext()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    m_tab->webView()->findText(le_search->text());
}

void MainWindow::findPrevious()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    m_tab->webView()->findText(le_search->text(), QWebEnginePage::FindBackward);
}

void MainWindow::refreshView()
{
    m_tab->refreshView();
}

void MainWindow::addRecent(const QString &name)
{
    for (int i = 0; i < lw_recent_docs->count();) {
        if (lw_recent_docs->item(i)->text() == name) {
            delete lw_recent_docs->item(i);
        } else { i++; }
    }
    lw_recent_docs->insertItem(0, name);
    lw_recent_docs->setCurrentRow(0);
}

void MainWindow::setAllEnabled(bool enable, bool everything)
{
    if (everything) {
        actionNew->setEnabled(enable);
        menuOpen->setEnabled(enable);
        actionOpen->setEnabled(enable);
        actionLocal_database->setEnabled(enable);
        actionRemote_database->setEnabled(enable);

        tabw_main->setTabBarVisible(enable);
    }

    actionSave->setEnabled(enable);
    actionSave_and_compact->setEnabled(enable);
    actionClose->setEnabled(enable);
    actionUndo->setEnabled(enable && !actionUndo->menu()->isEmpty());
    actionNew_Tab->setEnabled(enable);
    if (!enable)
        actionClose_Tab->setEnabled(enable);
    menuImport->setEnabled(enable);
    actionImport_data->setEnabled(enable);
    actionImport_CSV->setEnabled(enable);
    menuExport->setEnabled(enable || everything);
    actionExport->setEnabled(enable || everything);
    actionPDF_Portrait->setEnabled(enable || everything);
    actionPDF_Landscape->setEnabled(enable || everything);
    actionHTML->setEnabled(enable || everything);
    actionPrint_preview->setEnabled(enable || everything);
    actionPrint->setEnabled(enable || everything);

    menuView->setEnabled(enable);
    menuDatabase->setEnabled(enable);
    menuCustomer->setEnabled(enable);
    menuCooling_circuit->setEnabled(enable);
    menuInspection->setEnabled(enable);
    menuRepair->setEnabled(enable);
    menuInspector->setEnabled(enable);

    actionReporting->setEnabled(enable);

    actionFind->setEnabled(enable || everything);
    actionFind_next->setEnabled(enable || everything);
    actionFind_previous->setEnabled(enable || everything);

    actionLock->setEnabled(enable);
    actionConfigure_permissions->setEnabled(enable);
    actionAutosave->setEnabled(enable);
    actionOpen_Backup_Folder->setEnabled(enable && !isDatabaseRemote());
    actionEdit_Refrigerants->setEnabled(enable);

    actionAdd_Service_Company->setEnabled(enable);
    actionEdit_Service_Company->setEnabled(enable);
    actionRemove_Service_Company->setEnabled(enable);
    actionAdd_record_of_refrigerant_management->setEnabled(enable);

    actionAdd_customer->setEnabled(enable);
    actionAdd_repair->setEnabled(enable);
    actionAdd_inspector->setEnabled(enable);
    if (!enable) {
    // menuCustomer
        actionEdit_customer->setEnabled(enable);
        actionDuplicate_customer->setEnabled(enable);
        actionRemove_customer->setEnabled(enable);
    // menuCooling_circuit
        actionAdd_circuit->setEnabled(enable);
        actionEdit_circuit->setEnabled(enable);
        actionDuplicate_circuit->setEnabled(enable);
        actionDuplicate_and_decommission_circuit->setEnabled(enable);
        actionMove_Circuit_to_Another_Customer->setEnabled(enable);
        actionRemove_circuit->setEnabled(enable);
    // menuInspection
        actionAdd_inspection->setEnabled(enable);
        actionEdit_inspection->setEnabled(enable);
        actionDuplicate_inspection->setEnabled(enable);
        actionRemove_inspection->setEnabled(enable);
        actionSkip_Inspection->setEnabled(enable);
        actionPrint_label->setEnabled(enable);
    // menuRepair
        actionEdit_repair->setEnabled(enable);
        actionDuplicate_repair->setEnabled(enable);
        actionRemove_repair->setEnabled(enable);
    // menuInspector
        actionEdit_inspector->setEnabled(enable);
        actionRemove_inspector->setEnabled(enable);
    }
    dw_variables->setEnabled(enable);
    dw_tables->setEnabled(enable);
    dw_warnings->setEnabled(enable);
    dw_styles->setEnabled(enable);
}

void MainWindow::updateLockButton()
{
    if (DBInfo::isDatabaseLocked()) {
        actionLock->setIcon(QIcon(":/images/images/locked.png"));
        actionLock->setText(tr("Unlock..."));
    } else {
        actionLock->setIcon(QIcon(":/images/images/unlocked.png"));
        actionLock->setText(tr("Lock..."));
    }
}

void MainWindow::serviceCompanyInformationVisibilityChanged()
{
    actionPrint_Service_Company_Information->setChecked(m_settings.serviceCompanyInformationPrinted());
    actionShow_Service_Company_Information->setChecked(m_settings.serviceCompanyInformationVisible());
    if (QSqlDatabase::database().isOpen())
        refreshView();
}

void MainWindow::dateFormatChanged(MainWindowSettings::DateFormat date_format)
{
    QMapIterator<QAction *, MainWindowSettings::DateFormat> df(dict_action_date_format);
    if (df.findNext(date_format))
        df.key()->setChecked(true);
}

void MainWindow::dateFormatChanged(QAction *action)
{
    m_settings.setDateFormat(dict_action_date_format.value(action));
    refreshView();
}

void MainWindow::timeFormatChanged(MainWindowSettings::TimeFormat time_format)
{
    QMapIterator<QAction *, MainWindowSettings::TimeFormat> tf(dict_action_time_format);
    if (tf.findNext(time_format) && !tf.key()->isChecked())
        tf.key()->setChecked(true);
}

void MainWindow::timeFormatChanged(QAction *action)
{
    m_settings.setTimeFormat(dict_action_time_format.value(action));
    refreshView();
}

void MainWindow::enableTools()
{
    bool can_remove_service_company = m_tab && m_tab->toolBarStack()->canRemoveServiceCompany();
    bool customer_selected = m_tab && m_tab->isCustomerSelected();
    bool circuit_selected = m_tab && m_tab->isCircuitSelected();
    bool inspection_selected = m_tab && m_tab->isInspectionSelected();
    bool repair_selected = m_tab && m_tab->isRepairSelected();
    bool inspector_selected = m_tab && m_tab->isInspectorSelected();

    tabw_main->setTabsClosable(tabw_main->count() > 1);
    actionClose_Tab->setEnabled(tabw_main->count() > 1);

    actionSync->setEnabled(m_tab);

    actionRemove_Service_Company->setEnabled(can_remove_service_company);

    actionEdit_customer->setEnabled(customer_selected);
    actionDuplicate_customer->setEnabled(customer_selected);
    actionRemove_customer->setEnabled(customer_selected);
    actionDecommission_all_circuits->setEnabled(customer_selected);

    actionAdd_circuit->setEnabled(customer_selected);
    actionEdit_circuit->setEnabled(circuit_selected);
    actionDuplicate_circuit->setEnabled(circuit_selected);
    actionDuplicate_and_decommission_circuit->setEnabled(circuit_selected);
    actionMove_Circuit_to_Another_Customer->setEnabled(circuit_selected);
    actionRemove_circuit->setEnabled(circuit_selected);

    actionAdd_inspection->setEnabled(circuit_selected);
    actionEdit_inspection->setEnabled(inspection_selected);
    actionDuplicate_inspection->setEnabled(inspection_selected);
    actionRemove_inspection->setEnabled(inspection_selected);
    actionSkip_Inspection->setEnabled(circuit_selected);

    actionEdit_repair->setEnabled(repair_selected);
    actionDuplicate_repair->setEnabled(repair_selected);
    actionRemove_repair->setEnabled(repair_selected);
    actionPrint_detailed_label->setEnabled(circuit_selected);
    actionPrint_label->setEnabled(inspector_selected);

    actionNew_subvariable->setEnabled(trw_variables->currentIndex().isValid() && trw_variables->currentItem()->parent() == NULL && !variableNames().contains(trw_variables->currentItem()->text(1)));
    tbtn_edit_variable->setEnabled(trw_variables->currentIndex().isValid());
    tbtn_remove_variable->setEnabled(trw_variables->currentIndex().isValid() && !variableNames().contains(trw_variables->currentItem()->text(1)));

    tbtn_edit_table->setEnabled(cb_table_edit->currentIndex() >= 0);
    tbtn_remove_table->setEnabled(cb_table_edit->currentIndex() >= 0);
    tbtn_table_add_variable->setEnabled(cb_table_edit->currentIndex() >= 0);
    tbtn_table_remove_variable->setEnabled(trw_table_variables->currentIndex().isValid());
    tbtn_table_move_up->setEnabled(trw_table_variables->currentIndex().isValid());
    tbtn_table_move_down->setEnabled(trw_table_variables->currentIndex().isValid());

    tbtn_edit_warning->setEnabled(lw_warnings->currentIndex().isValid());
    tbtn_remove_warning->setEnabled(lw_warnings->currentIndex().isValid() && !Warnings::isPredefined(lw_warnings->currentItem()->data(Qt::UserRole).toString()));

    actionEdit_inspector->setEnabled(inspector_selected);
    actionRemove_inspector->setEnabled(inspector_selected);

    if (m_tab)
        m_tab->enableTools();
}

void MainWindow::toggleLocked()
{
    if (!DBInfo::isCurrentUserAdmin()) {
        showOperationNotPermittedMessage();
        return;
    }

    if (!DBInfo::isDatabaseLocked()) {
        int r = 0;

        QDialog d(this);
        d.setWindowTitle(tr("Lock database - Leaklog"));
        QGridLayout *gl = new QGridLayout(&d);

        QLabel *lbl = new QLabel(tr("Lock inspections and repairs older than:"), &d);
        lbl->setAlignment(Qt::AlignBottom | Qt::AlignLeft);
        gl->addWidget(lbl, r, 0, 1, 2);

        r++;

        QString last_date = DBInfo::valueForKey("lock_date");

        QRadioButton *static_lock = new QRadioButton(&d);
        gl->addWidget(static_lock, r, 0);

        QDateEdit *date = new QDateEdit(&d);
        date->setDisplayFormat(m_settings.dateFormatString());
        date->setDate(last_date.isEmpty() ? QDate::currentDate() : QDate::fromString(last_date, DATE_FORMAT));
        date->setCalendarPopup(true);
        date->calendarWidget()->setLocale(QLocale());
        date->calendarWidget()->setFirstDayOfWeek(QLocale().firstDayOfWeek());
        gl->addWidget(date, r, 1);

        r++;

        QRadioButton *autolock = new QRadioButton(&d);
        autolock->setChecked(true);
        gl->addWidget(autolock, r, 0);

        QSpinBox *days = new QSpinBox(&d);
        days->setSuffix(tr(" days"));
        days->setRange(0, 99999);
        days->setValue(7);
        gl->addWidget(days, r, 1);

        r++;

        QLineEdit *admin = NULL;
        if (isDatabaseRemote()) {
            lbl = new QLabel(tr("Administrator:"), &d);
            lbl->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
            gl->addWidget(lbl, r, 0);

            admin = new QLineEdit(&d);
            admin->setText(DBInfo::valueForKey("admin", currentUser()));
            gl->addWidget(admin, r, 1);

            r++;
        }

        lbl = new QLabel(tr("Password:"), &d);
        lbl->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
        gl->addWidget(lbl, r, 0);

        QLineEdit *password = new QLineEdit(&d);
        gl->addWidget(password, r, 1);

        r++;

        QDialogButtonBox *bb = new QDialogButtonBox(&d);
        bb->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        bb->button(QDialogButtonBox::Ok)->setText(tr("Lock"));
        bb->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));
        QObject::connect(bb, SIGNAL(accepted()), &d, SLOT(accept()));
        QObject::connect(bb, SIGNAL(rejected()), &d, SLOT(reject()));
        gl->addWidget(bb, r, 0, 1, 2);

        if (d.exec() != QDialog::Accepted) return;

        UndoCommand command(m_undo_stack, tr("Lock database"));
        m_undo_stack->savepoint();

        DBInfo::setValueForKey("lock_date", date->date().toString(DATE_FORMAT));
        DBInfo::setValueForKey("autolock_days", QString::number(days->value()));
        DBInfo::setValueForKey("lock_password", sha256(password->text()));
        DBInfo::setValueForKey("locked", static_lock->isChecked() ? "true" : "auto");
        if (admin)
            DBInfo::setValueForKey("admin", admin->text());
        updateLockButton();
        enableTools();
        setDatabaseModified(true);
        refreshView();
    } else {
        bool ok;
        QString password = QInputDialog::getText(this, tr("Unlock database - Leaklog"),
                                                 tr("Password:"), QLineEdit::Password,
                                                 "", &ok);
        if (!ok) return;

        if (sha256(password) != DBInfo::valueForKey("lock_password")) {
            QMessageBox::warning(this, tr("Unlock database - Leaklog"), tr("Wrong password."));
            return;
        }

        UndoCommand command(m_undo_stack, tr("Unlock database"));
        m_undo_stack->savepoint();

        DBInfo::setValueForKey("locked", "false");
        updateLockButton();
        enableTools();
        setDatabaseModified(true);
        refreshView();
    }
}

void MainWindow::showOperationNotPermittedMessage()
{
    QMessageBox message(this);
    message.setWindowModality(Qt::WindowModal);
    message.setWindowFlags(message.windowFlags() | Qt::Sheet);
    message.setIconPixmap(QIcon(QString::fromUtf8(":/images/images/locked.png")).pixmap(32, 32));
    message.setWindowTitle(tr("Permission denied - Leaklog"));
    message.setText(tr("This operation is not permitted."));
    message.setInformativeText(tr("For more information, contact your administrator."));
    message.addButton(tr("OK"), QMessageBox::AcceptRole);
    message.exec();
}

void MainWindow::configurePermissions()
{
    if (!DBInfo::isCurrentUserAdmin() || (!isDatabaseRemote() && DBInfo::isDatabaseLocked())) {
        showOperationNotPermittedMessage();
        return;
    }
    PermissionsDialogue d(this);
    if (d.exec() == QDialog::Accepted)
        setDatabaseModified(true);
}

void MainWindow::configureAutosave()
{
    if (!QSqlDatabase::database().isOpen())
        return;

    QString autosave_mode = DBInfo::autosaveMode();

    QDialog *d = new QDialog(this);
    d->setWindowTitle(tr("Configure Auto Save - Leaklog"));
    d->setWindowModality(Qt::WindowModal);
    d->setWindowFlags(d->windowFlags() | Qt::Sheet);

    QVBoxLayout *layout = new QVBoxLayout(d);

    QRadioButton *rbtn_off = new QRadioButton(tr("Do not save automatically"), d);
    rbtn_off->setChecked(autosave_mode.isEmpty());
    layout->addWidget(rbtn_off);

    layout->addSpacing(12);

    QRadioButton *rbtn_immediate = new QRadioButton(tr("Save all changes immediately"), d);
    rbtn_immediate->setChecked(autosave_mode == "immediate");
    layout->addWidget(rbtn_immediate);

    QLabel *lbl_immediate = new QLabel(tr("This will disable the Undo function."), d);
    QFont font = lbl_immediate->font();
    font.setItalic(true);
    lbl_immediate->setFont(font);
    layout->addWidget(lbl_immediate);

    layout->addSpacing(12);

    QRadioButton *rbtn_delayed = new QRadioButton(tr("Save all changes after 10 minutes of inactivity"), d);
    rbtn_delayed->setChecked(autosave_mode == "delayed");
    layout->addWidget(rbtn_delayed);

    layout->addSpacing(12);

    QRadioButton *rbtn_ask = new QRadioButton(tr("Ask to save changes after 10 minutes of inactivity"), d);
    rbtn_ask->setChecked(autosave_mode == "ask");
    layout->addWidget(rbtn_ask);

    layout->addSpacing(12);

    QDialogButtonBox *bb = new QDialogButtonBox(d);
    bb->addButton(tr("&Save"), QDialogButtonBox::AcceptRole);
    bb->addButton(tr("Cancel"), QDialogButtonBox::RejectRole);
    QObject::connect(bb, SIGNAL(accepted()), d, SLOT(accept()));
    QObject::connect(bb, SIGNAL(rejected()), d, SLOT(reject()));
    layout->addWidget(bb);

    if (d->exec() == QDialog::Accepted) {
        if (rbtn_immediate->isChecked())
            autosave_mode = "immediate";
        else if (rbtn_delayed->isChecked())
            autosave_mode = "delayed";
        else if (rbtn_ask->isChecked())
            autosave_mode = "ask";
        else
            autosave_mode.clear();

        if (DBInfo::autosaveMode() != autosave_mode) {
            DBInfo::setAutosaveMode(autosave_mode);
            setDatabaseModified(true);
        }
    }

    d->deleteLater();
}

void MainWindow::openBackupDirectory()
{
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen())
        return;

    if (isDatabaseRemote(db))
        return;

    QPair<bool, QDir> backup_dir = backupDirectoryForDatabasePath(db.databaseName());
    if (backup_dir.first) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(backup_dir.second.absolutePath()));
    } else {
        QMessageBox::critical(this, tr("Open Backup Folder - Leaklog"), tr("Could not create backup folder."));
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (!saveChangesBeforeProceeding(tr("Quit Leaklog"), true)) {
        saveSettings();
        event->accept();
    } else { event->ignore(); }
}

void MainWindow::loadSettings()
{
    QSettings settings("SZCHKT", "Leaklog");
    lw_recent_docs->addItems(settings.value("recent_docs").toStringList());
    move(settings.value("pos", pos()).toPoint() * scaleFactor());
    resize(settings.value("size", size()).toSize() * scaleFactor());
    restoreState(settings.value("window_state").toByteArray(), 0);
#ifdef Q_OS_MAC
    if (settings.value("fullscreen", isFullScreen()).toBool())
        showFullScreen();
#else
    if (settings.value("maximized", isMaximized()).toBool())
        showMaximized();
#endif
    actionShow_icons_only->setChecked(settings.value("toolbar_icons_only", false).toBool());
    showIconsOnly(actionShow_icons_only->isChecked());
    actionCompare_values->setChecked(settings.value("compare_values", true).toBool());
    actionShow_date_updated->setChecked(settings.value("columns/date_updated", false).toBool());
    actionShow_owner->setChecked(settings.value("columns/owner", false).toBool());
    actionShow_Notes->setChecked(settings.value("columns/notes", true).toBool());
    actionShow_Leaked->setChecked(settings.value("columns/leaked", false).toBool());
    actionShow_Place_of_operation->setChecked(settings.value("columns/operation", false).toBool());
    actionShow_Building->setChecked(settings.value("columns/building", false).toBool());
    actionShow_Device->setChecked(settings.value("columns/device", true).toBool());
    actionShow_Manufacturer->setChecked(settings.value("columns/manufacturer", true).toBool());
    actionShow_Type->setChecked(settings.value("columns/type", true).toBool());
    actionShow_Serial_number->setChecked(settings.value("columns/sn", true).toBool());
    actionShow_Year_of_purchase->setChecked(settings.value("columns/year", true).toBool());
    actionShow_Date_of_commissioning->setChecked(settings.value("columns/commissioning", true).toBool());
    actionShow_Field_of_application->setChecked(settings.value("columns/field", true).toBool());
    actionShow_Oil->setChecked(settings.value("columns/oil", false).toBool());
    actionMost_recent_first->setChecked(settings.value("most_recent_first", false).toBool());
    actionEdit_Refrigerants->setVisible(settings.value("edit_refrigerants_enabled", false).toBool());
    m_settings.restore(settings);
#ifndef QT_DEBUG
    if (settings.value("check_for_updates", true).toBool())
        checkForUpdates(true);
#endif
}

void MainWindow::saveSettings()
{
    QSettings settings("SZCHKT", "Leaklog");
    QStringList recent;
    for (int i = 0; i < lw_recent_docs->count(); ++i)
        recent << lw_recent_docs->item(i)->text();
    settings.setValue("recent_docs", recent);
    if (!isMaximized() && !isFullScreen()) {
        settings.setValue("pos", pos() / scaleFactor());
        settings.setValue("size", size() / scaleFactor());
    }
    settings.setValue("maximized", isMaximized());
    settings.setValue("fullscreen", isFullScreen());
    settings.setValue("window_state", saveState(0));
    settings.setValue("toolbar_icons_only", actionShow_icons_only->isChecked());
    settings.setValue("compare_values", actionCompare_values->isChecked());
    settings.setValue("columns/date_updated", actionShow_date_updated->isChecked());
    settings.setValue("columns/owner", actionShow_owner->isChecked());
    settings.setValue("columns/notes", actionShow_Notes->isChecked());
    settings.setValue("columns/leaked", actionShow_Leaked->isChecked());
    settings.setValue("columns/operation", actionShow_Place_of_operation->isChecked());
    settings.setValue("columns/building", actionShow_Building->isChecked());
    settings.setValue("columns/device", actionShow_Device->isChecked());
    settings.setValue("columns/manufacturer", actionShow_Manufacturer->isChecked());
    settings.setValue("columns/type", actionShow_Type->isChecked());
    settings.setValue("columns/sn", actionShow_Serial_number->isChecked());
    settings.setValue("columns/year", actionShow_Year_of_purchase->isChecked());
    settings.setValue("columns/commissioning", actionShow_Date_of_commissioning->isChecked());
    settings.setValue("columns/field", actionShow_Field_of_application->isChecked());
    settings.setValue("columns/oil", actionShow_Oil->isChecked());
    settings.setValue("most_recent_first", actionMost_recent_first->isChecked());
    m_settings.save(settings);
}

void MainWindow::changeLanguage()
{
    QWidget *w_lang = new QWidget(this, Qt::Dialog);
    w_lang->setWindowModality(Qt::WindowModal);
    w_lang->setAttribute(Qt::WA_DeleteOnClose);
#ifdef Q_OS_MAC
    w_lang->setWindowTitle(tr("Change language"));
#else
    w_lang->setWindowTitle(tr("Change language - Leaklog"));
#endif
    QGridLayout *glayout_lang = new QGridLayout(w_lang);
    glayout_lang->setContentsMargins(12, 12, 12, 12);
    glayout_lang->setSpacing(6);
    QLabel *lbl_lang = new QLabel(w_lang);
    lbl_lang->setText(tr("Select your preferred language"));
    glayout_lang->addWidget(lbl_lang, 0, 0);
    cb_lang = new QComboBox(w_lang);
    QStringList langs(leaklog_i18n.keys()); langs.sort();
    for (int i = 0; i < langs.count(); ++i) {
        cb_lang->addItem(langs.at(i));
        if (langs.at(i) == "English") { cb_lang->setCurrentIndex(i); }
    }
    glayout_lang->addWidget(cb_lang, 1, 0);
    QDialogButtonBox *bb_lang = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, w_lang);
    QObject::connect(bb_lang, SIGNAL(accepted()), this, SLOT(languageChanged()));
    QObject::connect(bb_lang, SIGNAL(rejected()), w_lang, SLOT(close()));
    glayout_lang->addWidget(bb_lang, 2, 0);
    w_lang->show();
}

void MainWindow::languageChanged()
{
    if (cb_lang == NULL) { return; }
    QString lang = leaklog_i18n.value(cb_lang->currentText(), cb_lang->currentText());
    QSettings settings("SZCHKT", "Leaklog");
    QString current_lang = settings.value("lang", "Slovak").toString();
    if (current_lang != lang) {
        settings.setValue("lang", lang);
        QMessageBox::information(this, "Leaklog", tr("You need to restart Leaklog for the changes to apply."));
    }
    if (cb_lang->parent() == NULL) { return; }
    QWidget *w_lang = (QWidget *)cb_lang->parent();
    w_lang->close();
    cb_lang = NULL;
}

void MainWindow::logIn()
{
    QWidget *w = new QWidget(this, Qt::Dialog);
    w->setWindowModality(Qt::WindowModal);
    w->setAttribute(Qt::WA_DeleteOnClose);
#ifdef Q_OS_MAC
    w->setWindowTitle(tr("Log In"));
#else
    w->setWindowTitle(tr("Log In - Leaklog"));
#endif

    QGridLayout *layout = new QGridLayout(w);
    layout->addWidget(new QLabel(tr("Enter your szchkt.org username and password:"), w), 0, 0, 1, 2);

    QLineEdit *username = new QLineEdit(authenticator->username(), w);
    username->setObjectName("username");
    layout->addWidget(username, 1, 0);

    QLineEdit *password = new QLineEdit(w);
    password->setObjectName("password");
    password->setEchoMode(QLineEdit::Password);
    layout->addWidget(password, 1, 1);

    QLabel *tos = new QLabel(tr("By using this service, you agree to the <a href=\"https://leaklog.org/terms\">Terms of Service</a>."), w);
    tos->setTextFormat(Qt::RichText);
    tos->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    tos->setOpenExternalLinks(true);
    layout->addWidget(tos, 2, 0, 1, 2);

    QDialogButtonBox *bb = new QDialogButtonBox(QDialogButtonBox::Reset | QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, w);
    bb->button(QDialogButtonBox::Ok)->setText(tr("Log In"));
    if (authenticator->token().isEmpty()) {
        bb->button(QDialogButtonBox::Reset)->setText(tr("Register"));
        QObject::connect(bb, SIGNAL(clicked(QAbstractButton *)), this, SLOT(doSignUp(QAbstractButton *)));
    } else {
        bb->button(QDialogButtonBox::Reset)->setText(tr("Log Out"));
        QObject::connect(bb, SIGNAL(clicked(QAbstractButton *)), this, SLOT(doLogOut(QAbstractButton *)));
    }
    QObject::connect(bb, SIGNAL(accepted()), this, SLOT(doLogIn()));
    QObject::connect(bb, SIGNAL(rejected()), w, SLOT(close()));
    layout->addWidget(bb, 3, 0, 1, 2);

    w->show();
}

void MainWindow::doLogIn()
{
    QWidget *button = qobject_cast<QWidget *>(sender());
    if (!button)
        return;

    QWidget *w = button->window();
    QString username = w->findChild<QLineEdit *>("username")->text();
    QString password = w->findChild<QLineEdit *>("password")->text();

    w->close();

    if (!username.isEmpty() && !password.isEmpty()) {
        authenticator->logIn(username, password);
    } else {
        logIn();
    }
}

void MainWindow::doLogOut(QAbstractButton *sender)
{
    if (qobject_cast<QDialogButtonBox *>(sender->parent())->buttonRole(sender) != QDialogButtonBox::ResetRole)
        return;

    sender->window()->close();

    authenticator->logOut();
}

void MainWindow::doSignUp(QAbstractButton *sender)
{
    if (qobject_cast<QDialogButtonBox *>(sender->parent())->buttonRole(sender) != QDialogButtonBox::ResetRole)
        return;

    sender->window()->close();

    QDesktopServices::openUrl(QUrl("https://leaklog.org"));
}

void MainWindow::loginFinished(bool success)
{
    actionLog_In->setText(success ? authenticator->username() : tr("Log In"));

    if (success) {
        sync();
    } else if (!authenticator->error().isEmpty()) {
        QMessageBox message(this);
#ifdef Q_OS_MAC
        message.setWindowTitle(tr("Log In"));
#else
        message.setWindowTitle(tr("Log In - Leaklog"));
#endif
        message.setWindowModality(Qt::WindowModal);
        message.setWindowFlags(message.windowFlags() | Qt::Sheet);
        message.setIcon(QMessageBox::Warning);
        message.setText(tr("Failed to log in."));
        message.setInformativeText(authenticator->error());
        message.addButton(tr("Cancel"), QMessageBox::AcceptRole);
        message.setDefaultButton(message.addButton(tr("Try Again"), QMessageBox::ActionRole));
        QObject::connect(&message, SIGNAL(buttonClicked(QAbstractButton *)), this, SLOT(loginFinished(QAbstractButton *)));
        message.exec();
    }
}

void MainWindow::loginFinished(QAbstractButton *button)
{
    if (qobject_cast<QDialogButtonBox *>(button->parent())->buttonRole(button) != QDialogButtonBox::ActionRole)
        return;

    logIn();
}

void MainWindow::logoutFinished()
{
    actionLog_In->setText(tr("Log In"));
}

void MainWindow::checkForUpdates(bool silent)
{
    QNetworkRequest request(QString("https://leaklog.org/current-version.php"
                                    "?version=%1"
                                    "&preview=%2"
                                    "&lang=%3"
                                    "&os=%4&os_version=%5"
                                    "&build_arch=%6"
                                    "&current_arch=%7"
                                    "&debug=%8"
                                    "&automatic=%9")
              .arg(F_LEAKLOG_VERSION)
              .arg(LEAKLOG_PREVIEW_VERSION)
              .arg(tr("en_GB"))
#ifdef Q_OS_WIN32
              .arg('W').arg(winVersion())
#elif defined Q_OS_MAC
              .arg('M').arg(macVersion())
#else
              .arg('O').arg(-1)
#endif
              .arg(QSysInfo::buildCpuArchitecture())
              .arg(QSysInfo::currentCpuArchitecture())
#ifdef QT_DEBUG
              .arg(1)
#else
              .arg(0)
#endif
              .arg(silent ? 1 : 0));
    request.setRawHeader("User-Agent", QString("Leaklog/%1").arg(LEAKLOG_VERSION).toUtf8());
    QNetworkReply *reply = network_access_manager->get(request);
    reply->setProperty("silent", silent);
}

void MainWindow::httpRequestFinished(QNetworkReply *reply)
{
    bool silent = reply->property("silent").toBool();

    QString str;

    if (reply->error() == QNetworkReply::NoError && reply->isReadable())
        str = QString::fromUtf8(reply->readAll());
    else
        return httpRequestFailed(silent);

    reply->deleteLater();

    QTextStream in(&str);
    if (in.readLine() != "[Leaklog.current-version]")
        return httpRequestFailed(silent);
    QString current_ver = in.readLine();
    if (in.readLine() != "[Leaklog.current-version.float]")
        return httpRequestFailed(silent);
    double f_current_ver = in.readLine().toDouble();
    if (in.readLine() != "[Leaklog.download-url]")
        return httpRequestFailed(silent);
    QString url = in.readLine();
    if (in.readLine() != "[Leaklog.release-notes]")
        return httpRequestFailed(silent);
    QString release_notes;
    while (!in.atEnd()) { release_notes.append(in.readLine()); }
    if ((f_current_ver <= F_LEAKLOG_VERSION && !LEAKLOG_PREVIEW_VERSION) ||
        (f_current_ver < F_LEAKLOG_VERSION && LEAKLOG_PREVIEW_VERSION)) {
        if (!silent) {
            QMessageBox message(this);
            message.setWindowTitle("Leaklog");
            message.setWindowModality(Qt::WindowModal);
            message.setWindowFlags(message.windowFlags() | Qt::Sheet);
            message.setIcon(QMessageBox::Information);
            message.setText(tr("You are running the latest version of Leaklog."));
            message.exec();
        }
    } else {
        QMessageBox message(this);
        message.setWindowTitle("Leaklog");
        message.setWindowModality(Qt::WindowModal);
        message.setWindowFlags(message.windowFlags() | Qt::Sheet);
        message.setIcon(QMessageBox::Information);
        message.setText(tr("Leaklog %1 is available now.").arg(current_ver));
        message.setInformativeText(QString("<html><head>"
                                           "<meta name=\"qrichtext\" content=\"1\" />"
                                           "<style type=\"text/css\">p, li { white-space: pre-wrap; }</style>"
                                           "</head><body><p>%1</p></body></html>")
                                   .arg(release_notes));
        message.addButton(tr("&Download Update"), QMessageBox::AcceptRole);
        message.addButton(tr("Remind Me &Later"), QMessageBox::RejectRole);
        switch (message.exec()) {
            case QMessageBox::AcceptRole: // Download
                QDesktopServices::openUrl(url);
                break;
            case QMessageBox::RejectRole: // Later
                break;
        }
    }
}

void MainWindow::httpRequestFailed(bool silent)
{
    if (silent)
        return;

    switch (QMessageBox::critical(this, "Leaklog", tr("Failed to check for updates."), tr("&Try again"), tr("Cancel"), 0, 1)) {
        case 0: // Try again
            checkForUpdates(); break;
        case 1: // Cancel
            break;
    }
}

void MainWindow::about()
{
    AboutWidget *leaklog_about = new AboutWidget;
    leaklog_about->setParent(this);
    leaklog_about->setWindowFlags(Qt::Dialog /*| Qt::WindowMaximizeButtonHint*/ | Qt::WindowStaysOnTopHint);
    leaklog_about->show();
}

void MainWindow::openDocumentation()
{
    QDesktopServices::openUrl(tr("https://github.com/szchkt/leaklog/wiki"));
}

void MainWindow::openReleaseNotes()
{
    QDesktopServices::openUrl(tr("https://github.com/szchkt/leaklog/releases"));
}
