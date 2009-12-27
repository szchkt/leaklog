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
#include "global.h"
#include "variables.h"
#include "warnings.h"
#include "modify_warning_dialogue.h"
#include "import_dialogue.h"
#include "records.h"
#include "mtlistwidget.h"

#include <QMessageBox>
#include <QFile>
#include <QFileDialog>
#include <QInputDialog>
#include <QDialogButtonBox>
#include <QSqlRecord>
#include <QSqlError>
#include <QDateTime>

using namespace Global;

bool MainWindow::saveChangesBeforeProceeding(const QString & title, bool close_)
{
    if (db.isOpen() && this->isWindowModified()) {
        QMessageBox message(this);
        message.setWindowTitle(title);
        message.setWindowModality(Qt::WindowModal);
        message.setWindowFlags(message.windowFlags() | Qt::Sheet);
        message.setIcon(QMessageBox::Information);
        message.setText(tr("The database has been modified."));
        message.setInformativeText(tr("Do you want to save your changes?"));
        message.addButton(tr("&Save"), QMessageBox::AcceptRole);
        message.addButton(tr("&Discard"), QMessageBox::DestructiveRole);
        message.addButton(tr("Cancel"), QMessageBox::RejectRole);
        switch (message.exec()) {
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

void MainWindow::initDatabase(QSqlDatabase * database, bool transaction)
{
    if (transaction) { database->transaction(); }
{ // (SCOPE)
    QSqlQuery query(*database);
    QStringList tables = database->tables();
    for (int i = 0; i < dict_dbtables.count(); ++i) {
        if (!tables.contains(dict_dbtables.key(i))) {
            query.exec("CREATE TABLE " + dict_dbtables.key(i) + " (" + dict_dbtables.value(i) + ")");
        } else {
            MTDictionary field_names = getTableFieldNames(dict_dbtables.key(i), database);
            QStringList all_field_names = dict_dbtables.value(i).split(", ");
            if (dict_dbtables.key(i) == "inspections") {
                for (int v = 0; v < dict_varnames.count(); ++v) {
                    all_field_names << dict_varnames.key(v) + " TEXT";
                }
            }
            for (int f = 0; f < all_field_names.count(); ++f) {
                if (!field_names.contains(all_field_names.at(f).split(" ").first())) {
                    addColumn(all_field_names.at(f), dict_dbtables.key(i), database);
                }
            }
        }
    }
    QStringList db_info_ids;
    db_info_ids << "created_with" << "date_created" << "saved_with" << "db_version" << "default_service_company"
            << "lock_date" << "lock_password" << "locked";
    for (int i = 0; i < db_info_ids.count(); ++i) {
        query.exec("SELECT value FROM db_info WHERE id = '" + db_info_ids.at(i) + "'");
        if (!query.next()) {
            query.exec("INSERT INTO db_info (id) VALUES ('" + db_info_ids.at(i) + "')");
        }
    }
    double v = DBInfoValueForKey("db_version").toDouble();
    if (v < 0.906) {
        query.exec("UPDATE inspections SET refr_add_am = 0 WHERE refr_add_am IS NULL");
        query.exec("UPDATE inspections SET refr_add_am_recy = 0 WHERE refr_add_am_recy IS NULL");
        query.exec("UPDATE repairs SET refr_add_am = 0 WHERE refr_add_am IS NULL");
        query.exec("UPDATE repairs SET refr_add_am_recy = 0 WHERE refr_add_am_recy IS NULL");
        query.exec("UPDATE inspections SET refr_reco = 0 WHERE refr_reco IS NULL");
        query.exec("UPDATE inspections SET refr_reco_cust = 0 WHERE refr_reco_cust IS NULL");
        query.exec("UPDATE repairs SET refr_reco = 0 WHERE refr_reco IS NULL");
        query.exec("UPDATE repairs SET refr_reco_cust = 0 WHERE refr_reco_cust IS NULL");
        query.exec("UPDATE inspections SET refr_add_am = refr_add_am + refr_add_am_recy, refr_add_am_recy = 0, refr_reco = refr_reco + refr_reco_cust, refr_reco_cust = 0");
        query.exec("UPDATE repairs SET refr_add_am = refr_add_am + refr_add_am_recy, refr_add_am_recy = 0, refr_reco = refr_reco + refr_reco_cust, refr_reco_cust = 0");
    }
    if (v < F_DB_VERSION) {
        query.exec("DROP INDEX IF EXISTS index_service_companies_id");
        query.exec("DROP INDEX IF EXISTS index_customers_id");
        query.exec("DROP INDEX IF EXISTS index_circuits_id");
        query.exec("DROP INDEX IF EXISTS index_inspections_id");
        query.exec("DROP INDEX IF EXISTS index_repairs_id");
        query.exec("DROP INDEX IF EXISTS index_inspectors_id");
        query.exec("DROP INDEX IF EXISTS index_variables_id");
        query.exec("DROP INDEX IF EXISTS index_subvariables_id");
        query.exec("DROP INDEX IF EXISTS index_tables_id");
        query.exec("DROP INDEX IF EXISTS index_warnings_id");
        query.exec("DROP INDEX IF EXISTS index_warnings_filters_parent");
        query.exec("DROP INDEX IF EXISTS index_warnings_conditions_parent");
        query.exec("DROP INDEX IF EXISTS index_refrigerant_management_id");
        query.exec("CREATE UNIQUE INDEX index_service_companies_id ON service_companies (id ASC)");
        query.exec("CREATE UNIQUE INDEX index_customers_id ON customers (id ASC)");
        query.exec("CREATE UNIQUE INDEX index_circuits_id ON circuits (parent ASC, id ASC)");
        query.exec("CREATE UNIQUE INDEX index_inspections_id ON inspections (customer ASC, circuit ASC, date ASC)");
        query.exec("CREATE UNIQUE INDEX index_repairs_id ON repairs (date ASC)");
        query.exec("CREATE UNIQUE INDEX index_inspectors_id ON inspectors (id ASC)");
        query.exec("CREATE UNIQUE INDEX index_variables_id ON variables (id ASC)");
        query.exec("CREATE UNIQUE INDEX index_subvariables_id ON subvariables (id ASC)");
        query.exec("CREATE UNIQUE INDEX index_tables_id ON tables (id ASC)");
        query.exec("CREATE UNIQUE INDEX index_warnings_id ON warnings (id ASC)");
        query.exec("CREATE INDEX index_warnings_filters_parent ON warnings_filters (parent ASC)");
        query.exec("CREATE INDEX index_warnings_conditions_parent ON warnings_conditions (parent ASC)");
        query.exec("CREATE UNIQUE INDEX index_refrigerant_management_id ON refrigerant_management (date ASC)");
    }
} // (SCOPE)
    if (transaction) { database->commit(); }
}

void MainWindow::initTables(bool transaction)
{
    if (transaction) { db.transaction(); }
{ // (SCOPE)
    StringVariantMap set;
    Table leakages("", "Leakages");
    if (!leakages.exists()) {
        set.insert("id", tr("Leakages"));
        set.insert("highlight_nominal", 0);
        set.insert("variables", "vis_aur_chk;dir_leak_chk;refr_add;refr_reco;inspector;operator;rmds;arno");
        set.insert("sum", "vis_aur_chk;refr_add;refr_reco");
        leakages.update(set);
        set.clear();
    }
    Table pressures_and_temperatures("", "Pressures and temperatures");
    if (!pressures_and_temperatures.exists()) {
        set.insert("id", tr("Pressures and temperatures"));
        set.insert("highlight_nominal", 1);
        set.insert("variables", "t_sec;p_0;t_0;delta_t_evap;t_evap_out;t_comp_in;t_sh;p_c;t_c;delta_t_c;t_ev;t_sc;t_comp_out");
        set.insert("sum", "");
        pressures_and_temperatures.update(set);
        set.clear();
    }
    Table electrical_parameters("", "Electrical parameters");
    if (!electrical_parameters.exists()) {
        set.insert("id", tr("Electrical parameters"));
        set.insert("highlight_nominal", 1);
        set.insert("variables", "ep_comp;ec;ev;ppsw;sftsw");
        set.insert("sum", "");
        electrical_parameters.update(set);
        //set.clear();
    }
    double v = DBInfoValueForKey("db_version").toDouble();
    if (v < 0.903) {
        Table table_of_leakages(tr("Table of leakages"));
        table_of_leakages.remove();
        Table table_of_parameters(tr("Table of parameters"));
        table_of_parameters.remove();
    } else if (v < 0.906) {
        QStringList table_vars = pressures_and_temperatures.stringValue("variables").split(";", QString::SkipEmptyParts);
        if (table_vars.contains("t")) {
            table_vars.replace(table_vars.indexOf("t"), "t_sec");
        } else if (!table_vars.contains("t_sec")) {
            table_vars.prepend("t_sec");
        }
        set.clear();
        set.insert("variables", table_vars.join(";"));
        pressures_and_temperatures.update(set);

        table_vars = leakages.stringValue("variables").split(";", QString::SkipEmptyParts);
        if (table_vars.contains("refr_recovery")) {
            table_vars.replace(table_vars.indexOf("refr_recovery"), "refr_reco");
            set.clear();
            set.insert("variables", table_vars.join(";"));
            leakages.update(set);
        }
    }
} // (SCOPE)
    if (transaction) { db.commit(); }
}

void MainWindow::newDatabase()
{
    if (saveChangesBeforeProceeding(tr("New database - Leaklog"), true)) { return; }
    QString path = QFileDialog::getSaveFileName(this, tr("New database - Leaklog"), tr("untitled.lklg"), tr("Leaklog Database (*.lklg)"));
    if (path.isEmpty()) { return; }
    if (!path.endsWith(".lklg", Qt::CaseInsensitive)) { path.append(".lklg"); }
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
    initTables();
    db.transaction();
    setDBInfoValueForKey("created_with", QString("Leaklog-%1").arg(F_LEAKLOG_VERSION));
    setDBInfoValueForKey("date_created", QDateTime::currentDateTime().toString("yyyy.MM.dd-hh:mm"));
    openDatabase(QString());
}

void MainWindow::openRecent(QListWidgetItem * item)
{
    QString s = item->text();
    addRecent(s);
    if (s.startsWith("db:")) {
        QStringList path = s.split("@");
        bool ok;
        QString password = QInputDialog::getText(this, tr("Open remote database - Leaklog"), tr("Enter password for %1@%2:").arg(path.at(0).split(":").at(2)).arg(path.at(1)), QLineEdit::Password, "", &ok);
        if (!ok) { return; }
        db = QSqlDatabase::addDatabase(path.at(0).split(":").at(1));
        db.setHostName(path.at(2).split(":").at(0));
        db.setPort(path.at(2).split(":").at(1).toInt());
        db.setDatabaseName(path.at(1));
        db.setUserName(path.at(0).split(":").at(2));
        db.setPassword(password);
        if (db.open()) {
            db.transaction();
            initDatabase(&db, false);
        }
        openDatabase(QString());
    } else {
        openDatabase(s);
    }
}

void MainWindow::open()
{
    if (saveChangesBeforeProceeding(tr("Open database - Leaklog"), true)) { return; }
    QString path = QFileDialog::getOpenFileName(this, tr("Open database - Leaklog"), "", tr("Leaklog Databases (*.lklg);;All files (*.*)"));
    if (path.isEmpty()) { return; }
    addRecent(path);
    openDatabase(path);
}

void MainWindow::openRemote()
{
    if (saveChangesBeforeProceeding(tr("Open remote database - Leaklog"), true)) { return; }
    QDialog * d = new QDialog(this);
	d->setWindowTitle(tr("Open remote database - Leaklog"));
        QGridLayout * gl = new QGridLayout(d);
            QLabel * lbl_driver_ = new QLabel(tr("Driver:"), d);
            lbl_driver_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        gl->addWidget(lbl_driver_, 0, 0);
            QLabel * lbl_driver = new QLabel("PostgreSQL", d);
        gl->addWidget(lbl_driver, 0, 1);
            QLabel * lbl_server = new QLabel(tr("Server:"), d);
            lbl_server->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        gl->addWidget(lbl_server, 1, 0);
            QLineEdit * le_server = new QLineEdit(d);
        gl->addWidget(le_server, 1, 1);
            QLabel * lbl_port = new QLabel(tr("Port:"), d);
            lbl_port->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        gl->addWidget(lbl_port, 2, 0);
            QSpinBox * spb_port = new QSpinBox(d);
            spb_port->setRange(0, 999999);
            spb_port->setValue(0);
            spb_port->setSpecialValueText(tr("Default"));
        gl->addWidget(spb_port, 2, 1);
            QLabel * lbl_db_name = new QLabel(tr("Database name:"), d);
            lbl_db_name->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        gl->addWidget(lbl_db_name, 3, 0);
            QLineEdit * le_db_name = new QLineEdit(d);
        gl->addWidget(le_db_name, 3, 1);
            QLabel * lbl_user_name = new QLabel(tr("User name:"), d);
            lbl_user_name->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        gl->addWidget(lbl_user_name, 4, 0);
            QLineEdit * le_user_name = new QLineEdit(d);
        gl->addWidget(le_user_name, 4, 1);
            QLabel * lbl_password = new QLabel(tr("Password:"), d);
            lbl_password->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        gl->addWidget(lbl_password, 5, 0);
            QLineEdit * le_password = new QLineEdit(d);
            le_password->setEchoMode(QLineEdit::Password);
        gl->addWidget(le_password, 5, 1);
            QDialogButtonBox * bb = new QDialogButtonBox(d);
            bb->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
            QObject::connect(bb, SIGNAL(accepted()), d, SLOT(accept()));
            QObject::connect(bb, SIGNAL(rejected()), d, SLOT(reject()));
        gl->addWidget(bb, 6, 0, 1, 2);
    if (d->exec() != QDialog::Accepted) { return; }
    db = QSqlDatabase::addDatabase("QPSQL");
    db.setHostName(le_server->text());
    db.setPort(spb_port->value());
    db.setDatabaseName(le_db_name->text());
    db.setUserName(le_user_name->text());
    db.setPassword(le_password->text());
    if (db.open()) {
        addRecent(QString("db:QPSQL:%1@%2@%3:%4").arg(le_user_name->text()).arg(le_db_name->text()).arg(le_server->text()).arg(spb_port->value()));
        db.transaction();
        initDatabase(&db, false);
    }
    openDatabase(QString());
}

void MainWindow::openDatabase(QString path)
{
    selected_repair.clear();
    clearAll();
    if (path.isEmpty()) {
        path = db.databaseName();
        if (!db.isOpen()) {
            QMessageBox::critical(this, tr("Open database - Leaklog"), tr("Cannot read file %1:\n%2.").arg(path).arg(db.lastError().text()));
            this->setWindowTitle(tr("Leaklog"));
            return;
        }
    } else {
        QFile file(path);
        if (!file.exists()) {
            QMessageBox::critical(this, tr("Open database - Leaklog"), tr("File %1 does not exist.").arg(path));
            this->setWindowTitle(tr("Leaklog"));
            return;
        }
        db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName(path);
        if (!db.open()) {
            QMessageBox::critical(this, tr("Open database - Leaklog"), tr("Cannot read file %1:\n%2.").arg(path).arg(db.lastError().text()));
            this->setWindowTitle(tr("Leaklog"));
            return;
        }
        db.transaction();
        initDatabase(&db, false);
    }
    initTables(false);
    QSqlQuery query;
    Variables variables;
    QString last_id; QTreeWidgetItem * last_item = NULL;
    while (variables.next()) {
        if (variables.value("VAR_ID").toString() != last_id) {
            last_item = new QTreeWidgetItem(trw_variables);
            last_item->setText(0, variables.value("VAR_NAME").toString());
            last_item->setText(1, variables.value("VAR_ID").toString());
            last_item->setText(2, variables.value("VAR_UNIT").toString());
            last_item->setText(3, variables.value("VAR_TOLERANCE").toString());
            last_id = variables.value("VAR_ID").toString();
        }
        if (!variables.value("SUBVAR_ID").toString().isEmpty() && last_item) {
            QTreeWidgetItem * subitem = new QTreeWidgetItem(last_item);
            subitem->setText(0, variables.value("SUBVAR_NAME").toString());
            subitem->setText(1, variables.value("SUBVAR_ID").toString());
            subitem->setText(2, variables.value("SUBVAR_UNIT").toString());
            subitem->setText(3, variables.value("SUBVAR_TOLERANCE").toString());
        }
    }
    query.exec("SELECT id FROM tables");
    while (query.next()) {
        cb_table_edit->addItem(query.value(0).toString());
        navigation->tableComboBox()->addItem(query.value(0).toString());
    }
    Warnings warnings;
    while (warnings.next()) {
        QListWidgetItem * item = new QListWidgetItem;
        item->setText(warnings.value("description").toString().isEmpty() ? warnings.value("name").toString() : tr("%1 (%2)").arg(warnings.value("name").toString()).arg(warnings.value("description").toString()));
        item->setData(Qt::UserRole, warnings.value("id").toString());
        lw_warnings->addItem(item);
    }
    database_lock_date = DBInfoValueForKey("lock_date");
    database_locked = DBInfoValueForKey("locked") == "true";
    updateLockButton();
#ifdef Q_WS_MAC
    this->setWindowTitle(QString("%1[*]").arg(QFileInfo(path).baseName()));
#else
    this->setWindowTitle(QString("%1[*] - Leaklog").arg(QFileInfo(path).baseName()));
#endif
    this->setWindowModified(false);
    setAllEnabled(true);
    stw_main->setCurrentIndex(1);
    enableTools();
    //loadTable(cb_table_edit->currentText());
    navigation->setView(Navigation::ServiceCompany);
    query.exec("SELECT date FROM refrigerant_management WHERE purchased > 0 OR purchased_reco > 0");
    if (!query.next())
        QMessageBox::information(this, tr("Refrigerant management"), tr("You should add a record of purchase for every kind of refrigerant you have in store. You can do so by clicking the \"Add record of refrigerant management\" button."));
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
    QStringList errors;
    setDBInfoValueForKey("saved_with", QString("Leaklog-%1").arg(F_LEAKLOG_VERSION));
    setDBInfoValueForKey("db_version", QString::number(F_DB_VERSION));
    db.commit();
    if (compact) {
        QSqlQuery query;
        query.exec("VACUUM");
        if (query.lastError().type() != QSqlError::NoError) { errors << query.lastError().text(); }
    }
    db.transaction();
    if (!errors.isEmpty()) {
        QMessageBox::critical(this, tr("Save database - Leaklog"), tr("Cannot write file %1:\n%2.").arg(db.databaseName()).arg(errors.join("; ")));
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
    db.rollback(); db.close(); db = QSqlDatabase(); QSqlDatabase::removeDatabase(db.connectionName());
    parsed_expressions.clear();
    clearAll(); enableTools(); setAllEnabled(false);
    stw_main->setCurrentIndex(0);
    this->setWindowTitle(tr("Leaklog"));
    this->setWindowModified(false);
}

QString MainWindow::DBInfoValueForKey(const QString & key)
{
    QSqlQuery query(QString("SELECT value FROM db_info WHERE id = '%1'").arg(key));
    if (!query.next()) { return QString(); }
    return query.value(0).toString();
}

QSqlError MainWindow::setDBInfoValueForKey(const QString & key, const QString & value)
{
    QSqlQuery query(QString("UPDATE db_info SET value = '%1' WHERE id = '%2'").arg(value).arg(key));
    return query.lastError();
}

void MainWindow::modifyServiceCompany()
{
    if (!db.isOpen()) { return; }
    ServiceCompany record(DBInfoValueForKey("default_service_company"));
    ModifyDialogue * md = new ModifyDialogue(&record, this);
    if (md->exec() == QDialog::Accepted) {
        setDBInfoValueForKey("default_service_company", record.id());
        this->setWindowModified(true);
        refreshView();
    }
}

void MainWindow::addRecordOfRefrigerantManagement()
{
    modifyRecordOfRefrigerantManagement("");
}

void MainWindow::modifyRecordOfRefrigerantManagement(const QString & date)
{
    if (!db.isOpen()) { return; }
    RecordOfRefrigerantManagement record(date);
    ModifyDialogue * md = new ModifyDialogue(&record, this);
    if (md->exec() == QDialog::Accepted) {
        StringVariantMap attributes = record.list();
        if (attributes.value("purchased").toDouble() <= 0.0 && attributes.value("purchased_reco").toDouble() <= 0.0 &&
            attributes.value("sold").toDouble() <= 0.0 && attributes.value("sold_reco").toDouble() <= 0.0 &&
            attributes.value("refr_rege").toDouble() <= 0.0 && attributes.value("refr_disp").toDouble() <= 0.0 &&
            attributes.value("leaked").toDouble() <= 0.0 && attributes.value("leaked_reco").toDouble() <= 0.0) {
            record.remove();
        }
        this->setWindowModified(true);
        refreshView();
    }
    delete md;
}

void MainWindow::addCustomer()
{
    if (!db.isOpen()) { return; }
    Customer record("");
    ModifyDialogue * md = new ModifyDialogue(&record, this);
    if (md->exec() == QDialog::Accepted) {
        this->setWindowModified(true);
        refreshView();
    }
    delete md;
}

void MainWindow::modifyCustomer()
{
    if (!db.isOpen()) { return; }
    if (!isCustomerSelected()) { return; }
    Customer record(selectedCustomer());
    ModifyDialogue * md = new ModifyDialogue(&record, this);
    QString old_id = selectedCustomer();
    if (md->exec() == QDialog::Accepted) {
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
    if (!isCustomerSelected()) { return; }
    bool ok;
    QString confirmation = QInputDialog::getText(this, tr("Remove customer - Leaklog"), tr("Are you sure you want to remove the selected customer?\nTo remove all data about the customer \"%1\" type REMOVE and confirm:").arg(selectedCustomer()), QLineEdit::Normal, "", &ok);
    if (!ok || confirmation != tr("REMOVE")) { return; }
    Customer record(selectedCustomer());
    record.remove();
    Circuit circuits(selectedCustomer(), "");
    circuits.remove();
    MTRecord inspections("inspections", "date", "", MTDictionary("customer", selectedCustomer()));
    inspections.remove();
    selected_customer = -1;
    selected_circuit = -1;
    selected_inspection.clear();
    enableTools();
    this->setWindowModified(true);
    navigation->setView(Navigation::ListOfCustomers);
}

void MainWindow::loadCustomer(int customer, bool refresh)
{
    if (customer < 0) { return; }
    selected_customer = customer;
    selected_customer_company = Customer(QString::number(customer)).stringValue("company");
    enableTools();
    if (refresh) {
        navigation->setView(Navigation::ListOfCircuits);
    }
}

void MainWindow::addCircuit()
{
    if (!db.isOpen()) { return; }
    if (!isCustomerSelected()) { return; }
    Circuit record(selectedCustomer(), "");
    ModifyDialogue * md = new ModifyDialogue(&record, this);
    if (md->exec() == QDialog::Accepted) {
        this->setWindowModified(true);
        refreshView();
    }
    delete md;
}

void MainWindow::modifyCircuit()
{
    if (!db.isOpen()) { return; }
    if (!isCustomerSelected()) { return; }
    if (!isCircuitSelected()) { return; }
    Circuit record(selectedCustomer(), selectedCircuit());
    ModifyDialogue * md = new ModifyDialogue(&record, this);
    QString old_id = selectedCircuit();
    if (md->exec() == QDialog::Accepted) {
        if (old_id != record.id()) {
            QSqlQuery update_inspections;
            update_inspections.prepare("UPDATE inspections SET circuit = :new_id WHERE customer = :customer_id AND circuit = :old_id");
            update_inspections.bindValue(":customer_id", selected_customer);
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
    if (!isCustomerSelected()) { return; }
    if (!isCircuitSelected()) { return; }
    bool ok;
    QString confirmation = QInputDialog::getText(this, tr("Remove circuit - Leaklog"), tr("Are you sure you want to remove the selected circuit?\nTo remove all data about the circuit \"%1\" type REMOVE and confirm:").arg(selectedCircuit()), QLineEdit::Normal, "", &ok);
    if (!ok || confirmation != tr("REMOVE")) { return; }
    Circuit record(selectedCustomer(), selectedCircuit());
    record.remove();
    Inspection inspections(selectedCustomer(), selectedCircuit(), "");
    inspections.remove();
    selected_circuit = -1;
    selected_inspection.clear();
    enableTools();
    this->setWindowModified(true);
    navigation->setView(Navigation::ListOfCircuits);
}

void MainWindow::loadCircuit(int circuit, bool refresh)
{
    if (!isCustomerSelected()) { return; }
    if (circuit < 0) { return; }
    selected_circuit = circuit;
    enableTools();
    if (refresh) {
        navigation->setView(Navigation::ListOfInspections);
    }
}

void MainWindow::addInspection()
{
    if (!db.isOpen()) { return; }
    if (!isCustomerSelected()) { return; }
    if (!isCircuitSelected()) { return; }
    Inspection record(selectedCustomer(), selectedCircuit(), "");
    ModifyDialogue * md = new ModifyDialogue(&record, this);
    if (md->exec() == QDialog::Accepted) {
        this->setWindowModified(true);
        refreshView();
    }
    delete md;
}

void MainWindow::modifyInspection()
{
    if (!db.isOpen()) { return; }
    if (!isCustomerSelected()) { return; }
    if (!isCircuitSelected()) { return; }
    if (!isInspectionSelected()) { return; }
    Inspection record(selectedCustomer(), selectedCircuit(), selectedInspection());
    ModifyDialogue * md = new ModifyDialogue(&record, this);
    if (md->exec() == QDialog::Accepted) {
        enableTools();
        this->setWindowModified(true);
        refreshView();
    }
    delete md;
}

void MainWindow::removeInspection()
{
    if (!db.isOpen()) { return; }
    if (!isCustomerSelected()) { return; }
    if (!isCircuitSelected()) { return; }
    if (!isInspectionSelected()) { return; }
    bool ok;
    QString confirmation = QInputDialog::getText(this, tr("Remove inspection - Leaklog"), tr("Are you sure you want to remove the selected inspection?\nTo remove all data about the inspection \"%1\" type REMOVE and confirm:").arg(selectedInspection()), QLineEdit::Normal, "", &ok);
    if (!ok || confirmation != tr("REMOVE")) { return; }
    Inspection record(selectedCustomer(), selectedCircuit(), selectedInspection());
    record.remove();
    selected_inspection.clear();
    enableTools();
    this->setWindowModified(true);
    navigation->setView(Navigation::ListOfInspections);
}

void MainWindow::loadInspection(const QString & inspection, bool refresh)
{
    if (!isCustomerSelected()) { return; }
    if (!isCircuitSelected()) { return; }
    if (inspection.isEmpty()) { return; }
    selected_inspection = inspection;
    selected_inspection_is_repair = Inspection(selectedCustomer(), selectedCircuit(), selectedInspection()).value("repair").toBool();
    enableTools();
    if (refresh) {
        navigation->setView(Navigation::Inspection);
    }
}

void MainWindow::addRepair()
{
    if (!db.isOpen()) { return; }
    Repair record("");
    if (isCustomerSelected()) {
        record.parents().insert("customer", Customer(selectedCustomer()).stringValue("company"));
    }
    ModifyDialogue * md = new ModifyDialogue(&record, this);
    if (md->exec() == QDialog::Accepted) {
        this->setWindowModified(true);
        refreshView();
    }
    delete md;
}

void MainWindow::modifyRepair()
{
    if (!db.isOpen()) { return; }
    if (!isRepairSelected()) { return; }
    Repair record(selectedRepair());
    ModifyDialogue * md = new ModifyDialogue(&record, this);
    if (md->exec() == QDialog::Accepted) {
        this->setWindowModified(true);
        refreshView();
    }
    delete md;
}

void MainWindow::removeRepair()
{
    if (!db.isOpen()) { return; }
    if (!isRepairSelected()) { return; }
    QString repair = selectedRepair();
    bool ok;
    QString confirmation = QInputDialog::getText(this, tr("Remove repair - Leaklog"), tr("Are you sure you want to remove the selected repair?\nTo remove all data about the repair \"%1\" type REMOVE and confirm:").arg(repair), QLineEdit::Normal, "", &ok);
    if (!ok || confirmation != tr("REMOVE")) { return; }
    Repair record(repair);
    record.remove();
    selected_repair.clear();
    enableTools();
    this->setWindowModified(true);
    navigation->setView(Navigation::ListOfRepairs);
}

void MainWindow::loadRepair(const QString & date, bool refresh)
{
    if (date.isEmpty()) { return; }
    selected_repair = date;
    enableTools();
    if (refresh) {
        if (actionBasic_logbook->isChecked() && navigation->view() == Navigation::ListOfCircuits)
            refreshView();
        else
            navigation->setView(Navigation::ListOfRepairs);
    }
}

void MainWindow::addVariable() { addVariable(false); }

void MainWindow::addSubvariable() { addVariable(true); }

void MainWindow::addVariable(bool subvar)
{
    if (!db.isOpen()) { return; }
    QString parent;
    if (subvar) {
        if (trw_variables->currentItem()->parent() != NULL) { return; }
        parent = trw_variables->currentItem()->text(1);
    }
    VariableRecord record((VariableRecord::Type)subvar, parent);
    ModifyDialogue * md = new ModifyDialogue(&record, this);
    if (md->exec() == QDialog::Accepted) {
        StringVariantMap attributes = record.list("name, unit, tolerance");
        QTreeWidgetItem * item = NULL;
        if (subvar) {
            item = new QTreeWidgetItem(trw_variables->currentItem());
        } else {
            item = new QTreeWidgetItem(trw_variables);
        }
        item->setText(0, attributes.value("name").toString());
        item->setText(1, record.id());
        item->setText(2, attributes.value("unit").toString());
        item->setText(3, attributes.value("tolerance").toString());
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
    QString parent;
    if (subvar) { parent = item->parent()->text(1); }
    QString id = item->text(1);
    VariableRecord record((VariableRecord::Type)subvar, subvar ? parent : id, subvar ? id : "");
    ModifyDialogue * md = new ModifyDialogue(&record, this);
    if (md->exec() == QDialog::Accepted) {
        StringVariantMap attributes = record.list("name, unit, tolerance");
        item->setText(0, attributes.value("name").toString());
        item->setText(1, record.id());
        item->setText(2, attributes.value("unit").toString());
        item->setText(3, attributes.value("tolerance").toString());
        if (id != record.id()) {
            renameColumn(id, record.id(), "inspections", &db);
            parsed_expressions.clear();
            if (!subvar) {
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
        MTRecord subvars("subvariables", "id", "", MTDictionary("parent", id));
        subvars.remove();
    }
    MTRecord record(subvar ? "subvariables" : "variables", "id", id, parents);
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
    Table record("");
    ModifyDialogue * md = new ModifyDialogue(&record, this);
    if (md->exec() == QDialog::Accepted) {
        navigation->tableComboBox()->addItem(record.id());
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
    Table record(cb_table_edit->currentText());
    ModifyDialogue * md = new ModifyDialogue(&record, this);
    if (md->exec() == QDialog::Accepted) {
        int i = cb_table_edit->currentIndex();
        int j = navigation->tableComboBox()->currentIndex();
        cb_table_edit->removeItem(i);
        navigation->tableComboBox()->removeItem(i);
        cb_table_edit->insertItem(i, record.id());
        navigation->tableComboBox()->insertItem(i, record.id());
        cb_table_edit->setCurrentIndex(i);
        navigation->tableComboBox()->setCurrentIndex(j);
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
    Table record(cb_table_edit->currentText());
    record.remove();
    int i = cb_table_edit->currentIndex();
    cb_table_edit->removeItem(i);
    navigation->tableComboBox()->removeItem(i);
    enableTools();
    this->setWindowModified(true);
    refreshView();
}

void MainWindow::loadTable(const QString &)
{
    if (!db.isOpen()) { return; }
    if (cb_table_edit->currentIndex() < 0) { enableTools(); return; }
    navigation->tableComboBox()->setCurrentIndex(cb_table_edit->currentIndex());
    trw_table_variables->clear();
    Table record(cb_table_edit->currentText());
    StringVariantMap attributes = record.list("variables, sum, avg");
    QStringList variables = attributes.value("variables").toString().split(";", QString::SkipEmptyParts);
    QStringList sum = attributes.value("sum").toString().split(";", QString::SkipEmptyParts);
    QStringList avg = attributes.value("avg").toString().split(";", QString::SkipEmptyParts);
    for (int i = 0; i < variables.count(); ++i) {
        Variable variable(variables.at(i));
        QTreeWidgetItem * item = new QTreeWidgetItem(trw_table_variables);
        if (variable.next()) { item->setText(0, variable.value("VAR_NAME").toString()); }
        item->setText(1, variables.at(i));
        QComboBox * cb_foot = new QComboBox;
        cb_foot->addItem(tr("None"));
        cb_foot->addItem(tr("Sum"));
        cb_foot->addItem(tr("Average"));
        if (sum.contains(variables.at(i))) { cb_foot->setCurrentIndex(1); }
        else if (avg.contains(variables.at(i))) { cb_foot->setCurrentIndex(2); }
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
    Table record(cb_table_edit->currentText());
    QStringList variables, sum, avg; QString value;
    for (int i = 0; i < trw_table_variables->topLevelItemCount(); ++i) {
        variables << trw_table_variables->topLevelItem(i)->text(1);
        QString value = ((QComboBox *)trw_table_variables->itemWidget(trw_table_variables->topLevelItem(i), 2))->currentText();
        if (value == tr("Sum")) { sum << trw_table_variables->topLevelItem(i)->text(1); }
        else if (value == tr("Average")) { avg << trw_table_variables->topLevelItem(i)->text(1); }
    }
    StringVariantMap set;
    set.insert("variables", variables.join(";"));
    set.insert("sum", sum.join(";"));
    set.insert("avg", avg.join(";"));
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
    Variable query;
    QString id, name; QStringList ids;
    while (query.next()) {
        id = query.value("VAR_ID").toString();
        if (ids.contains(id)) { continue; }
        ids << id;
        name = query.value("VAR_NAME").toString();
        if (!used_ids.contains(id)) {
            QListWidgetItem * item = new QListWidgetItem;
            item->setText(name.isEmpty() ? id : tr("%1 (%2)").arg(id).arg(name));
            item->setData(Qt::UserRole, id);
            lw->addItem(item);
        }
    }
    if (d->exec() == QDialog::Accepted && lw->currentIndex().isValid()) {
        Table record(cb_table_edit->currentText());
        QStringList variables = record.stringValue("variables").split(";", QString::SkipEmptyParts);
        variables << lw->currentItem()->data(Qt::UserRole).toString();
        StringVariantMap set;
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
    Table record(cb_table_edit->currentText());
    StringVariantMap attributes = record.list("variables, sum");
    QStringList variables = attributes.value("variables").toString().split(";", QString::SkipEmptyParts);
    QStringList sum = attributes.value("sum").toString().split(";", QString::SkipEmptyParts);
    QStringList avg = attributes.value("avg").toString().split(";", QString::SkipEmptyParts);
    variables.removeAll(item->text(1));
    sum.removeAll(item->text(1));
    StringVariantMap set;
    set.insert("variables", variables.join(";"));
    set.insert("sum", sum.join(";"));
    set.insert("avg", avg.join(";"));
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
    Table record(cb_table_edit->currentText());
    QStringList variables = record.stringValue("variables").split(";", QString::SkipEmptyParts);
    QString variable = variables.takeAt(i);
    if (up) {
        if (i != 0) { i--; } else { i = variables.count(); }
    } else {
        if (i != variables.count()) { i++; } else { i = 0; }
    }
    variables.insert(i, variable);
    StringVariantMap set;
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
    WarningRecord record("");
    ModifyWarningDialogue * md = new ModifyWarningDialogue(&record, this);
    if (md->exec() == QDialog::Accepted) {
        StringVariantMap attributes = record.list("name, description");
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
    WarningRecord record(item->data(Qt::UserRole).toString());
    ModifyWarningDialogue * md = new ModifyWarningDialogue(&record, this);
    if (md->exec() == QDialog::Accepted) {
        StringVariantMap attributes = record.list("name, description");
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
    WarningRecord record(item->data(Qt::UserRole).toString());
    record.remove();
    MTRecord filters("warnings_filters", "", "", MTDictionary("parent", item->data(Qt::UserRole).toString()));
    filters.remove();
    MTRecord conditions("warnings_conditions", "", "", MTDictionary("parent", item->data(Qt::UserRole).toString()));
    conditions.remove();
    delete item;
    enableTools();
    this->setWindowModified(true);
    refreshView();
}

void MainWindow::addInspector()
{
    if (!db.isOpen()) { return; }
    Inspector record("");
    ModifyDialogue * md = new ModifyDialogue(&record, this);
    if (md->exec() == QDialog::Accepted) {
        this->setWindowModified(true);
        refreshView();
    }
    delete md;
}

void MainWindow::modifyInspector()
{
    if (!db.isOpen()) { return; }
    if (!isInspectorSelected()) { return; }
    QString old_id = selectedInspector();
    Inspector record(old_id);
    ModifyDialogue * md = new ModifyDialogue(&record, this);
    if (md->exec() == QDialog::Accepted) {        
        if (old_id != record.id()) {
            QSqlQuery update_inspections;
            update_inspections.prepare("UPDATE inspections SET inspector = :new_id WHERE inspector = :old_id");
            update_inspections.bindValue(":old_id", old_id);
            update_inspections.bindValue(":new_id", record.id());
            update_inspections.exec();
            QSqlQuery update_repairs;
            update_repairs.prepare("UPDATE repairs SET repairman = :new_id WHERE repairman = :old_id");
            update_repairs.bindValue(":old_id", old_id);
            update_repairs.bindValue(":new_id", record.id());
            update_repairs.exec();
        }
        this->setWindowModified(true);
        refreshView();
    }
    delete md;
}

void MainWindow::removeInspector()
{
    if (!db.isOpen()) { return; }
    if (!isInspectorSelected()) { return; }
    bool ok;
    QString confirmation = QInputDialog::getText(this, tr("Remove inspector - Leaklog"), tr("Are you sure you want to remove the selected inspector?\nTo remove all data about the inspector \"%1\" type REMOVE and confirm:").arg(selectedInspector()), QLineEdit::Normal, "", &ok);
    if (!ok || confirmation != tr("REMOVE")) { return; }
    Inspector record(selectedInspector());
    record.remove();
    selected_inspector = -1;
    enableTools();
    this->setWindowModified(true);
    navigation->setView(Navigation::ListOfInspectors);
}

void MainWindow::loadInspector(int inspector, bool refresh)
{
    if (inspector < 0) { return; }
    selected_inspector = inspector;
    selected_inspector_name = Inspector(selectedInspector()).stringValue("person");
    enableTools();
    if (refresh) {
        navigation->setView(Navigation::ListOfInspectors);
    }
}

void MainWindow::exportCustomerData()
{
    exportData("customer");
}

void MainWindow::exportCircuitData()
{
    if (!isCircuitSelected()) { return; }
    exportData("circuit");
}

void MainWindow::exportInspectionData()
{
    if (!isCircuitSelected()) { return; }
    if (!isInspectionSelected()) { return; }
    exportData("inspection");
}

void MainWindow::exportData(const QString & type)
{
    if (!db.isOpen()) { return; }
    if (!isCustomerSelected()) { return; }
    QString title;
    if (type == "customer") { title = tr("Export customer data - Leaklog"); }
    else if (type == "circuit") { title = tr("Export circuit data - Leaklog"); }
    else if (type == "inspection") { title = tr("Export inspection data - Leaklog"); }
    else { title = tr("Export data - Leaklog"); }
    QString path = QFileDialog::getSaveFileName(this, title, tr("untitled.lklg"), tr("Leaklog Database (*.lklg)"));
	if (path.isEmpty()) { return; }
    if (!path.endsWith(".lklg", Qt::CaseInsensitive)) { path.append(".lklg"); }
    QFile file(path); if (file.exists()) { file.remove(); }
    { // BEGIN EXPORT (SCOPE)
        QSqlDatabase data = QSqlDatabase::addDatabase("QSQLITE", "exportData");
        data.setDatabaseName(path);
        if (!data.open()) {
            QMessageBox::critical(this, title, tr("Cannot write file %1:\n%2.").arg(path).arg(data.lastError().text()));
            return;
        }
        initDatabase(&data);
        data.transaction();
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
        data.commit();
        data.close();
    } // END EXPORT (SCOPE)
    QSqlDatabase::removeDatabase("exportData");
}

void MainWindow::importData()
{
    if (!db.isOpen()) { return; }
    QString path = QFileDialog::getOpenFileName(this, tr("Import data - Leaklog"), "", tr("Leaklog Databases (*.lklg);;All files (*.*)"));
    if (path.isEmpty()) { return; }
{ // BEGIN IMPORT (SCOPE)
    QSqlDatabase data = QSqlDatabase::addDatabase("QSQLITE", "importData");
    data.setDatabaseName(path);
    if (!data.open()) {
        QMessageBox::critical(this, tr("Import data - Leaklog"), tr("Cannot read file %1:\n%2.").arg(path).arg(data.lastError().text()));
        return;
    }
    data.transaction();
    QSqlQuery query(data);
    initDatabase(&data, false);
    ImportDialogue * id = new ImportDialogue(this);
    QString id_justified;
    query.exec("SELECT id, company FROM customers");
    while (query.next()) {
        id_justified = query.value(0).toString().rightJustified(8, '0');
        QListWidgetItem * item = new QListWidgetItem(id->customers());
        item->setCheckState(Qt::Unchecked);
        item->setText(query.value(1).toString().isEmpty() ? id_justified : tr("%1 (%2)").arg(id_justified).arg(query.value(1).toString()));
        item->setData(Qt::UserRole, query.value(0).toString());
    }
    query.exec("SELECT id, parent FROM circuits");
    while (query.next()) {
        QListWidgetItem * item = new QListWidgetItem(id->circuits());
        item->setCheckState(Qt::Unchecked);
        item->setHidden(true);
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable);
        item->setText(QString("%1 (%2)").arg(query.value(0).toString().rightJustified(4, '0')).arg(query.value(1).toString().rightJustified(8, '0')));
        item->setData(Qt::UserRole, QString("%1::%2").arg(query.value(1).toString()).arg(query.value(0).toString()));
    }
    query.exec("SELECT date, customer, circuit FROM inspections");
    while (query.next()) {
        QListWidgetItem * item = new QListWidgetItem(id->inspections());
        item->setCheckState(Qt::Checked);
        item->setHidden(true);
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable);
        item->setText(QString("%1 (%2::%3)").arg(query.value(0).toString()).arg(query.value(1).toString().rightJustified(8, '0')).arg(query.value(2).toString().rightJustified(4, '0')));
        item->setData(Qt::UserRole, QString("%1::%2::%3").arg(query.value(1).toString()).arg(query.value(2).toString()).arg(query.value(0).toString()));
    }
    Variables variables(data);
    QString last_id; QTreeWidgetItem * last_item = NULL;
    while (variables.next()) {
        if (variables.value("VAR_ID").toString() != last_id) {
            last_item = new QTreeWidgetItem(id->variables());
            last_item->setText(0, variables.value("VAR_NAME").toString());
            last_item->setText(1, variables.value("VAR_ID").toString());
            last_item->setText(2, variables.value("VAR_UNIT").toString());
            last_item->setText(3, dict_vartypes.value(variables.value("VAR_TYPE").toString()));
            last_item->setText(4, variables.value("VAR_VALUE").toString());
            last_item->setText(5, variables.value("VAR_COMPARE_NOM").toInt() ? tr("Yes") : tr("No"));
            last_item->setText(6, variables.value("VAR_TOLERANCE").toString());
            last_item->setText(7, variables.value("VAR_COL_BG").toString());
            Variable variable(variables.value("VAR_ID").toString());
            bool found = false, overwrite = false;
            if (variable.next()) {
                found = true;
                if (variable.value("VAR_NAME").toString() != variables.value("VAR_NAME").toString()) { overwrite = true; last_item->setBackground(0, QBrush(Qt::darkMagenta)); last_item->setForeground(0, QBrush(Qt::white)); }
                if (variable.value("VAR_UNIT").toString() != variables.value("VAR_UNIT").toString()) { overwrite = true; last_item->setBackground(2, QBrush(Qt::darkMagenta)); last_item->setForeground(2, QBrush(Qt::white)); }
                if (variable.value("VAR_TYPE").toString() != variables.value("VAR_TYPE").toString()) { overwrite = true; last_item->setBackground(3, QBrush(Qt::darkMagenta)); last_item->setForeground(3, QBrush(Qt::white)); }
                if (variable.value("VAR_VALUE").toString() != variables.value("VAR_VALUE").toString()) { overwrite = true; last_item->setBackground(4, QBrush(Qt::darkMagenta)); last_item->setForeground(4, QBrush(Qt::white)); }
                if ((variable.value("VAR_COMPARE_NOM").toInt() > 0) != (variables.value("VAR_COMPARE_NOM").toInt() > 0)) { overwrite = true; last_item->setBackground(5, QBrush(Qt::darkMagenta)); last_item->setForeground(5, QBrush(Qt::white)); }
                if (variable.value("VAR_TOLERANCE").toString() != variables.value("VAR_TOLERANCE").toString()) { overwrite = true; last_item->setBackground(6, QBrush(Qt::darkMagenta)); last_item->setForeground(6, QBrush(Qt::white)); }
                if (variable.value("VAR_COL_BG").toString() != variables.value("VAR_COL_BG").toString()) { overwrite = true; last_item->setBackground(7, QBrush(Qt::darkMagenta)); last_item->setForeground(7, QBrush(Qt::white)); }
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
            if (!dict_varnames.contains(variables.value("VAR_ID").toString())) {
                cb_action->insertItem(0, tr("Do not import"));
                cb_action->setCurrentIndex(1);
            } else {
                cb_action->setCurrentIndex(0);
            }
            id->variables()->setItemWidget(last_item, 8, cb_action);
            last_id = variables.value("VAR_ID").toString();
        }
        if (!variables.value("SUBVAR_ID").toString().isEmpty() && last_item) {
            QTreeWidgetItem * subitem = new QTreeWidgetItem(last_item);
            subitem->setText(0, variables.value("SUBVAR_NAME").toString());
            subitem->setText(1, variables.value("SUBVAR_ID").toString());
            subitem->setText(2, variables.value("SUBVAR_UNIT").toString());
            subitem->setText(3, dict_vartypes.value(variables.value("SUBVAR_TYPE").toString()));
            subitem->setText(4, variables.value("SUBVAR_VALUE").toString());
            subitem->setText(5, variables.value("SUBVAR_COMPARE_NOM").toInt() ? tr("Yes") : tr("No"));
            subitem->setText(6, variables.value("SUBVAR_TOLERANCE").toString());
            Subvariable variable(variables.value("VAR_ID").toString(), variables.value("SUBVAR_ID").toString());
            bool found = false, overwrite = false;
            if (variable.next()) {
                found = true;
                if (variable.value("SUBVAR_NAME").toString() != variables.value("SUBVAR_NAME").toString()) { overwrite = true; subitem->setBackground(0, QBrush(Qt::darkMagenta)); subitem->setForeground(0, QBrush(Qt::white)); }
                if (variable.value("SUBVAR_UNIT").toString() != variables.value("SUBVAR_UNIT").toString()) { overwrite = true; subitem->setBackground(2, QBrush(Qt::darkMagenta)); subitem->setForeground(2, QBrush(Qt::white)); }
                if (variable.value("SUBVAR_TYPE").toString() != variables.value("SUBVAR_TYPE").toString()) { overwrite = true; subitem->setBackground(3, QBrush(Qt::darkMagenta)); subitem->setForeground(3, QBrush(Qt::white)); }
                if (variable.value("SUBVAR_VALUE").toString() != variables.value("SUBVAR_VALUE").toString()) { overwrite = true; subitem->setBackground(4, QBrush(Qt::darkMagenta)); subitem->setForeground(4, QBrush(Qt::white)); }
                if ((variable.value("SUBVAR_COMPARE_NOM").toInt() > 0) != (variables.value("SUBVAR_COMPARE_NOM").toInt() > 0)) { overwrite = true; subitem->setBackground(5, QBrush(Qt::darkMagenta)); subitem->setForeground(5, QBrush(Qt::white)); }
                if (variable.value("SUBVAR_TOLERANCE").toString() != variables.value("SUBVAR_TOLERANCE").toString()) { overwrite = true; subitem->setBackground(6, QBrush(Qt::darkMagenta)); subitem->setForeground(6, QBrush(Qt::white)); }
                if (variable.value("VAR_ID").toString() != variables.value("VAR_ID").toString()) { overwrite = true; last_item->setBackground(1, QBrush(Qt::darkMagenta)); subitem->setForeground(1, QBrush(Qt::white)); }
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
            if (!dict_varnames.contains(variables.value("SUBVAR_ID").toString())) {
                cb_action->insertItem(0, tr("Do not import"));
                cb_action->setCurrentIndex(1);
            } else {
                cb_action->setCurrentIndex(0);
            }
            id->variables()->setItemWidget(subitem, 8, cb_action);
            last_item->setExpanded(true);
        }
    }
if (id->exec() == QDialog::Accepted) { // BEGIN IMPORT
    StringVariantMap set;
    for (int c = 0; c < id->customers()->count(); ++c) {
        if (id->customers()->item(c)->checkState() == Qt::Unchecked) { continue; }
        set.clear();
        QString c_id = id->customers()->item(c)->data(Qt::UserRole).toString();
        query.prepare("SELECT * FROM customers WHERE id = :id");
        query.bindValue(":id", c_id);
        query.exec();
        if (query.next()) {
            for (int f = 0; f < query.record().count(); ++f) {
                set.insert(query.record().fieldName(f), query.value(f));
            }
        }
        Customer(c_id).update(set);
    }
    for (int cc = 0; cc < id->circuits()->count(); ++cc) {
        if (id->circuits()->item(cc)->checkState() == Qt::Unchecked) { continue; }
        set.clear();
        QString cc_parent = id->circuits()->item(cc)->data(Qt::UserRole).toString().split("::").first();
        QString cc_id = id->circuits()->item(cc)->data(Qt::UserRole).toString().split("::").last();
        query.prepare("SELECT * FROM circuits WHERE parent = :parent AND id = :id");
        query.bindValue(":parent", cc_parent);
        query.bindValue(":id", cc_id);
        query.exec();
        if (query.next()) {
            for (int f = 0; f < query.record().count(); ++f) {
                set.insert(query.record().fieldName(f), query.value(f));
            }
            set.remove("parent");
        }
        Circuit(cc_parent, cc_id).update(set);
    }
    QStringList inspections_skip_columns; bool skip_parent = false;
    QString current_text; QTreeWidgetItem * item = NULL; QTreeWidgetItem * subitem = NULL;
    QTreeWidgetItem * new_item = NULL;
    for (int v = 0; v < id->variables()->topLevelItemCount(); ++v) {
        item = id->variables()->topLevelItem(v);
        skip_parent = false; new_item = NULL;
        current_text = ((QComboBox *)id->variables()->itemWidget(item, 8))->currentText();
        if (current_text == tr("Do not import")) {
            inspections_skip_columns << item->text(1);
            skip_parent = true;
        } else if (current_text == tr("Import") || current_text == tr("Overwrite and import")) {
            VariableRecord record(VariableRecord::VARIABLE, item->text(1));
            Variable variable(item->text(1));
            if (!variable.next()) {
                new_item = new QTreeWidgetItem(trw_variables);
                new_item->setText(0, item->text(0));
                new_item->setText(1, item->text(1));
                new_item->setText(2, item->text(2));
                new_item->setText(3, item->text(6));
            }
            set.clear();
            set.insert("name", item->text(0));
            set.insert("id", item->text(1));
            set.insert("unit", item->text(2));
            set.insert("type", dict_vartypes.firstKey(item->text(3)));
            set.insert("value", item->text(4));
            set.insert("compare_nom", item->text(5) == tr("Yes") ? 1 : 0);
            set.insert("tolerance", item->text(6));
            set.insert("col_bg", item->text(7));
            record.update(set);
        }
        for (int sv = 0; sv < item->childCount(); ++sv) {
            subitem = item->child(sv);
            current_text = ((QComboBox *)id->variables()->itemWidget(subitem, 8))->currentText();
            if (skip_parent || current_text == tr("Do not import")) {
                inspections_skip_columns << subitem->text(1);
            } else if (current_text == tr("Import") || current_text == tr("Overwrite and import")) {
                VariableRecord record(VariableRecord::SUBVARIABLE, item->text(1), subitem->text(1));
                Subvariable subvariable(item->text(1), subitem->text(1));
                if (new_item != NULL && !subvariable.next()) {
                    QTreeWidgetItem * new_subitem = new QTreeWidgetItem(new_item);
                    new_subitem->setText(0, subitem->text(0));
                    new_subitem->setText(1, subitem->text(1));
                    new_subitem->setText(2, subitem->text(2));
                    new_subitem->setText(3, subitem->text(6));
                }
                set.clear();
                set.insert("name", subitem->text(0));
                set.insert("id", subitem->text(1));
                set.insert("unit", subitem->text(2));
                set.insert("type", dict_vartypes.firstKey(subitem->text(3)));
                set.insert("value", subitem->text(4));
                set.insert("compare_nom", subitem->text(5) == tr("Yes") ? 1 : 0);
                set.insert("tolerance", subitem->text(6));
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
        query.prepare("SELECT * FROM inspections WHERE customer = :customer AND circuit = :circuit AND date = :date");
        query.bindValue(":customer", i_customer);
        query.bindValue(":circuit", i_circuit);
        query.bindValue(":date", i_date);
        query.exec();
        if (query.next()) {
            for (int f = 0; f < query.record().count(); ++f) {
                if (!inspections_skip_columns.contains(query.record().fieldName(f))) {
                    set.insert(query.record().fieldName(f), query.value(f));
                }
            }
        }
        Inspection(i_customer, i_circuit, i_date).update(set, j == 0);
        j++;
    }
} // END IMPORT
    data.rollback();
    data.close();
} // END IMPORT (SCOPE)
    QSqlDatabase::removeDatabase("importData");
    this->setWindowModified(true);
    refreshView();
}
