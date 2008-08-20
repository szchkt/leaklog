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

bool MainWindow::saveChangesBeforeProceeding(QString title, bool close_)
{
	if (document_open && this->isWindowModified()) {
		switch (QMessageBox::information(this, title, tr("Save changes before proceeding?"), tr("&Save"), tr("&Discard"), tr("Cancel"), 0, 2)) {
			case 0: // Save
				document_open = false;
				save(); if (close_) { closeDocument(); }; return false;
				break;
			case 1: // Discard
				document_open = false;
				if (close_) { closeDocument(); }; return false;
				break;
			case 2: // Cancel
				return true;
				break;
		}
	} else if (document_open && !this->isWindowModified()) {
		if (close_) { closeDocument(); }; return false;
	}
	return false;
}

void MainWindow::newDocument()
{
    if (saveChangesBeforeProceeding(tr("New document - Leaklog"), true)) { return; }
    QString path = QFileDialog::getSaveFileName(this, tr("New document - Leaklog"), tr("untitled.lklg"), tr("Leaklog Document (*.lklg)"));
	if (path.isNull() || path.isEmpty()) { return; }
    QFile file(path);
    if (!file.open(QFile::WriteOnly)) {
		QMessageBox::critical(this, tr("New document - Leaklog"), tr("Cannot write file %1:\n%2.").arg(path).arg(file.errorString()));
		this->setWindowTitle(tr("Leaklog"));
		return;
    }
    addRecent(path);
    clearAll();
    document.clear();
    QDomElement root = document.createElement("leaklog");
    document.appendChild(root);
    QDomElement variables = document.createElement("variables");
    root.appendChild(variables);
    QDomElement tables = document.createElement("tables");
    root.appendChild(tables);
    QDomElement customers = document.createElement("customers");
    root.appendChild(customers);
    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << document.toString(4);
    file.close();
    document_open = true;
    document_path = path;
#ifdef Q_WS_MAC
	this->setWindowTitle(QString("%1[*]").arg(QFileInfo(file).baseName()));
#else
    this->setWindowTitle(QString("%1[*] - Leaklog").arg(QFileInfo(file).baseName()));
#endif
    this->setWindowModified(false);
    setAllEnabled(true);
}

void MainWindow::openRecent(QListWidgetItem * item)
{
    QString s = item->text();
    addRecent(s);
    openDocument(s);
}

void MainWindow::open()
{
    if (saveChangesBeforeProceeding(tr("Open document - Leaklog"), true)) { return; }
    QString path = QFileDialog::getOpenFileName(this, tr("Open document - Leaklog"), "", tr("Leaklog Documents (*.lklg);;All files (*.*)"));
	if (path.isNull() || path.isEmpty()) { return; }
    addRecent(path);
    openDocument(path);
}

void MainWindow::openDocument(QString path)
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(path);
    if (!db.open()) {
		QMessageBox::critical(this, tr("Open document - Leaklog"), tr("Cannot read file %1:\n%2.").arg(path).arg(db.lastError().text()));
		this->setWindowTitle(tr("Leaklog"));
		return;
    }
    clearAll();
    QSqlQuery begin("BEGIN TRANSACTION");
    QSqlQuery query("SELECT id, company FROM customers");
    while (query.next()) {
        QListWidgetItem * item = new QListWidgetItem;
        item->setText(query.value(1).toString().isEmpty() ? query.value(0).toString() : tr("%1 (%2)").arg(query.value(0).toString()).arg(query.value(1).toString()));
        item->setData(Qt::UserRole, query.value(0).toString());
        lw_customers->addItem(item);
    }
    /*QDomElement el_variables = document.documentElement().firstChildElement("variables");
    if (!el_variables.isNull()) {
        QDomElement variable = el_variables.firstChildElement("var");
        while (!variable.isNull()) {
            QTreeWidgetItem * item = new QTreeWidgetItem(trw_variables);
            item->setText(0, variable.attribute("name"));
            item->setText(1, variable.attribute("id"));
            item->setText(2, variable.attribute("unit"));
            item->setText(3, dict_vartypes.value(variable.attribute("type")));
            QDomElement subvariable = variable.firstChildElement("var");
            while (!subvariable.isNull()) {
                QTreeWidgetItem * subitem = new QTreeWidgetItem(item);
                subitem->setText(0, subvariable.attribute("name"));
                subitem->setText(1, subvariable.attribute("id"));
                subitem->setText(2, subvariable.attribute("unit"));
                subitem->setText(3, dict_vartypes.value(subvariable.attribute("type")));
                subvariable = subvariable.nextSiblingElement();
            }
            variable = variable.nextSiblingElement();
        }
    }
    QDomElement el_tables = document.documentElement().firstChildElement("tables");
    if (!el_tables.isNull()) {
        QDomNodeList tables = el_tables.elementsByTagName("table");
        for (int i = 0; i < tables.count(); ++i) {
            QDomElement table = tables.at(i).toElement();
            cb_table_edit->addItem(table.attribute("id"));
            cb_table->addItem(table.attribute("id"));
        }
    }
    QDomElement el_warnings = document.documentElement().firstChildElement("warnings");
    if (!el_warnings.isNull()) {
        QDomNodeList warnings = el_warnings.elementsByTagName("warning");
        for (int i = 0; i < warnings.count(); ++i) {
            QDomElement element = warnings.at(i).toElement();
            QListWidgetItem * item = new QListWidgetItem;
            item->setText(element.attribute("description").isEmpty() ? element.attribute("id") : tr("%1 (%2)").arg(element.attribute("id")).arg(element.attribute("description")));
            item->setData(Qt::UserRole, element.attribute("id"));
            lw_warnings->addItem(item);
        }
    }*/
    //document_open = true;
    document_path = path;
#ifdef Q_WS_MAC
	this->setWindowTitle(QString("%1[*]").arg(QFileInfo(path).baseName()));
#else
    this->setWindowTitle(QString("%1[*] - Leaklog").arg(QFileInfo(path).baseName()));
#endif
    this->setWindowModified(false);
    setAllEnabled(true);
    loadTable(cb_table_edit->currentText());
    setView(tr("All customers"));
}

void MainWindow::save()
{
    saveDocument(document_path);
}

