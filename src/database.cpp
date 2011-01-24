/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2010 Matus & Michal Tomlein

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
#include "import_csv_dialogue.h"
#include "records.h"
#include "mtlistwidget.h"
#include "mtaddress.h"
#include "mtvariant.h"

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
    for (int i = 0; i < databaseTables().count(); ++i) {
        if (!tables.contains(databaseTables().key(i))) {
            query.exec("CREATE TABLE " + databaseTables().key(i) + " (" + databaseTables().value(i) + ")");
            if (databaseTables().key(i) == "inspections") {
                QString type; bool ok = true;
                for (int v = 0; v < variableNames().count(); ++v) {
                    type = variableTypeToSqlType(variableType(variableNames().key(v), &ok));
                    if (ok)
                        addColumn(variableNames().key(v) + " " + type, "inspections", database);
                }
            }
        } else {
            MTDictionary field_names = getTableFieldNames(databaseTables().key(i), database);
            QStringList all_field_names = databaseTables().value(i).split(", ");
            if (databaseTables().key(i) == "inspections") {
                QString type; bool ok = true;
                for (int v = 0; v < variableNames().count(); ++v) {
                    type = variableTypeToSqlType(variableType(variableNames().key(v), &ok));
                    if (ok)
                        all_field_names << variableNames().key(v) + " " + variableTypeToSqlType(variableType(variableNames().key(v)));
                }
            }
            for (int f = 0; f < all_field_names.count(); ++f) {
                if (!field_names.contains(all_field_names.at(f).split(" ").first())) {
                    addColumn(all_field_names.at(f), databaseTables().key(i), database);
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
    if (v > 0.902 && v < 0.906) {
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
    if (v < 0.9061) {
        Customer customers("");
        MultiMapOfVariantMaps customer_ids = customers.mapAll("company", "id");
        QVariantMap set;
        QSqlQuery repairs("SELECT date, customer FROM repairs WHERE parent IS NULL");
        while (repairs.next()) {
            if (customer_ids.contains(repairs.value(1).toString())) {
                Repair repair(repairs.value(0).toString());
                set.insert("parent", customer_ids.value(repairs.value(1).toString()).value("id"));
                repair.update(set);
            }
        }
        query.exec("UPDATE inspections SET nominal = 0 WHERE nominal IS NULL");
        query.exec("UPDATE inspections SET repair = 0 WHERE repair IS NULL");
    }
    if (v < 0.907) {
        query.exec("UPDATE inspections SET outside_interval = repair");
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
    QVariantMap set;
    Table leakages("", "Leakages");
    if (!leakages.exists()) {
        set.insert("id", tr("Leakages"));
        set.insert("highlight_nominal", 0);
        set.insert("variables", "vis_aur_chk;dir_leak_chk;refr_add_am;refr_add_per;refr_reco;inspector;operator;rmds;arno");
        set.insert("sum", "vis_aur_chk;refr_add_am;refr_add_per;refr_reco");
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
        set.clear();
        QStringList variables = pressures_and_temperatures.stringValue("variables").split(";", QString::SkipEmptyParts);
        if (variables.contains("t")) {
            variables.replace(variables.indexOf("t"), "t_sec");
            set.insert("variables", variables.join(";"));
        } else if (!variables.contains("t_sec")) {
            variables.prepend("t_sec");
            set.insert("variables", variables.join(";"));
        }
        if (!set.isEmpty())
            pressures_and_temperatures.update(set);

        set.clear();
        QVariantMap table_attributes = leakages.list("variables, sum");
        variables = table_attributes.value("variables").toString().split(";", QString::SkipEmptyParts);
        QStringList sum = table_attributes.value("sum").toString().split(";", QString::SkipEmptyParts);
        if (variables.contains("refr_add")) {
            variables.replace(variables.indexOf("refr_add"), "refr_add_am;refr_add_per");
            set.insert("variables", variables.join(";"));
        }
        if (sum.contains("refr_add")) {
            sum.replace(sum.indexOf("refr_add"), "refr_add_am;refr_add_per");
            set.insert("sum", sum.join(";"));
        }
        if (variables.contains("refr_recovery")) {
            variables.replace(variables.indexOf("refr_recovery"), "refr_reco");
            set.insert("variables", variables.join(";"));
        }
        if (sum.contains("refr_recovery")) {
            sum.replace(sum.indexOf("refr_recovery"), "refr_reco");
            set.insert("sum", sum.join(";"));
        }
        if (!set.isEmpty())
            leakages.update(set);
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
        QVariantMap attributes = record.list();
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
        loadCustomer(record.id().toInt(), true);
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
    QString old_company_name = record.stringValue("company");
    if (md->exec() == QDialog::Accepted) {
        this->setWindowModified(true);
        QString company_name = record.stringValue("company");
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
            QSqlQuery update_repairs;
            update_repairs.prepare("UPDATE repairs SET parent = :new_id, customer = :customer WHERE parent = :old_id");
            update_repairs.bindValue(":old_id", old_id);
            update_repairs.bindValue(":new_id", record.id());
            update_repairs.bindValue(":customer", company_name);
            update_repairs.exec();
            loadCustomer(record.id().toInt(), true);
        } else if (old_company_name != company_name) {
            QSqlQuery update_repairs;
            update_repairs.prepare("UPDATE repairs SET customer = :customer WHERE parent = :id");
            update_repairs.bindValue(":id", record.id());
            update_repairs.bindValue(":customer", company_name);
            update_repairs.exec();
            refreshView();
        } else {
            refreshView();
        }
    }
    delete md;
}

void MainWindow::duplicateCustomer()
{
    if (!db.isOpen()) { return; }
    if (!isCustomerSelected()) { return; }
    Customer record(selectedCustomer());
    record.readAttributes();
    record.id().clear();
    ModifyDialogue * md = new ModifyDialogue(&record, this);
    if (md->exec() == QDialog::Accepted) {
        this->setWindowModified(true);
        loadCustomer(record.id().toInt(), true);
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
    MTRecord repairs("repairs", "date", "", MTDictionary("parent", selectedCustomer()));
    repairs.remove();
    selected_customer = -1;
    selected_circuit = -1;
    selected_inspection.clear();
    selected_repair.clear();
    enableTools();
    this->setWindowModified(true);
    navigation->setView(Navigation::ListOfCustomers);
}

void MainWindow::loadCustomer(int customer, bool refresh)
{
    if (customer < 0) { return; }
    selected_customer = customer;
    selected_customer_company = Customer(QString::number(customer)).stringValue("company");
    selected_circuit = -1;
    selected_inspection.clear();
    enableTools();
    if (refresh) {
        if (actionService_company->isChecked())
            navigation->setView(Navigation::ListOfCustomers);
        else if (actionBasic_logbook->isChecked())
            navigation->setView(Navigation::ListOfRepairs);
        else
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
        loadCircuit(record.id().toInt(), true);
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
        this->setWindowModified(true);
        if (old_id != record.id()) {
            QSqlQuery update_inspections;
            update_inspections.prepare("UPDATE inspections SET circuit = :new_id WHERE customer = :customer_id AND circuit = :old_id");
            update_inspections.bindValue(":customer_id", selected_customer);
            update_inspections.bindValue(":old_id", old_id);
            update_inspections.bindValue(":new_id", record.id());
            update_inspections.exec();
            loadCircuit(record.id().toInt(), true);
        } else {
            refreshView();
        }
    }
    delete md;
}

void MainWindow::duplicateCircuit()
{
    if (!db.isOpen()) { return; }
    if (!isCustomerSelected()) { return; }
    if (!isCircuitSelected()) { return; }
    Circuit record(selectedCustomer(), selectedCircuit());
    record.readAttributes();
    record.id().clear();
    ModifyDialogue * md = new ModifyDialogue(&record, this);
    if (md->exec() == QDialog::Accepted) {
        this->setWindowModified(true);
        loadCircuit(record.id().toInt(), true);
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
    selected_inspection.clear();
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
        loadInspection(record.id(), true);
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
        this->setWindowModified(true);
        loadInspection(record.id(), true);
    }
    delete md;
}

void MainWindow::duplicateInspection()
{
    if (!db.isOpen()) { return; }
    if (!isCustomerSelected()) { return; }
    if (!isCircuitSelected()) { return; }
    if (!isInspectionSelected()) { return; }
    Inspection record(selectedCustomer(), selectedCircuit(), selectedInspection());
    record.readAttributes();
    record.id().clear();
    ModifyDialogue * md = new ModifyDialogue(&record, this);
    if (md->exec() == QDialog::Accepted) {
        this->setWindowModified(true);
        loadInspection(record.id(), true);
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
        record.parents().insert("parent", selectedCustomer());
        record.parents().insert("customer", Customer(selectedCustomer()).stringValue("company"));
    }
    ModifyDialogue * md = new ModifyDialogue(&record, this);
    if (md->exec() == QDialog::Accepted) {
        this->setWindowModified(true);
        loadRepair(record.id(), true);
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
        loadRepair(record.id(), true);
    }
    delete md;
}

void MainWindow::duplicateRepair()
{
    if (!db.isOpen()) { return; }
    if (!isRepairSelected()) { return; }
    Repair record(selectedRepair());
    record.readAttributes();
    record.id().clear();
    if (!record.stringValue("parent").isEmpty()) {
        record.parents().insert("parent", record.stringValue("parent"));
    }
    ModifyDialogue * md = new ModifyDialogue(&record, this);
    if (md->exec() == QDialog::Accepted) {
        this->setWindowModified(true);
        loadRepair(record.id(), true);
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
        QVariantMap attributes = record.list("name, unit, tolerance");
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
        QVariantMap attributes = record.list("name, unit, tolerance");
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
    if (variableNames().contains(item->text(1))) { return; }
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
    QVariantMap attributes = record.list("variables, sum, avg");
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
    QVariantMap set;
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
        QVariantMap set;
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
    QVariantMap attributes = record.list("variables, sum");
    QStringList variables = attributes.value("variables").toString().split(";", QString::SkipEmptyParts);
    QStringList sum = attributes.value("sum").toString().split(";", QString::SkipEmptyParts);
    QStringList avg = attributes.value("avg").toString().split(";", QString::SkipEmptyParts);
    variables.removeAll(item->text(1));
    sum.removeAll(item->text(1));
    QVariantMap set;
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
    QVariantMap set;
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
        QVariantMap attributes = record.list("name, description");
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
        QVariantMap attributes = record.list("name, description");
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
        loadInspector(record.id().toInt(), true);
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
        this->setWindowModified(true);
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
            loadInspector(record.id().toInt(), true);
        } else {
            refreshView();
        }
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
    QVariantMap attributes;
    QVariant attribute;
    bool modified, attribute_modified;
    // Customers
    query.exec("SELECT * FROM customers ORDER BY id");
    while (query.next()) {
        id_justified = QUERY("id").toString().rightJustified(8, '0');
        Customer customer(QUERY("id").toString());
        if (customer.exists()) {
            attributes = customer.list();
            modified = false;
            MTDictionary columns(true);
            columns.insert(id_justified, "0");
            for (int i = 1; i < Customer::attributes().count(); ++i) {
                attribute = QUERY(Customer::attributes().key(i));
                attribute_modified = attribute != attributes.value(Customer::attributes().key(i));
                if (attribute_modified) modified = true;
                columns.insert(MTVariant(attribute, (MTVariant::Type)dict_fieldtypes.value(Customer::attributes().key(i))).toString(),
                    attribute_modified ? "1" : "0");
            }
            if (modified) {
                QTreeWidgetItem * item = new QTreeWidgetItem(id->modifiedCustomers(), columns.keys());
                for (int i = 0; i < columns.count(); ++i) {
                    if (columns.value(i).toInt()) {
                        item->setBackground(i, QBrush(Qt::darkMagenta));
                        item->setForeground(i, QBrush(Qt::white));
                    }
                }
                item->setCheckState(0, QUERY("date_updated").toString() > attributes.value("date_updated").toString() ? Qt::Checked : Qt::Unchecked);
                if (item->checkState(0) == Qt::Unchecked)
                    item->setToolTip(0, tr("This record is older than the current record in your database"));
                item->setData(0, Qt::UserRole, QUERY("id"));
            }
        } else {
            QTreeWidgetItem * item = new QTreeWidgetItem(id->newCustomers());
            item->setText(0, id_justified);
            for (int i = 1; i < Customer::attributes().count(); ++i) {
                item->setText(i, MTVariant(QUERY(Customer::attributes().key(i)),
                                           (MTVariant::Type)dict_fieldtypes.value(Customer::attributes().key(i))).toString());
            }
            item->setCheckState(0, Qt::Checked);
            item->setData(0, Qt::UserRole, QUERY("id"));
        }
    }
    // Circuits
    QSet<int> shown_sections;
    shown_sections << 0 << 1 << 2;
    query.exec("SELECT circuits.*, customers.company FROM circuits LEFT JOIN customers ON circuits.parent = customers.id ORDER BY customers.company, circuits.id");
    while (query.next()) {
        id_justified = QUERY("id").toString().rightJustified(4, '0');
        Circuit circuit(QUERY("parent").toString(), QUERY("id").toString());
        if (circuit.exists()) {
            attributes = circuit.list();
            modified = false;
            MTDictionary columns(true);
            columns.insert(QUERY("company").toString(), "0");
            columns.insert(id_justified, "0");
            for (int i = 1; i < Circuit::attributes().count(); ++i) {
                attribute = QUERY(Circuit::attributes().key(i));
                attribute_modified = attribute != attributes.value(Circuit::attributes().key(i));
                if (attribute_modified) {
                    modified = true;
                    shown_sections << i + 1;
                }
                columns.insert(attribute.toString(), attribute_modified ? "1" : "0");
            }
            if (modified) {
                QTreeWidgetItem * item = new QTreeWidgetItem(id->modifiedCircuits(), columns.keys());
                for (int i = 0; i < columns.count(); ++i) {
                    if (columns.value(i).toInt()) {
                        item->setBackground(i, QBrush(Qt::darkMagenta));
                        item->setForeground(i, QBrush(Qt::white));
                    }
                }
                item->setCheckState(0, QUERY("date_updated").toString() > attributes.value("date_updated").toString() ? Qt::Checked : Qt::Unchecked);
                if (item->checkState(0) == Qt::Unchecked)
                    item->setToolTip(0, tr("This record is older than the current record in your database"));
                item->setData(0, Qt::UserRole, QUERY("parent"));
                item->setData(1, Qt::UserRole, QUERY("id"));
            }
        } else {
            QTreeWidgetItem * item = new QTreeWidgetItem(id->newCircuits());
            item->setText(0, QUERY("company").toString());
            item->setText(1, id_justified);
            for (int i = 1; i < Circuit::attributes().count(); ++i) {
                item->setText(i + 1, QUERY(Circuit::attributes().key(i)).toString());
            }
            item->setCheckState(0, Qt::Checked);
            item->setData(0, Qt::UserRole, QUERY("parent"));
            item->setData(1, Qt::UserRole, QUERY("id"));
        }
    }
    for (int i = 0; i < id->modifiedCircuits()->columnCount(); ++i) {
        if (!shown_sections.contains(i))
            id->modifiedCircuits()->header()->hideSection(i);
    }
    // Variables
    MTDictionary variable_names;
    variable_names.insert("nominal", QApplication::translate("Inspection", "Nominal"));
    variable_names.insert("repair", QApplication::translate("Inspection", "Repair"));
    Variables variables(data);
    QString last_id; QTreeWidgetItem * last_item = NULL;
    while (variables.next()) {
        if (variables.value("VAR_ID").toString() != last_id) {
            if (variables.value("VAR_VALUE").toString().isEmpty())
                variable_names.insert(variables.value("VAR_ID").toString(), variables.value("VAR_NAME").toString());
            last_item = new QTreeWidgetItem(id->variables());
            last_item->setText(0, variables.value("VAR_NAME").toString());
            last_item->setText(1, variables.value("VAR_ID").toString());
            last_item->setText(2, variables.value("VAR_UNIT").toString());
            last_item->setText(3, variableTypes().value(variables.value("VAR_TYPE").toString()));
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
            if (!variableNames().contains(variables.value("VAR_ID").toString())) {
                cb_action->insertItem(0, tr("Do not import"));
                cb_action->setCurrentIndex(1);
            } else {
                cb_action->setCurrentIndex(0);
            }
            id->variables()->setItemWidget(last_item, 8, cb_action);
            last_id = variables.value("VAR_ID").toString();
        }
        if (!variables.value("SUBVAR_ID").toString().isEmpty() && last_item) {
            if (variables.value("SUBVAR_VALUE").toString().isEmpty()) {
                variable_names.remove(variables.value("VAR_ID").toString());
                variable_names.insert(variables.value("SUBVAR_ID").toString(), tr("%1: %2").arg(variables.value("VAR_NAME").toString()).arg(variables.value("SUBVAR_NAME").toString()));
            }
            QTreeWidgetItem * subitem = new QTreeWidgetItem(last_item);
            subitem->setText(0, variables.value("SUBVAR_NAME").toString());
            subitem->setText(1, variables.value("SUBVAR_ID").toString());
            subitem->setText(2, variables.value("SUBVAR_UNIT").toString());
            subitem->setText(3, variableTypes().value(variables.value("SUBVAR_TYPE").toString()));
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
            if (!variableNames().contains(variables.value("SUBVAR_ID").toString())) {
                cb_action->insertItem(0, tr("Do not import"));
                cb_action->setCurrentIndex(1);
            } else {
                cb_action->setCurrentIndex(0);
            }
            id->variables()->setItemWidget(subitem, 8, cb_action);
            last_item->setExpanded(true);
        }
    }
    // Inspections
    bool database_locked = DBInfoValueForKey("locked") == "true";
    QString lock_date = DBInfoValueForKey("lock_date");
    bool record_locked;
    QTreeWidget * trw[] = { id->newInspections(), id->modifiedInspections() };
    for (int w = 0; w < 2; ++w) {
        trw[w]->setColumnCount(variable_names.count() + 3);
        trw[w]->setHeaderItem(new QTreeWidgetItem(QStringList() << tr("Customer") << tr("Circuit") << tr("Date") << variable_names.values()));
    }
    shown_sections.clear();
    shown_sections << 0 << 1 << 2;
    query.exec("SELECT inspections.*, customers.company FROM inspections LEFT JOIN customers ON inspections.customer = customers.id ORDER BY inspections.date");
    while (query.next()) {
        id_justified = QUERY("circuit").toString().rightJustified(4, '0');
        record_locked = database_locked && QUERY("date").toString() < lock_date;
        QTreeWidgetItem * item = NULL;
        Inspection inspection(QUERY("customer").toString(), QUERY("circuit").toString(), QUERY("date").toString());
        if (inspection.exists()) {
            attributes = inspection.list();
            modified = false;
            MTDictionary columns(true);
            columns.insert(QUERY("company").toString(), "0");
            columns.insert(id_justified, "0");
            columns.insert(QUERY("date").toString(), "0");
            for (int i = 0; i < variable_names.count(); ++i) {
                attribute = QUERY(variable_names.key(i));
                attribute_modified = attribute != attributes.value(variable_names.key(i));
                if (attribute_modified) {
                    modified = true;
                    shown_sections << i + 3;
                }
                columns.insert(attribute.toString(), attribute_modified ? "1" : "0");
            }
            if (modified) {
                item = new QTreeWidgetItem(id->modifiedInspections(), columns.keys());
                for (int i = 0; i < columns.count(); ++i) {
                    if (columns.value(i).toInt()) {
                        item->setBackground(i, QBrush(Qt::darkMagenta));
                        item->setForeground(i, QBrush(Qt::white));
                    }
                }
                item->setCheckState(0, QUERY("date_updated").toString() > attributes.value("date_updated").toString() ? Qt::Checked : Qt::Unchecked);
                if (item->checkState(0) == Qt::Unchecked)
                    item->setToolTip(0, tr("This record is older than the current record in your database"));
            }
        } else {
            item = new QTreeWidgetItem(id->newInspections());
            item->setText(0, QUERY("company").toString());
            item->setText(1, id_justified);
            item->setText(2, QUERY("date").toString());
            for (int i = 0; i < variable_names.count(); ++i) {
                item->setText(i + 3, QUERY(variable_names.key(i)).toString());
            }
            item->setCheckState(0, Qt::Checked);
        }
        if (item) {
            if (record_locked) {
                item->setCheckState(0, Qt::Unchecked);
                item->setDisabled(true);
                item->setIcon(2, QIcon(":/images/images/locked16.png"));
            } else {
                item->setData(0, Qt::UserRole, QUERY("customer"));
                item->setData(1, Qt::UserRole, QUERY("circuit"));
                item->setData(2, Qt::UserRole, QUERY("date"));
            }
        }
    }
    for (int i = 0; i < id->modifiedInspections()->columnCount(); ++i) {
        if (!shown_sections.contains(i))
            id->modifiedInspections()->header()->hideSection(i);
    }
    // Repairs
    query.exec("SELECT * FROM repairs ORDER BY date");
    while (query.next()) {
        record_locked = database_locked && QUERY("date").toString() < lock_date;
        QTreeWidgetItem * item = NULL;
        Repair repair(QUERY("date").toString());
        if (repair.exists()) {
            attributes = repair.list();
            modified = false;
            MTDictionary columns(true);
            columns.insert(QUERY("date").toString(), "0");
            for (int i = 1; i < Repair::attributes().count(); ++i) {
                attribute = QUERY(Repair::attributes().key(i));
                attribute_modified = attribute != attributes.value(Repair::attributes().key(i));
                if (attribute_modified) modified = true;
                columns.insert(attribute.toString(), attribute_modified ? "1" : "0");
            }
            if (modified) {
                item = new QTreeWidgetItem(id->modifiedRepairs(), columns.keys());
                for (int i = 0; i < columns.count(); ++i) {
                    if (columns.value(i).toInt()) {
                        item->setBackground(i, QBrush(Qt::darkMagenta));
                        item->setForeground(i, QBrush(Qt::white));
                    }
                }
                item->setCheckState(0, QUERY("date_updated").toString() > attributes.value("date_updated").toString() ? Qt::Checked : Qt::Unchecked);
                if (item->checkState(0) == Qt::Unchecked)
                    item->setToolTip(0, tr("This record is older than the current record in your database"));
            }
        } else {
            item = new QTreeWidgetItem(id->newRepairs());
            item->setText(0, QUERY("date").toString());
            for (int i = 1; i < Repair::attributes().count(); ++i) {
                item->setText(i, QUERY(Repair::attributes().key(i)).toString());
            }
            item->setCheckState(0, Qt::Checked);
        }
        if (item) {
            if (record_locked) {
                item->setCheckState(0, Qt::Unchecked);
                item->setDisabled(true);
                item->setIcon(0, QIcon(":/images/images/locked16.png"));
            } else {
                item->setData(0, Qt::UserRole, QUERY("date"));
            }
        }
    }
    // Refrigerant management
    query.exec("SELECT * FROM refrigerant_management ORDER BY date");
    while (query.next()) {
        record_locked = database_locked && QUERY("date").toString() < lock_date;
        QTreeWidgetItem * item = NULL;
        RecordOfRefrigerantManagement record(QUERY("date").toString());
        if (record.exists()) {
            attributes = record.list();
            modified = false;
            MTDictionary columns(true);
            columns.insert(QUERY("date").toString(), "0");
            for (int i = 1; i < RecordOfRefrigerantManagement::attributes().count(); ++i) {
                attribute = QUERY(RecordOfRefrigerantManagement::attributes().key(i));
                attribute_modified = attribute != attributes.value(RecordOfRefrigerantManagement::attributes().key(i));
                if (attribute_modified) modified = true;
                columns.insert(attribute.toString(), attribute_modified ? "1" : "0");
            }
            if (modified) {
                item = new QTreeWidgetItem(id->modifiedRefrigerantManagement(), columns.keys());
                for (int i = 0; i < columns.count(); ++i) {
                    if (columns.value(i).toInt()) {
                        item->setBackground(i, QBrush(Qt::darkMagenta));
                        item->setForeground(i, QBrush(Qt::white));
                    }
                }
                item->setCheckState(0, QUERY("date_updated").toString() > attributes.value("date_updated").toString() ? Qt::Checked : Qt::Unchecked);
                if (item->checkState(0) == Qt::Unchecked)
                    item->setToolTip(0, tr("This record is older than the current record in your database"));
            }
        } else {
            item = new QTreeWidgetItem(id->newRefrigerantManagement());
            item->setText(0, QUERY("date").toString());
            for (int i = 1; i < RecordOfRefrigerantManagement::attributes().count(); ++i) {
                item->setText(i, QUERY(RecordOfRefrigerantManagement::attributes().key(i)).toString());
            }
            item->setCheckState(0, Qt::Checked);
        }
        if (item) {
            if (record_locked) {
                item->setCheckState(0, Qt::Unchecked);
                item->setDisabled(true);
                item->setIcon(0, QIcon(":/images/images/locked16.png"));
            } else {
                item->setData(0, Qt::UserRole, QUERY("date"));
            }
        }
    }
    // Inspectors
    query.exec("SELECT * FROM inspectors ORDER BY person");
    while (query.next()) {
        Inspector inspector(QUERY("id").toString());
        if (inspector.exists()) {
            attributes = inspector.list();
            modified = false;
            MTDictionary columns(true);
            columns.insert(QUERY("id").toString().rightJustified(4, '0'), "0");
            for (int i = 1; i < Inspector::attributes().count(); ++i) {
                attribute = QUERY(Inspector::attributes().key(i));
                attribute_modified = attribute != attributes.value(Inspector::attributes().key(i));
                if (attribute_modified) modified = true;
                columns.insert(attribute.toString(), attribute_modified ? "1" : "0");
            }
            if (modified) {
                QTreeWidgetItem * item = new QTreeWidgetItem(id->modifiedInspectors(), columns.keys());
                for (int i = 0; i < columns.count(); ++i) {
                    if (columns.value(i).toInt()) {
                        item->setBackground(i, QBrush(Qt::darkMagenta));
                        item->setForeground(i, QBrush(Qt::white));
                    }
                }
                item->setCheckState(0, QUERY("date_updated").toString() > attributes.value("date_updated").toString() ? Qt::Checked : Qt::Unchecked);
                if (item->checkState(0) == Qt::Unchecked)
                    item->setToolTip(0, tr("This record is older than the current record in your database"));
                item->setData(0, Qt::UserRole, QUERY("id"));
            }
        } else {
            QTreeWidgetItem * item = new QTreeWidgetItem(id->newInspectors());
            item->setText(0, QUERY("id").toString().rightJustified(4, '0'));
            for (int i = 1; i < Inspector::attributes().count(); ++i) {
                item->setText(i, QUERY(Inspector::attributes().key(i)).toString());
            }
            item->setCheckState(0, Qt::Checked);
            item->setData(0, Qt::UserRole, QUERY("id"));
        }
    }
    if (id->exec() == QDialog::Accepted) { // BEGIN IMPORT
        QVariantMap set;
        QSet<QString> fields;
        // Import customers
        trw[0] = id->newCustomers();
        trw[1] = id->modifiedCustomers();
        fields = QSet<QString>::fromList(databaseTables().value("customers").split(QRegExp("[A-Z, ]+", Qt::CaseSensitive), QString::SkipEmptyParts));
        for (int w = 0; w < 2; ++w) {
            for (int c = 0; c < trw[w]->topLevelItemCount(); ++c) {
                if (trw[w]->topLevelItem(c)->checkState(0) == Qt::Unchecked) { continue; }
                set.clear();
                QString c_id = trw[w]->topLevelItem(c)->data(0, Qt::UserRole).toString();
                query.prepare("SELECT * FROM customers WHERE id = :id");
                query.bindValue(":id", c_id);
                query.exec();
                if (query.next()) {
                    for (int f = 0; f < query.record().count(); ++f) {
                        if (fields.contains(query.record().fieldName(f)))
                            set.insert(query.record().fieldName(f), query.value(f));
                    }
                }
                Customer(c_id).update(set);
            }
        }
        // Import circuits
        trw[0] = id->newCircuits();
        trw[1] = id->modifiedCircuits();
        fields = QSet<QString>::fromList(databaseTables().value("circuits").split(QRegExp("[A-Z, ]+", Qt::CaseSensitive), QString::SkipEmptyParts));
        for (int w = 0; w < 2; ++w) {
            for (int cc = 0; cc < trw[w]->topLevelItemCount(); ++cc) {
                if (trw[w]->topLevelItem(cc)->checkState(0) == Qt::Unchecked) { continue; }
                set.clear();
                QString cc_parent = trw[w]->topLevelItem(cc)->data(0, Qt::UserRole).toString();
                QString cc_id = trw[w]->topLevelItem(cc)->data(1, Qt::UserRole).toString();
                query.prepare("SELECT * FROM circuits WHERE parent = :parent AND id = :id");
                query.bindValue(":parent", cc_parent);
                query.bindValue(":id", cc_id);
                query.exec();
                if (query.next()) {
                    for (int f = 0; f < query.record().count(); ++f) {
                        if (fields.contains(query.record().fieldName(f)))
                            set.insert(query.record().fieldName(f), query.value(f));
                    }
                    set.remove("parent");
                }
                Circuit(cc_parent, cc_id).update(set);
            }
        }
        // Import variables
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
                set.insert("type", variableTypes().firstKey(item->text(3)));
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
                    set.insert("type", variableTypes().firstKey(subitem->text(3)));
                    set.insert("value", subitem->text(4));
                    set.insert("compare_nom", subitem->text(5) == tr("Yes") ? 1 : 0);
                    set.insert("tolerance", subitem->text(6));
                    record.update(set);
                }
            }
        }
        inspections_skip_columns << "customer" << "circuit";
        // Import inspections
        trw[0] = id->newInspections();
        trw[1] = id->modifiedInspections();
        for (int w = 0; w < 2; ++w) {
            for (int i = 0, j = 0; i < trw[w]->topLevelItemCount(); ++i) {
                if (trw[w]->topLevelItem(i)->checkState(0) == Qt::Unchecked) { continue; }
                set.clear();
                QString i_customer = trw[w]->topLevelItem(i)->data(0, Qt::UserRole).toString();
                QString i_circuit = trw[w]->topLevelItem(i)->data(1, Qt::UserRole).toString();
                QString i_date = trw[w]->topLevelItem(i)->data(2, Qt::UserRole).toString();
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
        }
        // Import repairs
        trw[0] = id->newRepairs();
        trw[1] = id->modifiedRepairs();
        fields = QSet<QString>::fromList(databaseTables().value("repairs").split(QRegExp("[A-Z, ]+", Qt::CaseSensitive), QString::SkipEmptyParts));
        for (int w = 0; w < 2; ++w) {
            for (int i = 0; i < trw[w]->topLevelItemCount(); ++i) {
                if (trw[w]->topLevelItem(i)->checkState(0) == Qt::Unchecked) { continue; }
                set.clear();
                QString r_date = trw[w]->topLevelItem(i)->data(0, Qt::UserRole).toString();
                query.prepare("SELECT * FROM repairs WHERE date = :date");
                query.bindValue(":date", r_date);
                query.exec();
                if (query.next()) {
                    for (int f = 0; f < query.record().count(); ++f) {
                        if (fields.contains(query.record().fieldName(f)))
                            set.insert(query.record().fieldName(f), query.value(f));
                    }
                }
                Repair(r_date).update(set);
            }
        }
        // Import refrigerant management
        trw[0] = id->newRefrigerantManagement();
        trw[1] = id->modifiedRefrigerantManagement();
        fields = QSet<QString>::fromList(databaseTables().value("refrigerant_management").split(QRegExp("[A-Z, ]+", Qt::CaseSensitive), QString::SkipEmptyParts));
        for (int w = 0; w < 2; ++w) {
            for (int i = 0; i < trw[w]->topLevelItemCount(); ++i) {
                if (trw[w]->topLevelItem(i)->checkState(0) == Qt::Unchecked) { continue; }
                set.clear();
                QString r_date = trw[w]->topLevelItem(i)->data(0, Qt::UserRole).toString();
                query.prepare("SELECT * FROM refrigerant_management WHERE date = :date");
                query.bindValue(":date", r_date);
                query.exec();
                if (query.next()) {
                    for (int f = 0; f < query.record().count(); ++f) {
                        if (fields.contains(query.record().fieldName(f)))
                            set.insert(query.record().fieldName(f), query.value(f));
                    }
                }
                RecordOfRefrigerantManagement(r_date).update(set);
            }
        }
        // Import inspectors
        trw[0] = id->newInspectors();
        trw[1] = id->modifiedInspectors();
        fields = QSet<QString>::fromList(databaseTables().value("inspectors").split(QRegExp("[A-Z, ]+", Qt::CaseSensitive), QString::SkipEmptyParts));
        for (int w = 0; w < 2; ++w) {
            for (int i = 0; i < trw[w]->topLevelItemCount(); ++i) {
                if (trw[w]->topLevelItem(i)->checkState(0) == Qt::Unchecked) { continue; }
                set.clear();
                QString i_id = trw[w]->topLevelItem(i)->data(0, Qt::UserRole).toString();
                query.prepare("SELECT * FROM inspectors WHERE id = :id");
                query.bindValue(":id", i_id);
                query.exec();
                if (query.next()) {
                    for (int f = 0; f < query.record().count(); ++f) {
                        if (fields.contains(query.record().fieldName(f)))
                            set.insert(query.record().fieldName(f), query.value(f));
                    }
                }
                Inspector(i_id).update(set);
            }
        }
        this->setWindowModified(true);
        refreshView();
    } // END IMPORT
    data.rollback();
    data.close();
} // END IMPORT (SCOPE)
    QSqlDatabase::removeDatabase("importData");
}

void MainWindow::importCSV()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Import CSV - Leaklog"), "", tr("CSV files (*.csv);;All files (*.*)"));
    if (path.isEmpty()) { return; }

    ImportCsvDialogue id(path, this);
    if (id.exec() == QDialog::Accepted) {
        QList<QStringList> file_content = id.fileContent();
        if (!file_content.count())
            return;
        QString table = id.table();
        QMap<QString, int> columns = id.columnIndexMap();

        QVariantMap set;
        bool ok; int num_failed = 0;
        QString string_value, column;
        QStringList id_columns;
        QStringList integer_columns;
        QMap<QString, QStringList> foreign_keys;
        QStringList text_columns;
        QStringList numeric_columns;
        QStringList boolean_columns;
        QStringList date_columns;
        QMap<QString, QMap<QString, QString> > select_columns;
        QMap<QString, QStringList> address_columns;

        if (table == "customers") {
            id_columns << "id";
            text_columns << "company" << "contact_person" << "mail" << "phone";
            address_columns.insert("address", QStringList() << "street" << "city" << "postal_code");
        } else if (table == "circuits") {
            id_columns << "id";
            integer_columns << "parent" << "year" << "inspection_interval";
            foreign_keys.insert("parent", QStringList() << "customers" << "id");
            text_columns << "name" << "operation" << "building" << "device" << "manufacturer" << "type" << "sn" << "commissioning";
            numeric_columns << "refrigerant_amount" << "oil_amount" << "runtime" << "utilisation";
            boolean_columns << "disused" << "hermetic" << "leak_detector";
            date_columns << "commissioning";
            QMap<QString, QString> map;
            for (int n = attributeValues().indexOfKey("field") + 1; n < attributeValues().count() && attributeValues().key(n).startsWith("field::"); ++n) {
                string_value = attributeValues().key(n).mid(attributeValues().key(n).lastIndexOf(':') + 1);
                map.insert(string_value, string_value);
                map.insert(attributeValues().value(n).toLower(), string_value);
            }
            select_columns.insert("field", map);
            map.clear();
            for (int n = attributeValues().indexOfKey("oil") + 1; n < attributeValues().count() && attributeValues().key(n).startsWith("oil::"); ++n) {
                string_value = attributeValues().key(n).mid(attributeValues().key(n).lastIndexOf(':') + 1);
                map.insert(string_value, string_value);
                map.insert(attributeValues().value(n).toLower(), string_value);
            }
            select_columns.insert("oil", map);
            map.clear();
            QStringList refrigerants(listRefrigerantsToString().split(';'));
            foreach (string_value, refrigerants)
                map.insert(string_value.toLower(), string_value);
            select_columns.insert("refrigerant", map);
        } else
            return;

        foreach (QStringList row, file_content) {
            set.clear();
            MTDictionary parents;

            ok = true;
            foreach (column, id_columns) {
                int id = row.value(columns.value(column, -1)).toInt(&ok);
                if (!ok)
                    break;
                set.insert(column, id);
            }
            if (!ok) {
                num_failed++;
                continue;
            }

            foreach (column, integer_columns) {
                int value = row.value(columns.value(column, -1)).toInt(&ok);
                if (foreign_keys.contains(column)) {
                    if (!ok) break;
                    QStringList foreign_key = foreign_keys.value(column);
                    MTRecord record(foreign_key.value(0), foreign_key.value(1), QString::number(value), MTDictionary());
                    if (!record.exists()) {
                        ok = false;
                        break;
                    }
                    parents.insert(column, QString::number(value));
                } else {
                    if (ok)
                        set.insert(column, value);
                    else
                        ok = true;
                }
            }
            if (!ok) {
                num_failed++;
                continue;
            }

            foreach (column, text_columns) {
                set.insert(column, row.value(columns.value(column, -1)));
            }

            foreach (column, numeric_columns) {
                string_value = row.value(columns.value(column, -1)).simplified().remove(' ');
                if (string_value.contains(',') && !string_value.contains('.'))
                    string_value.replace(',', '.');
                while (string_value.count('.') > 1)
                    string_value.remove(string_value.indexOf('.'), 1);
                double value = string_value.toDouble(&ok);
                if (ok)
                    set.insert(column, value);
            }

            foreach (column, boolean_columns) {
                string_value = row.value(columns.value(column, -1)).toLower().simplified().remove(' ');
                int value = ((string_value.toInt(&ok) && ok) || (!ok &&
                             (string_value == tr("Yes").toLower() ||
                             string_value == "yes" || string_value == "true")));
                set.insert(column, value);
            }

            foreach (column, date_columns) {
                string_value = row.value(columns.value(column, -1)).simplified().remove(' ');
                QDate date_value = QDate::fromString(string_value, "yyyy.MM.dd");
                if (!date_value.isValid())
                    date_value = QDate::fromString(string_value, "d.M.yyyy");
                if (date_value.isValid())
                    set.insert(column, date_value.toString("yyyy.MM.dd"));
            }

            QMapIterator<QString, QMap<QString, QString> > iterator1(select_columns);
            while (iterator1.hasNext()) { iterator1.next();
                int col = columns.value(iterator1.key(), -1);
                string_value = row.value(col).toLower().simplified();
                set.insert(iterator1.key(), iterator1.value().value(string_value));
            }

            QMapIterator<QString, QStringList> iterator2(address_columns);
            while (iterator2.hasNext()) { iterator2.next();
                MTAddress address;
                address.setStreet(row.value(columns.value(iterator2.value().value(0, "street"), -1)));
                address.setCity(row.value(columns.value(iterator2.value().value(1, "city"), -1)));
                address.setPostalCode(row.value(columns.value(iterator2.value().value(2, "postal_code"), -1)));
                set.insert(iterator2.key(), address.toString());
            }

            MTRecord record(table, id_columns.value(0), set.value(id_columns.value(0)).toString(), parents);
            record.update(set);
        }

        if (num_failed)
            QMessageBox::critical(this, tr("Import CSV - Leaklog"), tr("Failed to import %1 of %2 records.").arg(num_failed).arg(file_content.count()));
        else
            QMessageBox::information(this, tr("Import CSV - Leaklog"), tr("Successfully imported %n record(s).", "", file_content.count()));

        this->setWindowModified(true);
        refreshView();
    }
}
