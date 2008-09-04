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
	if (db.isOpen() && this->isWindowModified()) {
		switch (QMessageBox::information(this, title, tr("Save changes before proceeding?"), tr("&Save"), tr("&Discard"), tr("Cancel"), 0, 2)) {
			case 0: // Save
				save(); if (close_) { closeDocument(false); }; return false;
				break;
			case 1: // Discard
				if (close_) { closeDocument(false); }; return false;
				break;
			case 2: // Cancel
				return true;
				break;
		}
	} else if (db.isOpen() && !this->isWindowModified()) {
		if (close_) { closeDocument(false); }; return false;
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
    /*document.clear();
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
    file.close();*/
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
    QSqlQuery customers("SELECT id, company FROM customers");
    while (customers.next()) {
        QListWidgetItem * item = new QListWidgetItem;
        item->setText(customers.value(1).toString().isEmpty() ? customers.value(0).toString() : tr("%1 (%2)").arg(customers.value(0).toString()).arg(customers.value(1).toString()));
        item->setData(Qt::UserRole, customers.value(0).toString());
        lw_customers->addItem(item);
    }
    QSqlQuery query("SELECT variables.id, variables.name, variables.type, variables.unit, subvariables.id, subvariables.name, subvariables.type, subvariables.unit FROM variables LEFT JOIN subvariables ON variables.id = subvariables.parent");
    const int VAR_ID = 0; const int VAR_NAME = 1; const int VAR_TYPE = 2; const int VAR_UNIT = 3;
    const int SUBVAR_ID = 4; const int SUBVAR_NAME = 5; const int SUBVAR_TYPE = 6; const int SUBVAR_UNIT = 7;
    QString last_id; QTreeWidgetItem * last_item = NULL;
    while (query.next()) {
        if (query.value(VAR_ID).toString() != last_id) {
            last_item = new QTreeWidgetItem(trw_variables);
            last_item->setText(0, query.value(VAR_NAME).toString());
            last_item->setText(1, query.value(VAR_ID).toString());
            last_item->setText(2, query.value(VAR_UNIT).toString());
            last_item->setText(3, dict_vartypes.value(query.value(VAR_TYPE).toString()));
            last_id = query.value(VAR_ID).toString();
        }
        if (!query.value(SUBVAR_ID).toString().isEmpty() && last_item) {
            QTreeWidgetItem * subitem = new QTreeWidgetItem(last_item);
            subitem->setText(0, query.value(SUBVAR_NAME).toString());
            subitem->setText(1, query.value(SUBVAR_ID).toString());
            subitem->setText(2, query.value(SUBVAR_UNIT).toString());
            subitem->setText(3, dict_vartypes.value(query.value(SUBVAR_TYPE).toString()));
        }
    }
    QSqlQuery tables("SELECT id FROM tables");
    while (tables.next()) {
        cb_table_edit->addItem(tables.value(0).toString());
        cb_table->addItem(tables.value(0).toString());
    }
    QSqlQuery warnings("SELECT id, name, description FROM warnings");
    while (warnings.next()) {
        QListWidgetItem * item = new QListWidgetItem;
        item->setText(warnings.value(2).toString().isEmpty() ? warnings.value(1).toString() : tr("%1 (%2)").arg(warnings.value(1).toString()).arg(warnings.value(2).toString()));
        item->setData(Qt::UserRole, warnings.value(0).toString());
        lw_warnings->addItem(item);
    }
    document_path = path;
#ifdef Q_WS_MAC
	this->setWindowTitle(QString("%1[*]").arg(QFileInfo(document_path).baseName()));
#else
    this->setWindowTitle(QString("%1[*] - Leaklog").arg(QFileInfo(document_path).baseName()));
#endif
    this->setWindowModified(false);
    setAllEnabled(true);
    enableTools();
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

void MainWindow::saveDocument(QString)
{
    QString error;
    QSqlQuery commit("COMMIT");
    if (commit.lastError().type() != QSqlError::NoError) { error = commit.lastError().text(); }
    QSqlQuery begin("BEGIN TRANSACTION");
    if (begin.lastError().type() != QSqlError::NoError) { error = begin.lastError().text(); }
    if (!error.isEmpty()) {
		QMessageBox::critical(this, tr("Save document - Leaklog"), tr("Cannot write file %1:\n%2.").arg(document_path).arg(error));
		return;
    }
    //document_path = path;
#ifdef Q_WS_MAC
	this->setWindowTitle(QString("%1[*]").arg(QFileInfo(document_path).baseName()));
#else
    this->setWindowTitle(QString("%1[*] - Leaklog").arg(QFileInfo(document_path).baseName()));
#endif
    this->setWindowModified(false);
    refreshView();
}

void MainWindow::closeDocument(bool save)
{
	if (save && saveChangesBeforeProceeding(tr("Close document - Leaklog"), false)) { return; }
    db.close(); QSqlDatabase::removeDatabase(db.connectionName());
	clearAll(); setAllEnabled(false);
	this->setWindowTitle(tr("Leaklog"));
	this->setWindowModified(false);
}

void MainWindow::addCustomer()
{
    if (!db.isOpen()) { return; }
    MTRecord record("customer", "", MTDictionary());
    ModifyDialogue * md = new ModifyDialogue(record, this);
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
    if (selectedCustomer() < 0) { return; }
    MTRecord record("customer", toString(selectedCustomer()), MTDictionary());
    ModifyDialogue * md = new ModifyDialogue(record, this);
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
    if (!db.isOpen()) { return; }
    if (selectedCustomer() < 0) { return; }
    QListWidgetItem * item = lw_customers->highlightedItem();
    bool ok;
    QString confirmation = QInputDialog::getText(this, tr("Remove customer - Leaklog"), tr("Are you sure you want to remove the selected customer?\nTo remove all data about the customer \"%1\" type REMOVE and confirm:").arg(selectedCustomer()), QLineEdit::Normal, "", &ok);
    if (!ok || confirmation != tr("REMOVE")) { return; }
    MTRecord record("customer", toString(selectedCustomer()), MTDictionary());
    record.remove();
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

void MainWindow::addCircuit()
{
    if (!db.isOpen()) { return; }
    if (selectedCustomer() < 0) { return; }
    MTRecord record("circuit", "", MTDictionary("parent", toString(selectedCustomer())));
    ModifyDialogue * md = new ModifyDialogue(record, this);
    if (md->exec() == QDialog::Accepted) {
        record = md->record();
        QListWidgetItem * item = new QListWidgetItem;
        item->setText(record.id());
        item->setData(Qt::UserRole, record.id());
        lw_circuits->addItem(item);
        this->setWindowModified(true);
        refreshView();
    }
    delete md;
}

void MainWindow::modifyCircuit()
{
    if (!db.isOpen()) { return; }
    if (selectedCustomer() < 0) { return; }
    if (selectedCircuit() < 0) { return; }
    MTRecord record("circuit", toString(selectedCircuit()), MTDictionary("parent", toString(selectedCustomer())));
    ModifyDialogue * md = new ModifyDialogue(record, this);
    if (md->exec() == QDialog::Accepted) {
        record = md->record();
        QListWidgetItem * item = lw_circuits->highlightedItem();
        item->setText(record.id());
        item->setData(Qt::UserRole, record.id());
        this->setWindowModified(true);
        refreshView();
    }
    delete md;
}

void MainWindow::removeCircuit()
{
    if (!db.isOpen()) { return; }
    if (selectedCustomer() < 0) { return; }
    if (selectedCircuit() < 0) { return; }
    QListWidgetItem * item = lw_circuits->highlightedItem();
    bool ok;
    QString confirmation = QInputDialog::getText(this, tr("Remove circuit - Leaklog"), tr("Are you sure you want to remove the selected circuit?\nTo remove all data about the circuit \"%1\" type REMOVE and confirm:").arg(item->data(Qt::UserRole).toString()), QLineEdit::Normal, "", &ok);
    if (!ok || confirmation != tr("REMOVE")) { return; }
    MTRecord record("circuit", toString(selectedCircuit()), MTDictionary("parent", toString(selectedCustomer())));
    record.remove();
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
    MTDictionary parents("circuit", toString(selectedCircuit()));
    parents.insert("customer", toString(selectedCustomer()));
    MTRecord record("inspection", "", parents);
    QSqlQuery inspections = record.select("date");
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

void MainWindow::addInspection()
{
    if (!db.isOpen()) { return; }
    if (selectedCustomer() < 0) { return; }
    if (selectedCircuit() < 0) { return; }
    MTDictionary parents("customer", toString(selectedCustomer()));
    parents.insert("circuit", toString(selectedCircuit()));
    MTRecord record("inspection", "", parents);
    ModifyDialogue * md = new ModifyDialogue(record, this);
    if (md->exec() == QDialog::Accepted) {
        record = md->record();
        QListWidgetItem * item = new QListWidgetItem;
        item->setText(record.id());
        item->setData(Qt::UserRole, record.id());
        lw_inspections->addItem(item);
        this->setWindowModified(true);
        refreshView();
    }
    delete md;
}

void MainWindow::modifyInspection()
{
    if (!db.isOpen()) { return; }
    if (selectedCustomer() < 0) { return; }
    if (selectedCircuit() < 0) { return; }
    if (selectedInspection().isNull()) { return; }
    MTDictionary parents("customer", toString(selectedCustomer()));
    parents.insert("circuit", toString(selectedCircuit()));
    MTRecord record("inspection", selectedInspection(), parents);
    ModifyDialogue * md = new ModifyDialogue(record, this);
    if (md->exec() == QDialog::Accepted) {
        record = md->record();
        QListWidgetItem * item = lw_inspections->highlightedItem();
        item->setText(record.id());
        item->setData(Qt::UserRole, record.id());
        this->setWindowModified(true);
        refreshView();
    }
    delete md;
}

void MainWindow::removeInspection()
{
    if (!db.isOpen()) { return; }
    if (selectedCustomer() < 0) { return; }
    if (selectedCircuit() < 0) { return; }
    if (selectedInspection().isNull()) { return; }
    QListWidgetItem * item = lw_inspections->highlightedItem();
    bool ok;
    QString confirmation = QInputDialog::getText(this, tr("Remove inspection - Leaklog"), tr("Are you sure you want to remove the selected inspection?\nTo remove all data about the inspection \"%1\" type REMOVE and confirm:").arg(item->data(Qt::UserRole).toString()), QLineEdit::Normal, "", &ok);
    if (!ok || confirmation != tr("REMOVE")) { return; }
    MTDictionary parents("customer", toString(selectedCustomer()));
    parents.insert("circuit", toString(selectedCircuit()));
    MTRecord record("inspection", selectedInspection(), parents);
    record.remove();
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

void MainWindow::addVariable() { addVariable(false); }

void MainWindow::addSubvariable() { addVariable(true); }

void MainWindow::addVariable(bool subvar)
{
    if (!db.isOpen()) { return; }
    MTDictionary parents;
    if (subvar) {
        if (trw_variables->currentItem()->parent() != NULL) { return; }
        parents.insert("parent", trw_variables->currentItem()->text(1));
    }
    MTRecord record(subvar ? "subvariable" : "variable", "", parents);
    ModifyDialogue * md = new ModifyDialogue(record, this);
    if (md->exec() == QDialog::Accepted) {
        record = md->record();
        QMap<QString, QVariant> attributes = record.list("name, unit, type");
        QTreeWidgetItem * item = NULL;
        if (subvar) {
            item = new QTreeWidgetItem(trw_variables->currentItem());
        } else {
            item = new QTreeWidgetItem(trw_variables);
        }
        item->setText(0, attributes.value("name").toString());
        item->setText(1, record.id());
        item->setText(2, attributes.value("unit").toString());
        item->setText(3, dict_vartypes.value(attributes.value("type").toString()));
        this->setWindowModified(true);
        refreshView();
    }
    delete md;
}

void MainWindow::modifyVariable()
{
    if (!db.isOpen()) { return; }
    if (!trw_variables->currentIndex().isValid()) { return; }
    QTreeWidgetItem * item = trw_variables->currentItem();
    bool subvar = item->parent() != NULL;
    MTDictionary parents;
    if (subvar) { parents.insert("parent", item->parent()->text(1)); }
    MTRecord record(subvar ? "subvariable" : "variable", item->text(1), parents);
    ModifyDialogue * md = new ModifyDialogue(record, this);
    if (md->exec() == QDialog::Accepted) {
        record = md->record();
        QMap<QString, QVariant> attributes = record.list("name, unit, type");
        item->setText(0, attributes.value("name").toString());
        item->setText(1, record.id());
        item->setText(2, attributes.value("unit").toString());
        item->setText(3, dict_vartypes.value(attributes.value("type").toString()));
        this->setWindowModified(true);
        refreshView();
    }
    delete md;
}

void MainWindow::removeVariable()
{
    if (!db.isOpen()) { return; }
    if (!trw_variables->currentIndex().isValid()) { return; }
    QTreeWidgetItem * item = trw_variables->currentItem();
    bool subvar = item->parent() != NULL;
    bool ok;
    QString confirmation = QInputDialog::getText(this, subvar ? tr("Remove subvariable - Leaklog") : tr("Remove variable - Leaklog"), tr("Are you sure you want to remove the selected variable?\nTo remove the variable \"%1\" type REMOVE and confirm:").arg(item->text(1)), QLineEdit::Normal, "", &ok);
    if (!ok || confirmation != tr("REMOVE")) { return; }
    MTDictionary parents;
    if (subvar) { parents.insert("parent", item->parent()->text(1)); }
    else {
        MTRecord subvars("subvariable", "", MTDictionary("parent", item->text(1)));
        subvars.remove();
    }
    MTRecord record(subvar ? "subvariable" : "variable", item->text(1), parents);
    record.remove();
    if (item != NULL) { delete item; }
    enableTools();
    this->setWindowModified(true);
    refreshView();
}

void MainWindow::addTable()
{
    if (!db.isOpen()) { return; }
    MTRecord record("table", "", MTDictionary());
    ModifyDialogue * md = new ModifyDialogue(record, this);
    if (md->exec() == QDialog::Accepted) {
        record = md->record();
        cb_table->addItem(record.id());
        cb_table_edit->addItem(record.id());
        this->setWindowModified(true);
        refreshView();
    }
    delete md;
}

void MainWindow::modifyTable()
{
    if (!db.isOpen()) { return; }
    if (cb_table_edit->currentIndex() < 0) { return; }
    MTRecord record("table", cb_table_edit->currentText(), MTDictionary());
    ModifyDialogue * md = new ModifyDialogue(record, this);
    if (md->exec() == QDialog::Accepted) {
        record = md->record();
        int i = cb_table_edit->currentIndex();
        int j = cb_table->currentIndex();
        cb_table_edit->removeItem(i);
        cb_table->removeItem(i);
        cb_table_edit->insertItem(i, record.id());
        cb_table->insertItem(i, record.id());
        cb_table_edit->setCurrentIndex(i);
        cb_table->setCurrentIndex(j);
        this->setWindowModified(true);
        refreshView();
    }
    delete md;
}

void MainWindow::removeTable()
{
    if (!db.isOpen()) { return; }
    if (cb_table_edit->currentIndex() < 0) { return; }
    bool ok;
    QString confirmation = QInputDialog::getText(this, tr("Remove table - Leaklog"), tr("Are you sure you want to remove the selected table?\nTo remove the table \"%1\" type REMOVE and confirm:").arg(cb_table_edit->currentText()), QLineEdit::Normal, "", &ok);
    if (!ok || confirmation != tr("REMOVE")) { return; }
    MTRecord record("table", cb_table_edit->currentText(), MTDictionary());
    record.remove();
    int i = cb_table_edit->currentIndex();
    cb_table_edit->removeItem(i);
    cb_table->removeItem(i);
    enableTools();
    this->setWindowModified(true);
    refreshView();
}

void MainWindow::loadTable(const QString &)
{
    if (!db.isOpen()) { return; }
    if (cb_table_edit->currentIndex() < 0) { enableTools(); return; }
    trw_table_variables->clear();
    MTRecord record("table", cb_table_edit->currentText(), MTDictionary());
    QMap<QString, QVariant> attributes = record.list("variables, sum");
    QStringList variables = attributes.value("variables").toString().split(";");
    QStringList sum = attributes.value("sum").toString().split(";");
    for (int i = 0; i < variables.count(); ++i) {
        MTRecord variable("variable", variables.at(i), MTDictionary());
        QTreeWidgetItem * item = new QTreeWidgetItem(trw_table_variables);
        item->setText(0, variable.list("name").value("name").toString());
        item->setText(1, variable.id());
        QComboBox * cb_foot = new QComboBox;
        cb_foot->addItem(tr("None"));
        cb_foot->addItem(tr("Sum"));
        if (sum.contains(variable.id())) { cb_foot->setCurrentIndex(1); }
        else { cb_foot->setCurrentIndex(0); }
        QObject::connect(cb_foot, SIGNAL(currentIndexChanged(int)), this, SLOT(saveTable()));
        trw_table_variables->setItemWidget(item, 2, cb_foot);
    }
    enableTools();
}

void MainWindow::saveTable()
{
    if (!db.isOpen()) { return; }
    if (cb_table_edit->currentIndex() < 0) { return; }
    MTRecord record("table", cb_table_edit->currentText(), MTDictionary());
    QStringList variables, sum; QString value;
    for (int i = 0; i < trw_table_variables->topLevelItemCount(); ++i) {
        variables << trw_table_variables->topLevelItem(i)->text(1);
        QString value = ((QComboBox *)trw_table_variables->itemWidget(trw_table_variables->topLevelItem(i), 2))->currentText();
        if (value == tr("Sum")) { sum << trw_table_variables->topLevelItem(i)->text(1); }
    }
    QMap<QString, QVariant> set;
    set.insert("variables", variables.join(";"));
    set.insert("sum", sum.join(";"));
    record.update(set);
    this->setWindowModified(true);
    refreshView();
}

void MainWindow::addTableVariable()
{
    if (!db.isOpen()) { return; }
    if (cb_table_edit->currentIndex() < 0) { return; }
    QStringList used_ids;
    for (int i = 0; i < trw_table_variables->topLevelItemCount(); ++i) {
        used_ids << trw_table_variables->topLevelItem(i)->text(1);
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
    QSqlQuery query("SELECT id, name FROM variables");
    QString id, name;
    while (query.next()) {
        id = query.value(0).toString();
        name = query.value(1).toString();
        if (!used_ids.contains(id)) {
            QListWidgetItem * item = new QListWidgetItem;
            item->setText(name.isEmpty() ? id : tr("%1 (%2)").arg(id).arg(name));
            item->setData(Qt::UserRole, id);
            lw->addItem(item);
        }
    }
    if (d->exec() == QDialog::Accepted && lw->currentIndex().isValid()) {
        MTRecord record("table", cb_table_edit->currentText(), MTDictionary());
        QStringList variables = record.list("variables").value("variables").toString().split(";");
        variables << lw->currentItem()->data(Qt::UserRole).toString();
        QMap<QString, QVariant> set;
        set.insert("variables", variables.join(";"));
        record.update(set);
        loadTable(cb_table_edit->currentText());
        this->setWindowModified(true);
        refreshView();
    }
    delete d;
}

void MainWindow::removeTableVariable()
{
    if (!db.isOpen()) { return; }
    if (cb_table_edit->currentIndex() < 0) { return; }
    if (!trw_table_variables->currentIndex().isValid()) { return; }
    QTreeWidgetItem * item = trw_table_variables->currentItem();
    switch (QMessageBox::information(this, tr("Remove variable - Leaklog"), tr("Are you sure you want to remove the variable \"%1\" from the selected table?").arg(item->text(1)), tr("Remove"), tr("Cancel"), 0, 1)) {
        case 0: // Remove
            break;
        case 1: // Cancel
            return; break;
    }
    MTRecord record("table", cb_table_edit->currentText(), MTDictionary());
    QMap<QString, QVariant> attributes = record.list("variables, sum");
    QStringList variables = attributes.value("variables").toString().split(";");
    QStringList sum = attributes.value("sum").toString().split(";");
    variables.removeAll(item->text(1));
    sum.removeAll(item->text(1));
    QMap<QString, QVariant> set;
    set.insert("variables", variables.join(";"));
    set.insert("sum", sum.join(";"));
    record.update(set);
    loadTable(cb_table_edit->currentText());
    this->setWindowModified(true);
    refreshView();
}

void MainWindow::moveTableVariableUp()
{
    moveTableVariable(true);
}

void MainWindow::moveTableVariableDown()
{
    moveTableVariable(false);
}

void MainWindow::moveTableVariable(bool up)
{
    if (!db.isOpen()) { return; }
    if (cb_table_edit->currentIndex() < 0) { return; }
    if (!trw_table_variables->currentIndex().isValid()) { return; }
    int i = trw_table_variables->indexOfTopLevelItem(trw_table_variables->currentItem());
    if (i < 0) { return; }
    MTRecord record("table", cb_table_edit->currentText(), MTDictionary());
    QStringList variables = record.list("variables").value("variables").toString().split(";");
    QString variable = variables.takeAt(i);
    if (up) {
        if (i != 0) { i--; } else { i = variables.count(); }
    } else {
        if (i != variables.count()) { i++; } else { i = 0; }
    }
    variables.insert(i, variable);
    QMap<QString, QVariant> set;
    set.insert("variables", variables.join(";"));
    record.update(set);
    loadTable(cb_table_edit->currentText());
    trw_table_variables->setCurrentItem(trw_table_variables->topLevelItem(i));
    this->setWindowModified(true);
    refreshView();
}

void MainWindow::addWarning()
{
    if (!db.isOpen()) { return; }
    QStringList used_ids = listVariableIds();
    MTRecord record("warning", "", MTDictionary());
    ModifyWarningDialogue * md = new ModifyWarningDialogue(record, used_ids, this);
    if (md->exec() == QDialog::Accepted) {
        record = md->record();
        QMap<QString, QVariant> attributes = record.list("name, description");
        QString name = attributes.value("name").toString();
        QString description = attributes.value("description").toString();
        QListWidgetItem * item = new QListWidgetItem;
        item->setText(description.isEmpty() ? name : tr("%1 (%2)").arg(name).arg(description));
        item->setData(Qt::UserRole, record.id());
        lw_warnings->addItem(item);
        this->setWindowModified(true);
        refreshView();
    }
    delete md;
}

void MainWindow::modifyWarning()
{
    if (!db.isOpen()) { return; }
    if (!lw_warnings->currentIndex().isValid()) { return; }
    QListWidgetItem * item = lw_warnings->currentItem();
    QStringList used_ids = listVariableIds();
    MTRecord record("warning", item->data(Qt::UserRole).toString(), MTDictionary());
    ModifyWarningDialogue * md = new ModifyWarningDialogue(record, used_ids, this);
    if (md->exec() == QDialog::Accepted) {
        record = md->record();
        QMap<QString, QVariant> attributes = record.list("name, description");
        QString name = attributes.value("name").toString();
        QString description = attributes.value("description").toString();
        item->setText(description.isEmpty() ? name : tr("%1 (%2)").arg(name).arg(description));
        item->setData(Qt::UserRole, record.id());
        this->setWindowModified(true);
        refreshView();
    }
    delete md;
}

void MainWindow::removeWarning()
{
    if (!db.isOpen()) { return; }
    if (!lw_warnings->currentIndex().isValid()) { return; }
    QListWidgetItem * item = lw_warnings->currentItem();
    MTRecord record("warning", item->data(Qt::UserRole).toString(), MTDictionary());
    QMap<QString, QVariant> attributes = record.list("name, description");
    QString name = attributes.value("name").toString();
    QString description = attributes.value("description").toString();
    bool ok;
    QString confirmation = QInputDialog::getText(this, tr("Remove warning - Leaklog"), tr("Are you sure you want to remove the selected warning?\nTo remove the warning \"%1\" type REMOVE and confirm:").arg(description.isEmpty() ? name : tr("%1 (%2)").arg(name).arg(description)), QLineEdit::Normal, "", &ok);
    if (!ok || confirmation != tr("REMOVE")) { return; }
    record.remove();
    delete item;
    enableTools();
    this->setWindowModified(true);
    refreshView();
}

void MainWindow::exportCustomerData()
{
    /*if (!document_open) { return; }
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
    file.close();*/
}

void MainWindow::exportCircuitData()
{
    /*if (!document_open) { return; }
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
    file.close();*/
}

void MainWindow::exportInspectionData()
{
    /*if (!document_open) { return; }
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
    file.close();*/
}

void MainWindow::importData()
{
    /*QString path = QFileDialog::getOpenFileName(this, tr("Import data - Leaklog"), "", tr("Leaklog Documents (*.lklg);;All files (*.*)"));
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
    refreshView();*/
}

QStringList MainWindow::listVariableIds(bool all)
{
    QStringList ids; bool sub_empty = false;
    QSqlQuery query("SELECT variables.id, subvariables.id FROM variables LEFT JOIN subvariables ON variables.id = subvariables.parent");
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
