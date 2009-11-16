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

#include "main_window.h"

MainWindow::MainWindow()
{
    // Dictionaries
    dict_dbtables = get_dict_dbtables();
    dict_vartypes = get_dict_vartypes();
    dict_varnames = get_dict_varnames();
    dict_attrvalues = get_dict_attrvalues();
    dict_attrnames = get_dict_attrnames();
    dict_fieldtypes.insert("address", MTVariant::Address);
    // ------------
    // HTML
    QFile file; QTextStream in(&file); in.setCodec("UTF-8");
    file.setFileName(":/html/service_company.html"); file.open(QIODevice::ReadOnly | QIODevice::Text);
    dict_html.insert(Navigation::ServiceCompany, in.readAll());
    file.close();
    file.setFileName(":/html/customers.html"); file.open(QIODevice::ReadOnly | QIODevice::Text);
    dict_html.insert(Navigation::ListOfCustomers, in.readAll());
    file.close();
    file.setFileName(":/html/customer.html"); file.open(QIODevice::ReadOnly | QIODevice::Text);
    dict_html.insert(Navigation::ListOfCircuits, in.readAll());
    file.close();
    file.setFileName(":/html/circuit.html"); file.open(QIODevice::ReadOnly | QIODevice::Text);
    dict_html.insert(Navigation::ListOfInspections, in.readAll());
    file.close();
    file.setFileName(":/html/inspection.html"); file.open(QIODevice::ReadOnly | QIODevice::Text);
    dict_html.insert(Navigation::Inspection, in.readAll());
    file.close();
    file.setFileName(":/html/table.html"); file.open(QIODevice::ReadOnly | QIODevice::Text);
    dict_html.insert(Navigation::TableOfInspections, in.readAll());
    file.close();
    file.setFileName(":/html/repairs.html"); file.open(QIODevice::ReadOnly | QIODevice::Text);
    dict_html.insert(Navigation::ListOfRepairs, in.readAll());
    file.close();
    file.setFileName(":/html/inspectors.html"); file.open(QIODevice::ReadOnly | QIODevice::Text);
    dict_html.insert(Navigation::ListOfInspectors, in.readAll());
    file.close();
    file.setFileName(":/html/leakages.html"); file.open(QIODevice::ReadOnly | QIODevice::Text);
    dict_html.insert(Navigation::LeakagesByApplication, in.readAll());
    file.close();
    file.setFileName(":/html/agenda.html"); file.open(QIODevice::ReadOnly | QIODevice::Text);
    dict_html.insert(Navigation::Agenda, in.readAll());
    file.close();
    // ----
    selected_customer = -1;
    selected_circuit = -1;
    selected_inspection_is_repair = false;
    selected_inspector = -1;
    database_locked = false;
    show_leaked_in_store_in_service_company_view = false;
    // i18n
    QTranslator translator; translator.load(":/i18n/Leaklog-i18n.qm");
    leaklog_i18n.insert("English", "English");
    leaklog_i18n.insert(translator.translate("LanguageNames", "Slovak"), "Slovak");
    // ----
    if (tr("LTR") == "RTL") { qApp->setLayoutDirection(Qt::RightToLeft); }
    setupUi(this);
    http = new QHttp(this);
    http_buffer = new QBuffer(this);
    this->setUnifiedTitleAndToolBarOnMac(true);
    tbtn_open = new QToolButton(this);
    tbtn_open->setDefaultAction(actionOpen);
    tbtn_open->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    tbtn_open->setPopupMode(QToolButton::InstantPopup);
    toolBar->insertWidget(actionSave, tbtn_open);
    tbtn_add = new QToolButton(this);
    tbtn_add->setDefaultAction(actionAdd);
    tbtn_add->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    tbtn_add->setPopupMode(QToolButton::InstantPopup);
    toolBar->insertWidget(actionFind, tbtn_add);
    tbtn_modify = new QToolButton(this);
    tbtn_modify->setDefaultAction(actionModify);
    tbtn_modify->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    tbtn_modify->setPopupMode(QToolButton::InstantPopup);
    toolBar->insertWidget(actionFind, tbtn_modify);
    tbtn_export = new QToolButton(this);
    tbtn_export->setDefaultAction(actionExport);
    tbtn_export->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    tbtn_export->setPopupMode(QToolButton::InstantPopup);
    toolBar->insertWidget(actionPrint, tbtn_export);
    QMenu * menu_add = new QMenu(this);
    menu_add->addAction(actionAdd_customer);
    menu_add->addAction(actionAdd_circuit);
    menu_add->addAction(actionAdd_inspection);
    menu_add->addAction(actionAdd_repair);
    menu_add->addAction(actionAdd_inspector);
    menu_add->addAction(actionAdd_record_of_refrigerant_management);
    tbtn_add->setMenu(menu_add);
    QMenu * menu_modify = new QMenu(this);
    menu_modify->addAction(actionModify_customer);
    menu_modify->addAction(actionModify_circuit);
    menu_modify->addAction(actionModify_inspection);
    menu_modify->addAction(actionModify_repair);
    menu_modify->addAction(actionModify_inspector);
    tbtn_modify->setMenu(menu_modify);
    QWidget * spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    toolBar->insertWidget(actionLock, spacer);
    dw_variables->setVisible(false);
    dw_tables->setVisible(false);
    dw_warnings->setVisible(false);
    actionOpen->setMenu(menuOpen);
    actionExport->setMenu(menuExport);
    QMenu * menuAdd_variable = new QMenu(this);
    menuAdd_variable->addAction(actionNew_variable);
    menuAdd_variable->addAction(actionNew_subvariable);
    tbtn_add_variable->setMenu(menuAdd_variable);
    actgrp_view = new QActionGroup(this);
    actgrp_view->addAction(actionService_company);
    actgrp_view->addAction(actionBasic_logbook);
    actgrp_view->addAction(actionDetailed_logbook);
    actionShow_icons_only = new QAction(tr("Show icons only"), this);
    actionShow_icons_only->setCheckable(true);
    lbl_current_selection = new QLabel;
    lbl_current_selection->setVisible(false);
    statusbar->addWidget(lbl_current_selection);
    lbl_selected_repair = new QLabel;
    lbl_selected_repair->setVisible(false);
    statusbar->addWidget(lbl_selected_repair);
    lbl_selected_inspector = new QLabel;
    lbl_selected_inspector->setVisible(false);
    statusbar->addWidget(lbl_selected_inspector);
    btn_clear_selection = new QPushButton(this);
    btn_clear_selection->setFlat(true);
    btn_clear_selection->setMaximumSize(18, 18);
    if (layoutDirection() == Qt::LeftToRight) {
        btn_clear_selection->setIcon(QIcon(QString::fromUtf8(":/images/images/clear.png")));
    } else {
        btn_clear_selection->setIcon(QIcon(QString::fromUtf8(":/images/images/clear_rtl.png")));
    }
    btn_clear_selection->setToolTip(tr("Clear current selection"));
    btn_clear_selection->setVisible(false);
    statusbar->addWidget(btn_clear_selection);
    trw_variables->header()->setResizeMode(0, QHeaderView::Stretch);
    trw_variables->header()->setResizeMode(1, QHeaderView::ResizeToContents);
    trw_variables->header()->setResizeMode(2, QHeaderView::ResizeToContents);
    trw_variables->header()->setResizeMode(3, QHeaderView::ResizeToContents);
    trw_table_variables->header()->setResizeMode(0, QHeaderView::Stretch);
    trw_table_variables->header()->setResizeMode(1, QHeaderView::ResizeToContents);
    trw_table_variables->header()->setResizeMode(2, QHeaderView::ResizeToContents);
    setAllEnabled(false);
    navigation->connectSlots(this);
    QObject::connect(actionShow_icons_only, SIGNAL(toggled(bool)), this, SLOT(showIconsOnly(bool)));
    QObject::connect(actionAbout_Leaklog, SIGNAL(triggered()), this, SLOT(about()));
    QObject::connect(actionNew, SIGNAL(triggered()), this, SLOT(newDatabase()));
    QObject::connect(actionLocal_database, SIGNAL(triggered()), this, SLOT(open()));
    QObject::connect(actionRemote_database, SIGNAL(triggered()), this, SLOT(openRemote()));
    QObject::connect(actionSave, SIGNAL(triggered()), this, SLOT(save()));
    QObject::connect(actionSave_and_compact, SIGNAL(triggered()), this, SLOT(saveAndCompact()));
    QObject::connect(actionClose, SIGNAL(triggered()), this, SLOT(closeDatabase()));
    QObject::connect(actionPrint_preview, SIGNAL(triggered()), this, SLOT(printPreview()));
    QObject::connect(actionPrint, SIGNAL(triggered()), this, SLOT(print()));
    QObject::connect(actionPDF, SIGNAL(triggered()), this, SLOT(exportPDF()));
    QObject::connect(actionHTML, SIGNAL(triggered()), this, SLOT(exportHTML()));
    QObject::connect(actionFind, SIGNAL(triggered()), this, SLOT(find()));
    QObject::connect(actionFind_next, SIGNAL(triggered()), this, SLOT(findNext()));
    QObject::connect(actionFind_previous, SIGNAL(triggered()), this, SLOT(findPrevious()));
    QObject::connect(actionChange_language, SIGNAL(triggered()), this, SLOT(changeLanguage()));
    QObject::connect(actionService_company, SIGNAL(triggered()), navigation, SLOT(viewServiceCompany()));
    QObject::connect(actionBasic_logbook, SIGNAL(triggered()), navigation, SLOT(viewBasicLogbook()));
    QObject::connect(actionDetailed_logbook, SIGNAL(triggered()), navigation, SLOT(viewDetailedLogbook()));
    QObject::connect(actionPrinter_friendly_version, SIGNAL(triggered()), this, SLOT(refreshView()));
    QObject::connect(actionLock, SIGNAL(triggered()), this, SLOT(toggleLocked()));
    QObject::connect(actionReport_data, SIGNAL(triggered()), this, SLOT(reportData()));
    QObject::connect(actionModify_service_company_information, SIGNAL(triggered()), this, SLOT(modifyServiceCompany()));
    QObject::connect(actionAdd_record_of_refrigerant_management, SIGNAL(triggered()), this, SLOT(addRecordOfRefrigerantManagement()));
    QObject::connect(actionAdd_customer, SIGNAL(triggered()), this, SLOT(addCustomer()));
    QObject::connect(actionModify_customer, SIGNAL(triggered()), this, SLOT(modifyCustomer()));
    QObject::connect(actionRemove_customer, SIGNAL(triggered()), this, SLOT(removeCustomer()));
    QObject::connect(actionAdd_circuit, SIGNAL(triggered()), this, SLOT(addCircuit()));
    QObject::connect(actionModify_circuit, SIGNAL(triggered()), this, SLOT(modifyCircuit()));
    QObject::connect(actionRemove_circuit, SIGNAL(triggered()), this, SLOT(removeCircuit()));
    QObject::connect(actionAdd_inspection, SIGNAL(triggered()), this, SLOT(addInspection()));
    QObject::connect(actionModify_inspection, SIGNAL(triggered()), this, SLOT(modifyInspection()));
    QObject::connect(actionRemove_inspection, SIGNAL(triggered()), this, SLOT(removeInspection()));
    QObject::connect(actionAdd_repair, SIGNAL(triggered()), this, SLOT(addRepair()));
    QObject::connect(actionModify_repair, SIGNAL(triggered()), this, SLOT(modifyRepair()));
    QObject::connect(actionRemove_repair, SIGNAL(triggered()), this, SLOT(removeRepair()));
    QObject::connect(actionPrint_detailed_label, SIGNAL(triggered()), this, SLOT(printDetailedLabel()));
    QObject::connect(actionPrint_label, SIGNAL(triggered()), this, SLOT(printLabel()));
    QObject::connect(actionNew_variable, SIGNAL(triggered()), this, SLOT(addVariable()));
    QObject::connect(actionNew_subvariable, SIGNAL(triggered()), this, SLOT(addSubvariable()));
    QObject::connect(tbtn_modify_variable, SIGNAL(clicked()), this, SLOT(modifyVariable()));
    QObject::connect(tbtn_remove_variable, SIGNAL(clicked()), this, SLOT(removeVariable()));
    QObject::connect(tbtn_add_table, SIGNAL(clicked()), this, SLOT(addTable()));
    QObject::connect(tbtn_modify_table, SIGNAL(clicked()), this, SLOT(modifyTable()));
    QObject::connect(tbtn_remove_table, SIGNAL(clicked()), this, SLOT(removeTable()));
    QObject::connect(tbtn_table_add_variable, SIGNAL(clicked()), this, SLOT(addTableVariable()));
    QObject::connect(tbtn_table_remove_variable, SIGNAL(clicked()), this, SLOT(removeTableVariable()));
    QObject::connect(tbtn_table_move_up, SIGNAL(clicked()), this, SLOT(moveTableVariableUp()));
    QObject::connect(tbtn_table_move_down, SIGNAL(clicked()), this, SLOT(moveTableVariableDown()));
    QObject::connect(tbtn_add_warning, SIGNAL(clicked()), this, SLOT(addWarning()));
    QObject::connect(tbtn_modify_warning, SIGNAL(clicked()), this, SLOT(modifyWarning()));
    QObject::connect(tbtn_remove_warning, SIGNAL(clicked()), this, SLOT(removeWarning()));
    QObject::connect(actionAdd_inspector, SIGNAL(triggered()), this, SLOT(addInspector()));
    QObject::connect(actionModify_inspector, SIGNAL(triggered()), this, SLOT(modifyInspector()));
    QObject::connect(actionRemove_inspector, SIGNAL(triggered()), this, SLOT(removeInspector()));
    QObject::connect(actionExport_customer_data, SIGNAL(triggered()), this, SLOT(exportCustomerData()));
    QObject::connect(actionExport_circuit_data, SIGNAL(triggered()), this, SLOT(exportCircuitData()));
    QObject::connect(actionExport_inspection_data, SIGNAL(triggered()), this, SLOT(exportInspectionData()));
    QObject::connect(actionImport_data, SIGNAL(triggered()), this, SLOT(importData()));
    QObject::connect(actionCheck_for_updates, SIGNAL(triggered()), this, SLOT(checkForUpdates()));
    QObject::connect(lw_recent_docs, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(openRecent(QListWidgetItem *)));
    QObject::connect(trw_variables, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(modifyVariable()));
    QObject::connect(trw_variables, SIGNAL(itemSelectionChanged()), this, SLOT(enableTools()));
    QObject::connect(lbl_current_selection, SIGNAL(linkActivated(const QString &)), navigation, SLOT(setView(const QString &)));
    QObject::connect(lbl_selected_repair, SIGNAL(linkActivated(const QString &)), navigation, SLOT(setView(const QString &)));
    QObject::connect(lbl_selected_inspector, SIGNAL(linkActivated(const QString &)), navigation, SLOT(setView(const QString &)));
    QObject::connect(btn_clear_selection, SIGNAL(clicked()), this, SLOT(clearSelection()));
    QObject::connect(cb_table_edit, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(loadTable(const QString &)));
    QObject::connect(trw_table_variables, SIGNAL(itemSelectionChanged()), this, SLOT(enableTools()));
    QObject::connect(lw_warnings, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(modifyWarning()));
    QObject::connect(lw_warnings, SIGNAL(itemSelectionChanged()), this, SLOT(enableTools()));
    QObject::connect(wv_main, SIGNAL(linkClicked(const QUrl &)), this, SLOT(executeLink(const QUrl &)));
    QObject::connect(http, SIGNAL(done(bool)), this, SLOT(httpRequestFinished(bool)));
    MTWebPage * page = new MTWebPage(this);
    page->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    wv_main->setPage(page);
#ifdef Q_WS_MAC
    show();
#endif
    loadSettings();
    if (qApp->arguments().count() > 1) {
        openFile(qApp->arguments().at(1));
    }
#ifndef Q_WS_MAC
    show();
#endif
}