void MainWindow::saveAs()
{
	QString path = QFileDialog::getSaveFileName(this, tr("Save document - Leaklog"), QFileInfo(document_path).fileName(), tr("Leaklog Document (*.lklg)"));
	if (!path.isEmpty()) { addRecent(path); saveDocument(path); }
}

void MainWindow::saveDocument(QString path)
{
    QFile file(path);
    if (!file.open(QFile::WriteOnly)) {
		QMessageBox::critical(this, tr("Save document - Leaklog"), tr("Cannot write file %1:\n%2.").arg(path).arg(file.errorString()));
		return;
    }
    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << document.toString(4);
    file.close();
    document_path = path;
#ifdef Q_WS_MAC
	this->setWindowTitle(QString("%1[*]").arg(QFileInfo(document_path).baseName()));
#else
    this->setWindowTitle(QString("%1[*] - Leaklog").arg(QFileInfo(document_path).baseName()));
#endif
    this->setWindowModified(false);
    refreshView();
}

void MainWindow::closeDocument()
{
	if (saveChangesBeforeProceeding(tr("Close document - Leaklog"), false)) { return; }
    document_open = false;
	clearAll(); setAllEnabled(false);
	this->setWindowTitle(tr("Leaklog"));
	this->setWindowModified(false);
}

void MainWindow::addCustomer()
{
    if (!db.isOpen()) { return; }
    QStringList used_ids;
    QSqlQuery query("SELECT id FROM customers");
    while (query.next()) { used_ids << query.value(0).toString(); }
    MTRecord record("customer", "", MTDictionary());
    ModifyDialogue * md = new ModifyDialogue(record, used_ids, true, this);
    if (md->exec() == QDialog::Accepted) {
        record = md->record();
        QString company = record.list("company").value("company").toString();
        QListWidgetItem * item = new QListWidgetItem;
        item->setText(company.isEmpty() ? record.id() : tr("%1 (%2)").arg(record.id()).arg(company));
        item->setData(Qt::UserRole, record.id());
        lw_customers->addItem(item);
        this->setWindowModified(true);
        refreshView();
    }
    delete md;
}

void MainWindow::modifyCustomer()
{
    if (!db.isOpen()) { return; }
    QStringList used_ids;
    QSqlQuery query;
    query.prepare("SELECT id FROM customers WHERE id <> :id");
    query.bindValue(":id", selectedCustomer());
    query.exec();
    while (query.next()) { used_ids << query.value(0).toString(); }
    MTRecord record("customer", QString("%1").arg(selectedCustomer()), MTDictionary());
    ModifyDialogue * md = new ModifyDialogue(record, used_ids, true, this);
    if (md->exec() == QDialog::Accepted) {
        record = md->record();
        QString company = record.list("company").value("company").toString();
        QListWidgetItem * item = lw_customers->highlightedItem();
        item->setText(company.isEmpty() ? record.id() : tr("%1 (%2)").arg(record.id()).arg(company));
        item->setData(Qt::UserRole, record.id());
        this->setWindowModified(true);
        refreshView();
    }
    delete md;
}

void MainWindow::removeCustomer()
{
    QDomElement element = selectedCustomerElement();
    if (element.isNull()) { return; }
    QListWidgetItem * item = lw_customers->highlightedItem();
    bool ok;
    QString confirmation = QInputDialog::getText(this, tr("Remove customer - Leaklog"), tr("Are you sure you want to remove the selected customer?\nTo remove all data about the customer \"%1\" type REMOVE and confirm:").arg(item->data(Qt::UserRole).toString()), QLineEdit::Normal, "", &ok);
    if (!ok || confirmation != tr("REMOVE")) { return; }
    QDomElement el_customers = document.documentElement().firstChildElement("customers");
    if (el_customers.isNull()) { return; }
    el_customers.removeChild(element);
    if (item != NULL) { delete item; }
    lw_circuits->clear(); lw_inspections->clear();
    enableTools();
    this->setWindowModified(true);
    setView(tr("All customers"));
}

void MainWindow::loadCustomer(QListWidgetItem * item) { loadCustomer(item, true); }

void MainWindow::loadCustomer(QListWidgetItem * item, bool refresh)
{
    if (item == NULL) { return; }
    lw_customers->highlightItem(item);
    QSqlQuery query;
    query.prepare("SELECT company FROM customers WHERE id = :id");
    query.bindValue(":id", selectedCustomer());
    query.exec();
    if (!query.next()) { return; }
    lw_circuits->clear(); lw_inspections->clear();
    QSqlQuery circuits;
    circuits.prepare("SELECT id FROM circuits WHERE parent = :parent");
    circuits.bindValue(":parent", selectedCustomer());
    circuits.exec();
    while (circuits.next()) {
        QListWidgetItem * item = new QListWidgetItem;
        item->setText(circuits.value(0).toString());
        item->setData(Qt::UserRole, circuits.value(0).toString());
        lw_circuits->addItem(item);
    }
    enableTools();
    if (refresh) {
        setView(tr("Customer information"));
    }
}

QDomElement MainWindow::selectedCustomerElement(QStringList * used_ids)
{
    if (!document_open) { return QDomElement(); }
    if (lw_customers->highlightedRow() < 0) { return QDomElement(); }
    QListWidgetItem * item = lw_customers->highlightedItem();
    QDomElement el_customers = document.documentElement().firstChildElement("customers");
    if (el_customers.isNull()) { return QDomElement(); }
    QDomNodeList customers = el_customers.elementsByTagName("customer"); int n = -1;
    for (int i = 0; i < customers.count(); ++i) {
        if (n == -1 && customers.at(i).toElement().attribute("id") == item->data(Qt::UserRole).toString()) {
            n = i;
        } else {
            if (used_ids) { *used_ids << customers.at(i).toElement().attribute("id"); }
        }
    }
    if (n == -1) { return QDomElement(); }
    return customers.at(n).toElement();
}

