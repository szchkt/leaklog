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
    QFile file(path);
    if (!file.open(QFile::ReadOnly)) {
		QMessageBox::critical(this, tr("Open document - Leaklog"), tr("Cannot read file %1:\n%2.").arg(path).arg(file.errorString()));
		this->setWindowTitle(tr("Leaklog"));
		return;
    }
    clearAll();
    document.setContent(&file);
    file.close();
    QDomElement el_customers = document.documentElement().firstChildElement("customers");
    if (!el_customers.isNull()) {
        QDomNodeList customers = el_customers.elementsByTagName("customer");
        for (int i = 0; i < customers.count(); ++i) {
            QDomElement element = customers.at(i).toElement();
            QListWidgetItem * item = new QListWidgetItem;
            item->setText(element.attribute("company").isEmpty() ? element.attribute("id") : tr("%1 (%2)").arg(element.attribute("id")).arg(element.attribute("company")));
            item->setData(Qt::UserRole, element.attribute("id"));
            lw_customers->addItem(item);
        }
    }
    QDomElement el_variables = document.documentElement().firstChildElement("variables");
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
    document_open = true;
    document_path = path;
#ifdef Q_WS_MAC
	this->setWindowTitle(QString("%1[*]").arg(QFileInfo(file).baseName()));
#else
    this->setWindowTitle(QString("%1[*] - Leaklog").arg(QFileInfo(file).baseName()));
#endif
    this->setWindowModified(false);
    setAllEnabled(true);
    cb_view->setCurrentIndex(view_indices.value(tr("All customers")));
    viewChanged(cb_view->currentText());
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
    viewChanged(cb_view->currentText());
}

void MainWindow::closeDocument()
{
	if (saveChangesBeforeProceeding(tr("Close document - Leaklog"), false)) { return; }
    document_open = false;
	clearAll(); setAllEnabled(false);
	this->setWindowTitle(tr("Leaklog"));
	this->setWindowModified(false);
}

void MainWindow::viewChanged(const QString & view)
{
    if (!document_open) { wv_main->setHtml(QString()); return; }
    QBuffer device;
    device.setData(document.toString(1).toUtf8());
    device.open(QIODevice::ReadOnly);

    QXmlQuery query;
    query.bindVariable("inputDocument", &device);

    if (view == tr("All customers")) {
        query.setQuery(dict_queries.value(view).arg(dict_i18n_javascript));
    } else if (view == tr("Customer information") && lw_customers->highlightedRow() >= 0) {
        query.setQuery(dict_queries.value(view).arg(dict_i18n_javascript).arg(lw_customers->highlightedItem()->data(Qt::UserRole).toString()));
    } else if (view == tr("Circuit information") && lw_customers->highlightedRow() >= 0 && cb_circuit->currentIndex() >= 0) {
        query.setQuery(dict_queries.value(view).arg(dict_i18n_javascript).arg(lw_customers->highlightedItem()->data(Qt::UserRole).toString()).arg(cb_circuit->currentText()));
    } else if (view == tr("Inspection information") && lw_customers->highlightedRow() >= 0 && cb_circuit->currentIndex() >= 0 && cb_inspection->currentIndex() >= 0) {
        query.setQuery(dict_queries.value(view).arg(dict_i18n_javascript).arg(lw_customers->highlightedItem()->data(Qt::UserRole).toString()).arg(cb_circuit->currentText()).arg(cb_inspection->currentText()));
    }

    if (!query.isValid()) { wv_main->setHtml(QString()); return; }
    QByteArray out;
    QBuffer buffer(&out);
    buffer.open(QIODevice::ReadWrite);
    QXmlFormatter formatter(query, &buffer);
    if (!query.evaluateTo(&formatter)) { wv_main->setHtml(QString()); return; }
    buffer.close();
    wv_main->setHtml(QString::fromUtf8(out.constData()));
}

void MainWindow::addCustomer()
{
    if (!document_open) { return; }
    QDomElement el_customers = document.documentElement().firstChildElement("customers");
    if (el_customers.isNull()) {
        el_customers = document.createElement("customers");
        document.documentElement().appendChild(el_customers);
    }
    QDomNodeList customers = el_customers.elementsByTagName("customer");
    QStringList used_ids;
    for (int i = 0; i < customers.count(); ++i) {
        used_ids << customers.at(i).toElement().attribute("id");
    }
    QDomElement element = document.createElement("customer");
    ModifyDialogue * md = new ModifyDialogue(element, used_ids, true, this);
    if (md->exec() == QDialog::Accepted) {
        el_customers.appendChild(element);
        QListWidgetItem * item = new QListWidgetItem;
        item->setText(element.attribute("company").isEmpty() ? element.attribute("id") : tr("%1 (%2)").arg(element.attribute("id")).arg(element.attribute("company")));
        item->setData(Qt::UserRole, element.attribute("id"));
        lw_customers->addItem(item);
        this->setWindowModified(true);
        viewChanged(cb_view->currentText());
    }
    delete md;
}