void MainWindow::openFile(const QString & file)
{
    QFileInfo file_info(file);
    if (file_info.exists() && !saveChangesBeforeProceeding(tr("Open database - Leaklog"), true)) {
        addRecent(file_info.absoluteFilePath());
        openDatabase(file_info.absoluteFilePath());
    }
}

QMenu * MainWindow::createPopupMenu()
{
    QMenu * popup_menu = this->QMainWindow::createPopupMenu();
    popup_menu->addSeparator();
    popup_menu->addAction(actionShow_icons_only);
    return popup_menu;
}

void MainWindow::showIconsOnly(bool show)
{
    Qt::ToolButtonStyle tbtn_style = show ? Qt::ToolButtonIconOnly : Qt::ToolButtonTextUnderIcon;
    tbtn_open->setToolButtonStyle(tbtn_style);
    tbtn_add->setToolButtonStyle(tbtn_style);
    tbtn_modify->setToolButtonStyle(tbtn_style);
    tbtn_export->setToolButtonStyle(tbtn_style);
    toolBar->setToolButtonStyle(tbtn_style);
}

void MainWindow::executeLink(const QUrl & url)
{
    QStringList path = url.toString().split("/");
    QString id;
    if (path.count() > 0) {
        if (path.at(0).startsWith("customer:")) {
            id = path.at(0);
            id.remove(0, QString("customer:").length());
            if (id != toString(selectedCustomer())) {
                loadCustomer(id.toInt(), path.count() <= 1);
            } else if (path.count() <= 1) { navigation->setView(Navigation::ListOfCircuits); }
        } else if (path.at(0).startsWith("repair:")) {
            id = path.at(0);
            id.remove(0, QString("repair:").length());
            loadRepair(id, path.count() <= 1);
        } else if (path.at(0).startsWith("inspector:")) {
            id = path.at(0);
            id.remove(0, QString("inspector:").length());
            loadInspector(id.toInt(), path.count() <= 1);
        } else if (path.at(0).startsWith("allcustomers:")) {
            navigation->setView(Navigation::ListOfCustomers);
        } else if (path.at(0).startsWith("toggledetailedview:")) {
            id = path.at(0);
            id.remove(0, QString("toggledetailedview:").length());
            if (id == "leakedinstore") {
                show_leaked_in_store_in_service_company_view = !show_leaked_in_store_in_service_company_view;
            } else {
                if (years_expanded_in_service_company_view.contains(id.toInt())) {
                    years_expanded_in_service_company_view.remove(id.toInt());
                } else {
                    years_expanded_in_service_company_view << id.toInt();
                }
            }
            refreshView();
        } else if (path.at(0).startsWith("recordofrefrigerantmanagement:")) {
            id = path.at(0);
            id.remove(0, QString("recordofrefrigerantmanagement:").length());
        }
    }
    if (path.count() > 1) {
        if (path.at(1).startsWith("circuit:")) {
            id = path.at(1);
            id.remove(0, QString("circuit:").length());
            if (id != toString(selectedCircuit())) {
                loadCircuit(id.toInt(), path.count() <= 2);
            } else if (path.count() <= 2) { navigation->setView(Navigation::ListOfInspections); }
        } else if (path.at(1).startsWith("modify")) {
            if (path.at(0).startsWith("customer:")) { modifyCustomer(); }
            else if (path.at(0).startsWith("repair:")) { modifyRepair(); }
            else if (path.at(0).startsWith("inspector:")) { modifyInspector(); }
            else if (path.at(0).startsWith("servicecompany:")) { modifyServiceCompany(); }
            else if (path.at(0).startsWith("recordofrefrigerantmanagement:")) { modifyRecordOfRefrigerantManagement(id); }
        }
    }
    if (path.count() > 2) {
        if (path.at(2).startsWith("inspection:") || path.at(2).startsWith("repair:")) {
            id = path.at(2);
            id.remove(0, id.indexOf(":") + 1);
            if (id != selectedInspection()) {
                loadInspection(id, path.count() <= 3);
            } else if (path.count() <= 3) { navigation->setView(Navigation::Inspection); }
        } else if (path.at(2).startsWith("table")) {
            navigation->setView(Navigation::TableOfInspections);
        } else if (path.at(2).startsWith("modify")) { modifyCircuit(); }
    }
    if (path.count() > 3) {
        if (path.at(3).startsWith("modify")) { modifyInspection(); }
    }
}