void MainWindow::addCircuit()
{
    QDomElement customer = selectedCustomerElement();
    if (customer.isNull()) { return; }
    QDomNodeList circuits = customer.elementsByTagName("circuit");
    QStringList used_ids;
    for (int i = 0; i < circuits.count(); ++i) {
        used_ids << circuits.at(i).toElement().attribute("id");
    }
    QDomElement element = document.createElement("circuit");
    /*ModifyDialogue * md = new ModifyDialogue(element, used_ids, true, this);
    if (md->exec() == QDialog::Accepted) {
        customer.appendChild(element);
        QListWidgetItem * item = new QListWidgetItem;
        item->setText(element.attribute("id"));
        item->setData(Qt::UserRole, element.attribute("id"));
        lw_circuits->addItem(item);
        this->setWindowModified(true);
        refreshView();
    }
    delete md;*/
}

void MainWindow::modifyCircuit()
{
    QStringList used_ids;
    QDomElement element = selectedCircuitElement(&used_ids);
    if (element.isNull()) { return; }
    /*ModifyDialogue * md = new ModifyDialogue(element, used_ids, true, this);
    if (md->exec() == QDialog::Accepted) {
        QListWidgetItem * item = lw_circuits->highlightedItem();
        item->setText(element.attribute("id"));
        item->setData(Qt::UserRole, element.attribute("id"));
        this->setWindowModified(true);
        refreshView();
    }
    delete md;*/
}

void MainWindow::removeCircuit()
{
    QDomElement element = selectedCircuitElement();
    if (element.isNull()) { return; }
    QListWidgetItem * item = lw_circuits->highlightedItem();
    bool ok;
    QString confirmation = QInputDialog::getText(this, tr("Remove circuit - Leaklog"), tr("Are you sure you want to remove the selected circuit?\nTo remove all data about the circuit \"%1\" type REMOVE and confirm:").arg(item->data(Qt::UserRole).toString()), QLineEdit::Normal, "", &ok);
    if (!ok || confirmation != tr("REMOVE")) { return; }
    QDomElement customer = selectedCustomerElement();
    if (customer.isNull()) { return; }
    customer.removeChild(element);
    if (item != NULL) { delete item; }
    lw_inspections->clear();
    enableTools();
    this->setWindowModified(true);
    setView(tr("Customer information"));
}

void MainWindow::loadCircuit(QListWidgetItem * item) { loadCircuit(item, true); }

void MainWindow::loadCircuit(QListWidgetItem * item, bool refresh)
{
    if (item == NULL) { return; }
    lw_circuits->highlightItem(item);
    lw_inspections->clear();
    QSqlQuery inspections;
    inspections.prepare("SELECT date FROM inspections WHERE parent = :parent");
    inspections.bindValue(":parent", selectedCircuit());
    inspections.exec();
    while (inspections.next()) {
        QListWidgetItem * item = new QListWidgetItem;
        item->setText(inspections.value(0).toString());
        item->setData(Qt::UserRole, inspections.value(0).toString());
        lw_inspections->addItem(item);
    }
    enableTools();
    if (refresh) {
        setView(tr("Circuit information"));
    }
}

QDomElement MainWindow::selectedCircuitElement(QStringList * used_ids)
{
    if (!document_open) { return QDomElement(); }
    if (lw_customers->highlightedRow() < 0) { return QDomElement(); }
    if (lw_circuits->highlightedRow() < 0) { return QDomElement(); }
    QListWidgetItem * item = lw_circuits->highlightedItem();
    QDomNodeList circuits = selectedCustomerElement().elementsByTagName("circuit"); int n = -1;
    for (int i = 0; i < circuits.count(); ++i) {
        if (n == -1 && circuits.at(i).toElement().attribute("id") == item->data(Qt::UserRole).toString()) {
            n = i;
        } else {
            if (used_ids) { *used_ids << circuits.at(i).toElement().attribute("id"); }
        }
    }
    if (n == -1) { return QDomElement(); }
    return circuits.at(n).toElement();
}

void MainWindow::addInspection()
{
    QDomElement circuit = selectedCircuitElement();
    if (circuit.isNull()) { return; }
    QDomNodeList inspections = circuit.elementsByTagName("inspection");
    QStringList used_ids; bool nominal_allowed = true;
    for (int i = 0; i < inspections.count(); ++i) {
        used_ids << inspections.at(i).toElement().attribute("date");
        if (inspections.at(i).toElement().attribute("nominal", "false") == "true") { nominal_allowed = false; }
    }
    QDomElement element = document.createElement("inspection");
    element.setAttribute("date", QDateTime::currentDateTime().toString("yyyy.MM.dd-hh:mm"));
    /*ModifyDialogue * md = new ModifyDialogue(element, used_ids, nominal_allowed, this);
    if (md->exec() == QDialog::Accepted) {
        circuit.appendChild(element);
        QListWidgetItem * item = new QListWidgetItem;
        item->setText(element.attribute("date"));
        item->setData(Qt::UserRole, element.attribute("date"));
        lw_inspections->addItem(item);
        this->setWindowModified(true);
        refreshView();
    }
    delete md;*/
}

void MainWindow::modifyInspection()
{
    QStringList used_ids; bool nominal_allowed = true;
    QDomElement element = selectedInspectionElement(&used_ids, nominal_allowed);
    if (element.isNull()) { return; }
    /*ModifyDialogue * md = new ModifyDialogue(element, used_ids, nominal_allowed, this);
    if (md->exec() == QDialog::Accepted) {
        QListWidgetItem * item = lw_inspections->highlightedItem();
        item->setText(element.attribute("date"));
        item->setData(Qt::UserRole, element.attribute("date"));
        this->setWindowModified(true);
        refreshView();
    }
    delete md;*/
}

void MainWindow::removeInspection()
{
    QDomElement element = selectedInspectionElement();
    if (element.isNull()) { return; }
    QListWidgetItem * item = lw_inspections->highlightedItem();
    bool ok;
    QString confirmation = QInputDialog::getText(this, tr("Remove inspection - Leaklog"), tr("Are you sure you want to remove the selected inspection?\nTo remove all data about the inspection \"%1\" type REMOVE and confirm:").arg(item->data(Qt::UserRole).toString()), QLineEdit::Normal, "", &ok);
    if (!ok || confirmation != tr("REMOVE")) { return; }
    QDomElement circuit = selectedCircuitElement();
    if (circuit.isNull()) { return; }
    circuit.removeChild(element);
    if (item != NULL) { delete item; }
    enableTools();
    this->setWindowModified(true);
    setView(tr("Circuit information"));
}

