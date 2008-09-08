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
				save(); if (close_) { closeDatabase(false); }; return false;
				break;
			case 1: // Discard
				if (close_) { closeDatabase(false); }; return false;
				break;
			case 2: // Cancel
				return true;
				break;
		}
	} else if (db.isOpen() && !this->isWindowModified()) {
		if (close_) { closeDatabase(false); }; return false;
	}
	return false;
}

void MainWindow::initDatabase(QSqlDatabase * database)
{
    QSqlQuery begin("BEGIN TRANSACTION", *database);
    QSqlQuery create_customers("CREATE TABLE customers (id INTEGER PRIMARY KEY, company VARCHAR, contact_person VARCHAR, address VARCHAR, mail VARCHAR, phone VARCHAR)", *database);
    QSqlQuery create_circuits("CREATE TABLE circuits (parent INTEGER, id INTEGER, hermetic INTEGER, manufacturer VARCHAR, type VARCHAR, sn VARCHAR, year INTEGER, commissioning VARCHAR, field VARCHAR, refrigerant VARCHAR, refrigerant_amount NUMERIC, oil VARCHAR, oil_amount NUMERIC, life NUMERIC, runtime NUMERIC, utilisation NUMERIC)", *database);
    QSqlQuery create_inspections("CREATE TABLE inspections (customer INTEGER, circuit INTEGER, date VARCHAR, nominal INTEGER)", *database);
    QSqlQuery create_variables("CREATE TABLE variables (id VARCHAR, name VARCHAR, type VARCHAR, unit VARCHAR, value VARCHAR, compare_nom INTEGER, col_bg VARCHAR)", *database);
    QSqlQuery create_subvariables("CREATE TABLE subvariables (parent VARCHAR, id VARCHAR, name VARCHAR, type VARCHAR, unit VARCHAR, value VARCHAR, compare_nom INTEGER)", *database);
    QSqlQuery create_tables("CREATE TABLE tables (id VARCHAR, highlight_nominal INTEGER, variables VARCHAR, sum VARCHAR)", *database);
    QSqlQuery create_warnings("CREATE TABLE warnings (id INTEGER PRIMARY KEY, name VARCHAR, description VARCHAR)", *database);
    QSqlQuery create_warnings_filters("CREATE TABLE warnings_filters (parent INTEGER, circuit_attribute VARCHAR, function VARCHAR, value VARCHAR)", *database);
    QSqlQuery create_warnings_conditions("CREATE TABLE warnings_conditions (parent INTEGER, value_ins VARCHAR, function VARCHAR, value_nom VARCHAR)", *database);
    QSqlQuery create_index_customers_id("CREATE UNIQUE INDEX index_customers_id ON customers (id ASC)", *database);
    QSqlQuery create_index_circuits_id("CREATE UNIQUE INDEX index_circuits_id ON circuits (parent ASC, id ASC)", *database);
    QSqlQuery create_index_inspections_id("CREATE UNIQUE INDEX index_inspections_id ON inspections (customer ASC, circuit ASC, date ASC)", *database);
    QSqlQuery create_index_variables_id("CREATE UNIQUE INDEX index_variables_id ON variables (id ASC)", *database);
    QSqlQuery create_index_subvariables_id("CREATE UNIQUE INDEX index_subvariables_id ON subvariables (id ASC)", *database);
    QSqlQuery create_index_tables_id("CREATE UNIQUE INDEX index_tables_id ON tables (id ASC)", *database);
    QSqlQuery create_index_warnings_id("CREATE UNIQUE INDEX index_warnings_id ON warnings (id ASC)", *database);
    QSqlQuery create_index_warnings_filters_parent("CREATE INDEX index_warnings_filters_parent ON warnings_filters (parent ASC)", *database);
    QSqlQuery create_index_warnings_conditions_parent("CREATE INDEX index_warnings_conditions_parent ON warnings_conditions (parent ASC)", *database);
    QSqlQuery commit("COMMIT", *database);
}

