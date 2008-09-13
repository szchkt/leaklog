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
    QSqlQuery create_customers("CREATE TABLE customers (id INTEGER PRIMARY KEY, company TEXT, contact_person TEXT, address TEXT, mail TEXT, phone TEXT)", *database);
    QSqlQuery create_circuits("CREATE TABLE circuits (parent INTEGER, id INTEGER, hermetic INTEGER, manufacturer TEXT, type TEXT, sn TEXT, year INTEGER, commissioning TEXT, field TEXT, refrigerant TEXT, refrigerant_amount NUMERIC, oil TEXT, oil_amount NUMERIC, life NUMERIC, runtime NUMERIC, utilisation NUMERIC)", *database);
    QSqlQuery create_inspections("CREATE TABLE inspections (customer INTEGER, circuit INTEGER, date TEXT, nominal INTEGER)", *database);
    QSqlQuery create_inspectors("CREATE TABLE inspectors (id INTEGER PRIMARY KEY, person TEXT, company TEXT, person_reg_num TEXT, company_reg_num TEXT)", *database);
    QSqlQuery create_variables("CREATE TABLE variables (id TEXT, name TEXT, type TEXT, unit TEXT, value TEXT, compare_nom INTEGER, col_bg TEXT)", *database);
    QSqlQuery create_subvariables("CREATE TABLE subvariables (parent TEXT, id TEXT, name TEXT, type TEXT, unit TEXT, value TEXT, compare_nom INTEGER)", *database);
    QSqlQuery create_tables("CREATE TABLE tables (id TEXT, highlight_nominal INTEGER, variables TEXT, sum TEXT)", *database);
    QSqlQuery create_warnings("CREATE TABLE warnings (id INTEGER PRIMARY KEY, name TEXT, description TEXT)", *database);
    QSqlQuery create_warnings_filters("CREATE TABLE warnings_filters (parent INTEGER, circuit_attribute TEXT, function TEXT, value TEXT)", *database);
    QSqlQuery create_warnings_conditions("CREATE TABLE warnings_conditions (parent INTEGER, value_ins TEXT, function TEXT, value_nom TEXT)", *database);
    QSqlQuery create_index_customers_id("CREATE UNIQUE INDEX index_customers_id ON customers (id ASC)", *database);
    QSqlQuery create_index_circuits_id("CREATE UNIQUE INDEX index_circuits_id ON circuits (parent ASC, id ASC)", *database);
    QSqlQuery create_index_inspections_id("CREATE UNIQUE INDEX index_inspections_id ON inspections (customer ASC, circuit ASC, date ASC)", *database);
    QSqlQuery create_index_inspectors_id("CREATE UNIQUE INDEX index_inspectors_id ON inspectors (id ASC)", *database);
    QSqlQuery create_index_variables_id("CREATE UNIQUE INDEX index_variables_id ON variables (id ASC)", *database);
    QSqlQuery create_index_subvariables_id("CREATE UNIQUE INDEX index_subvariables_id ON subvariables (id ASC)", *database);
    QSqlQuery create_index_tables_id("CREATE UNIQUE INDEX index_tables_id ON tables (id ASC)", *database);
    QSqlQuery create_index_warnings_id("CREATE UNIQUE INDEX index_warnings_id ON warnings (id ASC)", *database);
    QSqlQuery create_index_warnings_filters_parent("CREATE INDEX index_warnings_filters_parent ON warnings_filters (parent ASC)", *database);
    QSqlQuery create_index_warnings_conditions_parent("CREATE INDEX index_warnings_conditions_parent ON warnings_conditions (parent ASC)", *database);
    QSqlQuery commit("COMMIT", *database);
}