void MainWindow::loadInspection(QListWidgetItem * item) { loadInspection(item, true); }

void MainWindow::loadInspection(QListWidgetItem * item, bool refresh)
{
    if (item == NULL) { return; }
    lw_inspections->highlightItem(item);
    enableTools();
    if (refresh) {
        setView(tr("Inspection information"));
    }
}

QDomElement MainWindow::selectedInspectionElement()
{
    bool nominal_allowed;
    return selectedInspectionElement(NULL, nominal_allowed);
}

QDomElement MainWindow::selectedInspectionElement(QStringList * used_ids, bool & nominal_allowed)
{
    if (!document_open) { return QDomElement(); }
    if (lw_customers->highlightedRow() < 0) { return QDomElement(); }
    if (lw_circuits->highlightedRow() < 0) { return QDomElement(); }
    if (lw_inspections->highlightedRow() < 0) { return QDomElement(); }
    QListWidgetItem * item = lw_inspections->highlightedItem();
    QDomNodeList inspections = selectedCircuitElement().elementsByTagName("inspection");
    int n = -1; nominal_allowed = true;
    for (int i = 0; i < inspections.count(); ++i) {
        if (n == -1 && inspections.at(i).toElement().attribute("date") == item->data(Qt::UserRole).toString()) {
            n = i;
        } else {
            if (used_ids) { *used_ids << inspections.at(i).toElement().attribute("date"); }
            if (inspections.at(i).toElement().attribute("nominal", "false") == "true") { nominal_allowed = false; }
        }
    }
    if (n == -1) { return QDomElement(); }
    return inspections.at(n).toElement();
}

void MainWindow::addVariable() { addVariable(false); }

void MainWindow::addSubvariable() { addVariable(true); }

void MainWindow::addVariable(bool subvar)
{
    if (!document_open) { return; }
    QDomElement selected; QStringList used_ids;
    if (subvar) {
        if (trw_variables->currentItem()->parent() != NULL) { return; }
        selected = selectedVariableElement(&used_ids);
        if (selected.isNull()) { return; }
        used_ids << selected.attribute("id");
    }
    QDomElement el_variables = document.documentElement().firstChildElement("variables");
    if (!subvar) {
        if (el_variables.isNull()) {
            el_variables = document.createElement("variables");
            document.documentElement().appendChild(el_variables);
        }
        QDomNodeList variables = el_variables.elementsByTagName("var");
        for (int i = 0; i < variables.count(); ++i) {
            used_ids << variables.at(i).toElement().attribute("id");
        }
    }
    QDomElement element = document.createElement("var");
    /*ModifyDialogue * md = new ModifyDialogue(element, used_ids, true, this);
    if (md->exec() == QDialog::Accepted) {
        QTreeWidgetItem * item = NULL;
        if (subvar) {
            selected.appendChild(element);
            item = new QTreeWidgetItem(trw_variables->currentItem());
        } else {
            el_variables.appendChild(element);
            item = new QTreeWidgetItem(trw_variables);
        }
        item->setText(0, element.attribute("name"));
        item->setText(1, element.attribute("id"));
        item->setText(2, element.attribute("unit"));
        item->setText(3, dict_vartypes.value(element.attribute("type")));
        this->setWindowModified(true);
        refreshView();
    }
    delete md;*/
}

void MainWindow::modifyVariable()
{
    QStringList used_ids;
    QDomElement element = selectedVariableElement(&used_ids);
    if (element.isNull()) { return; }
    /*ModifyDialogue * md = new ModifyDialogue(element, used_ids, true, this);
    if (md->exec() == QDialog::Accepted) {
        QTreeWidgetItem * item = trw_variables->currentItem();
        item->setText(0, element.attribute("name"));
        item->setText(1, element.attribute("id"));
        item->setText(2, element.attribute("unit"));
        item->setText(3, dict_vartypes.value(element.attribute("type")));
        this->setWindowModified(true);
        refreshView();
    }
    delete md;*/
}

void MainWindow::removeVariable()
{
    QDomElement element = selectedVariableElement();
    QTreeWidgetItem * item = trw_variables->currentItem();
    if (element.isNull()) { return; }
    bool ok;
    QString confirmation = QInputDialog::getText(this, tr("Remove variable - Leaklog"), tr("Are you sure you want to remove the selected variable?\nTo remove the variable \"%1\" type REMOVE and confirm:").arg(item->text(1)), QLineEdit::Normal, "", &ok);
    if (!ok || confirmation != tr("REMOVE")) { return; }
    element.parentNode().removeChild(element);
    if (item != NULL) { delete item; }
    enableTools();
    this->setWindowModified(true);
    refreshView();
}

QDomElement MainWindow::selectedVariableElement(QStringList * used_ids)
{
    if (!document_open) { return QDomElement(); }
    QTreeWidgetItem * item = NULL;
    if (trw_variables->currentIndex().isValid()) { item = trw_variables->currentItem(); }
    QDomElement el_variables = document.documentElement().firstChildElement("variables");
    if (el_variables.isNull()) { return QDomElement(); }
    QDomNodeList variables = el_variables.elementsByTagName("var"); int n = -1;
    for (int i = 0; i < variables.count(); ++i) {
        if (item && n == -1 && variables.at(i).toElement().attribute("id") == item->text(1)) {
            n = i;
        } else {
            if (used_ids) { *used_ids << variables.at(i).toElement().attribute("id"); }
        }
    }
    if (n == -1) { return QDomElement(); }
    return variables.at(n).toElement();
}

void MainWindow::addTable()
{
    if (!document_open) { return; }
    QStringList used_ids;
    QDomElement el_tables = document.documentElement().firstChildElement("tables");
    if (el_tables.isNull()) {
        el_tables = document.createElement("tables");
        document.documentElement().appendChild(el_tables);
    }
    QDomNodeList tables = el_tables.elementsByTagName("table");
    for (int i = 0; i < tables.count(); ++i) {
        used_ids << tables.at(i).toElement().attribute("id");
    }
    QDomElement element = document.createElement("table");
    /*ModifyDialogue * md = new ModifyDialogue(element, used_ids, true, this);
    if (md->exec() == QDialog::Accepted) {
        el_tables.appendChild(element);
        cb_table->addItem(element.attribute("id"));
        cb_table_edit->addItem(element.attribute("id"));
        this->setWindowModified(true);
        refreshView();
    }
    delete md;*/
}