void MainWindow::newDatabase()
{
    if (saveChangesBeforeProceeding(tr("New database - Leaklog"), true)) { return; }
    QString path = QFileDialog::getSaveFileName(this, tr("New database - Leaklog"), tr("untitled.lklg"), tr("Leaklog Database (*.lklg)"));
	if (path.isNull() || path.isEmpty()) { return; }
    QFile file(path); if (file.exists()) { file.remove(); }
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(path);
    if (!db.open()) {
		QMessageBox::critical(this, tr("New database - Leaklog"), tr("Cannot write file %1:\n%2.").arg(path).arg(db.lastError().text()));
		this->setWindowTitle(tr("Leaklog"));
		return;
    }
    addRecent(path);
    clearAll();
    initDatabase(&db);
    QSqlQuery begin("BEGIN TRANSACTION");
#ifdef Q_WS_MAC
	this->setWindowTitle(QString("%1[*]").arg(QFileInfo(path).baseName()));
#else
    this->setWindowTitle(QString("%1[*] - Leaklog").arg(QFileInfo(path).baseName()));
#endif
    this->setWindowModified(false);
    setAllEnabled(true);
}

void MainWindow::openRecent(QListWidgetItem * item)
{
    QString s = item->text();
    addRecent(s);
    openDatabase(s);
}

void MainWindow::open()
{
    if (saveChangesBeforeProceeding(tr("Open database - Leaklog"), true)) { return; }
    QString path = QFileDialog::getOpenFileName(this, tr("Open database - Leaklog"), "", tr("Leaklog Databases (*.lklg);;All files (*.*)"));
	if (path.isNull() || path.isEmpty()) { return; }
    addRecent(path);
    openDatabase(path);
}

void MainWindow::openDatabase(QString path)
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(path);
    if (!db.open()) {
		QMessageBox::critical(this, tr("Open database - Leaklog"), tr("Cannot read file %1:\n%2.").arg(path).arg(db.lastError().text()));
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
#ifdef Q_WS_MAC
	this->setWindowTitle(QString("%1[*]").arg(QFileInfo(path).baseName()));
#else
    this->setWindowTitle(QString("%1[*] - Leaklog").arg(QFileInfo(path).baseName()));
#endif
    this->setWindowModified(false);
    setAllEnabled(true);
    enableTools();
    loadTable(cb_table_edit->currentText());
    setView(tr("All customers"));
}

void MainWindow::save()
{
    saveDatabase(db.databaseName());
}

void MainWindow::saveAs()
{
	QString path = QFileDialog::getSaveFileName(this, tr("Save database - Leaklog"), QFileInfo(db.databaseName()).fileName(), tr("Leaklog Database (*.lklg)"));
	if (!path.isEmpty()) { addRecent(path); saveDatabase(path); }
}

void MainWindow::saveDatabase(QString)
{
    QString error;
    QSqlQuery commit("COMMIT");
    if (commit.lastError().type() != QSqlError::NoError) { error = commit.lastError().text(); }
    QSqlQuery begin("BEGIN TRANSACTION");
    if (begin.lastError().type() != QSqlError::NoError) { error = begin.lastError().text(); }
    if (!error.isEmpty()) {
		QMessageBox::critical(this, tr("Save database - Leaklog"), tr("Cannot write file %1:\n%2.").arg(db.databaseName()).arg(error));
		return;
    }
#ifdef Q_WS_MAC
	this->setWindowTitle(QString("%1[*]").arg(QFileInfo(db.databaseName()).baseName()));
#else
    this->setWindowTitle(QString("%1[*] - Leaklog").arg(QFileInfo(db.databaseName()).baseName()));
#endif
    this->setWindowModified(false);
    refreshView();
}

void MainWindow::closeDatabase(bool save)
{
	if (save && saveChangesBeforeProceeding(tr("Close database - Leaklog"), false)) { return; }
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
            QHBoxLayout * hl = new QHBoxLayout;
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
    exportData("customer");
}

void MainWindow::exportCircuitData()
{
    if (selectedCircuit() < 0) { return; }
    exportData("circuit");
}

void MainWindow::exportInspectionData()
{
    if (selectedCircuit() < 0) { return; }
    if (selectedInspection().isNull()) { return; }
    exportData("inspection");
}