void MainWindow::printPreview()
{
    QPrintPreviewDialog * d = new QPrintPreviewDialog(this);
    connect(d, SIGNAL(paintRequested(QPrinter *)), wv_main, SLOT(print(QPrinter *)));
    d->exec();
}

void MainWindow::print()
{
    QPrinter printer;
    QPrintDialog * d = new QPrintDialog(&printer, this);
    d->setWindowTitle(tr("Print"));
    if (d->exec() != QDialog::Accepted) { return; }
    wv_main->print(&printer);
}

void MainWindow::exportPDF()
{
    QString path = QFileDialog::getSaveFileName(this, tr("Export PDF - Leaklog"), QString("%1-%2.pdf").arg(QFileInfo(db.databaseName()).baseName()).arg(navigation->currentView()), tr("Adobe PDF (*.pdf)"));
    if (path.isEmpty()) { return; }
    if (!path.endsWith(".pdf", Qt::CaseInsensitive)) { path.append(".pdf"); }
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(path);
    wv_main->print(&printer);
}

void MainWindow::exportHTML()
{
    QString path = QFileDialog::getSaveFileName(this, tr("Export HTML - Leaklog"), QString("%1-%2.html").arg(QFileInfo(db.databaseName()).baseName()).arg(navigation->currentView()), tr("Webpage (*.html)"));
    if (path.isEmpty()) { return; }
    if (!path.endsWith(".html", Qt::CaseInsensitive)) { path.append(".html"); }
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, tr("Export HTML - Leaklog"), tr("Cannot write file %1:\n%2.").arg(path).arg(file.errorString()));
        return;
    }
    QString html = viewChanged(navigation->view());
    if (html.contains("<link href=\"default.css\" rel=\"stylesheet\" type=\"text/css\" />")) {
        QFile default_css(":/html/default.css");
        default_css.open(QIODevice::ReadOnly | QIODevice::Text);
        QTextStream in(&default_css); in.setCodec("UTF-8");
        html.replace("<link href=\"default.css\" rel=\"stylesheet\" type=\"text/css\" />", QString("<style type=\"text/css\">\n<!--\n%1\n-->\n</style>").arg(in.readAll()));
        default_css.close();
    }
    if (html.contains("<link href=\"colours.css\" rel=\"stylesheet\" type=\"text/css\" />")) {
        QFile colours_css(":/html/colours.css");
        colours_css.open(QIODevice::ReadOnly | QIODevice::Text);
        QTextStream in(&colours_css); in.setCodec("UTF-8");
        html.replace("<link href=\"colours.css\" rel=\"stylesheet\" type=\"text/css\" />", QString("<style type=\"text/css\">\n<!--\n%1\n-->\n</style>").arg(in.readAll()));
        colours_css.close();
    }
    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << html;
    file.close();
}