void MainWindow::initVariables()
{
    QSqlQuery begin("BEGIN TRANSACTION");
    initVariable("t", "", "", "", false, "");
    initSubvariable("t", "t_out", "float", tr("%1C").arg(degreeSign()), "", true);
    initSubvariable("t", "t_in", "float", tr("%1C").arg(degreeSign()), "", true);
    initVariable("p_0", "float", tr("Bar"), "", true, "");
    initVariable("p_c", "float", tr("Bar"), "", true, "");
    initVariable("t_0", "float", tr("%1C").arg(degreeSign()), "", true, "");
    initVariable("t_c", "float", tr("%1C").arg(degreeSign()), "", true, "");
    initVariable("t_ev", "float", tr("%1C").arg(degreeSign()), "", true, "");
    initVariable("t_evap_out", "float", tr("%1C").arg(degreeSign()), "", true, "");
    initVariable("t_comp_in", "float", tr("%1C").arg(degreeSign()), "", true, "");
    initVariable("t_sc", "float", tr("%1C").arg(degreeSign()), "t_c-t_ev", true, "");
    initVariable("t_sh", "", "", "", false, "");
    initSubvariable("t_sh", "t_sh_evap", "float", tr("%1C").arg(degreeSign()), "t_evap_out-t_0", true);
    initSubvariable("t_sh", "t_sh_comp", "float", tr("%1C").arg(degreeSign()), "t_comp_in-t_0", true);
    initVariable("t_comp_out", "float", tr("%1C").arg(degreeSign()), "", true, "");
    initVariable("delta_t_evap", "float", tr("%1C").arg(degreeSign()), "t_in-t_0", true, "");
    initVariable("delta_t_c", "float", tr("%1C").arg(degreeSign()), "t_out-t_c", true, "");
    initVariable("ep_comp", "float", tr("kW"), "", true, "");
    initVariable("ec", "", "", "", false, "");
    initSubvariable("ec", "ec_l1", "float", tr("A"), "", true);
    initSubvariable("ec", "ec_l2", "float", tr("A"), "", true);
    initSubvariable("ec", "ec_l3", "float", tr("A"), "", true);
    initVariable("ev", "", "", "", false, "");
    initSubvariable("ev", "ev_l1", "float", tr("V"), "", true);
    initSubvariable("ev", "ev_l2", "float", tr("V"), "", true);
    initSubvariable("ev", "ev_l3", "float", tr("V"), "", true);
    initVariable("ppsw", "", "", "", false, "");
    initSubvariable("ppsw", "ppsw_hip", "float", tr("Bar"), "", true);
    initSubvariable("ppsw", "ppsw_lop", "float", tr("Bar"), "", true);
    initSubvariable("ppsw", "ppsw_diff", "float", tr("Bar"), "", true);
    initVariable("sftsw", "float", tr("Bar"), "", true, "");
    initVariable("rmds", "string", "", "", false, "");
    initVariable("arno", "string", "", "", false, "");
    initVariable("vis_aur_chk", "", "", "", false, "");
    initSubvariable("vis_aur_chk", "corr_def", "bool", "", "", false);
    initSubvariable("vis_aur_chk", "noise_vibr", "bool", "", "", false);
    initSubvariable("vis_aur_chk", "bbl_lvl", "bool", "", "", false);
    initSubvariable("vis_aur_chk", "oil_leak", "bool", "", "", false);
    initSubvariable("vis_aur_chk", "oil_leak_am", "float", tr("kg"), "", false);
    initVariable("dir_leak_chk", "", "", "", false, "green");
    initSubvariable("dir_leak_chk", "el_detect", "bool", "", "", false);
    initSubvariable("dir_leak_chk", "uv_detect", "bool", "", "", false);
    initSubvariable("dir_leak_chk", "bbl_detect", "bool", "", "", false);
    initVariable("refr_add", "", "", "", false, "yellow");
    initSubvariable("refr_add", "refr_add_am", "float", tr("kg"), "", false);
    initSubvariable("refr_add", "refr_add_per", "float", tr("%"), "100*sum(refr_add_am)/refrigerant_amount", false);
    initVariable("refr_reco", "float", tr("kg"), "", false, "yellow");
    initVariable("refr_recy", "float", tr("kg"), "", false, "yellow");
    initVariable("refr_disp", "float", tr("kg"), "", false, "yellow");
    initVariable("inspector", "string", "", "", false, "");
    initVariable("operator", "string", "", "", false, "");
    QSqlQuery commit("COMMIT");
}

void MainWindow::initVariable(const QString & id, const QString & type, const QString & unit, const QString & value, bool compare_nom, const QString & col_bg)
{
    QMap<QString, QVariant> set;
    MTRecord variable("variable", "", MTDictionary());
    set.insert("id", id);
    set.insert("name", dict_varnames.value(id));
    set.insert("type", type);
    set.insert("unit", unit);
    set.insert("value", value);
    set.insert("compare_nom", compare_nom ? 1 : 0);
    set.insert("col_bg", col_bg);
    variable.update(set);
    addColumn(id, "inspections", &db);
}

void MainWindow::initSubvariable(const QString & parent, const QString & id, const QString & type, const QString & unit, const QString & value, bool compare_nom)
{
    QMap<QString, QVariant> set;
    MTRecord variable("subvariable", "", MTDictionary("parent", parent));
    set.insert("id", id);
    set.insert("name", dict_varnames.value(id));
    set.insert("type", type);
    set.insert("unit", unit);
    set.insert("value", value);
    set.insert("compare_nom", compare_nom ? 1 : 0);
    variable.update(set);
    addColumn(id, "inspections", &db);
}

void MainWindow::initTables()
{
    QSqlQuery begin("BEGIN TRANSACTION");
    QMap<QString, QVariant> set;
    MTRecord table_of_leakages("table", "", MTDictionary());
    set.insert("id", tr("Table of leakages"));
    set.insert("highlight_nominal", 0);
    set.insert("variables", "vis_aur_chk;dir_leak_chk;refr_add;refr_reco;refr_recy;refr_disp;inspector;operator;rmds;arno");
    set.insert("sum", "vis_aur_chk;refr_add;refr_reco;refr_recy;refr_disp");
    table_of_leakages.update(set);
    set.clear();
    MTRecord table_of_parameters("table", "", MTDictionary());
    set.insert("id", tr("Table of parameters"));
    set.insert("highlight_nominal", 1);
    set.insert("variables", "t;p_0;p_c;t_0;t_c;t_ev;t_evap_out;t_comp_in;t_sc;t_sh;t_comp_out;ep_comp;ec;ev;ppsw;sftsw;rmds;arno");
    set.insert("sum", "");
    table_of_parameters.update(set);
    QSqlQuery commit("COMMIT");
}

