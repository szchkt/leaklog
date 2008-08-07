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

#include "main_window.h"

QString escapeDoubleQuotes(const QString & s)
{
    QString r = s;
    r.replace("\\", "\\\\");
    r.replace("\"", "\\\"");
    return r;
}

MainWindow::MainWindow()
{
    leaklog_version = "0.9.0";
    f_leaklog_version = 0.9;
    // Dictionaries
    dict_vartypes.insert("int", tr("Integer"));
    dict_vartypes.insert("float", tr("Real number"));
    dict_vartypes.insert("string", tr("String"));
    // ------------
    // Queries
    QFile file; QTextStream in(&file); in.setCodec("UTF-8");
    file.setFileName(":/queries/all_customers.xq"); file.open(QIODevice::ReadOnly | QIODevice::Text);
    dict_queries.insert(tr("All customers"), in.readAll());
    file.close();
    file.setFileName(":/queries/single_customer.xq"); file.open(QIODevice::ReadOnly | QIODevice::Text);
    dict_queries.insert(tr("Customer information"), in.readAll());
    file.close();
    file.setFileName(":/queries/single_circuit.xq"); file.open(QIODevice::ReadOnly | QIODevice::Text);
    dict_queries.insert(tr("Circuit information"), in.readAll());
    file.close();
    file.setFileName(":/queries/single_inspection.xq"); file.open(QIODevice::ReadOnly | QIODevice::Text);
    dict_queries.insert(tr("Inspection information"), in.readAll());
    file.close();
    file.setFileName(":/queries/table.xq"); file.open(QIODevice::ReadOnly | QIODevice::Text);
    dict_queries.insert(tr("Table of inspections"), in.readAll());
    file.close();
    // -------
    // i18n -> JavaScript
    dict_i18n_javascript.append("function Dictionary(startValues) {\n");
    dict_i18n_javascript.append("    this.values = startValues || {};\n");
    dict_i18n_javascript.append("}\n");
    dict_i18n_javascript.append("Dictionary.prototype.add = function(name, value) {\n");
    dict_i18n_javascript.append("    this.values[name] = value;\n");
    dict_i18n_javascript.append("};\n");
    dict_i18n_javascript.append("Dictionary.prototype.value = function(name) {\n");
    dict_i18n_javascript.append("    return this.values[name];\n");
    dict_i18n_javascript.append("};\n");
    dict_i18n_javascript.append("Dictionary.prototype.contains = function(name) {\n");
    dict_i18n_javascript.append("    return Object.prototype.hasOwnProperty.call(this.values, name) &&\n");
    dict_i18n_javascript.append("        Object.prototype.propertyIsEnumerable.call(this.values, name);\n");
    dict_i18n_javascript.append("};\n");
    dict_i18n_javascript.append("Dictionary.prototype.each = function(action) {\n");
    dict_i18n_javascript.append("    forEachIn(this.values, action);\n");
    dict_i18n_javascript.append("};\n");
    dict_i18n_javascript.append("function translate(lang) {\n");
    dict_i18n_javascript.append("    var elements = document.getElementsByTagName(\"i18n\");\n");
    dict_i18n_javascript.append("    for (var i = 0; i < elements.length; i++) {\n");
    dict_i18n_javascript.append("        if (dict.contains(elements[i].innerHTML))\n");
    dict_i18n_javascript.append("            elements[i].innerHTML = dict.value(elements[i].innerHTML);\n");
    dict_i18n_javascript.append("    }\n");
    dict_i18n_javascript.append("}\n");
    dict_i18n_javascript.append("var dict = new Dictionary();");
    QMapIterator<QString, QString> i(dict_i18n.dictionary);
    while (i.hasNext()) { i.next();
        dict_i18n_javascript.append(QString("\ndict.add(\"%1\", \"%2\");").arg(escapeDoubleQuotes(i.key())).arg(escapeDoubleQuotes(i.value())));
    }
    // ------------------
    setupUi(this);
    this->setUnifiedTitleAndToolBarOnMac(true);
    dw_browser->setVisible(false);
    dw_variables->setVisible(false);
    dw_tables->setVisible(false);
    dw_warnings->setVisible(false);
    tbtn_add_variable->setMenu(menuAdd_variable);
    tbtn_remove_variable->setDefaultAction(actionRemove_variable);
    tbtn_add_table->setDefaultAction(actionAdd_table);
    tbtn_remove_table->setDefaultAction(actionRemove_table);
    tbtn_add_warning->setDefaultAction(actionAdd_warning);
    tbtn_remove_warning->setDefaultAction(actionRemove_warning);
    QStringList views;
    views << tr("All customers");
    views << tr("Customer information");
    views << tr("Circuit information");
    views << tr("Inspection information");
    views << tr("Table of inspections");
    QAction * action; actgrp_view = new QActionGroup(this);
    QAction * separator = menuView->actions().at(0);
    QObject::connect(actgrp_view, SIGNAL(triggered(QAction *)), this, SLOT(setView(QAction *)));
    for (int i = 0; i < views.count(); ++i) {
        action = new QAction(actgrp_view); action->setText(views.at(i));
        menuView->insertAction(separator, action);
        cb_view->addItem(views.at(i)); view_indices.insert(views.at(i), i);
    }
    trw_variables->header()->setResizeMode(0, QHeaderView::Stretch);
    trw_variables->header()->setResizeMode(1, QHeaderView::ResizeToContents);
    trw_variables->header()->setResizeMode(2, QHeaderView::ResizeToContents);
    trw_variables->header()->setResizeMode(3, QHeaderView::ResizeToContents);
    setAllEnabled(false);
    QObject::connect(actionAbout_Leaklog, SIGNAL(triggered()), this, SLOT(about()));
    QObject::connect(actionNew, SIGNAL(triggered()), this, SLOT(newDocument()));
    QObject::connect(actionOpen, SIGNAL(triggered()), this, SLOT(open()));
    QObject::connect(actionSave, SIGNAL(triggered()), this, SLOT(save()));
    QObject::connect(actionSave_as, SIGNAL(triggered()), this, SLOT(saveAs()));
    QObject::connect(actionClose, SIGNAL(triggered()), this, SLOT(closeDocument()));
    QObject::connect(actionPrint_preview, SIGNAL(triggered()), this, SLOT(printPreview()));
    QObject::connect(actionPrint, SIGNAL(triggered()), this, SLOT(print()));
    QObject::connect(actionFind, SIGNAL(triggered()), this, SLOT(find()));
    QObject::connect(actionFind_next, SIGNAL(triggered()), this, SLOT(findNext()));
    QObject::connect(actionFind_previous, SIGNAL(triggered()), this, SLOT(findPrevious()));
    QObject::connect(actionAdd_customer, SIGNAL(triggered()), this, SLOT(addCustomer()));
    QObject::connect(actionModify_customer, SIGNAL(triggered()), this, SLOT(modifyCustomer()));
    QObject::connect(actionRemove_customer, SIGNAL(triggered()), this, SLOT(removeCustomer()));
    QObject::connect(actionAdd_circuit, SIGNAL(triggered()), this, SLOT(addCircuit()));
    QObject::connect(actionModify_circuit, SIGNAL(triggered()), this, SLOT(modifyCircuit()));
    QObject::connect(actionRemove_circuit, SIGNAL(triggered()), this, SLOT(removeCircuit()));
    QObject::connect(actionAdd_inspection, SIGNAL(triggered()), this, SLOT(addInspection()));
    QObject::connect(actionModify_inspection, SIGNAL(triggered()), this, SLOT(modifyInspection()));
    QObject::connect(actionRemove_inspection, SIGNAL(triggered()), this, SLOT(removeInspection()));
    QObject::connect(actionNew_variable, SIGNAL(triggered()), this, SLOT(addVariable()));
    QObject::connect(actionNew_subvariable, SIGNAL(triggered()), this, SLOT(addSubvariable()));
    QObject::connect(actionModify_variable, SIGNAL(triggered()), this, SLOT(modifyVariable()));
    QObject::connect(actionRemove_variable, SIGNAL(triggered()), this, SLOT(removeVariable()));
    QObject::connect(actionAdd_table, SIGNAL(triggered()), this, SLOT(addTable()));
    QObject::connect(actionModify_table, SIGNAL(triggered()), this, SLOT(modifyTable()));
    QObject::connect(actionRemove_table, SIGNAL(triggered()), this, SLOT(removeTable()));
    QObject::connect(tbtn_table_add_variable, SIGNAL(clicked()), this, SLOT(addTableVariable()));
    QObject::connect(tbtn_table_remove_variable, SIGNAL(clicked()), this, SLOT(removeTableVariable()));
    QObject::connect(actionAdd_warning, SIGNAL(triggered()), this, SLOT(addWarning()));
    QObject::connect(actionModify_warning, SIGNAL(triggered()), this, SLOT(modifyWarning()));
    QObject::connect(actionRemove_warning, SIGNAL(triggered()), this, SLOT(removeWarning()));
    QObject::connect(actionExport_customer_data, SIGNAL(triggered()), this, SLOT(exportCustomerData()));
    QObject::connect(actionExport_circuit_data, SIGNAL(triggered()), this, SLOT(exportCircuitData()));
    QObject::connect(actionExport_inspection_data, SIGNAL(triggered()), this, SLOT(exportInspectionData()));
    QObject::connect(actionImport_data, SIGNAL(triggered()), this, SLOT(importData()));
    QObject::connect(lw_recent_docs, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(openRecent(QListWidgetItem *)));
    QObject::connect(le_search_customers, SIGNAL(textChanged(QLineEdit *, const QString &)), lw_customers, SLOT(filterItems(QLineEdit *, const QString &)));
    QObject::connect(le_search_circuits, SIGNAL(textChanged(QLineEdit *, const QString &)), lw_circuits, SLOT(filterItems(QLineEdit *, const QString &)));
    QObject::connect(le_search_inspections, SIGNAL(textChanged(QLineEdit *, const QString &)), lw_inspections, SLOT(filterItems(QLineEdit *, const QString &)));
    QObject::connect(lw_customers, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(loadCustomer(QListWidgetItem *)));
    QObject::connect(trw_variables, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(modifyVariable()));
    QObject::connect(trw_variables, SIGNAL(itemSelectionChanged()), this, SLOT(enableTools()));
    QObject::connect(lw_circuits, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(loadCircuit(QListWidgetItem *)));
    QObject::connect(lw_inspections, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(loadInspection(QListWidgetItem *)));
    QObject::connect(cb_view, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(viewChanged(const QString &)));
    QObject::connect(spb_since, SIGNAL(valueChanged(int)), this, SLOT(refreshView()));
    QObject::connect(cb_table, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(viewChanged(const QString &)));
    QObject::connect(cb_table_edit, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(loadTable(const QString &)));
    QObject::connect(lw_table_variables, SIGNAL(itemSelectionChanged()), this, SLOT(enableTools()));
    QObject::connect(lw_warnings, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(modifyWarning()));
    QObject::connect(lw_warnings, SIGNAL(itemSelectionChanged()), this, SLOT(enableTools()));
    QObject::connect(wv_main, SIGNAL(linkClicked(const QUrl &)), this, SLOT(executeLink(const QUrl &)));
    wv_main->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    loadSettings();
}