void MainWindow::printDetailedLabel()
{
    printLabel(true);
}

void MainWindow::printLabel(bool detailed)
{
    if (!db.isOpen()) { return; }
    if (detailed && selectedCustomer() < 0) { return; }
    if (detailed && selectedCircuit() < 0) { return; }
    if (!detailed && selectedInspector() < 0) { return; }

    QMap<QString, QCheckBox *> label_positions;
    QDialog * d = new QDialog(this);
        d->setWindowTitle(detailed ? tr("Print detailed label - Leaklog") : tr("Print label - Leaklog"));
        QGridLayout * gl = new QGridLayout(d);
            QLabel * lbl_print_labels = new QLabel(tr("Choose the position of the label on the paper:"), d);
        gl->addWidget(lbl_print_labels, 0, 0, 1, 2);
            for (int c = 0; c < 2; ++c) {
                for (int r = 0; r < 4; ++r) {
                    QCheckBox * chb = new QCheckBox(d);
                    chb->setText(tr("Row %1 Column %2").arg(r + 1).arg(c + 1));
                    label_positions.insert(QString("%1;%2").arg(r).arg(c), chb);
                    gl->addWidget(chb, r + 1, c);
                }
            }
            QDialogButtonBox * bb = new QDialogButtonBox(d);
            bb->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
            QObject::connect(bb, SIGNAL(accepted()), d, SLOT(accept()));
            QObject::connect(bb, SIGNAL(rejected()), d, SLOT(reject()));
        gl->addWidget(bb, 5, 0, 1, 2);
    if (d->exec() != QDialog::Accepted) { return; }
    bool ok = false;
    QMapIterator<QString, QCheckBox *> iterator(label_positions);
    while (iterator.hasNext()) { iterator.next();
        if (iterator.value()->isChecked()) { ok = true; break; }
    }
    if (!ok) { return; }

    QString selected_inspector = toString(selectedInspector());
    StringVariantMap attributes;
    if (detailed) {
        attributes.insert("circuit_id", toString(selectedCustomer()).rightJustified(8, '0') + "." + toString(selectedCircuit()).rightJustified(4, '0'));
        QSqlQuery query;
        query.prepare("SELECT * FROM inspections WHERE customer = :customer_id AND circuit = :circuit_id ORDER BY date DESC");
        query.bindValue(":customer_id", selectedCustomer());
        query.bindValue(":circuit_id", selectedCircuit());
        query.exec();
        if (query.next()) {
            StringVariantMap inspection;
            for (int i = 0; i < query.record().count(); ++i) {
                inspection.insert(query.record().fieldName(i), query.value(i));
            }
            attributes.insert("date", inspection.value("date").toString());
            selected_inspector = inspection.value("inspector").toString();
            Subvariable refr_add_per("refr_add", "refr_add_per");
            refr_add_per.next();
            QString unparsed_expression = refr_add_per.value("SUBVAR_VALUE").toString();
            if (!unparsed_expression.isEmpty()) {
                QStringList var_ids = listVariableIds();
                attributes.insert("refr_add_per", evaluateExpression(inspection, parseExpression(unparsed_expression, var_ids), toString(selectedCustomer()), toString(selectedCircuit())));
            }
        }
        Circuit circuit(toString(selectedCustomer()), toString(selectedCircuit()));
        attributes.unite(circuit.list("refrigerant, refrigerant_amount"));
    }
    Inspector inspector(selected_inspector);
    if (inspector.exists())
        attributes.unite(inspector.list("person, person_reg_num"));
    ServiceCompany service_company(DBInfoValueForKey("default_service_company"));
    if (service_company.exists())
        attributes.unite(service_company.list("id, name, address, mail, phone"));

    QPrinter * printer = new QPrinter(QPrinter::HighResolution);
    QPrintDialog * dialogue = new QPrintDialog(printer, this);
    dialogue->setWindowTitle(tr("Print label"));
    if (dialogue->exec() != QDialog::Accepted) return;
    printer->setOrientation(QPrinter::Portrait);
    printer->setFullPage(true);

    QPainter painter;
    painter.setRenderHint(QPainter::Antialiasing);
    painter.begin(printer);
    QRect rect = printer->pageRect();
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

    delete printer;
}