void MainWindow::initWarnings()
{
    QSqlQuery begin("BEGIN TRANSACTION");
    QString w;
    w = initWarning(tr("Refrigerant leakage above limit"), tr("3 - 10 kg, before 2011"));
    initWarningAddFilter(w, "commissioning", "<", "2011.07.04");
    initWarningAddFilter(w, "refrigerant_amount", ">=", "3");
    initWarningAddFilter(w, "refrigerant_amount", "<", "10");
    initWarningAddCondition(w, "100*refr_add_am/refrigerant_amount", ">", "6");
    w = initWarning(tr("Refrigerant leakage above limit"), tr("3 - 10 kg, after 2011"));
    initWarningAddFilter(w, "commissioning", ">=", "2011.07.04");
    initWarningAddFilter(w, "refrigerant_amount", ">=", "3");
    initWarningAddFilter(w, "refrigerant_amount", "<", "10");
    initWarningAddCondition(w, "100*refr_add_am/refrigerant_amount", ">", "8");
    w = initWarning(tr("Refrigerant leakage above limit"), tr("10 - 100 kg, before 2011"));
    initWarningAddFilter(w, "commissioning", "<", "2011.07.04");
    initWarningAddFilter(w, "refrigerant_amount", ">=", "10");
    initWarningAddFilter(w, "refrigerant_amount", "<", "100");
    initWarningAddCondition(w, "100*refr_add_am/refrigerant_amount", ">", "4");
    w = initWarning(tr("Refrigerant leakage above limit"), tr("10 - 100 kg, after 2011"));
    initWarningAddFilter(w, "commissioning", ">=", "2011.07.04");
    initWarningAddFilter(w, "refrigerant_amount", ">=", "10");
    initWarningAddFilter(w, "refrigerant_amount", "<", "100");
    initWarningAddCondition(w, "100*refr_add_am/refrigerant_amount", ">", "6");
    w = initWarning(tr("Refrigerant leakage above limit"), tr("above 100 kg, before 2011"));
    initWarningAddFilter(w, "commissioning", "<", "2011.07.04");
    initWarningAddFilter(w, "refrigerant_amount", ">=", "100");
    initWarningAddCondition(w, "100*refr_add_am/refrigerant_amount", ">", "2");
    w = initWarning(tr("Refrigerant leakage above limit"), tr("above 100 kg, after 2011"));
    initWarningAddFilter(w, "commissioning", ">=", "2011.07.04");
    initWarningAddFilter(w, "refrigerant_amount", ">=", "100");
    initWarningAddCondition(w, "100*refr_add_am/refrigerant_amount", ">", "4");
    QSqlQuery commit("COMMIT");
}

QString MainWindow::initWarning(const QString & name, const QString & description)
{
    MTRecord warning("warning", "", MTDictionary());
    QMap<QString, QVariant> set;
    set.insert("name", name);
    set.insert("description", description);
    warning.update(set);
    return warning.id();
}

void MainWindow::initWarningAddFilter(const QString & parent, const QString & circuit_attribute, const QString & function, const QString & value)
{
    QSqlQuery filter;
    filter.prepare("INSERT INTO warnings_filters (parent, circuit_attribute, function, value) VALUES (:parent, :circuit_attribute, :function, :value)");
    filter.bindValue(":parent", parent);
    filter.bindValue(":circuit_attribute", circuit_attribute);
    filter.bindValue(":function", function);
    filter.bindValue(":value", value);
    filter.exec();
}