void MainWindow::modifyTable()
{
    QStringList used_ids;
    QDomElement element = selectedTableElement(&used_ids);
    if (element.isNull()) { return; }
    /*ModifyDialogue * md = new ModifyDialogue(element, used_ids, true, this);
    if (md->exec() == QDialog::Accepted) {
        int i = cb_table_edit->currentIndex();
        int j = cb_table->currentIndex();
        cb_table_edit->removeItem(i);
        cb_table->removeItem(i);
        cb_table_edit->insertItem(i, element.attribute("id"));
        cb_table->insertItem(i, element.attribute("id"));
        cb_table_edit->setCurrentIndex(i);
        cb_table->setCurrentIndex(j);
        this->setWindowModified(true);
        refreshView();
    }
    delete md;*/
}

void MainWindow::removeTable()
{
    QDomElement element = selectedTableElement();
    if (element.isNull()) { return; }
    bool ok;
    QString confirmation = QInputDialog::getText(this, tr("Remove table - Leaklog"), tr("Are you sure you want to remove the selected table?\nTo remove the table \"%1\" type REMOVE and confirm:").arg(cb_table_edit->currentText()), QLineEdit::Normal, "", &ok);
    if (!ok || confirmation != tr("REMOVE")) { return; }
    element.parentNode().removeChild(element);
    int i = cb_table_edit->currentIndex();
    cb_table_edit->removeItem(i);
    cb_table->removeItem(i);
    enableTools();
    this->setWindowModified(true);
    refreshView();
}

void MainWindow::loadTable(const QString &)
{
    if (!document_open) { return; }
    if (cb_table_edit->currentIndex() < 0) { enableTools(); return; }
    lw_table_variables->clear();
    QDomElement el_tables = document.documentElement().firstChildElement("tables");
    if (el_tables.isNull()) { return; }
    QDomNodeList tables = el_tables.elementsByTagName("table");
    QDomElement table;
    for (int i = 0; i < tables.count(); ++i) {
        if (tables.at(i).toElement().attribute("id") == cb_table_edit->currentText())
            { table = tables.at(i).toElement(); break; }
    }
    if (table.isNull()) { return; }
    QDomNodeList variables = table.elementsByTagName("var");
    for (int i = 0; i < variables.count(); ++i) {
        QDomElement var = variables.at(i).toElement();
        QListWidgetItem * item = new QListWidgetItem;
        QDomElement el_variables = document.documentElement().firstChildElement("variables");
        QDomElement variable;
        if (!el_variables.isNull()) {
            variable = el_variables.firstChildElement("var");
            while (!variable.isNull()) {
                if (variable.attribute("id") == var.attribute("id")) { break; }
                variable = variable.nextSiblingElement();
            }
        }
        if (!variable.isNull()) {
            item->setText(tr("%1 (%2)").arg(var.attribute("id")).arg(variable.attribute("name")));
        } else {
            item->setText(var.attribute("id"));
        }
        item->setData(Qt::UserRole, var.attribute("id"));
        lw_table_variables->addItem(item);
    }
    enableTools();
}

QDomElement MainWindow::selectedTableElement(QStringList * used_ids)
{
    if (!document_open) { return QDomElement(); }
    if (cb_table_edit->currentIndex() < 0) { return QDomElement(); }
    QDomElement el_tables = document.documentElement().firstChildElement("tables");
    if (el_tables.isNull()) { return QDomElement(); }
    QDomNodeList tables = el_tables.elementsByTagName("table"); int n = -1;
    for (int i = 0; i < tables.count(); ++i) {
        if (n == -1 && tables.at(i).toElement().attribute("id") == cb_table_edit->currentText()) {
            n = i;
        } else {
            if (used_ids) { *used_ids << tables.at(i).toElement().attribute("id"); }
        }
    }
    if (n == -1) { return QDomElement(); }
    return tables.at(n).toElement();
}

void MainWindow::addTableVariable()
{
    if (!document_open) { return; }
    QDomElement table = selectedTableElement();
    if (table.isNull()) { return; }
    QDomElement el_variables = document.documentElement().firstChildElement("variables");
    if (el_variables.isNull()) { return; }
    QDomElement variable = el_variables.firstChildElement("var");
    if (variable.isNull()) { return; }
    QStringList used_ids;
    for (int i = 0; i < lw_table_variables->count(); ++i) {
        used_ids << lw_table_variables->item(i)->data(Qt::UserRole).toString();
    }
    QDialog * d = new QDialog(this);
	d->setWindowTitle(tr("Add existing variable - Leaklog"));
    d->setMinimumSize(QSize(300, 350));
        QVBoxLayout * vl = new QVBoxLayout(d);
        vl->setMargin(6); vl->setSpacing(6);
            QHBoxLayout * hl = new QHBoxLayout(d);
            hl->setMargin(0); hl->setSpacing(6);
                QLabel * lbl = new QLabel(tr("Search:"), d);
                ExtendedLineEdit * sle = new ExtendedLineEdit(d);
            hl->addWidget(lbl);
            hl->addWidget(sle);
        vl->addLayout(hl);
            MTListWidget * lw = new MTListWidget(d);
            QObject::connect(lw, SIGNAL(itemDoubleClicked(QListWidgetItem *)), d, SLOT(accept()));
            QObject::connect(sle, SIGNAL(textChanged(QLineEdit *, const QString &)), lw, SLOT(filterItems(QLineEdit *, const QString &)));
        vl->addWidget(lw);
            QDialogButtonBox * bb = new QDialogButtonBox(d);
            bb->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
            QObject::connect(bb, SIGNAL(accepted()), d, SLOT(accept()));
            QObject::connect(bb, SIGNAL(rejected()), d, SLOT(reject()));
        vl->addWidget(bb);
    while (!variable.isNull()) {
        if (!used_ids.contains(variable.attribute("id"))) {
            QListWidgetItem * item = new QListWidgetItem;
            item->setText(variable.attribute("name").isEmpty() ? variable.attribute("id") : tr("%1 (%2)").arg(variable.attribute("id")).arg(variable.attribute("name")));
            item->setData(Qt::UserRole, variable.attribute("id"));
            lw->addItem(item);
        }
        variable = variable.nextSiblingElement();
    }
    if (d->exec() == QDialog::Accepted && lw->currentIndex().isValid()) {
        QDomElement var = document.createElement("var");
        var.setAttribute("id", lw->currentItem()->data(Qt::UserRole).toString());
        table.appendChild(var);
        loadTable(cb_table_edit->currentText());
        this->setWindowModified(true);
        refreshView();
    }
    delete d;
}