void MainWindow::paintLabel(const StringVariantMap & attributes, QPainter & painter, int x, int y, int w, int h)
{
    bool detailed = attributes.contains("circuit_id");
    painter.save();
    QPen pen; pen.setWidthF(7.0); painter.setPen(pen);
    QFont font; painter.setFont(font);
#ifdef Q_WS_MAC
    font.setPointSize(font.pointSize() - 7);
#else
    font.setPointSize(font.pointSize() - 2);
#endif
    painter.setFont(font);
    int title_h = 3 * h / 14; int m = w / 75; int dm = m * 2;
    painter.drawRect(x, y, w, h);
    painter.drawLine(x, y + title_h, x + w, y + title_h);
    painter.drawLine(x + (w / 2), y, x + (w / 2), y + title_h);
    painter.drawText(m + x, m + y, w / 2 - dm, (title_h - dm) / 3, Qt::AlignLeft | Qt::AlignVCenter, tr("Certified company"));
    painter.drawText(m + x, m + y + 2 * (title_h - dm) / 3, w / 2 - dm, (title_h - dm) / 3, Qt::AlignLeft | Qt::AlignVCenter, MTAddress(attributes.value("address").toString()).toPlainText());
    painter.drawText(m + x, m + y + title_h, w / 3 - dm, h / 14 - m, Qt::AlignCenter, detailed ? tr("Circuit ID") : tr("3(6) - <30 kg"));
    painter.drawLine(x + (w / 3), y + title_h, x + (w / 3), y + h);
    painter.drawText(m + x + (w / 3), m + y + title_h, w / 3 - dm, h / 14 - m, Qt::AlignCenter, detailed ? tr("Refrigerant") : tr("30 - <300 kg"));
    painter.drawLine(x + (2 * w / 3), y + title_h, x + (2 * w / 3), y + h);
    painter.drawText(m + x + (2 * w / 3), m + y + title_h, w / 3 - dm, h / 14 - m, Qt::AlignCenter, detailed ? tr("Average leakage") : tr("above 300 kg"));
    painter.drawLine(x, y + title_h + (h / 7), x + w, y + title_h + (h / 7));
    painter.drawText(m + x, m + y + title_h + (h / 7), w / 3 - dm, 9 * h / 14 - dm, Qt::AlignLeft, tr("Date of inspection"));
    painter.drawText(m + x + (2 * w / 3), m + y + title_h + (h / 7), w / 3 - dm, 9 * h / 14 - dm, Qt::AlignLeft, tr("Date of the next inspection"));
    painter.drawLine(x + (w / 3), y + title_h + (2 * h / 7), x + (2 * w / 3), y + title_h + (2 * h / 7));
    painter.drawLine(x + (w / 3), y + title_h + (3 * h / 7), x + (2 * w / 3), y + title_h + (3 * h / 7));
    painter.drawLine(x + (w / 3), y + title_h + (4 * h / 7), x + (2 * w / 3), y + title_h + (4 * h / 7));
    painter.drawLine(x + (w / 3), y + title_h + (5 * h / 7), x + (2 * w / 3), y + title_h + (5 * h / 7));
    painter.drawText(m + x + (w / 3), y + title_h + (5 * h / 7), w / 6 - dm, h / 14, Qt::AlignCenter, attributes.value("person_reg_num").toString());
    painter.drawLine(x + (w / 2), y + title_h + (5 * h / 7), x + (w / 2), y + h);
    painter.drawText(m + x + (w / 2), y + title_h + (5 * h / 7), w / 6 - dm, h / 14, Qt::AlignCenter, attributes.value("id").toString());
    font.setBold(true); painter.setFont(font);
    painter.drawText(m + x, m + y + (title_h - dm) / 3, w / 2 - dm, (title_h - dm) / 3, Qt::AlignLeft | Qt::AlignVCenter, attributes.value("name").toString());
    painter.drawText(m + x + (w / 3), y + title_h + (h / 7) + h / 14, w / 3 - dm, h / 14 - m, Qt::AlignCenter, attributes.value("person").toString());
    font.setBold(false); painter.setFont(font);
    painter.drawText(m + x + (w / 2), m + y, w / 2 - dm, title_h - dm, Qt::AlignCenter, tr("Refrigerant leakage inspection\nin accordance with Regulation (EC)\nNo. 842/2006"));
    painter.drawText(m + x, y + title_h + h / 14, w / 3 - dm, h / 14 - m, Qt::AlignCenter, !detailed ? tr("once a year*") :
                     attributes.value("circuit_id").toString());
    painter.drawText(m + x + (w / 3), y + title_h + h / 14, w / 3 - dm, h / 14 - m, Qt::AlignCenter, !detailed ? tr("once in 6 months*") :
                     (attributes.value("refrigerant_amount").toString() + " " + tr("kg") + " " + attributes.value("refrigerant").toString()));
    painter.drawText(m + x + (2 * w / 3), y + title_h + h / 14, w / 3 - dm, h / 14 - m, Qt::AlignCenter, !detailed ? tr("once in 3 months") :
                     (attributes.value("refr_add_per").toString() + " " + tr("%")));
    painter.drawText(m + x + (w / 3), m + y + title_h + (h / 7), w / 3 - dm, h / 14 - m, Qt::AlignCenter, tr("Certified person"));
    painter.drawText(m + x + (w / 3), m + y + title_h + (2 * h / 7), w / 3 - dm, h / 14 - m, Qt::AlignCenter, tr("Telephone"));
    painter.drawText(m + x + (w / 3), y + title_h + (2 * h / 7) + h / 14, w / 3 - dm, h / 14 - m, Qt::AlignCenter, attributes.value("phone").toString());
    painter.drawText(m + x + (w / 3), m + y + title_h + (3 * h / 7), w / 3 - dm, h / 14 - m, Qt::AlignCenter, tr("E-mail"));
    painter.drawText(m + x + (w / 3), y + title_h + (3 * h / 7) + h / 14, w / 3 - dm, h / 14 - m, Qt::AlignCenter, attributes.value("mail").toString());
    painter.drawText(m + x + (w / 3), m + y + title_h + (4 * h / 7), w / 3 - dm, h / 7 - dm, Qt::AlignCenter, tr("Registry number of\nperson and company ID"));
    painter.restore(); painter.save(); painter.setPen(pen);
    int r = (w / 3 - dm) / 2;
    int c_y = y + title_h + (4 * h / 7) - dm;
    QRect circle1o(m + x, c_y - r, 2 * r, 2 * r);
    painter.drawEllipse(circle1o);
    QRect circle1i(m + x + 2 * dm, c_y - r + 2 * dm, 2 * r - 4 * dm, 2 * r - 4 * dm);
    painter.drawEllipse(circle1i);
    QRect circle2o(m + x + 2 * w / 3, c_y - r, 2 * r, 2 * r);
    painter.drawEllipse(circle2o);
    QRect circle2i(m + x + 2 * w / 3 + 2 * dm, c_y - r + 2 * dm, 2 * r - 4 * dm, 2 * r - 4 * dm);
    painter.drawEllipse(circle2i);
    painter.translate(m + x + r, c_y);
    for (int i = 0; i < 12; ++i) {
        painter.drawLine(0, - r, 0, 2 * dm - r);
        painter.rotate(30.0);
    }
    painter.restore(); painter.save(); painter.setPen(pen);
    painter.translate(m + x + 2 * w / 3 + r, c_y);
    for (int i = 0; i < 12; ++i) {
        painter.drawLine(0, - r, 0, 2 * dm - r);
        painter.rotate(30.0);
    }
    painter.restore();
}