void MainWindow::modifyCustomer()
{
    QStringList used_ids;
    QDomElement element = selectedCustomerElement(&used_ids);
    if (element.isNull()) { return; }
    ModifyDialogue * md = new ModifyDialogue(element, used_ids, true, this);
    if (md->exec() == QDialog::Accepted) {
        QListWidgetItem * item = lw_customers->highlightedItem();
        item->setText(element.attribute("company").isEmpty() ? element.attribute("id") : tr("%1 (%2)").arg(element.attribute("id")).arg(element.attribute("company")));
        item->setData(Qt::UserRole, element.attribute("id"));
        gb_customer->setTitle(item->text());
        this->setWindowModified(true);
        viewChanged(cb_view->currentText());
    }
    delete md;
}

void MainWindow::removeCustomer()
{
    if (!document_open) { return; }
    if (lw_customers->highlightedRow() < 0) { return; }
    QListWidgetItem * item = lw_customers->highlightedItem();
    bool ok;
    QString confirmation = QInputDialog::getText(this, tr("Remove customer - Leaklog"), tr("Are you sure you want to remove the selected customer?\nTo remove all data about the customer \"%1\" type REMOVE and confirm:").arg(item->data(Qt::UserRole).toString()), QLineEdit::Normal, "", &ok);
    if (!ok || confirmation != tr("REMOVE")) { return; }
    QDomElement el_customers = document.documentElement().firstChildElement("customers");
    if (el_customers.isNull()) { return; }
    QDomNodeList customers = el_customers.elementsByTagName("customer");
    for (int i = 0; i < customers.count(); ++i) {
        if (customers.at(i).toElement().attribute("id") == item->data(Qt::UserRole).toString()) {
            el_customers.removeChild(customers.at(i));
            break;
        }
    }
    delete item;
    gb_customer->setEnabled(false);
    actionRemove_customer->setEnabled(false);
    this->setWindowModified(true);
    viewChanged(cb_view->currentText());
}

void MainWindow::loadCustomer(QListWidgetItem * item) { loadCustomer(item, true); }

void MainWindow::loadCustomer(QListWidgetItem * item, bool refresh)
{
    if (item == NULL) { return; }
    lw_customers->highlightItem(item);
    QDomElement customer = selectedCustomerElement();
    if (!customer.isNull()) { loadCustomer(customer, refresh); }
}