void MainWindow::executeLink(const QUrl & url)
{
    QStringList path = url.toString().split("/");
    QString id;
    if (path.count() > 0) {
        if (path.at(0).startsWith("customer:")) {
            id = path.at(0);
            id.remove(0, QString("customer:").length());
            for (int i = 0; i < lw_customers->count(); ++i) {
                if (lw_customers->item(i)->data(Qt::UserRole).toString() == id) {
                    loadCustomer(lw_customers->item(i), path.count() <= 1); break;
                }
            }
        }
    }
    if (path.count() > 1) {
        if (path.at(1).startsWith("circuit:")) {
            id = path.at(1);
            id.remove(0, QString("circuit:").length());
            for (int i = 0; i < lw_circuits->count(); ++i) {
                if (lw_circuits->item(i)->data(Qt::UserRole).toString() == id) {
                    loadCircuit(lw_circuits->item(i), path.count() <= 2); break;
                }
            }
        } else if (path.at(1).startsWith("modify")) { modifyCustomer(); }
    }
    if (path.count() > 2) {
        if (path.at(2).startsWith("inspection:")) {
            id = path.at(2);
            id.remove(0, QString("inspection:").length());
            for (int i = 0; i < lw_inspections->count(); ++i) {
                if (lw_inspections->item(i)->data(Qt::UserRole).toString() == id) {
                    loadInspection(lw_inspections->item(i), path.count() <= 3); break;
                }
            }
        } else if (path.at(2).startsWith("table")) {
            setView(tr("Table of inspections"));
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

void MainWindow::find()
{
    if (!document_open) { return; }
    bool ok;
    QString keyword = QInputDialog::getText(this, tr("Find - Leaklog"), tr("Find:"), QLineEdit::Normal, last_search_keyword, &ok);
    if (ok && !keyword.isEmpty()) {
        last_search_keyword = keyword;
        wv_main->findText(last_search_keyword);
    }
}

void MainWindow::findNext()
{
    if (!document_open) { return; }
    if (last_search_keyword.isEmpty()) { return; }
    wv_main->findText(last_search_keyword);
}

void MainWindow::findPrevious()
{
    if (!document_open) { return; }
    if (last_search_keyword.isEmpty()) { return; }
    wv_main->findText(last_search_keyword, QWebPage::FindBackward);
}

void MainWindow::setView(QAction * action)
{
    setView(action->text());
}

void MainWindow::setView(const QString & view)
{
    cb_view->setCurrentIndex(view_indices.value(view));
    refreshView();
}

void MainWindow::refreshView()
{
    viewChanged(cb_view->currentText());
}

void MainWindow::addRecent(QString name)
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
    lw_customers->clear();
    lw_circuits->clear();
    lw_inspections->clear();
    cb_table->clear();
    cb_table_edit->clear();
    trw_variables->clear();
    lw_table_variables->clear();
    lw_warnings->clear();
}

void MainWindow::setAllEnabled(bool enable)
{
    actionSave->setEnabled(enable);
    actionSave_as->setEnabled(enable);
    actionClose->setEnabled(enable);
    actionImport_data->setEnabled(enable);
    actionPrint_preview->setEnabled(enable);
    actionPrint->setEnabled(enable);
    actgrp_view->setEnabled(enable);
    menuDocument->setEnabled(enable);
    menuCustomer->setEnabled(enable);
        actionAdd_customer->setEnabled(enable);
        if (!enable) actionModify_customer->setEnabled(enable);
        if (!enable) actionRemove_customer->setEnabled(enable);
    menuCooling_circuit->setEnabled(enable);
        if (!enable) actionAdd_circuit->setEnabled(enable);
        if (!enable) actionModify_circuit->setEnabled(enable);
        if (!enable) actionRemove_circuit->setEnabled(enable);
    menuInspection->setEnabled(enable);
        if (!enable) actionAdd_inspection->setEnabled(enable);
        if (!enable) actionModify_inspection->setEnabled(enable);
        if (!enable) actionRemove_inspection->setEnabled(enable);
    dw_browser->setEnabled(enable);
    dw_variables->setEnabled(enable);
    dw_tables->setEnabled(enable);
    dw_warnings->setEnabled(enable);
    stw_main->setCurrentIndex(enable ? 1 : 0);
}

void MainWindow::enableTools()
{
    bool customer_selected = lw_customers->highlightedRow() >= 0;
    bool circuit_selected = lw_circuits->highlightedRow() >= 0;
    bool inspection_selected = lw_inspections->highlightedRow() >= 0;
    lbl_selected_customer->setText(customer_selected ? lw_customers->highlightedItem()->text() : QString());
    lbl_current_selection_arrow1->setVisible(circuit_selected);
    lbl_selected_circuit->setVisible(circuit_selected);
    lbl_selected_circuit->setText(circuit_selected ? lw_circuits->highlightedItem()->text() : QString());
    lbl_current_selection_arrow2->setVisible(inspection_selected);
    lbl_selected_inspection->setVisible(inspection_selected);
    lbl_selected_inspection->setText(inspection_selected ? lw_inspections->highlightedItem()->text() : QString());
    actionModify_customer->setEnabled(customer_selected);
    actionRemove_customer->setEnabled(customer_selected);
    actionExport_customer_data->setEnabled(customer_selected);
    actionAdd_circuit->setEnabled(customer_selected);
    actionModify_circuit->setEnabled(circuit_selected);
    actionRemove_circuit->setEnabled(circuit_selected);
    actionExport_circuit_data->setEnabled(circuit_selected);
    actionAdd_inspection->setEnabled(circuit_selected);
    actionModify_inspection->setEnabled(inspection_selected);
    actionRemove_inspection->setEnabled(inspection_selected);
    actionExport_inspection_data->setEnabled(inspection_selected);
    actionNew_subvariable->setEnabled(trw_variables->currentIndex().isValid() && trw_variables->currentItem()->parent() == NULL);
    actionModify_variable->setEnabled(trw_variables->currentIndex().isValid());
    actionRemove_variable->setEnabled(trw_variables->currentIndex().isValid());
    actionModify_table->setEnabled(cb_table_edit->currentIndex() >= 0);
    actionRemove_table->setEnabled(cb_table_edit->currentIndex() >= 0);
    tbtn_table_add_variable->setEnabled(cb_table_edit->currentIndex() >= 0);
    tbtn_table_remove_variable->setEnabled(lw_table_variables->currentIndex().isValid());
    actionModify_warning->setEnabled(lw_warnings->currentIndex().isValid());
    actionRemove_warning->setEnabled(lw_warnings->currentIndex().isValid());
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
}

void MainWindow::saveSettings()
{
    QSettings settings("SZCHKT", "Leaklog");
    QStringList recent;
    for (int i = 0; i < lw_recent_docs->count(); ++i)
    { recent << lw_recent_docs->item(i)->text(); }
    settings.setValue("recent_docs", recent);
    settings.setValue("pos", this->pos());
    settings.setValue("size", this->size());
    settings.setValue("window_state", this->saveState(0));
}

void MainWindow::about()
{
    AboutWidget * leaklog_about = new AboutWidget(leaklog_version, QString());
	leaklog_about->setParent(this);
    leaklog_about->setWindowFlags(Qt::Dialog /*| Qt::WindowMaximizeButtonHint*/ | Qt::WindowStaysOnTopHint);
	leaklog_about->show();
}