void MainWindow::removeTableVariable()
{
    if (!document_open) { return; }
    if (!lw_table_variables->currentIndex().isValid()) { return; }
    QListWidgetItem * item = lw_table_variables->currentItem();
    QDomElement table = selectedTableElement();
    if (table.isNull()) { return; }
    switch (QMessageBox::information(this, tr("Remove variable - Leaklog"), tr("Are you sure you want to remove the variable \"%1\" from the selected table?").arg(item->data(Qt::UserRole).toString()), tr("Remove"), tr("Cancel"), 0, 1)) {
        case 0: // Remove
            break;
        case 1: // Cancel
            return; break;
    }
    QDomNodeList variables = table.elementsByTagName("var");
    for (int i = 0; i < variables.count(); ++i) {
        if (variables.at(i).toElement().attribute("id") == item->data(Qt::UserRole).toString())
            { table.removeChild(variables.at(i)); }
    }
    loadTable(cb_table_edit->currentText());
    this->setWindowModified(true);
    refreshView();
}

void MainWindow::addWarning()
{
    if (!document_open) { return; }
    QStringList used_ids; selectedVariableElement(&used_ids);
    QDomElement el_warnings = document.documentElement().firstChildElement("warnings");
    if (el_warnings.isNull()) {
        el_warnings = document.createElement("warnings");
        document.documentElement().appendChild(el_warnings);
    }
    QDomElement element = document.createElement("warning");
    /*ModifyWarningDialogue * md = new ModifyWarningDialogue(element, used_ids, this);
    if (md->exec() == QDialog::Accepted) {
        el_warnings.appendChild(element);
        QListWidgetItem * item = new QListWidgetItem;
        item->setText(element.attribute("description").isEmpty() ? element.attribute("id") : tr("%1 (%2)").arg(element.attribute("id")).arg(element.attribute("description")));
        item->setData(Qt::UserRole, element.attribute("id"));
        lw_warnings->addItem(item);
        this->setWindowModified(true);
        refreshView();
    }
    delete md;*/
}

void MainWindow::modifyWarning()
{
    QStringList used_ids; selectedVariableElement(&used_ids);
    QDomElement element = selectedWarningElement();
    if (element.isNull()) { return; }
    /*ModifyWarningDialogue * md = new ModifyWarningDialogue(element, used_ids, this);
    if (md->exec() == QDialog::Accepted) {
        QListWidgetItem * item = lw_warnings->currentItem();
        item->setText(element.attribute("description").isEmpty() ? element.attribute("id") : tr("%1 (%2)").arg(element.attribute("id")).arg(element.attribute("description")));
        item->setData(Qt::UserRole, element.attribute("id"));
        this->setWindowModified(true);
        refreshView();
    }
    delete md;*/
}

void MainWindow::removeWarning()
{
    QDomElement element = selectedWarningElement();
    if (element.isNull()) { return; }
    bool ok;
    QString confirmation = QInputDialog::getText(this, tr("Remove warning - Leaklog"), tr("Are you sure you want to remove the selected warning?\nTo remove the warning \"%1\" type REMOVE and confirm:").arg(element.attribute("description").isEmpty() ? element.attribute("id") : tr("%1 (%2)").arg(element.attribute("id")).arg(element.attribute("description"))), QLineEdit::Normal, "", &ok);
    if (!ok || confirmation != tr("REMOVE")) { return; }
    element.parentNode().removeChild(element);
    delete lw_warnings->currentItem();
    enableTools();
    this->setWindowModified(true);
    refreshView();
}

QDomElement MainWindow::selectedWarningElement()
{
    if (!document_open) { return QDomElement(); }
    if (!lw_warnings->currentIndex().isValid()) { return QDomElement(); }
    QListWidgetItem * item = lw_warnings->currentItem();
    QDomElement el_warnings = document.documentElement().firstChildElement("warnings");
    if (el_warnings.isNull()) { return QDomElement(); }
    QDomNodeList warnings = el_warnings.elementsByTagName("warning"); int n = -1;
    for (int i = 0; i < warnings.count(); ++i) {
        if (warnings.at(i).toElement().attribute("id") == item->data(Qt::UserRole).toString()) { n = i; break; }
    }
    if (n == -1) { return QDomElement(); }
    return warnings.at(n).toElement();
}

void MainWindow::exportCustomerData()
{
    if (!document_open) { return; }
    QDomElement customer = selectedCustomerElement();
    if (customer.isNull()) { return; }
    QString path = QFileDialog::getSaveFileName(this, tr("Export customer data - Leaklog"), QString("%1.lklg").arg(customer.attribute("id")), tr("Leaklog Document (*.lklg)"));
	if (path.isNull() || path.isEmpty()) { return; }
    QFile file(path);
    if (!file.open(QFile::WriteOnly)) {
		QMessageBox::critical(this, tr("Export customer data - Leaklog"), tr("Cannot write file %1:\n%2.").arg(path).arg(file.errorString()));
		return;
    }
    QDomDocument data;
    QDomElement root = data.createElement("leaklog");
    data.appendChild(root);
    QDomElement variables = document.documentElement().firstChildElement("variables");
    if (!variables.isNull()) { root.appendChild(variables.cloneNode(true)); }
    QDomElement tables = document.documentElement().firstChildElement("tables");
    if (!tables.isNull()) { root.appendChild(tables.cloneNode(true)); }
    QDomElement customers = document.createElement("customers");
    root.appendChild(customers);
    customers.appendChild(customer.cloneNode(true));
    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << data.toString(4);
    file.close();
}