void MainWindow::initWarningAddCondition(const QString & parent, const QString & ins_value, const QString & function, const QString & nom_value)
{
    QSqlQuery condition;
    condition.prepare("INSERT INTO warnings_conditions (parent, ins_value, function, nom_value) VALUES (:parent, :ins_value, :function, :nom_value)");
    condition.bindValue(":parent", parent);
    condition.bindValue(":ins_value", ins_value);
    condition.bindValue(":function", function);
    condition.bindValue(":nom_value", nom_value);
    condition.exec();
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
    initDatabase(&db);
    initVariables();
    initTables();
    initWarnings();
    openDatabase(QString());
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
    if (path.isEmpty()) {
        path = db.databaseName();
        if (!db.isOpen()) {
            QMessageBox::critical(this, tr("Open database - Leaklog"), tr("Cannot read file %1:\n%2.").arg(path).arg(db.lastError().text()));
            this->setWindowTitle(tr("Leaklog"));
            return;
        }
    } else {
        db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName(path);
        if (!db.open()) {
            QMessageBox::critical(this, tr("Open database - Leaklog"), tr("Cannot read file %1:\n%2.").arg(path).arg(db.lastError().text()));
            this->setWindowTitle(tr("Leaklog"));
            return;
        }
    }
    clearAll();
    QString id;
    QSqlQuery begin("BEGIN TRANSACTION");
    QSqlQuery customers("SELECT id, company FROM customers");
    while (customers.next()) {
        id = customers.value(0).toString().rightJustified(8, '0');
        QListWidgetItem * item = new QListWidgetItem;
        item->setText(customers.value(1).toString().isEmpty() ? id : tr("%1 (%2)").arg(id).arg(customers.value(1).toString()));
        item->setData(Qt::UserRole, customers.value(0).toString());
        lw_customers->addItem(item);
    }
    QSqlQuery inspectors("SELECT id, person FROM inspectors");
    while (inspectors.next()) {
        id = inspectors.value(0).toString().rightJustified(4, '0');
        QListWidgetItem * item = new QListWidgetItem;
        item->setText(inspectors.value(1).toString().isEmpty() ? id : tr("%1 (%2)").arg(id).arg(inspectors.value(1).toString()));
        item->setData(Qt::UserRole, inspectors.value(0).toString());
        lw_inspectors->addItem(item);
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
    saveDatabase(false);
}

void MainWindow::saveAndCompact()
{
	saveDatabase(true);
}

void MainWindow::saveDatabase(bool compact)
{
    QString error;
    QSqlQuery commit("COMMIT");
    if (commit.lastError().type() != QSqlError::NoError) { error = commit.lastError().text(); }
    if (compact) {
        QSqlQuery compact("VACUUM");
        if (compact.lastError().type() != QSqlError::NoError) { error = compact.lastError().text(); }
    }
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
    parsed_expressions.clear();
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
        QString id = record.id().rightJustified(8, '0');
        QString company = record.list("company").value("company").toString();
        QListWidgetItem * item = new QListWidgetItem;
        item->setText(company.isEmpty() ? id : tr("%1 (%2)").arg(id).arg(company));
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
    QString old_id = toString(selectedCustomer());
    if (md->exec() == QDialog::Accepted) {
        record = md->record();
        QString id = record.id().rightJustified(8, '0');
        QString company = record.list("company").value("company").toString();
        QListWidgetItem * item = lw_customers->highlightedItem();
        item->setText(company.isEmpty() ? id : tr("%1 (%2)").arg(id).arg(company));
        item->setData(Qt::UserRole, record.id());
        if (old_id != record.id()) {
            QSqlQuery update_circuits;
            update_circuits.prepare("UPDATE circuits SET parent = :new_id WHERE parent = :old_id");
            update_circuits.bindValue(":old_id", old_id);
            update_circuits.bindValue(":new_id", record.id());
            update_circuits.exec();
            QSqlQuery update_inspections;
            update_inspections.prepare("UPDATE inspections SET customer = :new_id WHERE customer = :old_id");
            update_inspections.bindValue(":old_id", old_id);
            update_inspections.bindValue(":new_id", record.id());
            update_inspections.exec();
        }
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
    MTRecord circuits("circuit", "", MTDictionary("parent", toString(selectedCustomer())));
    circuits.remove();
    MTRecord inspections("inspection", "", MTDictionary("customer", toString(selectedCustomer())));
    inspections.remove();
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
        item->setText(circuits.value(0).toString().rightJustified(4, '0'));
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
        item->setText(record.id().rightJustified(4, '0'));
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
    QString old_id = toString(selectedCircuit());
    if (md->exec() == QDialog::Accepted) {
        record = md->record();
        QListWidgetItem * item = lw_circuits->highlightedItem();
        item->setText(record.id().rightJustified(4, '0'));
        item->setData(Qt::UserRole, record.id());
        if (old_id != record.id()) {
            QSqlQuery update_inspections;
            update_inspections.prepare("UPDATE inspections SET circuit = :new_id WHERE customer = :customer_id AND circuit = :old_id");
            update_inspections.bindValue(":customer_id", selectedCustomer());
            update_inspections.bindValue(":old_id", old_id);
            update_inspections.bindValue(":new_id", record.id());
            update_inspections.exec();
        }
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
    MTDictionary parents("customer", toString(selectedCustomer()));
    parents.insert("circuit", toString(selectedCircuit()));
    MTRecord inspections("inspection", "", parents);
    inspections.remove();
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
        addColumn(record.id(), "inspections", &db);
        parsed_expressions.clear();
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
    QString id = item->text(1);
    ModifyDialogue * md = new ModifyDialogue(record, this);
    if (md->exec() == QDialog::Accepted) {
        record = md->record();
        QMap<QString, QVariant> attributes = record.list("name, unit, type");
        item->setText(0, attributes.value("name").toString());
        item->setText(1, record.id());
        item->setText(2, attributes.value("unit").toString());
        item->setText(3, dict_vartypes.value(attributes.value("type").toString()));
        if (id != record.id()) {
            renameColumn(id, record.id(), "inspections", &db);
            parsed_expressions.clear();
            if (record.type() == "variable") {
                QSqlQuery update_subvariables;
                update_subvariables.prepare("UPDATE subvariables SET parent = :new_id WHERE parent = :old_id");
                update_subvariables.bindValue(":old_id", id);
                update_subvariables.bindValue(":new_id", record.id());
                update_subvariables.exec();
            }
        }
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
    if (dict_varnames.contains(item->text(1))) { return; }
    bool subvar = item->parent() != NULL;
    QString id = item->text(1);
    bool ok;
    QString confirmation = QInputDialog::getText(this, subvar ? tr("Remove subvariable - Leaklog") : tr("Remove variable - Leaklog"), tr("Are you sure you want to remove the selected variable?\nTo remove the variable \"%1\" type REMOVE and confirm:").arg(id), QLineEdit::Normal, "", &ok);
    if (!ok || confirmation != tr("REMOVE")) { return; }
    MTDictionary parents;
    if (subvar) { parents.insert("parent", item->parent()->text(1)); }
    else {
        MTRecord subvars("subvariable", "", MTDictionary("parent", id));
        subvars.remove();
    }
    MTRecord record(subvar ? "subvariable" : "variable", id, parents);
    record.remove();
    if (item != NULL) { delete item; }
    dropColumn(id, "inspections", &db);
    parsed_expressions.clear();
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
    bool ok;
    QString confirmation = QInputDialog::getText(this, tr("Remove warning - Leaklog"), tr("Are you sure you want to remove the selected warning?\nTo remove the warning \"%1\" type REMOVE and confirm:").arg(item->text()), QLineEdit::Normal, "", &ok);
    if (!ok || confirmation != tr("REMOVE")) { return; }
    MTRecord record("warning", item->data(Qt::UserRole).toString(), MTDictionary());
    record.remove();
    MTRecord filters("warnings_filter", "", MTDictionary("parent", item->data(Qt::UserRole).toString()));
    filters.remove();
    MTRecord conditions("warnings_condition", "", MTDictionary("parent", item->data(Qt::UserRole).toString()));
    conditions.remove();
    delete item;
    enableTools();
    this->setWindowModified(true);
    refreshView();
}

void MainWindow::addInspector()
{
    if (!db.isOpen()) { return; }
    MTRecord record("inspector", "", MTDictionary());
    ModifyDialogue * md = new ModifyDialogue(record, this);
    if (md->exec() == QDialog::Accepted) {
        record = md->record();
        QString id = record.id().rightJustified(4, '0');
        QString person = record.list("person").value("person").toString();
        QListWidgetItem * item = new QListWidgetItem;
        item->setText(person.isEmpty() ? id : tr("%1 (%2)").arg(id).arg(person));
        item->setData(Qt::UserRole, record.id());
        lw_inspectors->addItem(item);
        this->setWindowModified(true);
        refreshView();
    }
    delete md;
}

void MainWindow::modifyInspector()
{
    if (!db.isOpen()) { return; }
    if (selectedInspector() < 0) { return; }
    MTRecord record("inspector", toString(selectedInspector()), MTDictionary());
    ModifyDialogue * md = new ModifyDialogue(record, this);
    QString old_id = toString(selectedInspector());
    if (md->exec() == QDialog::Accepted) {
        record = md->record();
        QString id = record.id().rightJustified(4, '0');
        QString person = record.list("person").value("person").toString();
        QListWidgetItem * item = lw_inspectors->highlightedItem();
        item->setText(person.isEmpty() ? id : tr("%1 (%2)").arg(id).arg(person));
        item->setData(Qt::UserRole, record.id());
        if (old_id != record.id()) {
            QSqlQuery update_inspections;
            update_inspections.prepare("UPDATE inspections SET inspector = :new_id WHERE inspector = :old_id");
            update_inspections.bindValue(":old_id", old_id);
            update_inspections.bindValue(":new_id", record.id());
            update_inspections.exec();
        }
        this->setWindowModified(true);
        refreshView();
    }
    delete md;
}

void MainWindow::removeInspector()
{
    if (!db.isOpen()) { return; }
    if (selectedInspector() < 0) { return; }
    QListWidgetItem * item = lw_inspectors->highlightedItem();
    bool ok;
    QString confirmation = QInputDialog::getText(this, tr("Remove inspector - Leaklog"), tr("Are you sure you want to remove the selected inspector?\nTo remove all data about the inspector \"%1\" type REMOVE and confirm:").arg(selectedInspector()), QLineEdit::Normal, "", &ok);
    if (!ok || confirmation != tr("REMOVE")) { return; }
    MTRecord record("inspector", toString(selectedInspector()), MTDictionary());
    record.remove();
    if (item != NULL) { delete item; }
    enableTools();
    this->setWindowModified(true);
    setView(tr("Inspectors"));
}

void MainWindow::loadInspector(QListWidgetItem * item) { loadInspector(item, true); }

void MainWindow::loadInspector(QListWidgetItem * item, bool refresh)
{
    if (item == NULL) { return; }
    lw_inspections->highlightItem(item);
    enableTools();
    if (refresh) {
        setView(tr("Inspectors"));
    }
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
    QString id_justified;
    QSqlQuery customers("SELECT id, company FROM customers", data);
    while (customers.next()) {
        id_justified = customers.value(0).toString().rightJustified(8, '0');
        QListWidgetItem * item = new QListWidgetItem(id->customers());
        item->setCheckState(Qt::Unchecked);
        item->setText(customers.value(1).toString().isEmpty() ? id_justified : tr("%1 (%2)").arg(id_justified).arg(customers.value(1).toString()));
        item->setData(Qt::UserRole, customers.value(0).toString());
    }
    QSqlQuery circuits("SELECT id, parent FROM circuits", data);
    while (circuits.next()) {
        QListWidgetItem * item = new QListWidgetItem(id->circuits());
        item->setCheckState(Qt::Unchecked);
        item->setHidden(true);
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable);
        item->setText(QString("%1 (%2)").arg(circuits.value(0).toString().rightJustified(4, '0')).arg(circuits.value(1).toString().rightJustified(8, '0')));
        item->setData(Qt::UserRole, QString("%1::%2").arg(circuits.value(1).toString()).arg(circuits.value(0).toString()));
    }
    QSqlQuery inspections("SELECT date, customer, circuit FROM inspections", data);
    while (inspections.next()) {
        QListWidgetItem * item = new QListWidgetItem(id->inspections());
        item->setCheckState(Qt::Checked);
        item->setHidden(true);
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable);
        item->setText(QString("%1 (%2::%3)").arg(inspections.value(0).toString()).arg(inspections.value(1).toString().rightJustified(8, '0')).arg(inspections.value(2).toString().rightJustified(4, '0')));
        item->setData(Qt::UserRole, QString("%1::%2::%3").arg(inspections.value(1).toString()).arg(inspections.value(2).toString()).arg(inspections.value(0).toString()));
    }
    QSqlQuery query("SELECT variables.id, variables.name, variables.type, variables.unit, variables.value, variables.compare_nom, variables.col_bg, subvariables.id, subvariables.name, subvariables.type, subvariables.unit, subvariables.value, subvariables.compare_nom FROM variables LEFT JOIN subvariables ON variables.id = subvariables.parent", data);
    const int VAR_ID = 0; const int VAR_NAME = 1; const int VAR_TYPE = 2; const int VAR_UNIT = 3; const int VAR_VALUE = 4; const int VAR_COMPARE_NOM = 5; const int VAR_COL_BG = 6;
    const int SUBVAR_ID = 7; const int SUBVAR_NAME = 8; const int SUBVAR_TYPE = 9; const int SUBVAR_UNIT = 10; const int SUBVAR_VALUE = 11; const int SUBVAR_COMPARE_NOM = 12;
    QString last_id; QTreeWidgetItem * last_item = NULL;
    while (query.next()) {
        if (query.value(VAR_ID).toString() != last_id) {
            last_item = new QTreeWidgetItem(id->variables());
            last_item->setText(0, query.value(VAR_NAME).toString());
            last_item->setText(1, query.value(VAR_ID).toString());
            last_item->setText(2, query.value(VAR_UNIT).toString());
            last_item->setText(3, dict_vartypes.value(query.value(VAR_TYPE).toString()));
            last_item->setText(4, query.value(VAR_VALUE).toString());
            last_item->setText(5, query.value(VAR_COMPARE_NOM).toInt() ? tr("Yes") : tr("No"));
            last_item->setText(6, query.value(VAR_COL_BG).toString());
            QSqlQuery variable;
            variable.prepare("SELECT id, name, type, unit, value, compare_nom, col_bg FROM variables WHERE id = :id");
            variable.bindValue(":id", query.value(VAR_ID));
            variable.exec();
            bool found = false, overwrite = false;
            if (variable.next()) {
                found = true;
                if (variable.value(VAR_NAME).toString() != query.value(VAR_NAME).toString()) { overwrite = true; last_item->setBackground(0, QBrush(Qt::darkMagenta)); last_item->setForeground(0, QBrush(Qt::white)); }
                if (variable.value(VAR_UNIT).toString() != query.value(VAR_UNIT).toString()) { overwrite = true; last_item->setBackground(2, QBrush(Qt::darkMagenta)); last_item->setForeground(2, QBrush(Qt::white)); }
                if (variable.value(VAR_TYPE).toString() != query.value(VAR_TYPE).toString()) { overwrite = true; last_item->setBackground(3, QBrush(Qt::darkMagenta)); last_item->setForeground(3, QBrush(Qt::white)); }
                if (variable.value(VAR_VALUE).toString() != query.value(VAR_VALUE).toString()) { overwrite = true; last_item->setBackground(4, QBrush(Qt::darkMagenta)); last_item->setForeground(4, QBrush(Qt::white)); }
                if (variable.value(VAR_COMPARE_NOM).toInt() != query.value(VAR_COMPARE_NOM).toInt()) { overwrite = true; last_item->setBackground(5, QBrush(Qt::darkMagenta)); last_item->setForeground(5, QBrush(Qt::white)); }
                if (variable.value(VAR_COL_BG).toString() != query.value(VAR_COL_BG).toString()) { overwrite = true; last_item->setBackground(6, QBrush(Qt::darkMagenta)); last_item->setForeground(6, QBrush(Qt::white)); }
            }
            QComboBox * cb_action = new QComboBox;
            if (!found) {
                cb_action->addItem(tr("Import"));
                last_item->setIcon(0, QIcon(QString::fromUtf8(":/images/images/item_new16.png")));
            } else {
                cb_action->addItem(tr("Use existing and import"));
                if (overwrite) {
                    cb_action->addItem(tr("Overwrite and import"));
                    last_item->setIcon(0, QIcon(QString::fromUtf8(":/images/images/item_found_diff16.png")));
                } else {
                    last_item->setIcon(0, QIcon(QString::fromUtf8(":/images/images/item_found16.png")));
                }
            }
            if (!dict_varnames.contains(query.value(VAR_ID).toString())) {
                cb_action->insertItem(0, tr("Do not import"));
                cb_action->setCurrentIndex(1);
            } else {
                cb_action->setCurrentIndex(0);
            }
            id->variables()->setItemWidget(last_item, 7, cb_action);
            last_id = query.value(VAR_ID).toString();
        }
        if (!query.value(SUBVAR_ID).toString().isEmpty() && last_item) {
            QTreeWidgetItem * subitem = new QTreeWidgetItem(last_item);
            subitem->setText(0, query.value(SUBVAR_NAME).toString());
            subitem->setText(1, query.value(SUBVAR_ID).toString());
            subitem->setText(2, query.value(SUBVAR_UNIT).toString());
            subitem->setText(3, dict_vartypes.value(query.value(SUBVAR_TYPE).toString()));
            subitem->setText(4, query.value(SUBVAR_VALUE).toString());
            subitem->setText(5, query.value(SUBVAR_COMPARE_NOM).toInt() ? tr("Yes") : tr("No"));
            QSqlQuery variable;
            variable.prepare("SELECT id, name, type, unit, value, compare_nom, parent FROM subvariables WHERE id = :id");
            variable.bindValue(":id", query.value(SUBVAR_ID));
            variable.exec();
            bool found = false, overwrite = false;
            if (variable.next()) {
                found = true;
                if (variable.value(VAR_NAME).toString() != query.value(SUBVAR_NAME).toString()) { overwrite = true; subitem->setBackground(0, QBrush(Qt::darkMagenta)); subitem->setForeground(0, QBrush(Qt::white)); }
                if (variable.value(VAR_UNIT).toString() != query.value(SUBVAR_UNIT).toString()) { overwrite = true; subitem->setBackground(2, QBrush(Qt::darkMagenta)); subitem->setForeground(2, QBrush(Qt::white)); }
                if (variable.value(VAR_TYPE).toString() != query.value(SUBVAR_TYPE).toString()) { overwrite = true; subitem->setBackground(3, QBrush(Qt::darkMagenta)); subitem->setForeground(3, QBrush(Qt::white)); }
                if (variable.value(VAR_VALUE).toString() != query.value(SUBVAR_VALUE).toString()) { overwrite = true; subitem->setBackground(4, QBrush(Qt::darkMagenta)); subitem->setForeground(4, QBrush(Qt::white)); }
                if (variable.value(VAR_COMPARE_NOM).toInt() != query.value(SUBVAR_COMPARE_NOM).toInt()) { overwrite = true; subitem->setBackground(5, QBrush(Qt::darkMagenta)); subitem->setForeground(5, QBrush(Qt::white)); }
                // VAR_COL_BG == VAR_PARENT
                if (variable.value(VAR_COL_BG).toString() != query.value(VAR_ID).toString()) { overwrite = true; last_item->setBackground(1, QBrush(Qt::darkMagenta)); subitem->setForeground(1, QBrush(Qt::white)); }
            }
            QComboBox * cb_action = new QComboBox;
            if (!found) {
                cb_action->addItem(tr("Import"));
                subitem->setIcon(0, QIcon(QString::fromUtf8(":/images/images/item_new16.png")));
            } else {
                cb_action->addItem(tr("Use existing and import"));
                if (overwrite) {
                    cb_action->addItem(tr("Overwrite and import"));
                    subitem->setIcon(0, QIcon(QString::fromUtf8(":/images/images/item_found_diff16.png")));
                } else {
                    subitem->setIcon(0, QIcon(QString::fromUtf8(":/images/images/item_found16.png")));
                }
            }
            if (!dict_varnames.contains(query.value(SUBVAR_ID).toString())) {
                cb_action->insertItem(0, tr("Do not import"));
                cb_action->setCurrentIndex(1);
            } else {
                cb_action->setCurrentIndex(0);
            }
            id->variables()->setItemWidget(subitem, 7, cb_action);
            last_item->setExpanded(true);
        }
    }
    if (id->exec() != QDialog::Accepted) { data.close(); QSqlDatabase::removeDatabase(data.connectionName()); return; }
    QMap<QString, QVariant> set;
    for (int c = 0; c < id->customers()->count(); ++c) {
        if (id->customers()->item(c)->checkState() == Qt::Unchecked) { continue; }
        set.clear();
        QString c_id = id->customers()->item(c)->data(Qt::UserRole).toString();
        QSqlQuery customer_data(data);
        customer_data.prepare("SELECT * FROM customers WHERE id = :id");
        customer_data.bindValue(":id", c_id);
        customer_data.exec();
        if (customer_data.next()) {
            for (int f = 0; f < customer_data.record().count(); ++f) {
                set.insert(customer_data.record().fieldName(f), customer_data.value(f));
            }
        }
        MTRecord record("customer", c_id, MTDictionary());
        if (!record.exists()) {
            id_justified = c_id.rightJustified(8, '0');
            QListWidgetItem * item = new QListWidgetItem;
            item->setText(set.value("name", QString()).toString().isEmpty() ? id_justified : tr("%1 (%2)").arg(id_justified).arg(set.value("name").toString()));
            item->setData(Qt::UserRole, c_id);
            lw_customers->addItem(item);
        }
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
        if (toString(selectedCustomer()) == cc_parent && !record.exists()) {
            QListWidgetItem * item = new QListWidgetItem;
            item->setText(cc_id.rightJustified(4, '0'));
            item->setData(Qt::UserRole, cc_id);
            lw_circuits->addItem(item);
        }
        record.update(set);
    }
    QStringList inspections_skip_columns; bool skip_parent = false;
    QString current_text; QTreeWidgetItem * item = NULL; QTreeWidgetItem * subitem = NULL;
    QTreeWidgetItem * new_item = NULL;
    for (int v = 0; v < id->variables()->topLevelItemCount(); ++v) {
        item = id->variables()->topLevelItem(v);
        skip_parent = false; new_item = NULL;
        current_text = ((QComboBox *)id->variables()->itemWidget(item, 7))->currentText();
        if (current_text == tr("Do not import")) {
            inspections_skip_columns << item->text(1);
            skip_parent = true;
        } else if (current_text == tr("Import") || current_text == tr("Overwrite and import")) {
            MTRecord record("variable", item->text(1), MTDictionary());
            if (!record.exists()) {
                new_item = new QTreeWidgetItem(trw_variables);
                new_item->setText(0, item->text(0));
                new_item->setText(1, item->text(1));
                new_item->setText(2, item->text(2));
                new_item->setText(3, item->text(3));
            }
            set.clear();
            set.insert("name", item->text(0));
            set.insert("id", item->text(1));
            set.insert("unit", item->text(2));
            set.insert("type", dict_vartypes.firstKey(item->text(3)));
            set.insert("value", item->text(4));
            set.insert("compare_nom", item->text(5) == tr("Yes") ? 1 : 0);
            set.insert("col_bg", item->text(6));
            record.update(set);
        }
        for (int sv = 0; sv < item->childCount(); ++sv) {
            subitem = item->child(sv);
            current_text = ((QComboBox *)id->variables()->itemWidget(subitem, 7))->currentText();
            if (skip_parent || current_text == tr("Do not import")) {
                inspections_skip_columns << subitem->text(1);
            } else if (current_text == tr("Import") || current_text == tr("Overwrite and import")) {
                MTRecord record("subvariable", subitem->text(1), MTDictionary("parent", item->text(1)));
                if (new_item != NULL && !record.exists()) {
                    QTreeWidgetItem * new_subitem = new QTreeWidgetItem(new_item);
                    new_subitem->setText(0, subitem->text(0));
                    new_subitem->setText(1, subitem->text(1));
                    new_subitem->setText(2, subitem->text(2));
                    new_subitem->setText(3, subitem->text(3));
                }
                set.clear();
                set.insert("name", subitem->text(0));
                set.insert("id", subitem->text(1));
                set.insert("unit", subitem->text(2));
                set.insert("type", dict_vartypes.firstKey(subitem->text(3)));
                set.insert("value", subitem->text(4));
                set.insert("compare_nom", subitem->text(5) == tr("Yes") ? 1 : 0);
                record.update(set);
            }
        }
    }
    inspections_skip_columns << "customer" << "circuit";
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
                if (!inspections_skip_columns.contains(inspection_data.record().fieldName(f))) {
                    set.insert(inspection_data.record().fieldName(f), inspection_data.value(f));
                }
            }
        }
        MTDictionary parents("customer", i_customer);
        parents.insert("circuit", i_circuit);
        MTRecord record("inspection", i_date, parents);
        if (toString(selectedCustomer()) == i_customer && toString(selectedCircuit()) == i_circuit && !record.exists()) {
            QListWidgetItem * item = new QListWidgetItem;
            item->setText(i_date);
            item->setData(Qt::UserRole, i_date);
            lw_inspections->addItem(item);
        }
        record.update(set, j == 0);
        j++;
    }
    data.close(); QSqlDatabase::removeDatabase(data.connectionName());
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