void MainWindow::exportData(const QString & type)
{
    if (!db.isOpen()) { return; }
    if (selectedCustomer() < 0) { return; }
    QString path = QFileDialog::getSaveFileName(this, tr("Export customer data - Leaklog"), tr("untitled.lklg"), tr("Leaklog Database (*.lklg)"));
	if (path.isNull() || path.isEmpty()) { return; }
    QFile file(path); if (file.exists()) { file.remove(); }
    QSqlDatabase data = QSqlDatabase::addDatabase("QSQLITE", "exportData");
    data.setDatabaseName(path);
    if (!data.open()) {
		QMessageBox::critical(this, tr("Export customer data - Leaklog"), tr("Cannot write file %1:\n%2.").arg(path).arg(data.lastError().text()));
		return;
    }
    initDatabase(&data);
    QSqlQuery begin("BEGIN TRANSACTION", data);
    copyTable("variables", &db, &data);
    copyTable("subvariables", &db, &data);
    copyTable("tables", &db, &data);
    copyTable("customers", &db, &data, QString("id = %1").arg(selectedCustomer()));
    if (type == "customer") {
        copyTable("circuits", &db, &data, QString("parent = %1").arg(selectedCustomer()));
        copyTable("inspections", &db, &data, QString("customer = %1").arg(selectedCustomer()));
    } else if (type == "circuit") {
        copyTable("circuits", &db, &data, QString("parent = %1 AND id = %2").arg(selectedCustomer()).arg(selectedCircuit()));
        copyTable("inspections", &db, &data, QString("customer = %1 AND circuit = %2").arg(selectedCustomer()).arg(selectedCircuit()));
    } else if (type == "inspection") {
        copyTable("circuits", &db, &data, QString("parent = %1 AND id = %2").arg(selectedCustomer()).arg(selectedCircuit()));
        copyTable("inspections", &db, &data, QString("customer = %1 AND circuit = %2 AND date = '%3'").arg(selectedCustomer()).arg(selectedCircuit()).arg(selectedInspection()));
    }
    QSqlQuery commit("COMMIT", data);
    data.close(); QSqlDatabase::removeDatabase(data.connectionName());
}