void MainWindow::exportCircuitData()
{
    if (!document_open) { return; }
    QDomElement customer = selectedCustomerElement();
    if (customer.isNull()) { return; }
    QDomElement circuit = selectedCircuitElement();
    if (circuit.isNull()) { return; }
    QString path = QFileDialog::getSaveFileName(this, tr("Export circuit data - Leaklog"), QString("%1_%2.lklg").arg(customer.attribute("id")).arg(circuit.attribute("id")), tr("Leaklog Document (*.lklg)"));
	if (path.isNull() || path.isEmpty()) { return; }
    QFile file(path);
    if (!file.open(QFile::WriteOnly)) {
		QMessageBox::critical(this, tr("Export circuit data - Leaklog"), tr("Cannot write file %1:\n%2.").arg(path).arg(file.errorString()));
		return;
    }
    QDomDocument data;
    QDomElement root = data.createElement("leaklog");
    data.appendChild(root);
    QDomElement variables = document.documentElement().firstChildElement("variables");
    if (!variables.isNull()) { root.appendChild(variables.cloneNode(true)); }
    QDomElement tables = document.documentElement().firstChildElement("tables");
    if (!tables.isNull()) { root.appendChild(tables.cloneNode(true)); }
    QDomElement customers = document.createElement("customers");
    root.appendChild(customers);
    QDomNode customer_clone = customer.cloneNode(false);
    customers.appendChild(customer_clone);
    customer_clone.appendChild(circuit.cloneNode(true));
    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << data.toString(4);
    file.close();
}

void MainWindow::exportInspectionData()
{
    if (!document_open) { return; }
    QDomElement customer = selectedCustomerElement();
    if (customer.isNull()) { return; }
    QDomElement circuit = selectedCircuitElement();
    if (circuit.isNull()) { return; }
    QDomElement inspection = selectedInspectionElement();
    if (inspection.isNull()) { return; }
    QString path = QFileDialog::getSaveFileName(this, tr("Export inspection data - Leaklog"), QString("%1_%2_%3.lklg").arg(customer.attribute("id")).arg(circuit.attribute("id")).arg(inspection.attribute("date").replace(":", ".")), tr("Leaklog Document (*.lklg)"));
	if (path.isNull() || path.isEmpty()) { return; }
    QFile file(path);
    if (!file.open(QFile::WriteOnly)) {
		QMessageBox::critical(this, tr("Export inspection data - Leaklog"), tr("Cannot write file %1:\n%2.").arg(path).arg(file.errorString()));
		return;
    }
    QDomDocument data;
    QDomElement root = data.createElement("leaklog");
    data.appendChild(root);
    QDomElement variables = document.documentElement().firstChildElement("variables");
    if (!variables.isNull()) { root.appendChild(variables.cloneNode(true)); }
    QDomElement tables = document.documentElement().firstChildElement("tables");
    if (!tables.isNull()) { root.appendChild(tables.cloneNode(true)); }
    QDomElement customers = document.createElement("customers");
    root.appendChild(customers);
    QDomNode customer_clone = customer.cloneNode(false);
    customers.appendChild(customer_clone);
    QDomNode circuit_clone = circuit.cloneNode(false);
    customer_clone.appendChild(circuit_clone);
    circuit_clone.appendChild(inspection.cloneNode(true));
    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << data.toString(4);
    file.close();
}