void MainWindow::reportData()
{
    setAllEnabled(false, true);
    ReportDataController * controller = new ReportDataController(wv_main, navigation);
    QObject::connect(controller, SIGNAL(processing(bool)), this, SLOT(setDisabled(bool)));
    QObject::connect(controller, SIGNAL(destroyed()), this, SLOT(reportDataFinished()));
}

void MainWindow::reportDataFinished()
{
    setAllEnabled(true, true);
}

void MainWindow::find()
{
    if (!db.isOpen()) { return; }
    bool ok;
    QString keyword = QInputDialog::getText(this, tr("Find - Leaklog"), tr("Find:"), QLineEdit::Normal, last_search_keyword, &ok);
    if (ok && !keyword.isEmpty()) {
        last_search_keyword = keyword;
        wv_main->findText(last_search_keyword);
    }
}

void MainWindow::findNext()
{
    if (!db.isOpen()) { return; }
    if (last_search_keyword.isEmpty()) { return; }
    wv_main->findText(last_search_keyword);
}

void MainWindow::findPrevious()
{
    if (!db.isOpen()) { return; }
    if (last_search_keyword.isEmpty()) { return; }
    wv_main->findText(last_search_keyword, QWebPage::FindBackward);
}

void MainWindow::clearSelection(bool refresh)
{
    selected_customer = -1;
    selected_customer_company.clear();
    selected_circuit = -1;
    selected_inspection.clear();
    selected_inspection_is_repair = false;
    selected_repair.clear();
    selected_inspector = -1;
    selected_inspector_name.clear();
    if (refresh) {
        enableTools();
        refreshView();
    }
}

void MainWindow::refreshView()
{
    viewChanged(navigation->view());
}

void MainWindow::addRecent(const QString & name)
{
    for (int i = 0; i < lw_recent_docs->count();) {
        if (lw_recent_docs->item(i)->text() == name) {
            delete lw_recent_docs->item(i);
        } else { i++; }
    }
    lw_recent_docs->insertItem(0, name);
    lw_recent_docs->setCurrentRow(0);
}

void MainWindow::clearAll()
{
    clearSelection(false);
    navigation->tableComboBox()->clear();
    cb_table_edit->clear();
    trw_variables->clear();
    trw_table_variables->clear();
    lw_warnings->clear();
    actionService_company->setChecked(true);
    navigation->viewServiceCompany();
    navigation->restoreDefaults();
    // ----
    years_expanded_in_service_company_view.clear();
}

void MainWindow::setAllEnabled(bool enable, bool everything)
{
    if (everything) {
        actionNew->setEnabled(enable);
        actionOpen->setEnabled(enable);
        actionLocal_database->setEnabled(enable);
        actionRemote_database->setEnabled(enable);
        actionPrinter_friendly_version->setEnabled(enable);
    }

    actionSave->setEnabled(enable);
    actionSave_and_compact->setEnabled(enable);
    actionClose->setEnabled(enable);
    actionImport_data->setEnabled(enable);
    actionExport->setEnabled(enable || everything);
    actionPDF->setEnabled(enable || everything);
    actionHTML->setEnabled(enable || everything);
    actionPrint_preview->setEnabled(enable || everything);
    actionPrint->setEnabled(enable || everything);
    actgrp_view->setEnabled(enable);

    menuDatabase->setEnabled(enable);
    menuCustomer->setEnabled(enable);
    menuCooling_circuit->setEnabled(enable);
    menuInspection->setEnabled(enable);
    menuInspector->setEnabled(enable);

    tbtn_add->setEnabled(enable);
    tbtn_modify->setEnabled(enable);
    tbtn_export->setEnabled(enable || everything);

    actionFind->setEnabled(enable);
    actionFind_next->setEnabled(enable);
    actionFind_previous->setEnabled(enable);

    actionLock->setEnabled(enable);

    actionReport_data->setEnabled(enable);
    actionModify_service_company_information->setEnabled(enable);
    actionAdd_record_of_refrigerant_management->setEnabled(enable);

    tbtn_add_table->setEnabled(enable);
    tbtn_add_warning->setEnabled(enable);

    actionAdd_customer->setEnabled(enable);
    actionAdd_repair->setEnabled(enable);
    actionAdd_inspector->setEnabled(enable);
    if (!enable) {
    // menuCustomer
        actionModify_customer->setEnabled(enable);
        actionRemove_customer->setEnabled(enable);
    // menuCooling_circuit
        actionAdd_circuit->setEnabled(enable);
        actionModify_circuit->setEnabled(enable);
        actionRemove_circuit->setEnabled(enable);
    // menuInspection
        actionAdd_inspection->setEnabled(enable);
        actionModify_inspection->setEnabled(enable);
        actionRemove_inspection->setEnabled(enable);
        actionPrint_label->setEnabled(enable);
    // menuRepair
        actionModify_repair->setEnabled(enable);
        actionRemove_repair->setEnabled(enable);
    // menuInspector
        actionModify_inspector->setEnabled(enable);
        actionRemove_inspector->setEnabled(enable);
    }
    dw_variables->setEnabled(enable);
    dw_tables->setEnabled(enable);
    dw_warnings->setEnabled(enable);
}

void MainWindow::updateLockButton()
{
    if (database_locked) {
        actionLock->setIcon(QIcon::QIcon(":/images/images/locked.png"));
        actionLock->setText(tr("Unlock"));
    } else {
        actionLock->setIcon(QIcon::QIcon(":/images/images/unlocked.png"));
        actionLock->setText(tr("Lock"));
    }
}

