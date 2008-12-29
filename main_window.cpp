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

MainWindow::MainWindow()
{
    leaklog_version = "0.9.2"; f_leaklog_version = 0.902;
    db_version = "0.9.2"; f_db_version = 0.902;
    // Dictionaries
    dict_dbtables = get_dict_dbtables();
    dict_vartypes = get_dict_vartypes();
    dict_varnames = get_dict_varnames();
    dict_attrvalues = get_dict_attrvalues();
    dict_attrnames = get_dict_attrnames();
    // ------------
    // HTML
    QFile file; QTextStream in(&file); in.setCodec("UTF-8");
    file.setFileName(":/html/service_company.html"); file.open(QIODevice::ReadOnly | QIODevice::Text);
    dict_html.insert(tr("Service company"), in.readAll());
    file.close();
    file.setFileName(":/html/customers.html"); file.open(QIODevice::ReadOnly | QIODevice::Text);
    dict_html.insert(tr("All customers"), in.readAll());
    file.close();
    file.setFileName(":/html/customer.html"); file.open(QIODevice::ReadOnly | QIODevice::Text);
    dict_html.insert(tr("Customer information"), in.readAll());
    file.close();
    file.setFileName(":/html/circuit.html"); file.open(QIODevice::ReadOnly | QIODevice::Text);
    dict_html.insert(tr("Circuit information"), in.readAll());
    file.close();
    file.setFileName(":/html/inspection.html"); file.open(QIODevice::ReadOnly | QIODevice::Text);
    dict_html.insert(tr("Inspection information"), in.readAll());
    file.close();
    file.setFileName(":/html/table.html"); file.open(QIODevice::ReadOnly | QIODevice::Text);
    dict_html.insert(tr("Table of inspections"), in.readAll());
    file.close();
    file.setFileName(":/html/repairs.html"); file.open(QIODevice::ReadOnly | QIODevice::Text);
    dict_html.insert(tr("List of repairs"), in.readAll());
    file.close();
    file.setFileName(":/html/inspectors.html"); file.open(QIODevice::ReadOnly | QIODevice::Text);
    dict_html.insert(tr("Inspectors"), in.readAll());
    file.close();
    file.setFileName(":/html/refrigerant_consumption.html"); file.open(QIODevice::ReadOnly | QIODevice::Text);
    dict_html.insert(tr("Refrigerant consumption"), in.readAll());
    file.close();
    file.setFileName(":/html/agenda.html"); file.open(QIODevice::ReadOnly | QIODevice::Text);
    dict_html.insert(tr("Agenda"), in.readAll());
    file.close();
    // ----
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
    dw_browser->setVisible(false);
    dw_inspectors->setVisible(false);
    dw_variables->setVisible(false);
    dw_tables->setVisible(false);
    dw_warnings->setVisible(false);
    actionOpen->setMenu(menuOpen);
    tbtn_add_variable->setMenu(menuAdd_variable);
    tbtn_remove_variable->setDefaultAction(actionRemove_variable);
    tbtn_add_table->setDefaultAction(actionAdd_table);
    tbtn_remove_table->setDefaultAction(actionRemove_table);
    tbtn_add_warning->setDefaultAction(actionAdd_warning);
    tbtn_remove_warning->setDefaultAction(actionRemove_warning);
    tbtn_add_inspector->setDefaultAction(actionAdd_inspector);
    tbtn_remove_inspector->setDefaultAction(actionRemove_inspector);
    QStringList views = dict_html.keys();
    QAction * action; actgrp_view = new QActionGroup(this);
    QAction * separator = menuView->actions().at(0);
    QObject::connect(actgrp_view, SIGNAL(triggered(QAction *)), this, SLOT(setView(QAction *)));
    for (int i = 0; i < views.count(); ++i) {
        action = new QAction(actgrp_view); action->setText(views.at(i));
        menuView->insertAction(separator, action);
        cb_view->addItem(views.at(i)); view_indices.insert(views.at(i), i);
    }
    actionShow_icons_only = new QAction(tr("Show icons only"), this);
    actionShow_icons_only->setCheckable(true);
    trw_variables->header()->setResizeMode(0, QHeaderView::Stretch);
    trw_variables->header()->setResizeMode(1, QHeaderView::ResizeToContents);
    trw_variables->header()->setResizeMode(2, QHeaderView::ResizeToContents);
    trw_variables->header()->setResizeMode(3, QHeaderView::ResizeToContents);
    trw_table_variables->header()->setResizeMode(0, QHeaderView::Stretch);
    trw_table_variables->header()->setResizeMode(1, QHeaderView::ResizeToContents);
    trw_table_variables->header()->setResizeMode(2, QHeaderView::ResizeToContents);
    setAllEnabled(false);
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
    QObject::connect(actionFind, SIGNAL(triggered()), this, SLOT(find()));
    QObject::connect(actionFind_next, SIGNAL(triggered()), this, SLOT(findNext()));
    QObject::connect(actionFind_previous, SIGNAL(triggered()), this, SLOT(findPrevious()));
    QObject::connect(actionChange_language, SIGNAL(triggered()), this, SLOT(changeLanguage()));
    QObject::connect(actionService_company_information, SIGNAL(triggered()), this, SLOT(modifyServiceCompany()));
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
    QObject::connect(actionPrint_label, SIGNAL(triggered()), this, SLOT(printLabel()));
    QObject::connect(actionNew_variable, SIGNAL(triggered()), this, SLOT(addVariable()));
    QObject::connect(actionNew_subvariable, SIGNAL(triggered()), this, SLOT(addSubvariable()));
    QObject::connect(actionModify_variable, SIGNAL(triggered()), this, SLOT(modifyVariable()));
    QObject::connect(actionRemove_variable, SIGNAL(triggered()), this, SLOT(removeVariable()));
    QObject::connect(actionAdd_table, SIGNAL(triggered()), this, SLOT(addTable()));
    QObject::connect(actionModify_table, SIGNAL(triggered()), this, SLOT(modifyTable()));
    QObject::connect(actionRemove_table, SIGNAL(triggered()), this, SLOT(removeTable()));
    QObject::connect(tbtn_table_add_variable, SIGNAL(clicked()), this, SLOT(addTableVariable()));
    QObject::connect(tbtn_table_remove_variable, SIGNAL(clicked()), this, SLOT(removeTableVariable()));
    QObject::connect(tbtn_table_move_up, SIGNAL(clicked()), this, SLOT(moveTableVariableUp()));
    QObject::connect(tbtn_table_move_down, SIGNAL(clicked()), this, SLOT(moveTableVariableDown()));
    QObject::connect(actionAdd_warning, SIGNAL(triggered()), this, SLOT(addWarning()));
    QObject::connect(actionModify_warning, SIGNAL(triggered()), this, SLOT(modifyWarning()));
    QObject::connect(actionRemove_warning, SIGNAL(triggered()), this, SLOT(removeWarning()));
    QObject::connect(actionAdd_inspector, SIGNAL(triggered()), this, SLOT(addInspector()));
    QObject::connect(actionModify_inspector, SIGNAL(triggered()), this, SLOT(modifyInspector()));
    QObject::connect(actionRemove_inspector, SIGNAL(triggered()), this, SLOT(removeInspector()));
    QObject::connect(actionExport_customer_data, SIGNAL(triggered()), this, SLOT(exportCustomerData()));
    QObject::connect(actionExport_circuit_data, SIGNAL(triggered()), this, SLOT(exportCircuitData()));
    QObject::connect(actionExport_inspection_data, SIGNAL(triggered()), this, SLOT(exportInspectionData()));
    QObject::connect(actionImport_data, SIGNAL(triggered()), this, SLOT(importData()));
    QObject::connect(actionCheck_for_updates, SIGNAL(triggered()), this, SLOT(checkForUpdates()));
    QObject::connect(lw_recent_docs, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(openRecent(QListWidgetItem *)));
    QObject::connect(le_search_customers, SIGNAL(textChanged(QLineEdit *, const QString &)), lw_customers, SLOT(filterItems(QLineEdit *, const QString &)));
    QObject::connect(le_search_circuits, SIGNAL(textChanged(QLineEdit *, const QString &)), lw_circuits, SLOT(filterItems(QLineEdit *, const QString &)));
    QObject::connect(le_search_inspections, SIGNAL(textChanged(QLineEdit *, const QString &)), lw_inspections, SLOT(filterItems(QLineEdit *, const QString &)));
    QObject::connect(le_search_inspectors, SIGNAL(textChanged(QLineEdit *, const QString &)), lw_inspectors, SLOT(filterItems(QLineEdit *, const QString &)));
    QObject::connect(lw_customers, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(loadCustomer(QListWidgetItem *)));
    QObject::connect(trw_variables, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(modifyVariable()));
    QObject::connect(trw_variables, SIGNAL(itemSelectionChanged()), this, SLOT(enableTools()));
    QObject::connect(lw_circuits, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(loadCircuit(QListWidgetItem *)));
    QObject::connect(lw_inspections, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(loadInspection(QListWidgetItem *)));
    QObject::connect(lw_inspectors, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(loadInspector(QListWidgetItem *)));
    QObject::connect(btn_clear_current_selection, SIGNAL(clicked()), this, SLOT(clearSelection()));
    QObject::connect(cb_view, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(viewChanged(const QString &)));
    QObject::connect(tbtn_view_level_up, SIGNAL(clicked()), this, SLOT(viewLevelUp()));
    QObject::connect(tbtn_view_level_down, SIGNAL(clicked()), this, SLOT(viewLevelDown()));
    QObject::connect(spb_since, SIGNAL(valueChanged(int)), this, SLOT(refreshView()));
    QObject::connect(cb_table, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(viewChanged(const QString &)));
    QObject::connect(cb_table_edit, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(loadTable(const QString &)));
    QObject::connect(trw_table_variables, SIGNAL(itemSelectionChanged()), this, SLOT(enableTools()));
    QObject::connect(lw_warnings, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(modifyWarning()));
    QObject::connect(lw_warnings, SIGNAL(itemSelectionChanged()), this, SLOT(enableTools()));
    QObject::connect(wv_main, SIGNAL(linkClicked(const QUrl &)), this, SLOT(executeLink(const QUrl &)));
    QObject::connect(http, SIGNAL(done(bool)), this, SLOT(httpRequestFinished(bool)));
    wv_main->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    loadSettings();
    if (qApp->arguments().count() > 1) {
        QFileInfo file_info(qApp->arguments().at(1));
        if (file_info.exists()) {
            addRecent(file_info.absoluteFilePath());
            openDatabase(file_info.absoluteFilePath());
        }
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
    if (show) {
        toolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    } else {
        toolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    }
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
        } else if (path.at(0).startsWith("repair:")) {
            id = path.at(0);
            id.remove(0, QString("repair:").length());
            loadRepair(id, path.count() <= 1);
        } else if (path.at(0).startsWith("inspector:")) {
            id = path.at(0);
            id.remove(0, QString("inspector:").length());
            for (int i = 0; i < lw_inspectors->count(); ++i) {
                if (lw_inspectors->item(i)->data(Qt::UserRole).toString() == id) {
                    loadInspector(lw_inspectors->item(i), path.count() <= 1); break;
                }
            }
        } else if (path.at(0).startsWith("allcustomers:")) {
            setView(tr("All customers"));
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
        } else if (path.at(1).startsWith("modify")) {
            if (path.at(0).startsWith("customer:")) { modifyCustomer(); }
            else if (path.at(0).startsWith("repair:")) { modifyRepair(); }
            else if (path.at(0).startsWith("inspector:")) { modifyInspector(); }
        }
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

void MainWindow::printLabel()
{
    if (!db.isOpen()) { return; }
    if (selectedCustomer() < 0) { return; }
    if (selectedCircuit() < 0) { return; }
    if (selectedInspection().isEmpty()) { return; }

    QMap<QString, QCheckBox *> label_positions;
    QDialog * d = new QDialog(this);
	d->setWindowTitle(tr("Print label - Leaklog"));
        QGridLayout * gl = new QGridLayout(d);
        gl->setContentsMargins(9, 9, 9, 9); gl->setSpacing(9);
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

    QMap<QString, QVariant> attributes;
    attributes.insert("id", toString(selectedCustomer()) + "::" + toString(selectedCircuit()));
    MTDictionary parents("customer", toString(selectedCustomer()));
    parents.insert("circuit", toString(selectedCircuit()));
    MTRecord inspection_record("inspection", toString(selectedInspection()), parents);
    QMap<QString, QVariant> inspection = inspection_record.list();
    attributes.insert("date", inspection.value("date").toString());
    Subvariable refr_add_per("refr_add", "refr_add_per");
    refr_add_per.next();
    QString unparsed_expression = refr_add_per.value("SUBVAR_VALUE").toString();
    if (!unparsed_expression.isEmpty()) {
        if (!parsed_expressions.contains(unparsed_expression)) {
            QStringList var_ids = listVariableIds();
            parsed_expressions.insert(unparsed_expression, parseExpression(unparsed_expression, &var_ids));
        }
        MTDictionary expression = parsed_expressions.value(unparsed_expression);
        attributes.insert("refr_add_per", evaluateExpression(inspection, expression, toString(selectedCustomer()), toString(selectedCircuit())));
    }
    MTRecord circuit("circuit", toString(selectedCircuit()), MTDictionary("parent", toString(selectedCustomer())));
    attributes.unite(circuit.list("refrigerant, refrigerant_amount"));
    MTRecord inspector("inspector", inspection.value("inspector").toString(), MTDictionary());
    if (inspector.exists()) {
        attributes.unite(inspector.list("person, company, person_reg_num, company_reg_num"));
    }

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
    int margin = rect.width() / 50;
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

void MainWindow::paintLabel(const QMap<QString, QVariant> & attributes, QPainter & painter, int x, int y, int w, int h)
{
    painter.save();
    QPen pen; pen.setWidthF(15.0); painter.setPen(pen);
    QFont font; painter.setFont(font);
#ifdef Q_WS_MAC
    font.setPointSize(font.pointSize() - 5);
#else
    font.setPointSize(font.pointSize());
#endif
    painter.setFont(font);
    int title_h = 3 * h / 14; int m = w / 75; int dm = m * 2;
    painter.drawRect(x, y, w, h);
    painter.drawLine(x, y + title_h, x + w, y + title_h);
    painter.drawText(m + x, m + y + title_h, w / 3 - dm, h / 14 - m, Qt::AlignCenter, tr("Circuit ID"));
    painter.drawLine(x + (w / 3), y + title_h, x + (w / 3), y + h);
    painter.drawText(m + x + (w / 3), m + y + title_h, w / 3 - dm, h / 14 - m, Qt::AlignCenter, tr("Refrigerant"));
    painter.drawLine(x + (2 * w / 3), y + title_h, x + (2 * w / 3), y + h);
    painter.drawText(m + x + (2 * w / 3), m + y + title_h, w / 3 - dm, h / 14 - m, Qt::AlignCenter, tr("Amount of refrigerant"));
    painter.drawLine(x, y + title_h + (h / 7), x + w, y + title_h + (h / 7));
    painter.drawText(m + x, m + y + title_h + (h / 7), w / 3 - dm, 9 * h / 14 - dm, Qt::AlignLeft, tr("Date of\ninspection"));
    painter.drawText(m + x + (2 * w / 3), m + y + title_h + (h / 7), w / 3 - dm, 9 * h / 14 - dm, Qt::AlignLeft, tr("Date of\nthe next\ninspection"));
    painter.drawLine(x + (w / 3), y + title_h + (2 * h / 7), x + (2 * w / 3), y + title_h + (2 * h / 7));
    painter.drawLine(x + (w / 3), y + title_h + (3 * h / 7), x + (2 * w / 3), y + title_h + (3 * h / 7));
    painter.drawLine(x + (w / 3), y + title_h + (4 * h / 7), x + (2 * w / 3), y + title_h + (4 * h / 7));
    painter.drawLine(x + (w / 3), y + title_h + (5 * h / 7), x + (2 * w / 3), y + title_h + (5 * h / 7));
    painter.drawText(m + x + (w / 3), m + y + title_h + (5 * h / 7), w / 6 - dm, h / 14 - dm, Qt::AlignCenter, attributes.value("person_reg_num", QString()).toString());
    painter.drawLine(x + (w / 2), y + title_h + (5 * h / 7), x + (w / 2), y + h);
    painter.drawText(m + x + (w / 2), m + y + title_h + (5 * h / 7), w / 6 - dm, h / 14 - dm, Qt::AlignCenter, attributes.value("company_reg_num", QString()).toString());
    font.setBold(true); painter.setFont(font);
    painter.drawText(m + x, m + y, w - dm, title_h - dm, Qt::AlignCenter, tr("RECORD OF INSPECTION OF COOLING CIRCUIT\nin accordance with Regulation (EC) No. 842/2006"));
    painter.drawText(m + x, y + title_h + h / 14, w / 3 - dm, h / 14 - m, Qt::AlignCenter, attributes.value("id", QString()).toString());
    painter.drawText(m + x + (w / 3), y + title_h + h / 14, w / 3 - dm, h / 14 - m, Qt::AlignCenter, attributes.value("refrigerant", QString()).toString());
    painter.drawText(m + x + (2 * w / 3), y + title_h + h / 14, w / 3 - dm, h / 14 - m, Qt::AlignCenter, attributes.value("refrigerant_amount", QString()).toString() + " " + tr("kg"));
    painter.drawText(m + x + (w / 3), y + title_h + (h / 7) + h / 14, w / 3 - dm, h / 14 - m, Qt::AlignCenter, attributes.value("refr_add_per", QString()).toString() + " " + tr("%"));
    painter.drawText(m + x + (w / 3), y + title_h + (2 * h / 7) + h / 14, w / 3 - dm, h / 14 - m, Qt::AlignCenter, attributes.value("person", QString()).toString());
    painter.drawText(m + x + (w / 3), y + title_h + (3 * h / 7) + h / 14, w / 3 - dm, h / 14 - m, Qt::AlignCenter, attributes.value("company", QString()).toString());
    font.setBold(false); font.setItalic(true); painter.setFont(font);
    painter.drawText(m + x + (w / 3), m + y + title_h + (h / 7), w / 3 - dm, h / 14 - m, Qt::AlignCenter, tr("Average leakage"));
    painter.drawText(m + x + (w / 3), m + y + title_h + (2 * h / 7), w / 3 - dm, h / 14 - m, Qt::AlignCenter, tr("Certified person"));
    painter.drawText(m + x + (w / 3), m + y + title_h + (3 * h / 7), w / 3 - dm, h / 14 - m, Qt::AlignCenter, tr("Certified company"));
    painter.drawText(m + x + (w / 3), m + y + title_h + (4 * h / 7), w / 3 - dm, h / 7 - dm, Qt::AlignCenter, tr("Registry number of\nperson and company"));
    pen.setWidthF(7.0);
    painter.restore(); painter.save(); painter.setPen(pen);
    int r = (w / 3 - dm) / 2;
    QRect circle1o(m + x, y + title_h + (4 * h / 7) - m - r, 2 * r, 2 * r);
    painter.drawEllipse(circle1o);
    QRect circle1i(m + x + 2 * dm, y + title_h + (4 * h / 7) - m - r + 2 * dm, 2 * r - 4 * dm, 2 * r - 4 * dm);
    painter.drawEllipse(circle1i);
    QRect circle2o(m + x + 2 * w / 3, y + title_h + (4 * h / 7) - m - r, 2 * r, 2 * r);
    painter.drawEllipse(circle2o);
    QRect circle2i(m + x + 2 * w / 3 + 2 * dm, y + title_h + (4 * h / 7) - m - r + 2 * dm, 2 * r - 4 * dm, 2 * r - 4 * dm);
    painter.drawEllipse(circle2i);
    painter.translate(m + x + r, y + title_h + (4 * h / 7) - m);
    for (int i = 0; i < 12; ++i) {
        painter.drawLine(0, - r, 0, 2 * dm - r);
        painter.rotate(30.0);
    }
    painter.restore(); painter.save(); painter.setPen(pen);
    painter.translate(m + x + 2 * w / 3 + r, y + title_h + (4 * h / 7) - m);
    for (int i = 0; i < 12; ++i) {
        painter.drawLine(0, - r, 0, 2 * dm - r);
        painter.rotate(30.0);
    }
    painter.restore();
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

void MainWindow::clearSelection(bool clear_selected_repair)
{
    lw_inspections->clear();
    lw_circuits->clear();
    lw_customers->unhighlightAllItems();
    if (clear_selected_repair) {
        selected_repair.clear();
        refreshView();
    }
    enableTools();
}

void MainWindow::setView(QAction * action)
{
    setView(action->text());
}

void MainWindow::setView(const QString & view)
{
    int i = view_indices.value(view);
    if (cb_view->currentIndex() == i) {
        refreshView();
    } else {
        cb_view->setCurrentIndex(i);
    }
}

void MainWindow::refreshView()
{
    viewChanged(cb_view->currentText());
}

void MainWindow::viewLevelUp()
{
    if (cb_view->currentIndex() > 0) { cb_view->setCurrentIndex(cb_view->currentIndex() - 1); }
}

void MainWindow::viewLevelDown()
{
    if (cb_view->currentIndex() < cb_view->count() - 1) {
        QString view = cb_view->currentText();
        if ((view == tr("All customers") && selectedCustomer() < 0) || (view == tr("Customer information") && selectedCircuit() < 0)) {
            setView(tr("List of repairs"));
        } else if (view == tr("Circuit information") && selectedInspection().isEmpty()) {
            setView(tr("Table of inspections"));
        } else {
            cb_view->setCurrentIndex(cb_view->currentIndex() + 1);
        }
    }
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
    lw_inspectors->clear();
    cb_table->clear();
    cb_table_edit->clear();
    trw_variables->clear();
    trw_table_variables->clear();
    lw_warnings->clear();
}

void MainWindow::setAllEnabled(bool enable)
{
    actionSave->setEnabled(enable);
    actionSave_and_compact->setEnabled(enable);
    actionClose->setEnabled(enable);
    actionImport_data->setEnabled(enable);
    actionPrint_preview->setEnabled(enable);
    actionPrint->setEnabled(enable);
    actgrp_view->setEnabled(enable);

    menuDatabase->setEnabled(enable);
    menuCustomer->setEnabled(enable);
    menuCooling_circuit->setEnabled(enable);
    menuInspection->setEnabled(enable);
    menuInspector->setEnabled(enable);

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
        actionModify_repair->setEnabled(enable);
        actionRemove_repair->setEnabled(enable);
        actionPrint_label->setEnabled(enable);
    // menuInspector
        actionModify_inspector->setEnabled(enable);
        actionRemove_inspector->setEnabled(enable);
    }
    dw_browser->setEnabled(enable);
    dw_inspectors->setEnabled(enable);
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
    bool inspector_selected = lw_inspectors->highlightedRow() >= 0;
    bool repair_selected = !selected_repair.isEmpty();
    lbl_selected_customer->setText(customer_selected ? lw_customers->highlightedItem()->text() : QString());
    lbl_current_selection_arrow1->setVisible(circuit_selected);
    lbl_selected_circuit->setVisible(circuit_selected);
    lbl_selected_circuit->setText(circuit_selected ? lw_circuits->highlightedItem()->text() : QString());
    lbl_current_selection_arrow2->setVisible(inspection_selected);
    lbl_selected_inspection->setVisible(inspection_selected || repair_selected);
    if (inspection_selected) {
        lbl_selected_inspection->setText(lw_inspections->highlightedItem()->text());
    } else if (repair_selected) {
        lbl_selected_inspection->setText(selectedRepair());
    } else {
        lbl_selected_inspection->setText(QString());
    }
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
    actionModify_repair->setEnabled(repair_selected);
    actionRemove_repair->setEnabled(repair_selected);
    actionPrint_label->setEnabled(inspection_selected);
    actionExport_inspection_data->setEnabled(inspection_selected);
    actionNew_subvariable->setEnabled(trw_variables->currentIndex().isValid() && trw_variables->currentItem()->parent() == NULL && !dict_varnames.contains(trw_variables->currentItem()->text(1)));
    actionModify_variable->setEnabled(trw_variables->currentIndex().isValid());
    actionRemove_variable->setEnabled(trw_variables->currentIndex().isValid() && !dict_varnames.contains(trw_variables->currentItem()->text(1)));
    actionModify_table->setEnabled(cb_table_edit->currentIndex() >= 0);
    actionRemove_table->setEnabled(cb_table_edit->currentIndex() >= 0);
    tbtn_table_add_variable->setEnabled(cb_table_edit->currentIndex() >= 0);
    tbtn_table_remove_variable->setEnabled(trw_table_variables->currentIndex().isValid());
    tbtn_table_move_up->setEnabled(trw_table_variables->currentIndex().isValid());
    tbtn_table_move_down->setEnabled(trw_table_variables->currentIndex().isValid());
    actionModify_warning->setEnabled(lw_warnings->currentIndex().isValid());
    actionRemove_warning->setEnabled(lw_warnings->currentIndex().isValid() && lw_warnings->currentItem()->data(Qt::UserRole).toInt() < 1000);
    actionModify_inspector->setEnabled(inspector_selected);
    actionRemove_inspector->setEnabled(inspector_selected);
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
    { recent << lw_recent_docs->item(i)->text(); }
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
	if (f_current_ver <= f_leaklog_version) {
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
    AboutWidget * leaklog_about = new AboutWidget(leaklog_version, QString());
	leaklog_about->setParent(this);
    leaklog_about->setWindowFlags(Qt::Dialog /*| Qt::WindowMaximizeButtonHint*/ | Qt::WindowStaysOnTopHint);
	leaklog_about->show();
}