void MainWindow::importData()
{
    if (!db.isOpen()) { return; }
    QString path = QFileDialog::getOpenFileName(this, tr("Import data - Leaklog"), "", tr("Leaklog Databases (*.lklg);;All files (*.*)"));
	if (path.isNull() || path.isEmpty()) { return; }
    QSqlDatabase data = QSqlDatabase::addDatabase("QSQLITE", "importData");
    data.setDatabaseName(path);
    if (!data.open()) {
		QMessageBox::critical(this, tr("Import data - Leaklog"), tr("Cannot read file %1:\n%2.").arg(path).arg(data.lastError().text()));
		return;
    }
    ImportDialogue * id = new ImportDialogue(this);
    QSqlQuery customers("SELECT id, company FROM customers", data);
    while (customers.next()) {
        QListWidgetItem * item = new QListWidgetItem(id->customers());
        item->setCheckState(Qt::Unchecked);
        item->setText(customers.value(1).toString().isEmpty() ? customers.value(0).toString() : tr("%1 (%2)").arg(customers.value(0).toString()).arg(customers.value(1).toString()));
        item->setData(Qt::UserRole, customers.value(0).toString());
    }
    QSqlQuery circuits("SELECT id, parent FROM circuits", data);
    while (circuits.next()) {
        QListWidgetItem * item = new QListWidgetItem(id->circuits());
        item->setCheckState(Qt::Unchecked);
        item->setHidden(true);
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable);
        item->setText(QString("%1 (%2)").arg(circuits.value(0).toString()).arg(circuits.value(1).toString()));
        item->setData(Qt::UserRole, QString("%1::%2").arg(circuits.value(1).toString()).arg(circuits.value(0).toString()));
    }
    QSqlQuery inspections("SELECT date, customer, circuit FROM inspections", data);
    while (inspections.next()) {
        QListWidgetItem * item = new QListWidgetItem(id->inspections());
        item->setCheckState(Qt::Checked);
        item->setHidden(true);
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable);
        item->setText(QString("%1 (%2::%3)").arg(inspections.value(0).toString()).arg(inspections.value(1).toString()).arg(inspections.value(2).toString()));
        item->setData(Qt::UserRole, QString("%1::%2::%3").arg(inspections.value(1).toString()).arg(inspections.value(2).toString()).arg(inspections.value(0).toString()));
    }
    QSqlQuery query("SELECT variables.id, variables.name, subvariables.id, subvariables.name FROM variables LEFT JOIN subvariables ON variables.id = subvariables.parent", data);
    const int VAR_ID = 0; const int VAR_NAME = 1; const int SUBVAR_ID = 2; const int SUBVAR_NAME = 3;
    QString last_id; QString var_id, var_name;
    while (query.next()) {
        var_id = query.value(VAR_ID).toString();
        if (!query.value(VAR_NAME).toString().isEmpty()) {
            var_name = query.value(VAR_NAME).toString();
        }
        if (!query.value(SUBVAR_ID).toString().isEmpty()) {
            var_id.append("/" + query.value(SUBVAR_ID).toString());
        }
        if (!query.value(SUBVAR_NAME).toString().isEmpty()) {
            if (!var_name.isEmpty()) { var_name.append("/"); }
            var_name.append(query.value(SUBVAR_NAME).toString());
        }
        QListWidgetItem * item = new QListWidgetItem(id->variables());
        item->setCheckState(Qt::Checked);
        item->setText(var_name.isEmpty() ? var_id : tr("%1 (%2)").arg(var_id).arg(var_name));
        item->setData(Qt::UserRole, var_id);
    }
    if (id->exec() != QDialog::Accepted) { return; }
    QMap<QString, QVariant> set;
    for (int c = 0; c < id->customers()->count(); ++c) {
        if (id->customers()->item(c)->checkState() == Qt::Unchecked) { continue; }
        set.clear();
        QSqlQuery customer_data(data);
        customer_data.prepare("SELECT * FROM customers WHERE id = :id");
        customer_data.bindValue(":id", id->customers()->item(c)->data(Qt::UserRole));
        customer_data.exec();
        if (customer_data.next()) {
            for (int f = 0; f < customer_data.record().count(); ++f) {
                set.insert(customer_data.record().fieldName(f), customer_data.value(f));
            }
        }
        MTRecord record("customer", id->customers()->item(c)->data(Qt::UserRole).toString(), MTDictionary());
        record.update(set);
    }
    for (int cc = 0; cc < id->circuits()->count(); ++cc) {
        if (id->circuits()->item(cc)->checkState() == Qt::Unchecked) { continue; }
        set.clear();
        QString cc_parent = id->circuits()->item(cc)->data(Qt::UserRole).toString().split("::").first();
        QString cc_id = id->circuits()->item(cc)->data(Qt::UserRole).toString().split("::").last();
        QSqlQuery circuit_data(data);
        circuit_data.prepare("SELECT * FROM circuits WHERE parent = :parent AND id = :id");
        circuit_data.bindValue(":parent", cc_parent);
        circuit_data.bindValue(":id", cc_id);
        circuit_data.exec();
        if (circuit_data.next()) {
            for (int f = 0; f < circuit_data.record().count(); ++f) {
                set.insert(circuit_data.record().fieldName(f), circuit_data.value(f));
            }
            set.remove("parent");
        }
        MTRecord record("circuit", cc_id, MTDictionary("parent", cc_parent));
        record.update(set);
    }
    for (int i = 0, j = 0; i < id->inspections()->count(); ++i) {
        if (id->inspections()->item(i)->checkState() == Qt::Unchecked) { continue; }
        set.clear();
        QString i_customer = id->inspections()->item(i)->data(Qt::UserRole).toString().split("::").at(0);
        QString i_circuit = id->inspections()->item(i)->data(Qt::UserRole).toString().split("::").at(1);
        QString i_date = id->inspections()->item(i)->data(Qt::UserRole).toString().split("::").at(2);
        QSqlQuery inspection_data(data);
        inspection_data.prepare("SELECT * FROM inspections WHERE customer = :customer AND circuit = :circuit AND date = :date");
        inspection_data.bindValue(":customer", i_customer);
        inspection_data.bindValue(":circuit", i_circuit);
        inspection_data.bindValue(":date", i_date);
        inspection_data.exec();
        if (inspection_data.next()) {
            for (int f = 0; f < inspection_data.record().count(); ++f) {
                set.insert(inspection_data.record().fieldName(f), inspection_data.value(f));
            }
            set.remove("customer");
            set.remove("circuit");
        }
        MTDictionary parents("customer", i_customer);
        parents.insert("circuit", i_circuit);
        MTRecord record("inspection", i_date, parents);
        record.update(set, j == 0);
        j++;
    }
    this->setWindowModified(true);
    refreshView();
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