bool MainWindow::isRecordLocked(const QString & date)
{
    if (!database_locked) return false;
    return date < database_lock_date;
}

void MainWindow::enableTools()
{
    bool customer_selected = selected_customer >= 0;
    bool circuit_selected = selected_circuit >= 0;
    bool inspection_selected = !selected_inspection.isEmpty();
    bool repair_selected = !selected_repair.isEmpty();
    bool record_locked = false;
    if (database_locked && (inspection_selected || repair_selected)) {
        QString date = inspection_selected ? selected_inspection : selected_repair;
        if (date < database_lock_date)
            record_locked = true;
    }
    bool inspector_selected = selected_inspector >= 0;
    QString current_selection;
    if (customer_selected)
        current_selection.append(QString("<a style=\"color: #000000; text-decoration: none;\" href=\"%1\">%2</a>")
            .arg(Navigation::ListOfCircuits)
            .arg(tr("<b>Customer:</b> %1%2")
                .arg(selected_customer)
                .arg(selected_customer_company.isEmpty() ? "" : QString(" (%1)").arg(selected_customer_company))));
    if (circuit_selected)
        current_selection.append(QString(" &gt; <a style=\"color: #000000; text-decoration: none;\" href=\"%1\">%2</a>")
            .arg(Navigation::ListOfInspections)
            .arg(tr("<b>Circuit:</b> %1")
                .arg(selected_circuit)));
    if (inspection_selected) {
        current_selection.append(QString(" &gt; <a style=\"color: #000000; text-decoration: none;\" href=\"%1\">%2</a>")
            .arg(Navigation::Inspection)
            .arg((selected_inspection_is_repair ? tr("<b>Repair:</b> %1") : tr("<b>Inspection:</b> %1"))
                .arg(selected_inspection)));
    }
    lbl_current_selection->setText(current_selection);
    lbl_current_selection->setVisible(!current_selection.isEmpty());
    if (repair_selected) {
        lbl_selected_repair->setText(QString("<a style=\"color: #000000; text-decoration: none;\" href=\"%1\">%2</a>")
            .arg(Navigation::ListOfRepairs)
            .arg(tr("<b>Repair:</b> %1")
                .arg(selectedRepair())));
    }
    lbl_selected_repair->setVisible(repair_selected);
    if (inspector_selected) {
        lbl_selected_inspector->setText(QString("<a style=\"color: #000000; text-decoration: none;\" href=\"%1\">%2</a>")
            .arg(Navigation::ListOfInspectors)
            .arg(tr("<b>Inspector:</b> %1")
                .arg(selected_inspector_name)));
    }
    lbl_selected_inspector->setVisible(inspector_selected);
    btn_clear_selection->setVisible(!current_selection.isEmpty() || repair_selected || inspector_selected);
    navigation->enableTools(customer_selected, circuit_selected, inspection_selected, repair_selected, inspector_selected);
    actionModify_customer->setEnabled(customer_selected);
    actionRemove_customer->setEnabled(customer_selected && !database_locked);
    actionExport_customer_data->setEnabled(customer_selected);
    actionAdd_circuit->setEnabled(customer_selected);
    actionModify_circuit->setEnabled(circuit_selected);
    actionRemove_circuit->setEnabled(circuit_selected && !database_locked);
    actionExport_circuit_data->setEnabled(circuit_selected);
    actionAdd_inspection->setEnabled(circuit_selected);
    actionModify_inspection->setEnabled(inspection_selected && !record_locked);
    actionRemove_inspection->setEnabled(inspection_selected && !record_locked);
    actionModify_repair->setEnabled(repair_selected && !record_locked);
    actionRemove_repair->setEnabled(repair_selected && !record_locked);
    actionPrint_detailed_label->setEnabled(circuit_selected);
    actionPrint_label->setEnabled(inspector_selected);
    actionExport_inspection_data->setEnabled(inspection_selected);
    actionNew_subvariable->setEnabled(trw_variables->currentIndex().isValid() && trw_variables->currentItem()->parent() == NULL && !dict_varnames.contains(trw_variables->currentItem()->text(1)));
    tbtn_modify_variable->setEnabled(trw_variables->currentIndex().isValid());
    tbtn_remove_variable->setEnabled(trw_variables->currentIndex().isValid() && !dict_varnames.contains(trw_variables->currentItem()->text(1)) && !database_locked);
    tbtn_modify_table->setEnabled(cb_table_edit->currentIndex() >= 0);
    tbtn_remove_table->setEnabled(cb_table_edit->currentIndex() >= 0 && !database_locked);
    tbtn_table_add_variable->setEnabled(cb_table_edit->currentIndex() >= 0);
    tbtn_table_remove_variable->setEnabled(trw_table_variables->currentIndex().isValid());
    tbtn_table_move_up->setEnabled(trw_table_variables->currentIndex().isValid());
    tbtn_table_move_down->setEnabled(trw_table_variables->currentIndex().isValid());
    tbtn_modify_warning->setEnabled(lw_warnings->currentIndex().isValid());
    tbtn_remove_warning->setEnabled(lw_warnings->currentIndex().isValid() && lw_warnings->currentItem()->data(Qt::UserRole).toInt() < 1000 && !database_locked);
    actionModify_inspector->setEnabled(inspector_selected);
    actionRemove_inspector->setEnabled(inspector_selected && !database_locked);
}

void MainWindow::toggleLocked()
{
    if (actionLock->text() == tr("Lock")) {
        QDialog * d = new QDialog(this);
        d->setWindowTitle(tr("Lock database - Leaklog"));
        QGridLayout * gl = new QGridLayout(d);

        QLabel * lbl = new QLabel(tr("Lock inspections and repairs older than:"), d);
        lbl->setAlignment(Qt::AlignBottom | Qt::AlignLeft);
        gl->addWidget(lbl, 0, 0, 1, 2);

        QString last_date = DBInfoValueForKey("lock_date");

        QDateEdit * date = new QDateEdit(d);
        date->setDisplayFormat("yyyy.MM.dd");
        date->setDate(last_date.isEmpty() ? QDate::currentDate() : QDate::fromString(last_date, "yyyy.MM.dd"));
        gl->addWidget(date, 1, 1);

        lbl = new QLabel(tr("Password:"), d);
        lbl->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
        gl->addWidget(lbl, 2, 0);

        QLineEdit * password = new QLineEdit(d);
        gl->addWidget(password, 2, 1);

        QDialogButtonBox * bb = new QDialogButtonBox(d);
        bb->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        bb->button(QDialogButtonBox::Ok)->setText(tr("Lock"));
        QObject::connect(bb, SIGNAL(accepted()), d, SLOT(accept()));
        QObject::connect(bb, SIGNAL(rejected()), d, SLOT(reject()));
        gl->addWidget(bb, 3, 0, 1, 2);

        if (d->exec() != QDialog::Accepted) return;

        database_lock_date = date->date().toString("yyyy.MM.dd");
        setDBInfoValueForKey("lock_date", database_lock_date);
        setDBInfoValueForKey("lock_password", sha256(password->text()));
        setDBInfoValueForKey("locked", "true");
        database_locked = true;
        updateLockButton();
        enableTools();
        this->setWindowModified(true);
        refreshView();
    } else {
        bool ok;
        QString password = QInputDialog::getText(this, tr("Unlock database - Leaklog"),
                                                 tr("Password:"), QLineEdit::Password,
                                                 "", &ok);
        if (!ok) return;

        if (sha256(password) != DBInfoValueForKey("lock_password")) {
            QMessageBox::warning(this, tr("Unlock database - Leaklog"), tr("Wrong password."));
            return;
        }

        setDBInfoValueForKey("locked", "false");
        database_locked = false;
        updateLockButton();
        enableTools();
        this->setWindowModified(true);
        refreshView();
    }
}