void MainWindow::importData()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Import data - Leaklog"), "", tr("Leaklog Documents (*.lklg);;All files (*.*)"));
	if (path.isNull() || path.isEmpty()) { return; }
    QFile file(path);
    if (!file.open(QFile::ReadOnly)) {
		QMessageBox::critical(this, tr("Import data - Leaklog"), tr("Cannot read file %1:\n%2.").arg(path).arg(file.errorString()));
		return;
    }
    QDomDocument data;
    data.setContent(&file);
    file.close();
    QDomElement el_customers = document.documentElement().firstChildElement("customers");
    if (el_customers.isNull()) { return; }
    QDomNodeList customers = el_customers.elementsByTagName("customer");
    QDomElement data_el_customers = data.documentElement().firstChildElement("customers");
    if (data_el_customers.isNull()) { return; }
    QDomNodeList data_customers = data_el_customers.elementsByTagName("customer");
    for (int dc = 0; dc < data_customers.count(); ++dc) {
        bool customer_found = false;
        for (int c = 0; c < customers.count(); ++c) {
            if (data_customers.at(dc).toElement().attribute("id") != customers.at(c).toElement().attribute("id")) { continue; }
            customer_found = true;
            QDomElement data_customer = data_customers.at(dc).toElement();
            QDomElement customer = customers.at(c).toElement();
            QDomNamedNodeMap dc_attributes = data_customer.attributes();
            for (int a = 0; a < dc_attributes.count(); ++a) {
                customer.setAttribute(dc_attributes.item(a).nodeName(), dc_attributes.item(a).nodeValue());
            }
            QDomNodeList data_circuits = data_customer.elementsByTagName("circuit");
            QDomNodeList circuits = customer.elementsByTagName("circuit");
            for (int dcc = 0; dcc < data_circuits.count(); ++dcc) {
                bool circuit_found = false;
                for (int cc = 0; cc < circuits.count(); ++cc) {
                    if (data_circuits.at(dcc).toElement().attribute("id") != circuits.at(cc).toElement().attribute("id")) { continue; }
                    circuit_found = true;
                    QDomElement data_circuit = data_circuits.at(dcc).toElement();
                    QDomElement circuit = circuits.at(cc).toElement();
                    QDomNamedNodeMap dcc_attributes = data_circuit.attributes();
                    for (int a = 0; a < dcc_attributes.count(); ++a) {
                        circuit.setAttribute(dcc_attributes.item(a).nodeName(), dcc_attributes.item(a).nodeValue());
                    }
                    QDomNodeList data_inspections = data_circuit.elementsByTagName("inspection");
                    QDomNodeList inspections = circuit.elementsByTagName("inspection");
                    for (int di = 0; di < data_inspections.count(); ++di) {
                        bool inspection_found = false;
                        for (int i = 0; i < inspections.count(); ++i) {
                            if (data_inspections.at(di).toElement().attribute("date") != inspections.at(i).toElement().attribute("date")) { continue; }
                            inspection_found = true;
                            QDomElement data_inspection = data_inspections.at(di).toElement();
                            QDomElement inspection = inspections.at(i).toElement();
                            QDomNamedNodeMap di_attributes = data_inspection.attributes();
                            for (int a = 0; a < di_attributes.count(); ++a) {
                                inspection.setAttribute(di_attributes.item(a).nodeName(), di_attributes.item(a).nodeValue());
                            }
                            QDomNodeList data_variables = data_inspection.elementsByTagName("var");
                            QDomNodeList variables = inspection.elementsByTagName("var");
                            for (int dv = 0; dv < data_variables.count(); ++dv) {
                                bool variable_found = false;
                                for (int v = 0; v < variables.count(); ++v) {
                                    if (data_variables.at(dv).toElement().attribute("id") != variables.at(v).toElement().attribute("id")) { continue; }
                                    variable_found = true;
                                    QDomElement data_variable = data_variables.at(dv).toElement();
                                    QDomElement variable = variables.at(v).toElement();
                                    QDomNamedNodeMap dv_attributes = data_variable.attributes();
                                    for (int a = 0; a < dv_attributes.count(); ++a) {
                                        variable.setAttribute(dv_attributes.item(a).nodeName(), dv_attributes.item(a).nodeValue());
                                    }
                                    while (variable.hasChildNodes()) { variable.removeChild(variable.firstChild()); }
                                    QDomNodeList dv_content = data_variable.childNodes();
                                    for (int dvc = 0; dvc < dv_content.count(); ++dvc) {
                                        variable.appendChild(dv_content.at(dvc).cloneNode(true));
                                    }
                                }
                                if (!variable_found) {
                                    inspection.appendChild(data_variables.at(dv).cloneNode(true));
                                }
                            }
                        }
                        if (!inspection_found) {
                            QDomElement element = data_inspections.at(di).cloneNode(true).toElement();
                            circuit.appendChild(element);
                            QDomElement selected_circuit = selectedCircuitElement();
                            if (!selected_circuit.isNull() && selected_circuit.attribute("id") == circuit.attribute("id")) {
                                QListWidgetItem * item = new QListWidgetItem;
                                item->setText(element.attribute("date"));
                                item->setData(Qt::UserRole, element.attribute("date"));
                                lw_inspections->addItem(item);
                            }
                        }
                    }
                }
                if (!circuit_found) {
                    QDomElement element = data_circuits.at(dcc).cloneNode(true).toElement();
                    customer.appendChild(element);
                    QDomElement selected_customer = selectedCustomerElement();
                    if (!selected_customer.isNull() && selected_customer.attribute("id") == customer.attribute("id")) {
                        QListWidgetItem * item = new QListWidgetItem;
                        item->setText(element.attribute("id"));
                        item->setData(Qt::UserRole, element.attribute("id"));
                        lw_circuits->addItem(item);
                    }
                }
            }
        }
        if (!customer_found) {
            QDomElement element = data_customers.at(dc).cloneNode(true).toElement();
            el_customers.appendChild(element);
            QListWidgetItem * item = new QListWidgetItem;
            item->setText(element.attribute("company").isEmpty() ? element.attribute("id") : tr("%1 (%2)").arg(element.attribute("id")).arg(element.attribute("company")));
            item->setData(Qt::UserRole, element.attribute("id"));
            lw_customers->addItem(item);
        }
    }
    this->setWindowModified(true);
    refreshView();
}

QStringList MainWindow::listVariableIds(bool all)
{
    QStringList ids; bool sub_empty = false;
    QSqlQuery query("SELECT variables.var_id, subvariables.var_id FROM variables LEFT JOIN subvariables ON variables.var_id = subvariables.parent");
    while (query.next()) {
        sub_empty = query.value(1).toString().isEmpty();
        if (all || sub_empty) { ids << query.value(0).toString(); }
        if (!sub_empty) { ids << query.value(1).toString(); }
    }
    return ids;
}

MTDictionary MainWindow::parseExpression(const QString & exp, QStringList * used_ids)
{
    MTDictionary dict_exp(true);
    if (!exp.isEmpty()) {
        if (!used_ids->contains("refrigerant_amount")) { *used_ids << "refrigerant_amount"; }
        if (!used_ids->contains("oil_amount")) { *used_ids << "oil_amount"; }
        if (!used_ids->contains("sum")) { *used_ids << "sum"; }
        QSet<int> matched;
        for (int i = 0; i < used_ids->count(); ++i) {
            QRegExp expression(QString("\\b%1\\b").arg(used_ids->at(i)));
            int index = exp.indexOf(expression);
            while (index >= 0) {
                int length = expression.matchedLength();
                if (!matched.contains(index)) {
                    for (int j = index; j < index + length; j++) { matched << j; }
                }
                index = exp.indexOf(expression, index + length);
            }
        }
        QString id_, f_; bool last_id = false; bool last_sum = false;
        for (int i = 0; i < exp.length(); ++i) {
            if (matched.contains(i)) {
                if (!f_.isEmpty()) {
                    dict_exp.insert(f_, "function");
                    f_.clear();
                }
                last_id = true;
                id_.append(exp.at(i));
            } else {
                if (!id_.isEmpty()) {
                    if (id_ == "sum") {
                        last_sum = true;
                    } else {
                        if (id_ == "refrigerant_amount" || id_ == "oil_amount") {
                            dict_exp.insert(id_, "circuit_attribute");
                        } else {
                            dict_exp.insert(id_, last_sum ? "sum" : "id");
                        }
                        last_sum = false;
                    }
                    id_.clear();
                }
                last_id = false;
                f_.append(exp.at(i));
            }
        }
        if (!f_.isEmpty()) {
            dict_exp.insert(f_, "function");
        }
        if (!id_.isEmpty()) {
            if (id_ == "refrigerant_amount" || id_ == "oil_amount") {
                dict_exp.insert(id_, "circuit_attribute");
            } else {
                dict_exp.insert(id_, last_sum ? "sum" : "id");
            }
        }
    }
    return dict_exp;
}