void MainWindow::loadCustomer(const QDomElement & element, bool refresh)
{
    gb_customer->setEnabled(true);
    gb_customer->setTitle(lw_customers->highlightedItem()->text());
    cb_circuit->clear(); cb_inspection->clear();
    QDomNodeList circuits = element.elementsByTagName("circuit");
    for (int i = 0; i < circuits.count(); ++i) {
        cb_circuit->addItem(circuits.at(i).toElement().attribute("id"));
    }
    enableTools();
    if (refresh) {
        cb_view->setCurrentIndex(view_indices.value(tr("Customer information")));
        viewChanged(cb_view->currentText());
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
    ModifyDialogue * md = new ModifyDialogue(element, used_ids, true, this);
    if (md->exec() == QDialog::Accepted) {
        customer.appendChild(element);
        cb_circuit->addItem(element.attribute("id"));
        this->setWindowModified(true);
        viewChanged(cb_view->currentText());
    }
    delete md;
}

void MainWindow::modifyCircuit()
{
    QStringList used_ids;
    QDomElement element = selectedCircuitElement(&used_ids);
    if (element.isNull()) { return; }
    ModifyDialogue * md = new ModifyDialogue(element, used_ids, true, this);
    if (md->exec() == QDialog::Accepted) {
        int i = cb_circuit->currentIndex();
        cb_circuit->removeItem(i);
        cb_circuit->insertItem(i, element.attribute("id"));
        cb_circuit->setCurrentIndex(i);
        this->setWindowModified(true);
        viewChanged(cb_view->currentText());
    }
    delete md;
}

void MainWindow::removeCircuit()
{
    
}

void MainWindow::loadCircuit(const QString & id) { loadCircuit(id, true); }

void MainWindow::loadCircuit(const QString &, bool refresh)
{
    QDomElement element = selectedCircuitElement();
    if (element.isNull()) { return; }
    cb_inspection->clear();
    QDomNodeList inspections = element.elementsByTagName("inspection");
    for (int i = 0; i < inspections.count(); ++i) {
        cb_inspection->addItem(inspections.at(i).toElement().attribute("date"));
    }
    enableTools();
    if (refresh) {
        cb_view->setCurrentIndex(view_indices.value(tr("Circuit information")));
        viewChanged(cb_view->currentText());
    }
}

QDomElement MainWindow::selectedCircuitElement(QStringList * used_ids)
{
    if (!document_open) { return QDomElement(); }
    if (lw_customers->highlightedRow() < 0) { return QDomElement(); }
    if (cb_circuit->currentIndex() < 0) { return QDomElement(); }
    QDomNodeList circuits = selectedCustomerElement().elementsByTagName("circuit"); int n = -1;
    for (int i = 0; i < circuits.count(); ++i) {
        if (n == -1 && circuits.at(i).toElement().attribute("id") == cb_circuit->currentText()) {
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
    ModifyDialogue * md = new ModifyDialogue(element, used_ids, nominal_allowed, this);
    if (md->exec() == QDialog::Accepted) {
        circuit.appendChild(element);
        cb_inspection->addItem(element.attribute("date"));
        this->setWindowModified(true);
        viewChanged(cb_view->currentText());
    }
    delete md;
}

void MainWindow::modifyInspection()
{
    QStringList used_ids; bool nominal_allowed = true;
    QDomElement element = selectedInspectionElement(&used_ids, nominal_allowed);
    if (element.isNull()) { return; }
    ModifyDialogue * md = new ModifyDialogue(element, used_ids, nominal_allowed, this);
    if (md->exec() == QDialog::Accepted) {
        int i = cb_inspection->currentIndex();
        cb_inspection->removeItem(i);
        cb_inspection->insertItem(i, element.attribute("date"));
        cb_inspection->setCurrentIndex(i);
        this->setWindowModified(true);
        viewChanged(cb_view->currentText());
    }
    delete md;
}

void MainWindow::loadInspection(const QString & id) { loadInspection(id, true); }

void MainWindow::loadInspection(const QString &, bool refresh)
{
    //QDomElement element = selectedInspectionElement();
    //if (element.isNull()) { return; }
    if (cb_inspection->currentIndex() < 0) { return; }
    enableTools();
    if (refresh) {
        cb_view->setCurrentIndex(view_indices.value(tr("Inspection information")));
        viewChanged(cb_view->currentText());
    }
}

QDomElement MainWindow::selectedInspectionElement(QStringList * used_ids, bool & nominal_allowed)
{
    if (!document_open) { return QDomElement(); }
    if (lw_customers->highlightedRow() < 0) { return QDomElement(); }
    if (cb_circuit->currentIndex() < 0) { return QDomElement(); }
    if (cb_inspection->currentIndex() < 0) { return QDomElement(); }
    QDomNodeList inspections = selectedCircuitElement().elementsByTagName("inspection");
    int n = -1; nominal_allowed = true;
    for (int i = 0; i < inspections.count(); ++i) {
        if (n == -1 && inspections.at(i).toElement().attribute("date") == cb_inspection->currentText()) {
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
    ModifyDialogue * md = new ModifyDialogue(element, used_ids, true, this);
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
        viewChanged(cb_view->currentText());
    }
    delete md;
}

void MainWindow::modifyVariable()
{
    QStringList used_ids;
    QDomElement element = selectedVariableElement(&used_ids);
    if (element.isNull()) { return; }
    ModifyDialogue * md = new ModifyDialogue(element, used_ids, true, this);
    if (md->exec() == QDialog::Accepted) {
        QTreeWidgetItem * item = trw_variables->currentItem();
        item->setText(0, element.attribute("name"));
        item->setText(1, element.attribute("id"));
        item->setText(2, element.attribute("unit"));
        item->setText(3, dict_vartypes.value(element.attribute("type")));
        this->setWindowModified(true);
        viewChanged(cb_view->currentText());
    }
    delete md;
}

QDomElement MainWindow::selectedVariableElement(QStringList * used_ids)
{
    if (!document_open) { return QDomElement(); }
    if (!trw_variables->currentIndex().isValid()) { return QDomElement(); }
    QTreeWidgetItem * item = trw_variables->currentItem();
    QDomElement el_variables = document.documentElement().firstChildElement("variables");
    if (el_variables.isNull()) { return QDomElement(); }
    QDomNodeList variables = el_variables.elementsByTagName("var"); int n = -1;
    for (int i = 0; i < variables.count(); ++i) {
        if (n == -1 && variables.at(i).toElement().attribute("id") == item->text(1)) {
            n = i;
        } else {
            if (used_ids) { *used_ids << variables.at(i).toElement().attribute("id"); }
        }
    }
    if (n == -1) { return QDomElement(); }
    return variables.at(n).toElement();
}