void MainWindow::closeEvent(QCloseEvent * event)
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
    this->move(settings.value("pos", this->pos()).toPoint());
    this->resize(settings.value("size", this->size()).toSize());
    this->restoreState(settings.value("window_state").toByteArray(), 0);
    actionShow_icons_only->setChecked(settings.value("toolbar_icons_only", false).toBool());
    showIconsOnly(actionShow_icons_only->isChecked());
}

void MainWindow::saveSettings()
{
    QSettings settings("SZCHKT", "Leaklog");
    QStringList recent;
    for (int i = 0; i < lw_recent_docs->count(); ++i)
        recent << lw_recent_docs->item(i)->text();
    settings.setValue("recent_docs", recent);
    settings.setValue("pos", this->pos());
    settings.setValue("size", this->size());
    settings.setValue("window_state", this->saveState(0));
    settings.setValue("toolbar_icons_only", actionShow_icons_only->isChecked());
}

void MainWindow::changeLanguage()
{
    QWidget * w_lang = new QWidget(this, Qt::Dialog);
    w_lang->setWindowModality(Qt::WindowModal);
    w_lang->setAttribute(Qt::WA_DeleteOnClose);
#ifdef Q_WS_MAC
    w_lang->setWindowTitle(tr("Change language"));
#else
    w_lang->setWindowTitle(tr("Change language - Leaklog"));
#endif
    QGridLayout * glayout_lang = new QGridLayout(w_lang);
    glayout_lang->setMargin(6); glayout_lang->setSpacing(6);
    QLabel * lbl_lang = new QLabel(w_lang);
    lbl_lang->setText(tr("Select your preferred language"));
    glayout_lang->addWidget(lbl_lang, 0, 0);
    cb_lang = new QComboBox(w_lang);
    QStringList langs(leaklog_i18n.keys()); langs.sort();
    for (int i = 0; i < langs.count(); ++i) {
        cb_lang->addItem(langs.at(i));
        if (langs.at(i) == "English") { cb_lang->setCurrentIndex(i); }
    }
    glayout_lang->addWidget(cb_lang, 1, 0);
    QDialogButtonBox * bb_lang = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, w_lang);
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
        QMessageBox::information(this, tr("Leaklog"), tr("You need to restart Leaklog for the changes to apply."));
    }
    if (cb_lang->parent() == NULL) { return; }
    QWidget * w_lang = (QWidget *)cb_lang->parent();
    w_lang->close();
    cb_lang = NULL;
}

void MainWindow::checkForUpdates()
{
    delete http_buffer; http_buffer = new QBuffer(this);
    http->setHost("leaklog.sourceforge.net");
    http->get("/current-version", http_buffer);
}

void MainWindow::httpRequestFinished(bool error)
{
    httpRequestFinished_start:
    if (error) {
        switch (QMessageBox::critical(this, tr("Leaklog"), tr("Failed to check for updates."), tr("&Try again"), tr("Cancel"), 0, 1)) {
            case 0: // Try again
                checkForUpdates(); return; break;
            case 1: // Cancel
                return; break;
        }
    }
    QString str(http_buffer->data()); QTextStream in(&str);
    if (in.readLine() != "[Leaklog.current-version]") { error = true; goto httpRequestFinished_start; }
    QString current_ver = in.readLine();
    if (in.readLine() != "[Leaklog.current-version.float]") { error = true; goto httpRequestFinished_start; }
    double f_current_ver = in.readLine().toDouble();
    if (in.readLine() != "[Leaklog.download-url.src]") { error = true; goto httpRequestFinished_start; }
    QString src_url = in.readLine();
    if (in.readLine() != "[Leaklog.download-url.macx]") { error = true; goto httpRequestFinished_start; }
#ifdef Q_WS_MAC
    QString macx_url = in.readLine();
#else
    in.readLine();
#endif
    if (in.readLine() != "[Leaklog.download-url.win32]") { error = true; goto httpRequestFinished_start; }
#ifdef Q_WS_WIN
    QString win32_url = in.readLine();
#else
    in.readLine();
#endif
    if (in.readLine() != "[Leaklog.release-notes]") { error = true; goto httpRequestFinished_start; }
    QString release_notes;
    while (!in.atEnd()) { release_notes.append(in.readLine()); }
    if (f_current_ver <= F_LEAKLOG_VERSION) {
        QMessageBox::information(this, tr("Leaklog"), tr("You are running the latest version of Leaklog."));
    } else {
        QString info; QTextStream out(&info);
        out << "<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">p, li { white-space: pre-wrap; }</style></head><body><p>" << endl;
        out << "<b>" << tr("Leaklog %1 is available now.").arg(current_ver) << "</b><br><br>" << endl;
        out << release_notes << endl << "<br><br>" << endl;
#ifdef Q_WS_MAC
        out << "<a href=\"" << macx_url << "\">" << tr("Download Leaklog %1 for Mac OS X").arg(current_ver) << "</a><br>" << endl;
#elif defined Q_WS_WIN
        out << "<a href=\"" << win32_url << "\">" << tr("Download Leaklog %1 for Windows").arg(current_ver) << "</a><br>" << endl;
#endif
        out << "<a href=\"" << src_url << "\">" << tr("Download source code") << "</a>" << endl;
        out << "</p></body></html>";
        QMessageBox::information(this, tr("Leaklog"), info);
    }
}

void MainWindow::about()
{
    AboutWidget * leaklog_about = new AboutWidget;
    leaklog_about->setParent(this);
    leaklog_about->setWindowFlags(Qt::Dialog /*| Qt::WindowMaximizeButtonHint*/ | Qt::WindowStaysOnTopHint);
    leaklog_about->show();
}
