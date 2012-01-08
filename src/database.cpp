/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2012 Matus & Michal Tomlein

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
#include "variable_evaluation.h"
#include "warnings.h"
#include "edit_warning_dialogue.h"
#include "tabbed_edit_dialogue.h"
#include "edit_customer_dialogue.h"
#include "edit_inspection_dialogue.h"
#include "edit_assembly_record_dialogue.h"
#include "edit_circuit_dialogue.h"
#include "edit_inspector_dialogue.h"
#include "import_dialogue.h"
#include "import_csv_dialogue.h"
#include "records.h"
#include "mtlistwidget.h"
#include "mtaddress.h"
#include "mtvariant.h"

#include <QBuffer>
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
    bool db_open = QSqlDatabase::database().isOpen();
    if (db_open && this->isWindowModified()) {
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
    } else if (db_open && !this->isWindowModified()) {
        if (close_) { closeDatabase(false); }; return false;
    }
    return false;
}

void MainWindow::initDatabase(QSqlDatabase & database, bool transaction)
{
    if (transaction) { database.transaction(); }
{ // (SCOPE)
    MTSqlQuery query(database);
    QStringList tables = database.tables();
    Variables * variables = Variables::defaultVariables();
    for (int i = 0; i < databaseTables().count(); ++i) {
        if (!tables.contains(databaseTables().key(i))) {
            query.exec("CREATE TABLE " + databaseTables().key(i) + " (" + sqlStringForDatabaseType(databaseTables().value(i), database) + ")");
            if (databaseTables().key(i) == "inspections") {
                QString type; bool ok = true;
                for (int v = 0; v < variableNames().count(); ++v) {
                    type = variableTypeToSqlType(variableType(variableNames().key(v), &ok));
                    if (ok)
                        addColumn(variableNames().key(v) + " " + type, "inspections", database);
                }
            } else if (databaseTables().key(i) == "inspections_compressors") {
                QString type; bool ok = true;
                for (int v = 0; v < variableNames().count(); ++v) {
                    type = variableTypeToSqlType(variableType(variableNames().key(v), &ok));
                    if (ok && (variables->variable(variableNames().key(v)).scope() & Variable::Compressor))
                        addColumn(variableNames().key(v) + " " + type, "inspections_compressors", database);
                }
            }
        } else {
            MTDictionary field_names = getTableFieldNames(databaseTables().key(i), database);
            QStringList all_field_names = sqlStringForDatabaseType(databaseTables().value(i), database).split(", ");
            if (databaseTables().key(i) == "inspections") {
                QString type; bool ok = true;
                for (int v = 0; v < variableNames().count(); ++v) {
                    type = variableTypeToSqlType(variableType(variableNames().key(v), &ok));
                    if (ok)
                        all_field_names << variableNames().key(v) + " " + variableTypeToSqlType(variableType(variableNames().key(v)));
                }
            } else if (databaseTables().key(i) == "inspections_compressors") {
                QString type; bool ok = true;
                for (int v = 0; v < variableNames().count(); ++v) {
                    type = variableTypeToSqlType(variableType(variableNames().key(v), &ok));
                    if (ok && (variables->variable(variableNames().key(v)).scope() & Variable::Compressor))
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
    delete variables;

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
    if (v > 0 && v < 0.9061) {
        Customer customers("");
        MultiMapOfVariantMaps customer_ids = customers.mapAll("company", "id");
        QVariantMap set;
        MTSqlQuery repairs("SELECT date, customer FROM repairs WHERE parent IS NULL");
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
    if (v > 0 && v < 0.907) {
        query.exec("UPDATE inspections SET outside_interval = repair");
    }
    if (v < 0.908) {
        if (v > 0) {
            MTSqlQuery subvariables(database);
            subvariables.exec("SELECT parent, id, name, type, unit, value, compare_nom, tolerance FROM subvariables");
            while (subvariables.next()) {
                query.prepare("INSERT INTO variables (parent_id, id, name, type, unit, scope, value, compare_nom, tolerance, date_updated, updated_by) "
                              "VALUES (:parent_id, :id, :name, :type, :unit, :scope, :value, :compare_nom, :tolerance, :date_updated, :updated_by)");
                query.bindValue(":parent_id", subvariables.value(0));
                query.bindValue(":id", subvariables.value(1));
                query.bindValue(":name", subvariables.value(2));
                query.bindValue(":type", subvariables.value(3));
                query.bindValue(":unit", subvariables.value(4));
                query.bindValue(":scope", Variable::Inspection);
                query.bindValue(":value", subvariables.value(5));
                query.bindValue(":compare_nom", subvariables.value(6));
                query.bindValue(":tolerance", subvariables.value(7));
                query.bindValue(":date_updated", QDateTime::currentDateTime().toString("yyyy.MM.dd-hh:mm"));
                query.bindValue(":updated_by", currentUser());
                query.exec();
                query.prepare("UPDATE variables SET type = 'group', date_updated = :date_updated, updated_by = :updated_by WHERE id = :id");
                query.bindValue(":id", subvariables.value(0));
                query.bindValue(":date_updated", QDateTime::currentDateTime().toString("yyyy.MM.dd-hh:mm"));
                query.bindValue(":updated_by", currentUser());
                query.exec();
            }

            query.exec("DROP TABLE subvariables");

            // Contact persons separated from customers table
            Customer customers_rec("");
            ListOfVariantMaps customers = customers_rec.listAll();

            Person person;
            int next_id = person.max("id");
            QVariantMap person_values;
            for (int i = 0; i < customers.count(); ++i) {
                if (customers.at(i).value("contact_person").isNull())
                    continue;

                person_values.insert("company_id", customers.at(i).value("id"));
                person_values.insert("name", customers.at(i).value("contact_person"));
                person.setId(QString::number(++next_id));
                person.update(person_values);
            }

            // Create a compressor for each circuit
            ListOfVariantMaps circuits = Circuit().listAll();
            QVariantMap map;
            for (int i = 0; i < circuits.count(); ++i) {
                map.insert("customer_id", circuits.at(i).value("parent"));
                map.insert("circuit_id", circuits.at(i).value("id"));
                map.insert("name", tr("Compressor"));
                map.insert("manufacturer", QString());
                map.insert("type", QString());
                map.insert("sn", QString());
                Compressor().update(map);
            }

            // Copy values of compressor variables from inspections table to inspections_compressors
            ListOfVariantMaps inspections = Inspection().listAll("*, (SELECT MIN(id) FROM compressors WHERE customer_id = inspections.customer AND circuit_id = inspections.circuit) AS compressor_id");

            VariableEvaluation::EvaluationContext variable_evaluation("", "", Variable::Compressor);
            QList<VariableEvaluation::Variable *> vars_list = variable_evaluation.listVariables();

            for (int i = 0; i < inspections.count(); ++i) {
                if (inspections.at(i).value("compressor_id").isNull())
                    continue;

                QVariantMap values;
                values.insert("customer_id", inspections.at(i).value("customer").toString());
                values.insert("circuit_id", inspections.at(i).value("circuit").toString());
                values.insert("date", inspections.at(i).value("date").toString());
                values.insert("compressor_id", inspections.at(i).value("compressor_id").toString());

                for (int n = 0; n < vars_list.count(); ++n) {
                    if (vars_list.at(n)->countSubvariables())
                        continue;

                    if (inspections.at(i).contains(vars_list.at(n)->id()) && !inspections.at(i).value(vars_list.at(n)->id()).isNull())
                        values.insert(vars_list.at(n)->id(), inspections.at(i).value(vars_list.at(n)->id()).toString());
                }

                InspectionsCompressor().update(values, true);
            }
        }
        query.exec(QString("INSERT INTO assembly_record_item_categories (id, name, display_options, display_position) VALUES (%1, '%2', 31, 0)").arg(INSPECTORS_CATEGORY_ID).arg(tr("Inspectors")));
        query.exec(QString("INSERT INTO assembly_record_item_categories (id, name, display_options, display_position) VALUES (%1, '%2', 31, 0)").arg(CIRCUIT_UNITS_CATEGORY_ID).arg(tr("Circuit units")));
    }
    if (v > 0 && v < 0.9083) {
        MTSqlQuery files(database);
        files.exec("SELECT id, data FROM files");
        while (files.next()) {
            QByteArray png;
            if (isDatabaseRemote(database))
                png = QByteArray::fromBase64(QByteArray::fromHex(files.value(1).toByteArray()));
            else
                png = files.value(1).toByteArray();

            QImage image = QImage::fromData(png, "PNG");
            QBuffer buffer;
            buffer.open(QIODevice::WriteOnly);
            image.save(&buffer, "JPG", JPEG_QUALITY);
            buffer.close();

            QByteArray jpg;
            if (isDatabaseRemote(database))
                jpg = buffer.data().toBase64();
            else
                jpg = buffer.data();

            query.prepare("UPDATE files SET data = :data WHERE id = :id");
            query.bindValue(":id", files.value(0));
            query.bindValue(":data", jpg);
            query.exec();
        }
    }
    if (v > 0 && v < 0.9084) {
        if (isDatabaseRemote(database)) {
            query.exec("ALTER TABLE persons ALTER COLUMN id TYPE BIGINT");
            query.exec("ALTER TABLE compressors ALTER COLUMN id TYPE BIGINT");
            query.exec("ALTER TABLE inspections_compressors ALTER COLUMN compressor_id TYPE BIGINT");
        }
    }
    if (v < F_DB_VERSION) {
        bool remote = isDatabaseRemote(database);
        QString unique_index = remote ? "CREATE UNIQUE INDEX " : "CREATE UNIQUE INDEX IF NOT EXISTS ";
        QString index = remote ? "CREATE INDEX " : "CREATE INDEX IF NOT EXISTS ";
        if (!remote || v == 0) {
            query.exec(unique_index + "index_db_info_id ON db_info (id ASC)");
            query.exec(unique_index + "index_circuits_id ON circuits (parent ASC, id ASC)");
            query.exec(unique_index + "index_inspections_id ON inspections (customer ASC, circuit ASC, date ASC)");
            query.exec(unique_index + "index_repairs_id ON repairs (date ASC)");
            query.exec(unique_index + "index_variables_id ON variables (id ASC)");
            query.exec(unique_index + "index_tables_id ON tables (id ASC)");
            query.exec(index + "index_warnings_filters_parent ON warnings_filters (parent ASC)");
            query.exec(index + "index_warnings_conditions_parent ON warnings_conditions (parent ASC)");
            query.exec(unique_index + "index_refrigerant_management_id ON refrigerant_management (date ASC)");
        }
        if (!remote || v < 0.9082) {
            query.exec(unique_index + "index_compressors_id ON compressors (customer_id ASC, circuit_id ASC, id ASC)");
            query.exec(unique_index + "index_inspections_compressors_id ON inspections_compressors (customer_id ASC, circuit_id ASC, date ASC, id ASC)");
            query.exec(index + "index_inspection_images_parent ON inspection_images (customer ASC, circuit ASC, date ASC)");
            query.exec(index + "index_assembly_record_items_parent ON assembly_record_items (arno ASC)");
            query.exec(unique_index + "index_circuit_units_id ON circuit_units (company_id ASC, circuit_id ASC, id ASC)");
            query.exec(unique_index + "index_styles_id ON styles (id ASC)");
        }
    }
} // (SCOPE)
    if (transaction) { database.commit(); }
}

void MainWindow::initTables(bool transaction)
{
    QSqlDatabase db = QSqlDatabase::database();
    if (transaction) { db.transaction(); }
{ // (SCOPE)
    double v = DBInfoValueForKey("db_version").toDouble();
    if (v > 0 && v < 0.9082) {
        Table("", "Leakages").remove();
        Table("", "Pressures and temperatures").remove();
        Table("", "Electrical parameters").remove();
        Table("", "Compressors").remove();
        Table(tr("Leakages")).remove();
        Table(tr("Pressures and temperatures")).remove();
        Table(tr("Compressors")).remove();
    }
    QVariantMap set;
    Table leakages("", "90");
    if (!leakages.exists()) {
        set.insert("id", tr("Leakages"));
        set.insert("highlight_nominal", 0);
        set.insert("variables", "vis_aur_chk;dir_leak_chk;refr_add_am;refr_add_per;refr_reco;oil_leak_am;inspector;operator;rmds;arno");
        set.insert("sum", "vis_aur_chk;refr_add_am;refr_add_per;refr_reco;oil_leak_am");
        set.insert("scope", QString::number(Variable::Inspection));
        leakages.update(set);
        set.clear();
    }
    Table pressures_and_temperatures("", "70");
    if (!pressures_and_temperatures.exists()) {
        set.insert("id", tr("Pressures and temperatures"));
        set.insert("highlight_nominal", 1);
        set.insert("variables", "t_sec;p_0;t_0;delta_t_evap;t_evap_out;t_sh;p_c;t_c;delta_t_c;t_ev;t_sc;sftsw");
        set.insert("sum", "");
        set.insert("scope", QString::number(Variable::Inspection));
        pressures_and_temperatures.update(set);
        set.clear();
    }
    Table compressors("", "40");
    if (!compressors.exists()) {
        set.insert("id", tr("Compressors"));
        set.insert("highlight_nominal", 1);
        set.insert("variables", "t_comp_in;t_comp_out;ep_comp;ec;ev;oil_shortage;noise_vibr_comp;comp_runtime");
        set.insert("sum", "");
        set.insert("scope", QString::number(Variable::Compressor));
        compressors.update(set);
        //set.clear();
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
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(path);
    if (!db.open()) {
        QMessageBox::critical(this, tr("New database - Leaklog"), tr("Cannot write file %1:\n%2.").arg(path).arg(db.lastError().text()));
        this->setWindowTitle(tr("Leaklog"));
        return;
    }
    addRecent(path);
    initDatabase(db);
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
        QSqlDatabase db = QSqlDatabase::addDatabase(path.at(0).split(":").at(1));
        db.setHostName(path.at(2).split(":").at(0));
        db.setPort(path.at(2).split(":").at(1).toInt());
        db.setDatabaseName(path.at(1));
        db.setUserName(path.at(0).split(":").at(2));
        db.setPassword(password);
        if (db.open()) {
            db.transaction();
            initDatabase(db, false);
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
    int port = spb_port->value() == 0 ? 5432 : spb_port->value();
    QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL");
    db.setHostName(le_server->text());
    db.setPort(port);
    db.setDatabaseName(le_db_name->text());
    db.setUserName(le_user_name->text());
    db.setPassword(le_password->text());
    if (db.open()) {
        addRecent(QString("db:QPSQL:%1@%2@%3:%4").arg(le_user_name->text()).arg(le_db_name->text()).arg(le_server->text()).arg(port));
        db.transaction();
        initDatabase(db, false);
    }
    openDatabase(QString());
}

void MainWindow::openDatabase(QString path)
{
    m_settings.clearSelectedRepair();
    clearAll();
    if (path.isEmpty()) {
        QSqlDatabase db = QSqlDatabase::database();
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
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName(path);
        if (!db.open()) {
            QMessageBox::critical(this, tr("Open database - Leaklog"), tr("Cannot read file %1:\n%2.").arg(path).arg(db.lastError().text()));
            this->setWindowTitle(tr("Leaklog"));
            return;
        }
        db.transaction();
        initDatabase(db, false);
    }
    if (DBInfoValueForKey("min_leaklog_version", DBInfoValueForKey("db_version")).toDouble() > F_LEAKLOG_VERSION) {
        QMessageBox::warning(this, tr("Open database - Leaklog"), tr("A newer version of Leaklog is required to open this database."));
        closeDatabase(false);
        return;
    }
    initTables(false);
    loadVariables(trw_variables);

    MTSqlQuery query;
    query.exec("SELECT id FROM tables ORDER BY uid DESC, id ASC");
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

    ListOfVariantMaps styles = Style().listAll("id, name");
    for (int i = 0; i < styles.count(); ++i) {
        QListWidgetItem * item = new QListWidgetItem;
        item->setText(styles.at(i).value("name").toString());
        item->setData(Qt::UserRole, styles.at(i).value("id"));
        lw_styles->addItem(item);
    }

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
    setDBInfoValueForKey("saved_with", QString("Leaklog-%1").arg(F_LEAKLOG_VERSION));
    setDBInfoValueForKey("db_version", QString::number(F_DB_VERSION));
    setDBInfoValueForKey("min_leaklog_version", QString::number(F_DB_MIN_LEAKLOG_VERSION));

    QStringList errors;
    QSqlDatabase db = QSqlDatabase::database();
    db.commit();
    if (compact) {
        MTSqlQuery query;
        query.exec("VACUUM");
        if (query.lastError().type() != QSqlError::NoError)
            errors << query.lastError().text();
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
    if (save && saveChangesBeforeProceeding(tr("Close database - Leaklog"), false))
        return;

    parsed_expressions.clear();
    clearAll();
    enableTools();
    setAllEnabled(false);

    QString connection_name;
    {
        QSqlDatabase db = QSqlDatabase::database();
        connection_name = db.connectionName();
        db.rollback();
        db.close();
    }
    QSqlDatabase::removeDatabase(connection_name);

    stw_main->setCurrentIndex(0);
    this->setWindowTitle(tr("Leaklog"));
    this->setWindowModified(false);
}

bool MainWindow::isOperationPermitted(const QString & operation, const QString & record_owner)
{
    int permitted = Global::isOperationPermitted(operation, record_owner);
    if (permitted <= 0) {
        QMessageBox message(this);
        message.setWindowModality(Qt::WindowModal);
        message.setWindowFlags(message.windowFlags() | Qt::Sheet);
        message.setIconPixmap(QIcon(QString::fromUtf8(":/images/images/locked.png")).pixmap(32, 32));
        message.setWindowTitle(tr("Permission denied - Leaklog"));
        if (permitted == -2)
            message.setText(tr("This record was created by another user. Operation not permitted."));
        else
            message.setText(tr("This operation is not permitted."));
        message.setInformativeText(tr("For more information, contact your administrator."));
        message.addButton(tr("OK"), QMessageBox::AcceptRole);
        message.exec();
        return false;
    }
    return true;
}

bool MainWindow::isRecordLocked(const QString & date)
{
    if (Global::isRecordLocked(date)) {
        QMessageBox message(this);
        message.setWindowModality(Qt::WindowModal);
        message.setWindowFlags(message.windowFlags() | Qt::Sheet);
        message.setIconPixmap(QIcon(QString::fromUtf8(":/images/images/locked.png")).pixmap(32, 32));
        message.setWindowTitle(tr("Permission denied - Leaklog"));
        message.setText(tr("This record is locked."));
        message.setInformativeText(tr("For more information, contact your administrator."));
        message.addButton(tr("OK"), QMessageBox::AcceptRole);
        message.exec();
        return true;
    }
    return false;
}

void MainWindow::editServiceCompany()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!isOperationPermitted("edit_service_company")) { return; }
    ServiceCompany record(DBInfoValueForKey("default_service_company"));
    EditDialogue * md = new EditDialogue(&record, this);
    if (md->exec() == QDialog::Accepted) {
        setDBInfoValueForKey("default_service_company", record.id());
        this->setWindowModified(true);
        refreshView();
    }
}

void MainWindow::addRecordOfRefrigerantManagement()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!isOperationPermitted("add_refrigerant_management")) { return; }
    editRecordOfRefrigerantManagement("");
}

void MainWindow::editRecordOfRefrigerantManagement(const QString & date)
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!date.isEmpty() && isRecordLocked(date)) { return; }
    RecordOfRefrigerantManagement record(date);
    if (!isOperationPermitted("edit_refrigerant_management", record.stringValue("updated_by"))) { return; }
    EditDialogue * md = new EditDialogue(&record, this);
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
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!isOperationPermitted("add_customer")) { return; }
    Customer record("");
    EditDialogue * md = new EditCustomerDialogue(&record, this);
    if (md->exec() == QDialog::Accepted) {
        this->setWindowModified(true);
        loadCustomer(record.id().toInt(), true);
    }
    delete md;
}

void MainWindow::editCustomer()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!isCustomerSelected()) { return; }
    Customer record(selectedCustomer());
    if (!isOperationPermitted("edit_customer", record.stringValue("updated_by"))) { return; }
    EditDialogue * md = new EditCustomerDialogue(&record, this);
    QString old_id = selectedCustomer();
    QString old_company_name = record.stringValue("company");
    if (md->exec() == QDialog::Accepted) {
        this->setWindowModified(true);
        QString company_name = record.stringValue("company");
        if (old_id != record.id()) {
            MTSqlQuery update_circuits;
            update_circuits.prepare("UPDATE circuits SET parent = :new_id WHERE parent = :old_id");
            update_circuits.bindValue(":old_id", old_id);
            update_circuits.bindValue(":new_id", record.id());
            update_circuits.exec();
            MTSqlQuery update_compressors;
            update_compressors.prepare("UPDATE compressors SET customer_id = :new_id WHERE customer_id = :old_id");
            update_compressors.bindValue(":old_id", old_id);
            update_compressors.bindValue(":new_id", record.id());
            update_compressors.exec();
            MTSqlQuery update_circuit_units;
            update_circuit_units.prepare("UPDATE circuit_units SET company_id = :new_id WHERE company_id = :old_id");
            update_circuit_units.bindValue(":old_id", old_id);
            update_circuit_units.bindValue(":new_id", record.id());
            update_circuit_units.exec();
            MTSqlQuery update_inspections;
            update_inspections.prepare("UPDATE inspections SET customer = :new_id WHERE customer = :old_id");
            update_inspections.bindValue(":old_id", old_id);
            update_inspections.bindValue(":new_id", record.id());
            update_inspections.exec();
            MTSqlQuery update_inspections_compressors;
            update_inspections_compressors.prepare("UPDATE inspections_compressors SET customer_id = :new_id WHERE customer_id = :old_id");
            update_inspections_compressors.bindValue(":old_id", old_id);
            update_inspections_compressors.bindValue(":new_id", record.id());
            update_inspections_compressors.exec();
            MTSqlQuery update_inspection_images;
            update_inspection_images.prepare("UPDATE inspection_images SET customer = :new_id WHERE customer = :old_id");
            update_inspection_images.bindValue(":old_id", old_id);
            update_inspection_images.bindValue(":new_id", record.id());
            update_inspection_images.exec();
            MTSqlQuery update_repairs;
            update_repairs.prepare("UPDATE repairs SET parent = :new_id, customer = :customer WHERE parent = :old_id");
            update_repairs.bindValue(":old_id", old_id);
            update_repairs.bindValue(":new_id", record.id());
            update_repairs.bindValue(":customer", company_name);
            update_repairs.exec();
            loadCustomer(record.id().toInt(), true);
        } else if (old_company_name != company_name) {
            MTSqlQuery update_repairs;
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
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!isCustomerSelected()) { return; }
    if (!isOperationPermitted("add_customer")) { return; }
    Customer record(selectedCustomer());
    record.readValues();
    record.id().clear();
    EditDialogue * md = new EditDialogue(&record, this);
    if (md->exec() == QDialog::Accepted) {
        this->setWindowModified(true);
        loadCustomer(record.id().toInt(), true);
    }
    delete md;
}

void MainWindow::removeCustomer()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!isCustomerSelected()) { return; }
    Customer record(selectedCustomer());
    if (!isOperationPermitted("remove_customer", record.stringValue("updated_by"))) { return; }
    bool ok;
    QString confirmation = QInputDialog::getText(this, tr("Remove customer - Leaklog"), tr("Are you sure you want to remove the selected customer?\nTo remove all data about the customer \"%1\" type REMOVE and confirm:").arg(selectedCustomer()), QLineEdit::Normal, "", &ok);
    if (!ok || confirmation != tr("REMOVE")) { return; }
    record.remove();
    Circuit circuits(selectedCustomer(), "");
    circuits.remove();
    MTRecord inspections("inspections", "date", "", MTDictionary("customer", selectedCustomer()));
    inspections.remove();
    MTRecord repairs("repairs", "date", "", MTDictionary("parent", selectedCustomer()));
    repairs.remove();
    m_settings.setSelectedCustomer(-1);
    m_settings.setSelectedCircuit(-1);
    m_settings.setSelectedCompressor(-1);
    m_settings.clearSelectedInspection();
    m_settings.clearSelectedRepair();
    enableTools();
    this->setWindowModified(true);
    navigation->setView(Navigation::ListOfCustomers);
}

void MainWindow::loadCustomer(int customer, bool refresh)
{
    if (customer < 0) { return; }
    m_settings.setSelectedCustomer(customer, Customer(QString::number(customer)).stringValue("company"));
    m_settings.setSelectedCircuit(-1);
    m_settings.setSelectedCompressor(-1);
    m_settings.clearSelectedInspection();
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
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!isCustomerSelected()) { return; }
    if (!isOperationPermitted("add_circuit")) { return; }
    Circuit record(selectedCustomer(), "");
    EditDialogue * md = new EditCircuitDialogue(&record, this);
    if (md->exec() == QDialog::Accepted) {
        this->setWindowModified(true);
        loadCircuit(record.id().toInt(), true);
    }
    delete md;
}

void MainWindow::editCircuit()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!isCustomerSelected()) { return; }
    if (!isCircuitSelected()) { return; }
    Circuit record(selectedCustomer(), selectedCircuit());
    if (!isOperationPermitted("edit_circuit", record.stringValue("updated_by"))) { return; }
    EditDialogue * md = new EditCircuitDialogue(&record, this);
    QString old_id = selectedCircuit();
    if (md->exec() == QDialog::Accepted) {
        this->setWindowModified(true);
        if (old_id != record.id()) {
            MTSqlQuery update_inspections;
            update_inspections.prepare("UPDATE inspections SET circuit = :new_id WHERE customer = :customer_id AND circuit = :old_id");
            update_inspections.bindValue(":customer_id", selectedCustomer());
            update_inspections.bindValue(":old_id", old_id);
            update_inspections.bindValue(":new_id", record.id());
            update_inspections.exec();
            MTSqlQuery update_inspections_compressors;
            update_inspections_compressors.prepare("UPDATE inspections_compressors SET circuit_id = :new_id WHERE customer_id = :customer_id AND circuit_id = :old_id");
            update_inspections_compressors.bindValue(":customer_id", selectedCustomer());
            update_inspections_compressors.bindValue(":old_id", old_id);
            update_inspections_compressors.bindValue(":new_id", record.id());
            update_inspections_compressors.exec();
            MTSqlQuery update_inspection_images;
            update_inspection_images.prepare("UPDATE inspection_images SET circuit = :new_id WHERE customer = :customer_id AND circuit = :old_id");
            update_inspection_images.bindValue(":customer_id", selectedCustomer());
            update_inspection_images.bindValue(":old_id", old_id);
            update_inspection_images.bindValue(":new_id", record.id());
            update_inspection_images.exec();
            loadCircuit(record.id().toInt(), true);
        } else {
            refreshView();
        }
    }
    delete md;
}

void MainWindow::duplicateCircuit()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!isCustomerSelected()) { return; }
    if (!isCircuitSelected()) { return; }
    if (!isOperationPermitted("add_circuit")) { return; }
    Circuit record(selectedCustomer(), selectedCircuit());
    record.readValues();
    record.id().clear();
    EditDialogue * md = new EditDialogue(&record, this);
    if (md->exec() == QDialog::Accepted) {
        this->setWindowModified(true);
        loadCircuit(record.id().toInt(), true);
    }
    delete md;
}

void MainWindow::removeCircuit()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!isCustomerSelected()) { return; }
    if (!isCircuitSelected()) { return; }
    Circuit record(selectedCustomer(), selectedCircuit());
    if (!isOperationPermitted("remove_circuit", record.stringValue("updated_by"))) { return; }
    bool ok;
    QString confirmation = QInputDialog::getText(this, tr("Remove circuit - Leaklog"), tr("Are you sure you want to remove the selected circuit?\nTo remove all data about the circuit \"%1\" type REMOVE and confirm:").arg(selectedCircuit()), QLineEdit::Normal, "", &ok);
    if (!ok || confirmation != tr("REMOVE")) { return; }
    record.remove();
    MTDictionary parents(QStringList() << "customer_id" << "circuit_id",
                         QStringList() << selectedCustomer() << selectedCircuit());
    Compressor("", parents).remove();
    CircuitUnit("", parents).remove();
    Inspection(selectedCustomer(), selectedCircuit(), "").remove();
    InspectionsCompressor("", parents).remove();
    MTRecord("inspection_images", "", "", MTDictionary(QStringList() << "customer" << "circuit",
                                                       QStringList() << selectedCustomer() << selectedCircuit())).remove();
    m_settings.clearSelectedCircuit();
    enableTools();
    this->setWindowModified(true);
    navigation->setView(Navigation::ListOfCircuits);
}

void MainWindow::loadCircuit(int circuit, bool refresh)
{
    if (!isCustomerSelected()) { return; }
    if (circuit < 0) { return; }
    m_settings.setSelectedCircuit(circuit);
    enableTools();
    if (refresh) {
        navigation->setView(Navigation::ListOfInspections);
    }
}

void MainWindow::addInspection()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!isCustomerSelected()) { return; }
    if (!isCircuitSelected()) { return; }
    if (!isOperationPermitted("add_inspection")) { return; }
    Inspection record(selectedCustomer(), selectedCircuit(), "");
    EditDialogue * md = new EditInspectionDialogue(&record, this);
    if (md->exec() == QDialog::Accepted) {
        this->setWindowModified(true);
        loadInspection(record.id(), true);
    }
    delete md;
}

void MainWindow::editInspection()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!isCustomerSelected()) { return; }
    if (!isCircuitSelected()) { return; }
    if (!isInspectionSelected()) { return; }
    Inspection record(selectedCustomer(), selectedCircuit(), selectedInspection());
    if (!isOperationPermitted("edit_inspection", record.stringValue("updated_by"))) { return; }
    if (isRecordLocked(selectedInspection())) { return; }
    EditDialogue * md = new EditInspectionDialogue(&record, this);
    if (md->exec() == QDialog::Accepted) {
        this->setWindowModified(true);
        loadInspection(record.id(), true);
    }
    delete md;
}

void MainWindow::duplicateInspection()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!isCustomerSelected()) { return; }
    if (!isCircuitSelected()) { return; }
    if (!isInspectionSelected()) { return; }
    if (!isOperationPermitted("add_inspection")) { return; }
    Inspection record(selectedCustomer(), selectedCircuit(), selectedInspection());
    record.readValues();
    record.id().clear();
    EditDialogue * md = new EditDialogue(&record, this);
    if (md->exec() == QDialog::Accepted) {
        this->setWindowModified(true);
        loadInspection(record.id(), true);
    }
    delete md;
}

void MainWindow::removeInspection()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!isCustomerSelected()) { return; }
    if (!isCircuitSelected()) { return; }
    if (!isInspectionSelected()) { return; }
    Inspection record(selectedCustomer(), selectedCircuit(), selectedInspection());
    if (!isOperationPermitted("remove_inspection", record.stringValue("updated_by"))) { return; }
    if (isRecordLocked(selectedInspection())) { return; }
    bool ok;
    QString confirmation = QInputDialog::getText(this, tr("Remove inspection - Leaklog"), tr("Are you sure you want to remove the selected inspection?\nTo remove all data about the inspection \"%1\" type REMOVE and confirm:").arg(selectedInspection()), QLineEdit::Normal, "", &ok);
    if (!ok || confirmation != tr("REMOVE")) { return; }
    record.remove();
    MTDictionary parents(QStringList() << "customer_id" << "circuit_id" << "date",
                         QStringList() << selectedCustomer() << selectedCircuit() << selectedInspection());
    InspectionsCompressor("", parents).remove();
    InspectionImage(selectedCustomer(), selectedCircuit(), selectedInspection()).remove();
    m_settings.clearSelectedInspection();
    enableTools();
    this->setWindowModified(true);
    navigation->setView(Navigation::ListOfInspections);
}

void MainWindow::loadInspection(const QString & inspection, bool refresh)
{
    if (!isCustomerSelected()) { return; }
    if (!isCircuitSelected()) { return; }
    if (inspection.isEmpty()) { return; }
    Inspection inspection_rec(selectedCustomer(), selectedCircuit(), inspection);
    m_settings.setSelectedInspection(inspection, !inspection_rec.value("arno").toString().isEmpty());
    m_settings.setSelectedInspectionIsRepair(inspection_rec.value("repair").toBool());
    enableTools();
    if (refresh) {
        navigation->setView(Navigation::Inspection);
    }
}

void MainWindow::addRepair()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!isOperationPermitted("add_repair")) { return; }
    Repair record("");
    if (isCustomerSelected()) {
        record.parents().insert("parent", selectedCustomer());
        record.parents().insert("customer", Customer(selectedCustomer()).stringValue("company"));
    }
    EditDialogue * md = new EditDialogue(&record, this);
    if (md->exec() == QDialog::Accepted) {
        this->setWindowModified(true);
        loadRepair(record.id(), true);
    }
    delete md;
}

void MainWindow::editRepair()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!isRepairSelected()) { return; }
    Repair record(selectedRepair());
    if (!isOperationPermitted("edit_repair", record.stringValue("updated_by"))) { return; }
    if (isRecordLocked(selectedRepair())) { return; }
    EditDialogue * md = new EditDialogue(&record, this);
    if (md->exec() == QDialog::Accepted) {
        this->setWindowModified(true);
        loadRepair(record.id(), true);
    }
    delete md;
}

void MainWindow::duplicateRepair()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!isRepairSelected()) { return; }
    if (!isOperationPermitted("add_repair")) { return; }
    Repair record(selectedRepair());
    record.readValues();
    record.id().clear();
    if (!record.stringValue("parent").isEmpty()) {
        record.parents().insert("parent", record.stringValue("parent"));
    }
    EditDialogue * md = new EditDialogue(&record, this);
    if (md->exec() == QDialog::Accepted) {
        this->setWindowModified(true);
        loadRepair(record.id(), true);
    }
    delete md;
}

void MainWindow::removeRepair()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!isRepairSelected()) { return; }
    QString repair = selectedRepair();
    Repair record(repair);
    if (!isOperationPermitted("remove_repair", record.stringValue("updated_by"))) { return; }
    if (isRecordLocked(repair)) { return; }
    bool ok;
    QString confirmation = QInputDialog::getText(this, tr("Remove repair - Leaklog"), tr("Are you sure you want to remove the selected repair?\nTo remove all data about the repair \"%1\" type REMOVE and confirm:").arg(repair), QLineEdit::Normal, "", &ok);
    if (!ok || confirmation != tr("REMOVE")) { return; }
    record.remove();
    m_settings.clearSelectedRepair();
    enableTools();
    this->setWindowModified(true);
    navigation->setView(Navigation::ListOfRepairs);
}

void MainWindow::loadRepair(const QString & date, bool refresh)
{
    if (date.isEmpty()) { return; }
    m_settings.setSelectedRepair(date);
    enableTools();
    if (refresh) {
        if (actionBasic_logbook->isChecked() && navigation->view() == Navigation::ListOfCircuits)
            refreshView();
        else
            navigation->setView(Navigation::ListOfRepairs);
    }
}

void MainWindow::loadVariables(QTreeWidget * trw, QSqlDatabase database)
{
    Variables variables(database);
    QMap<QString, QTreeWidgetItem *> variable_items;
    while (variables.next()) {
        QTreeWidgetItem * item = variable_items.value(variables.id(), NULL);
        if (!item) {
            if (variables.parentID().isEmpty())
                item = new QTreeWidgetItem(trw);
            else
                item = new QTreeWidgetItem;
            variable_items.insert(variables.id(), item);
        }

        if (!variables.parentID().isEmpty()) {
            QTreeWidgetItem * parent_item = variable_items.value(variables.parentID(), NULL);
            if (!parent_item) {
                parent_item = new QTreeWidgetItem(trw);
                variable_items.insert(variables.parentID(), parent_item);
            }
            parent_item->addChild(item);
        }

        item->setText(0, variables.name());
        item->setText(1, variables.id());
        item->setText(2, variables.unit());
        item->setText(3, variables.value(Variable::Tolerance).toString());
    }
}

void MainWindow::addVariable() { addVariable(false); }

void MainWindow::addSubvariable() { addVariable(true); }

void MainWindow::addVariable(bool subvar)
{
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) { return; }
    if (!isOperationPermitted("add_variable")) { return; }
    QString parent_id;
    if (subvar) {
        if (trw_variables->currentItem()->parent() != NULL)
            return;
        parent_id = trw_variables->currentItem()->text(1);
    }
    VariableRecord record("", parent_id);
    EditDialogue * md = new EditDialogue(&record, this);
    if (md->exec() == QDialog::Accepted) {
        QVariantMap attributes = record.list("name, unit, scope, tolerance");
        QTreeWidgetItem * item = NULL;
        if (subvar) {
            item = new QTreeWidgetItem(trw_variables->currentItem());
            QVariantMap set;
            set.insert("type", "group");
            VariableRecord(parent_id).update(set);
        } else {
            item = new QTreeWidgetItem(trw_variables);
        }
        item->setText(0, attributes.value("name").toString());
        item->setText(1, record.id());
        item->setText(2, attributes.value("unit").toString());
        item->setText(3, attributes.value("tolerance").toString());

        int scope = attributes.value("scope").toInt();
        if (scope & Variable::Inspection)
            addColumn(record.id(), "inspections", db);
        if (scope & Variable::Compressor)
            addColumn(record.id(), "inspections_compressors", db);
        parsed_expressions.clear();
        this->setWindowModified(true);
        refreshView();
    }
    delete md;
}

void MainWindow::editVariable()
{
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) { return; }
    if (!trw_variables->currentIndex().isValid()) { return; }
    if (!isOperationPermitted("edit_variable")) { return; }

    QTreeWidgetItem * item = trw_variables->currentItem();
    QString id = item->text(1);
    VariableRecord record(id);
    EditDialogue * md = new EditDialogue(&record, this);
    if (md->exec() == QDialog::Accepted) {
        QVariantMap attributes = record.list("name, unit, scope, tolerance");
        item->setText(0, attributes.value("name").toString());
        item->setText(1, record.id());
        item->setText(2, attributes.value("unit").toString());
        item->setText(3, attributes.value("tolerance").toString());

        if (id != record.id()) {
            int scope = attributes.value("scope").toInt();
            if (scope & Variable::Inspection)
                renameColumn(id, record.id(), "inspections", db);
            if (scope & Variable::Compressor)
                renameColumn(id, record.id(), "inspections_compressors", db);

            parsed_expressions.clear();

            MTSqlQuery update_subvariables;
            update_subvariables.prepare("UPDATE variables SET parent_id = :new_id WHERE parent_id = :old_id");
            update_subvariables.bindValue(":old_id", id);
            update_subvariables.bindValue(":new_id", record.id());
            update_subvariables.exec();
        }
        this->setWindowModified(true);
        refreshView();
    }
    delete md;
}

void MainWindow::removeVariable()
{
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) { return; }
    if (!trw_variables->currentIndex().isValid()) { return; }
    if (!isOperationPermitted("remove_variable")) { return; }
    QTreeWidgetItem * item = trw_variables->currentItem();
    if (variableNames().contains(item->text(1))) { return; }
    bool subvar = item->parent() != NULL;
    QString id = item->text(1);
    bool ok;
    QString confirmation = QInputDialog::getText(this, subvar ? tr("Remove subvariable - Leaklog") : tr("Remove variable - Leaklog"), tr("Are you sure you want to remove the selected variable?\nTo remove the variable \"%1\" type REMOVE and confirm:").arg(id), QLineEdit::Normal, "", &ok);
    if (!ok || confirmation != tr("REMOVE")) { return; }

    MTRecord("variables", "id", "", MTDictionary("parent_id", id)).remove();
    MTRecord("variables", "id", id, MTDictionary()).remove();
    delete item;
    dropColumn(id, "inspections", db);
    dropColumn(id, "inspections_compressors", db);
    parsed_expressions.clear();
    enableTools();
    this->setWindowModified(true);
    refreshView();
}

void MainWindow::addTable()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!isOperationPermitted("add_table")) { return; }
    Table record("");
    EditDialogue * md = new EditDialogue(&record, this);
    if (md->exec() == QDialog::Accepted) {
        navigation->tableComboBox()->addItem(record.id());
        cb_table_edit->addItem(record.id());
        this->setWindowModified(true);
        refreshView();
    }
    delete md;
}

void MainWindow::editTable()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (cb_table_edit->currentIndex() < 0) { return; }
    if (!isOperationPermitted("edit_table")) { return; }
    Table record(cb_table_edit->currentText());
    EditDialogue * md = new EditDialogue(&record, this);
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
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (cb_table_edit->currentIndex() < 0) { return; }
    if (!isOperationPermitted("remove_table")) { return; }
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
    if (!QSqlDatabase::database().isOpen()) { return; }
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
        if (variable.next()) { item->setText(0, variable.name()); }
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
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (cb_table_edit->currentIndex() < 0) { return; }
    if (!isOperationPermitted("edit_table")) { loadTable(cb_table_edit->currentText()); return; }
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
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (cb_table_edit->currentIndex() < 0) { return; }
    if (!isOperationPermitted("edit_table")) { return; }
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
    Variable variable;
    QString id, name; QStringList ids;
    while (variable.next()) {
        id = variable.id();
        if (ids.contains(id)) { continue; }
        ids << id;
        name = variable.name();
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
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (cb_table_edit->currentIndex() < 0) { return; }
    if (!trw_table_variables->currentIndex().isValid()) { return; }
    if (!isOperationPermitted("edit_table")) { return; }
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
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (cb_table_edit->currentIndex() < 0) { return; }
    if (!trw_table_variables->currentIndex().isValid()) { return; }
    if (!isOperationPermitted("edit_table")) { return; }
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
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!isOperationPermitted("add_warning")) { return; }
    WarningRecord record("");
    EditWarningDialogue * md = new EditWarningDialogue(&record, this);
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

void MainWindow::editWarning()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!lw_warnings->currentIndex().isValid()) { return; }
    if (!isOperationPermitted("edit_warning")) { return; }
    QListWidgetItem * item = lw_warnings->currentItem();
    WarningRecord record(item->data(Qt::UserRole).toString());
    EditWarningDialogue * md = new EditWarningDialogue(&record, this);
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
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!lw_warnings->currentIndex().isValid()) { return; }
    if (!isOperationPermitted("remove_warning")) { return; }
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
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!isOperationPermitted("add_inspector")) { return; }
    Inspector record("");
    EditDialogue * md = new EditInspectorDialogue(&record, this);
    if (md->exec() == QDialog::Accepted) {
        this->setWindowModified(true);
        loadInspector(record.id().toInt(), true);
    }
    delete md;
}

void MainWindow::editInspector()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!isInspectorSelected()) { return; }
    if (!isOperationPermitted("edit_inspector")) { return; }
    QString old_id = selectedInspector();
    Inspector record(old_id);
    EditDialogue * md = new EditInspectorDialogue(&record, this);
    if (md->exec() == QDialog::Accepted) {
        this->setWindowModified(true);
        if (old_id != record.id()) {
            MTSqlQuery update_inspections;
            update_inspections.prepare("UPDATE inspections SET inspector = :new_id WHERE inspector = :old_id");
            update_inspections.bindValue(":old_id", old_id);
            update_inspections.bindValue(":new_id", record.id());
            update_inspections.exec();
            MTSqlQuery update_repairs;
            update_repairs.prepare("UPDATE repairs SET repairman = :new_id WHERE repairman = :old_id");
            update_repairs.bindValue(":old_id", old_id);
            update_repairs.bindValue(":new_id", record.id());
            update_repairs.exec();
            MTSqlQuery update_assembly_record_items;
            update_assembly_record_items.prepare(QString("UPDATE assembly_record_items SET item_type_id = :new_id WHERE item_type_id = :old_id AND source = %1")
                                                 .arg(AssemblyRecordItem::Inspectors));
            update_assembly_record_items.bindValue(":old_id", old_id);
            update_assembly_record_items.bindValue(":new_id", record.id());
            update_assembly_record_items.exec();
            loadInspector(record.id().toInt(), true);
        } else {
            refreshView();
        }
    }
    delete md;
}

void MainWindow::removeInspector()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!isInspectorSelected()) { return; }
    if (!isOperationPermitted("remove_inspector")) { return; }
    bool ok;
    QString confirmation = QInputDialog::getText(this, tr("Remove inspector - Leaklog"), tr("Are you sure you want to remove the selected inspector?\nTo remove all data about the inspector \"%1\" type REMOVE and confirm:").arg(selectedInspector()), QLineEdit::Normal, "", &ok);
    if (!ok || confirmation != tr("REMOVE")) { return; }
    Inspector record(selectedInspector());
    record.remove();
    m_settings.setSelectedInspector(-1);
    enableTools();
    this->setWindowModified(true);
    navigation->setView(Navigation::ListOfInspectors);
}

void MainWindow::loadInspector(int inspector, bool refresh)
{
    if (inspector < 0) { return; }
    m_settings.setSelectedInspector(inspector, Inspector(selectedInspector()).stringValue("person"));
    enableTools();
    if (refresh) {
        navigation->setView(Navigation::ListOfInspectors);
    }
}

void MainWindow::loadInspectorReport(int inspector, bool refresh)
{
    if (inspector < 0) { return; }
    m_settings.setSelectedInspector(inspector, Inspector(selectedInspector()).stringValue("person"));
    enableTools();
    if (refresh) {
        navigation->setView(Navigation::Inspector);
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
    QSqlDatabase db = QSqlDatabase::database();
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
        initDatabase(data);
        data.transaction();
        copyTable("variables", db, data);
        copyTable("tables", db, data);
        copyTable("customers", db, data, QString("id = %1").arg(selectedCustomer()));
        if (type == "customer") {
            copyTable("circuits", db, data, QString("parent = %1").arg(selectedCustomer()));
            copyTable("inspections", db, data, QString("customer = %1").arg(selectedCustomer()));
        } else if (type == "circuit") {
            copyTable("circuits", db, data, QString("parent = %1 AND id = %2").arg(selectedCustomer()).arg(selectedCircuit()));
            copyTable("inspections", db, data, QString("customer = %1 AND circuit = %2").arg(selectedCustomer()).arg(selectedCircuit()));
        } else if (type == "inspection") {
            copyTable("circuits", db, data, QString("parent = %1 AND id = %2").arg(selectedCustomer()).arg(selectedCircuit()));
            copyTable("inspections", db, data, QString("customer = %1 AND circuit = %2 AND date = '%3'")
                      .arg(selectedCustomer()).arg(selectedCircuit()).arg(selectedInspection()));
        }
        data.commit();
        data.close();
    } // END EXPORT (SCOPE)
    QSqlDatabase::removeDatabase("exportData");
}

void MainWindow::importData()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!isOperationPermitted("import_data")) { return; }
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
    MTSqlQuery query(data);
    initDatabase(data, false);
    ImportDialogue * id = new ImportDialogue(this);
    QVariantMap attributes;
    QVariant attribute;
    bool modified, attribute_modified;

    // Customers
    query.exec("SELECT * FROM customers ORDER BY company");
    while (query.next()) {
        QString company_id_justified = QUERY("id").toString().rightJustified(8, '0');
        Customer customer(QUERY("id").toString());
        if (customer.exists()) {
            attributes = customer.list();
            modified = false;

            MTDictionary columns(true);
            columns.insert(company_id_justified, "0");

            for (int i = 1; i < Customer::attributes().count(); ++i) {
                attribute = QUERY(Customer::attributes().key(i));
                attribute_modified = attribute != attributes.value(Customer::attributes().key(i));
                if (attribute_modified) modified = true;
                if (Customer::attributes().key(i) == "operator_id") {
                    switch (attribute.toInt()) {
                    case -1:
                        columns.insert(QApplication::translate("Customer", "Service company"), attribute_modified ? "1" : "0");
                        break;
                    case 0:
                        columns.insert(QApplication::translate("Customer", "Customer"), attribute_modified ? "1" : "0");
                    default:
                        columns.insert(attribute.toString().rightJustified(8, '0'), attribute_modified ? "1" : "0");
                    }
                } else {
                    columns.insert(MTVariant(attribute, (MTVariant::Type)dict_fieldtypes.value(Customer::attributes().key(i))).toString(),
                        attribute_modified ? "1" : "0");
                }
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
            item->setText(0, company_id_justified);
            for (int i = 1; i < Customer::attributes().count(); ++i) {
                item->setText(i, MTVariant(QUERY(Customer::attributes().key(i)),
                                           (MTVariant::Type)dict_fieldtypes.value(Customer::attributes().key(i))).toString());
            }
            item->setCheckState(0, Qt::Checked);
            item->setData(0, Qt::UserRole, QUERY("id"));
        }
    }

    // Contact persons
    query.exec("SELECT persons.*, customers.company FROM persons LEFT JOIN customers ON persons.company_id = customers.id"
               " ORDER BY customers.company, persons.name");
    while (query.next()) {
        Person person(QUERY("id").toString());
        if (person.exists()) {
            attributes = person.list();
            modified = false;

            MTDictionary columns(true);
            columns.insert(QUERY("company").toString(), QUERY("company_id") != attributes.value("company_id") ? "1" : "0");

            for (int i = 2; i < Person::attributes().count(); ++i) {
                attribute = QUERY(Person::attributes().key(i));
                attribute_modified = attribute != attributes.value(Person::attributes().key(i));
                if (attribute_modified) modified = true;
                columns.insert(attribute.toString(), attribute_modified ? "1" : "0");
            }

            if (modified) {
                QTreeWidgetItem * item = new QTreeWidgetItem(id->modifiedPersons(), columns.keys());
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
            QTreeWidgetItem * item = new QTreeWidgetItem(id->newPersons());
            item->setText(0, QUERY("company").toString());
            for (int i = 2; i < Person::attributes().count(); ++i) {
                item->setText(i - 1, QUERY(Person::attributes().key(i)).toString());
            }
            item->setCheckState(0, Qt::Checked);
            item->setData(0, Qt::UserRole, QUERY("id"));
        }
    }

    // Circuits
    QSet<int> shown_sections;
    shown_sections << 0 << 1 << 2;
    QMap<int, QString> compressor_attributes;
    compressor_attributes.insert(2, "name");
    compressor_attributes.insert(6, "manufacturer");
    compressor_attributes.insert(7, "type");
    compressor_attributes.insert(8, "sn");
    QStringList compressors_columns = databaseTables().value("compressors").split(QRegExp("[A-Z, ]+", Qt::CaseSensitive), QString::SkipEmptyParts);
    for (QStringList::iterator i = compressors_columns.begin(); i != compressors_columns.end(); ++i)
        *i = QString("compressors.%1 AS compressor_%1").arg(*i);
    query.exec(QString("SELECT customers.company, circuits.*, %1 FROM circuits"
                       " LEFT JOIN customers ON circuits.parent = customers.id"
                       " LEFT JOIN compressors ON compressors.customer_id = circuits.parent AND compressors.circuit_id = circuits.id"
                       " ORDER BY customers.company, circuits.id, compressors.id").arg(compressors_columns.join(", ")));
    QString last_id;
    QTreeWidgetItem * last_item = NULL;
    while (query.next()) {
        QString current_id = QString("%1:%2").arg(QUERY("parent").toString()).arg(QUERY("id").toString());
        if (last_id != current_id) {
            if (last_item && last_item->treeWidget() == NULL)
                delete last_item;
            last_id = current_id;

            QString circuit_id_justified = QUERY("id").toString().rightJustified(5, '0');
            Circuit circuit(QUERY("parent").toString(), QUERY("id").toString());
            if (circuit.exists()) {
                attributes = circuit.list();
                modified = false;

                MTDictionary columns(true);
                columns.insert(QUERY("company").toString(), "0");
                columns.insert(circuit_id_justified, "0");

                for (int i = 1; i < Circuit::attributes().count(); ++i) {
                    attribute = QUERY(Circuit::attributes().key(i));
                    attribute_modified = attribute != attributes.value(Circuit::attributes().key(i));
                    if (attribute_modified) {
                        modified = true;
                        shown_sections << i + 1;
                    }
                    columns.insert(attribute.toString(), attribute_modified ? "1" : "0");
                }

                last_item = new QTreeWidgetItem(modified ? id->modifiedCircuits() : NULL, columns.keys());
                for (int i = 0; i < columns.count(); ++i) {
                    if (columns.value(i).toInt()) {
                        last_item->setBackground(i, QBrush(Qt::darkMagenta));
                        last_item->setForeground(i, QBrush(Qt::white));
                    }
                }
                last_item->setCheckState(0, QUERY("date_updated").toString() > attributes.value("date_updated").toString() ? Qt::Checked : Qt::Unchecked);
                if (last_item->checkState(0) == Qt::Unchecked)
                    last_item->setToolTip(0, tr("This record is older than the current record in your database"));
                last_item->setData(0, Qt::UserRole, QUERY("parent"));
                last_item->setData(1, Qt::UserRole, QUERY("id"));
            } else {
                last_item = new QTreeWidgetItem(id->newCircuits());
                last_item->setText(0, QUERY("company").toString());
                last_item->setText(1, circuit_id_justified);
                for (int i = 1; i < Circuit::attributes().count(); ++i) {
                    last_item->setText(i + 1, QUERY(Circuit::attributes().key(i)).toString());
                }
                last_item->setCheckState(0, Qt::Checked);
                last_item->setData(0, Qt::UserRole, QUERY("parent"));
                last_item->setData(1, Qt::UserRole, QUERY("id"));
            }
        }

        if (last_item && QUERY("compressor_id").toLongLong()) {
            Compressor compressor(QUERY("compressor_id").toString());
            if (compressor.exists()) {
                attributes = compressor.list();
                modified = false;

                MTDictionary columns(true);
                QMapIterator<int, QString> i(compressor_attributes);
                while (i.hasNext()) { i.next();
                    attribute = QUERY("compressor_" + i.value());
                    attribute_modified = attribute != attributes.value(i.value());
                    if (attribute_modified) {
                        modified = true;
                        shown_sections << i.key();
                    }
                    columns.insert(attribute.toString(), attribute_modified ? "1" : "0");
                }

                if (modified && QUERY("compressor_date_updated").toString() > attributes.value("date_updated").toString()) {
                    QTreeWidgetItem * item = new QTreeWidgetItem(last_item);
                    int c = 0;
                    i.toFront();
                    while (i.hasNext()) { i.next();
                        item->setText(i.key(), columns.key(c));
                        if (columns.value(c).toInt()) {
                            item->setBackground(i.key(), QBrush(Qt::darkMagenta));
                            item->setForeground(i.key(), QBrush(Qt::white));
                        }
                        c++;
                    }
                    item->setData(0, Qt::UserRole, QUERY("compressor_id"));
                    if (last_item->treeWidget() == NULL)
                        id->modifiedCircuits()->addTopLevelItem(last_item);
                }
            } else {
                QTreeWidgetItem * item = new QTreeWidgetItem(last_item);
                item->setData(0, Qt::UserRole, QUERY("compressor_id"));
                QMapIterator<int, QString> i(compressor_attributes);
                while (i.hasNext()) { i.next();
                    item->setText(i.key(), QUERY("compressor_" + i.value()).toString());
                }
                if (last_item->treeWidget() == NULL)
                    id->modifiedCircuits()->addTopLevelItem(last_item);
            }
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
    QStringList compressor_variable_names;
    Variables variables(data);
    QMap<QString, QTreeWidgetItem *> variable_items;
    while (variables.next()) {
        QTreeWidgetItem * item = variable_items.value(variables.id(), NULL);
        if (!item) {
            if (variables.parentID().isEmpty())
                item = new QTreeWidgetItem(id->variables());
            else
                item = new QTreeWidgetItem;
            variable_items.insert(variables.id(), item);
        }

        if (!variables.parentID().isEmpty()) {
            QTreeWidgetItem * parent_item = variable_items.value(variables.parentID(), NULL);
            if (!parent_item) {
                parent_item = new QTreeWidgetItem(id->variables());
                variable_items.insert(variables.parentID(), parent_item);
            }
            parent_item->addChild(item);
            parent_item->setExpanded(true);

            variable_names.remove(variables.parentID());
            if (variables.valueExpression().isEmpty() && variables.type() != "group") {
                variable_names.insert(variables.id(),
                                      tr("%1: %2")
                                      .arg(variables.parentVariable().name())
                                      .arg(variables.name()));
                if (variables.scope() & Variable::Compressor)
                    compressor_variable_names << variables.id();
            }
        } else if (variables.valueExpression().isEmpty() && variables.type() != "group") {
            variable_names.insert(variables.id(), variables.name());
            if (variables.scope() & Variable::Compressor)
                compressor_variable_names << variables.id();
        }

        item->setText(0, variables.name());
        item->setText(1, variables.id());
        item->setText(2, variables.unit());
        item->setText(3, variableTypes().value(variables.type()));
        item->setText(4, variables.valueExpression());
        item->setText(5, variables.compareNom() ? tr("Yes") : tr("No"));
        item->setText(6, variables.value(Variable::Tolerance).toString());
        item->setText(7, variables.colBg());

        Variable variable(variables.id());
        bool found = false, overwrite = false;
        if (variable.next()) {
            found = true;
            if (variable.name() != variables.name()) {
                overwrite = true;
                item->setBackground(0, QBrush(Qt::darkMagenta));
                item->setForeground(0, QBrush(Qt::white));
            }
            if (variable.unit() != variables.unit()) {
                overwrite = true;
                item->setBackground(2, QBrush(Qt::darkMagenta));
                item->setForeground(2, QBrush(Qt::white));
            }
            if (variable.type() != variables.type()) {
                overwrite = true;
                item->setBackground(3, QBrush(Qt::darkMagenta));
                item->setForeground(3, QBrush(Qt::white));
            }
            if (variable.valueExpression() != variables.valueExpression()) {
                overwrite = true;
                item->setBackground(4, QBrush(Qt::darkMagenta));
                item->setForeground(4, QBrush(Qt::white));
            }
            if ((variable.compareNom() > 0) != (variables.compareNom() > 0)) {
                overwrite = true;
                item->setBackground(5, QBrush(Qt::darkMagenta));
                item->setForeground(5, QBrush(Qt::white));
            }
            if (variable.tolerance() != variables.tolerance()) {
                overwrite = true;
                item->setBackground(6, QBrush(Qt::darkMagenta));
                item->setForeground(6, QBrush(Qt::white));
            }
            if (variable.colBg() != variables.colBg()) {
                overwrite = true;
                item->setBackground(7, QBrush(Qt::darkMagenta));
                item->setForeground(7, QBrush(Qt::white));
            }
        }
        QComboBox * cb_action = new QComboBox;
        if (!found) {
            cb_action->addItem(tr("Import"));
            item->setIcon(0, QIcon(QString::fromUtf8(":/images/images/item_new16.png")));
        } else {
            cb_action->addItem(tr("Use existing and import"));
            if (overwrite) {
                cb_action->addItem(tr("Overwrite and import"));
                item->setIcon(0, QIcon(QString::fromUtf8(":/images/images/item_found_diff16.png")));
            } else {
                item->setIcon(0, QIcon(QString::fromUtf8(":/images/images/item_found16.png")));
            }
        }
        if (!variableNames().contains(variables.id())) {
            cb_action->insertItem(0, tr("Do not import"));
            cb_action->setCurrentIndex(1);
        } else {
            cb_action->setCurrentIndex(0);
        }
        id->variables()->setItemWidget(item, 8, cb_action);
    }

    // Inspections
    bool record_locked;
    QTreeWidget * trw[] = { id->newInspections(), id->modifiedInspections() };
    for (int w = 0; w < 2; ++w) {
        trw[w]->setColumnCount(variable_names.count() + 3);
        trw[w]->setHeaderItem(new QTreeWidgetItem(QStringList() << tr("Customer") << tr("Circuit") << tr("Date") << variable_names.values()));
    }
    shown_sections.clear();
    shown_sections << 0 << 1 << 2;
    QStringList inspections_columns = databaseTables().value("inspections").split(QRegExp("[A-Z, ]+", Qt::CaseSensitive), QString::SkipEmptyParts);
    for (QStringList::iterator i = inspections_columns.begin(); i != inspections_columns.end(); ++i)
        *i = QString("inspections.%1").arg(*i);
    foreach (const QString & variable_name, variable_names.keys())
        inspections_columns << QString("inspections.%1").arg(variable_name);
    QStringList inspections_compressors_columns = databaseTables().value("inspections_compressors").split(QRegExp("[A-Z, ]+", Qt::CaseSensitive), QString::SkipEmptyParts);
    for (QStringList::iterator i = inspections_compressors_columns.begin(); i != inspections_compressors_columns.end(); ++i)
        *i = QString("inspections_compressors.%1 AS compressor_%1").arg(*i);
    foreach (const QString & variable_name, compressor_variable_names)
        inspections_compressors_columns << QString("inspections_compressors.%1 AS compressor_%1").arg(variable_name);
    query.exec(QString("SELECT customers.company, %1, %2, compressors.name AS compressor_name"
                       " FROM inspections LEFT JOIN customers ON inspections.customer = customers.id"
                       " LEFT JOIN inspections_compressors ON inspections_compressors.customer_id = inspections.customer"
                       " AND inspections_compressors.circuit_id = inspections.circuit"
                       " AND inspections_compressors.date = inspections.date"
                       " LEFT JOIN compressors ON inspections_compressors.compressor_id = compressors.id"
                       " ORDER BY inspections.date, inspections_compressors.compressor_id")
               .arg(inspections_columns.join(", "))
               .arg(inspections_compressors_columns.join(", ")));
    last_id.clear();
    last_item = NULL;
    while (query.next()) {
        QString current_id = QString("%1:%2:%3")
                .arg(QUERY("customer").toString())
                .arg(QUERY("circuit").toString())
                .arg(QUERY("date").toString());
        if (last_id != current_id) {
            if (last_item && last_item->treeWidget() == NULL)
                delete last_item;
            last_id = current_id;

            QString circuit_id_justified = QUERY("circuit").toString().rightJustified(5, '0');
            record_locked = Global::isRecordLocked(QUERY("date").toString());
            Inspection inspection(QUERY("customer").toString(), QUERY("circuit").toString(), QUERY("date").toString());
            if (inspection.exists()) {
                attributes = inspection.list();
                modified = false;

                MTDictionary columns(true);
                columns.insert(QUERY("company").toString(), "0");
                columns.insert(circuit_id_justified, "0");
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

                last_item = new QTreeWidgetItem(modified ? id->modifiedInspections() : NULL, columns.keys());
                for (int i = 0; i < columns.count(); ++i) {
                    if (columns.value(i).toInt()) {
                        last_item->setBackground(i, QBrush(Qt::darkMagenta));
                        last_item->setForeground(i, QBrush(Qt::white));
                    }
                }
                last_item->setCheckState(0, QUERY("date_updated").toString() > attributes.value("date_updated").toString() ? Qt::Checked : Qt::Unchecked);
                if (last_item->checkState(0) == Qt::Unchecked)
                    last_item->setToolTip(0, tr("This record is older than the current record in your database"));
            } else {
                last_item = new QTreeWidgetItem(id->newInspections());
                last_item->setText(0, QUERY("company").toString());
                last_item->setText(1, circuit_id_justified);
                last_item->setText(2, QUERY("date").toString());
                for (int i = 0; i < variable_names.count(); ++i) {
                    last_item->setText(i + 3, QUERY(variable_names.key(i)).toString());
                }
                last_item->setCheckState(0, Qt::Checked);
            }
            if (record_locked) {
                last_item->setCheckState(0, Qt::Unchecked);
                last_item->setDisabled(true);
                last_item->setIcon(2, QIcon(":/images/images/locked16.png"));
            } else {
                last_item->setData(0, Qt::UserRole, QUERY("customer"));
                last_item->setData(1, Qt::UserRole, QUERY("circuit"));
                last_item->setData(2, Qt::UserRole, QUERY("date"));
            }
        }

        if (last_item && QUERY("compressor_id").toInt()) {
            MTDictionary inspections_compressor_parents(QStringList() << "customer_id" << "circuit_id" << "date" << "compressor_id",
                                                        QStringList() << QUERY("customer").toString() << QUERY("circuit").toString()
                                                        << QUERY("date").toString() << QUERY("compressor_compressor_id").toString());
            InspectionsCompressor inpections_compressor(QString(), inspections_compressor_parents);
            if (inpections_compressor.exists()) {
                attributes = inpections_compressor.list();
                modified = false;

                MTDictionary columns(true);
                columns.insert(QString(), "0");
                columns.insert(QUERY("compressor_name").toString(), "0");
                columns.insert(QString(), "0");

                for (int i = 0; i < variable_names.count(); ++i) {
                    if (!query.record().contains("compressor_" + variable_names.key(i))) {
                        columns.insert(QString(), "0");
                        continue;
                    }
                    attribute = QUERY("compressor_" + variable_names.key(i));
                    attribute_modified = attribute != attributes.value(variable_names.key(i));
                    if (attribute_modified) {
                        modified = true;
                        shown_sections << i + 3;
                    }
                    columns.insert(attribute.toString(), attribute_modified ? "1" : "0");
                }

                if (modified && QUERY("compressor_date_updated").toString() > attributes.value("date_updated").toString()) {
                    QTreeWidgetItem * item = new QTreeWidgetItem(last_item, columns.keys());
                    for (int i = 0; i < columns.count(); ++i) {
                        if (columns.value(i).toInt()) {
                            item->setBackground(i, QBrush(Qt::darkMagenta));
                            item->setForeground(i, QBrush(Qt::white));
                        }
                    }
                    item->setData(0, Qt::UserRole, QUERY("compressor_compressor_id"));
                    if (last_item->treeWidget() == NULL)
                        id->modifiedInspections()->addTopLevelItem(last_item);
                }
            } else {
                QTreeWidgetItem * item = new QTreeWidgetItem(last_item);
                item->setData(0, Qt::UserRole, QUERY("compressor_compressor_id"));
                item->setText(1, QUERY("compressor_name").toString());
                for (int i = 0; i < variable_names.count(); ++i) {
                    if (!query.record().contains("compressor_" + variable_names.key(i)))
                        continue;
                    shown_sections << i + 3;
                    item->setText(i + 3, QUERY("compressor_" + variable_names.key(i)).toString());
                }
                if (last_item->treeWidget() == NULL)
                    id->modifiedInspections()->addTopLevelItem(last_item);
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
        record_locked = Global::isRecordLocked(QUERY("date").toString());
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
        record_locked = Global::isRecordLocked(QUERY("date").toString());
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
                    Customer(c_id).update(set);
                }
            }
        }

        // Import contact persons
        trw[0] = id->newPersons();
        trw[1] = id->modifiedPersons();
        fields = QSet<QString>::fromList(databaseTables().value("persons").split(QRegExp("[A-Z, ]+", Qt::CaseSensitive), QString::SkipEmptyParts));
        for (int w = 0; w < 2; ++w) {
            for (int p = 0; p < trw[w]->topLevelItemCount(); ++p) {
                if (trw[w]->topLevelItem(p)->checkState(0) == Qt::Unchecked) { continue; }
                set.clear();
                QString p_id = trw[w]->topLevelItem(p)->data(0, Qt::UserRole).toString();
                query.prepare("SELECT * FROM persons WHERE id = :id");
                query.bindValue(":id", p_id);
                query.exec();
                if (query.next()) {
                    for (int f = 0; f < query.record().count(); ++f) {
                        if (fields.contains(query.record().fieldName(f)))
                            set.insert(query.record().fieldName(f), query.value(f));
                    }
                    Person(p_id).update(set);
                }
            }
        }

        // Import circuits
        trw[0] = id->newCircuits();
        trw[1] = id->modifiedCircuits();
        fields = QSet<QString>::fromList(databaseTables().value("circuits").split(QRegExp("[A-Z, ]+", Qt::CaseSensitive), QString::SkipEmptyParts));
        QSet<QString> compressors_fields = QSet<QString>::fromList(databaseTables().value("compressors").split(QRegExp("[A-Z, ]+", Qt::CaseSensitive), QString::SkipEmptyParts));
        for (int w = 0; w < 2; ++w) {
            for (int cc = 0; cc < trw[w]->topLevelItemCount(); ++cc) {
                QTreeWidgetItem * item = trw[w]->topLevelItem(cc);
                if (item->checkState(0) == Qt::Unchecked) { continue; }
                set.clear();
                QString cc_parent = item->data(0, Qt::UserRole).toString();
                QString cc_id = item->data(1, Qt::UserRole).toString();
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
                    Circuit(cc_parent, cc_id).update(set);
                }

                for (int c = 0; c < item->childCount(); ++c) {
                    set.clear();
                    QString c_id = item->child(c)->data(0, Qt::UserRole).toString();
                    query.prepare("SELECT * FROM compressors WHERE id = :id");
                    query.bindValue(":id", c_id);
                    query.exec();
                    if (query.next()) {
                        for (int f = 0; f < query.record().count(); ++f) {
                            if (compressors_fields.contains(query.record().fieldName(f)))
                                set.insert(query.record().fieldName(f), query.value(f));
                        }
                        Compressor(c_id).update(set);
                    }
                }
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
                VariableRecord record(item->text(1));
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
                    VariableRecord record(subitem->text(1));
                    Variable subvariable(subitem->text(1));
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
        QSet<QString> inspections_compressors_fields = QSet<QString>::fromList(databaseTables().value("inspections_compressors").split(QRegExp("[A-Z, ]+", Qt::CaseSensitive), QString::SkipEmptyParts));
        inspections_compressors_fields.unite(QSet<QString>::fromList(compressor_variable_names));
        for (int w = 0; w < 2; ++w) {
            for (int i = 0, j = 0; i < trw[w]->topLevelItemCount(); ++i) {
                QTreeWidgetItem * item = trw[w]->topLevelItem(i);
                if (item->checkState(0) == Qt::Unchecked) { continue; }
                set.clear();
                QString i_customer = item->data(0, Qt::UserRole).toString();
                QString i_circuit = item->data(1, Qt::UserRole).toString();
                QString i_date = item->data(2, Qt::UserRole).toString();
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

                for (int c = 0; c < item->childCount(); ++c) {
                    set.clear();
                    QString c_id = item->child(c)->data(0, Qt::UserRole).toString();
                    query.prepare("SELECT * FROM inspections_compressors WHERE customer_id = :customer_id AND circuit_id = :circuit_id AND date = :date AND compressor_id = :compressor_id");
                    query.bindValue(":customer_id", i_customer);
                    query.bindValue(":circuit_id", i_circuit);
                    query.bindValue(":date", i_date);
                    query.bindValue(":compressor_id", c_id);
                    query.exec();
                    if (query.next()) {
                        for (int f = 0; f < query.record().count(); ++f) {
                            if (inspections_compressors_fields.contains(query.record().fieldName(f))
                                && !inspections_skip_columns.contains(query.record().fieldName(f)))
                                set.insert(query.record().fieldName(f), query.value(f));
                        }
                        set.remove("id");
                        set.remove("customer_id");
                        set.remove("circuit_id");
                        set.remove("date");
                        set.remove("compressor_id");
                        MTDictionary inspections_compressor_parents(QStringList() << "customer_id" << "circuit_id" << "date" << "compressor_id",
                                                                    QStringList() << i_customer << i_circuit << i_date << c_id);
                        InspectionsCompressor(QString(), inspections_compressor_parents).update(set, false, true);
                    }
                }
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
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!isOperationPermitted("import_data")) { return; }
    QString path = QFileDialog::getOpenFileName(this, tr("Import CSV - Leaklog"), "", tr("CSV files (*.csv);;All files (*.*)"));
    if (path.isEmpty()) { return; }

    QString string_value;
    QStringList refrigerants(listRefrigerantsToString().split(';'));

    QList<ImportDialogueTable *> tables;
    ImportDialogueTable * table = new ImportDialogueTable(tr("Customers"), "customers");
    table->addColumn(tr("ID"), "id", ImportDialogueTableColumn::ID);
    table->addColumn(tr("Company"), "company", ImportDialogueTableColumn::Text);
    table->addColumn(tr("E-mail"), "mail", ImportDialogueTableColumn::Text);
    table->addColumn(tr("Phone"), "phone", ImportDialogueTableColumn::Text);
    table->addColumn(tr("Street"), "street", ImportDialogueTableColumn::AddressStreet);
    table->addColumn(tr("City"), "city", ImportDialogueTableColumn::AddressCity);
    table->addColumn(tr("Postal code"), "postal_code", ImportDialogueTableColumn::AddressPostalCode);
    tables.append(table);

    table = table->addChildTableTemplate(tr("Contact persons"), "persons", MTDictionary("id", "company_id"), true);
    table->addColumn(tr("Contact person name"), "name", ImportDialogueTableColumn::Text);
    table->addColumn(tr("Contact person e-mail"), "mail", ImportDialogueTableColumn::Text);
    table->addColumn(tr("Contact person phone"), "phone", ImportDialogueTableColumn::Text);

    ImportDialogueTable * circuits_table = new ImportDialogueTable(tr("Circuits"), "circuits");
    circuits_table->addColumn(tr("ID"), "id", ImportDialogueTableColumn::Integer);
    circuits_table->addForeignKeyColumn(tr("Customer ID"), "parent", "id", "customers");
    circuits_table->addColumn(tr("Name"), "name", ImportDialogueTableColumn::Text);
    circuits_table->addColumn(tr("Place of operation"), "operation", ImportDialogueTableColumn::Text);
    circuits_table->addColumn(tr("Building"), "building", ImportDialogueTableColumn::Text);
    circuits_table->addColumn(tr("Device"), "device", ImportDialogueTableColumn::Text);
    circuits_table->addColumn(tr("Manufacturer"), "manufacturer", ImportDialogueTableColumn::Text);
    circuits_table->addColumn(tr("Type"), "type", ImportDialogueTableColumn::Text);
    circuits_table->addColumn(tr("Serial number"), "sn", ImportDialogueTableColumn::Text);
    circuits_table->addColumn(tr("Amount of refrigerant"), "refrigerant_amount", ImportDialogueTableColumn::Numeric);
    circuits_table->addColumn(tr("Amount of oil"), "oil_amount", ImportDialogueTableColumn::Numeric);
    circuits_table->addColumn(tr("Run-time per day"), "runtime", ImportDialogueTableColumn::Numeric);
    circuits_table->addColumn(tr("Rate of utilisation"), "utilisation", ImportDialogueTableColumn::Numeric);
    circuits_table->addColumn(tr("Disused"), "disused", ImportDialogueTableColumn::Boolean);
    circuits_table->addColumn(tr("Hermetically sealed"), "hermetic", ImportDialogueTableColumn::Boolean);
    circuits_table->addColumn(tr("Fixed leakage detector installed"), "leak_detector", ImportDialogueTableColumn::Boolean);
    circuits_table->addColumn(tr("Year of purchase"), "year", ImportDialogueTableColumn::Integer);
    circuits_table->addColumn(tr("Date of commissioning"), "commissioning", ImportDialogueTableColumn::Date);
    ImportDialogueTableColumn * col = circuits_table->addColumn(tr("Field of application"), "field", ImportDialogueTableColumn::Select);
    for (int n = attributeValues().indexOfKey("field") + 1; n < attributeValues().count() && attributeValues().key(n).startsWith("field::"); ++n) {
        string_value = attributeValues().key(n).mid(attributeValues().key(n).lastIndexOf(':') + 1);
        col->addSelectValue(string_value, string_value);
        col->addSelectValue(attributeValues().value(n).toLower(), string_value);
    }
    col = circuits_table->addColumn(tr("Oil"), "oil", ImportDialogueTableColumn::Select);
    for (int n = attributeValues().indexOfKey("oil") + 1; n < attributeValues().count() && attributeValues().key(n).startsWith("oil::"); ++n) {
        string_value = attributeValues().key(n).mid(attributeValues().key(n).lastIndexOf(':') + 1);
        col->addSelectValue(string_value, string_value);
        col->addSelectValue(attributeValues().value(n).toLower(), string_value);
    }
    col = circuits_table->addColumn(tr("Refrigerant"), "refrigerant", ImportDialogueTableColumn::Select);
    foreach (string_value, refrigerants)
        col->addSelectValue(string_value.toLower(), string_value);
    tables.append(circuits_table);

    table = circuits_table->addChildTableTemplate(tr("Compressors"), "compressors",
        MTDictionary(QStringList() << "parent" << "id", QStringList() << "customer_id" << "circuit_id"), true);
    table->addColumn(tr("Compressor name"), "name", ImportDialogueTableColumn::Text);
    table->addColumn(tr("Manufacturer"), "manufacturer", ImportDialogueTableColumn::Text);
    table->addColumn(tr("Type"), "type", ImportDialogueTableColumn::Text);
    table->addColumn(tr("Serial number"), "sn", ImportDialogueTableColumn::Text);

    table = circuits_table->addChildTableTemplate(tr("Circuit units"), "circuit_units",
        MTDictionary(QStringList() << "parent" << "id", QStringList() << "company_id" << "circuit_id"), true);
    table->addForeignKeyColumn(tr("Unit type ID"), "unit_type_id", "id", "circuit_unit_types");
    table->addColumn(tr("Unit serial number"), "sn", ImportDialogueTableColumn::Text);

    table = new ImportDialogueTable(tr("Circuit unit types"), "circuit_unit_types");
    table->addColumn(tr("ID"), "id", ImportDialogueTableColumn::ID);
    table->addColumn(tr("Amount of refrigerant"), "refrigerant_amount", ImportDialogueTableColumn::Numeric);
    table->addColumn(tr("Amount of oil"), "oil_amount", ImportDialogueTableColumn::Numeric);
    table->addColumn(tr("Acquisition price"), "acquisition_price", ImportDialogueTableColumn::Numeric);
    table->addColumn(tr("List price"), "list_price", ImportDialogueTableColumn::Numeric);
    table->addColumn(tr("Manufacturer"), "manufacturer", ImportDialogueTableColumn::Text);
    table->addColumn(tr("Type"), "type", ImportDialogueTableColumn::Text);
    col = table->addColumn(tr("Oil"), "oil", ImportDialogueTableColumn::Select);
    for (int n = attributeValues().indexOfKey("oil") + 1; n < attributeValues().count() && attributeValues().key(n).startsWith("oil::"); ++n) {
        string_value = attributeValues().key(n).mid(attributeValues().key(n).lastIndexOf(':') + 1);
        col->addSelectValue(string_value, string_value);
        col->addSelectValue(attributeValues().value(n).toLower(), string_value);
    }
    col = table->addColumn(tr("Refrigerant"), "refrigerant", ImportDialogueTableColumn::Select);
    foreach (string_value, refrigerants)
        col->addSelectValue(string_value.toLower(), string_value);
    col = table->addColumn(tr("Location"), "location", ImportDialogueTableColumn::Select);
    col->addSelectValue("external", QString::number(CircuitUnitType::External));
    col->addSelectValue("internal", QString::number(CircuitUnitType::Internal));
    table->addColumn(tr("Output"), "output", ImportDialogueTableColumn::Numeric);
    table->addColumn(tr("Output unit"), "output_unit", ImportDialogueTableColumn::Text);
    table->addColumn(tr("Output at t0/tc"), "output_t0_tc", ImportDialogueTableColumn::Numeric);
    table->addColumn(tr("Notes"), "notes", ImportDialogueTableColumn::Text);
    tables.append(table);

    table = new ImportDialogueTable(tr("Assembly record item types"), "assembly_record_item_types");
    table->addColumn(tr("ID"), "id", ImportDialogueTableColumn::ID);
    table->addColumn(tr("Name"), "name", ImportDialogueTableColumn::Text);
    table->addColumn(tr("Unit"), "unit", ImportDialogueTableColumn::Text);
    table->addColumn(tr("Acquisition price"), "acquisition_price", ImportDialogueTableColumn::Numeric);
    table->addColumn(tr("List price"), "list_price", ImportDialogueTableColumn::Numeric);
    table->addColumn(tr("Discount"), "discount", ImportDialogueTableColumn::Numeric);
    table->addColumn(tr("EAN code"), "ean", ImportDialogueTableColumn::Text);
    col = table->addColumn(tr("Data type"), "value_data_type", ImportDialogueTableColumn::Select);
    col->addSelectValue(tr("string"), QString::number(Global::String));
    col->addSelectValue(tr("integer"), QString::number(Global::Integer));
    col->addSelectValue(tr("numeric"), QString::number(Global::Numeric));
    col->addSelectValue(tr("text"), QString::number(Global::Text));
    col->addSelectValue(tr("boolean"), QString::number(Global::Boolean));
    table->addColumn(tr("Automatically add to assembly record"), "auto_show", ImportDialogueTableColumn::Boolean);
    table->addForeignKeyColumn(tr("Category ID"), "category_id", "id", "assembly_record_item_categories");
    tables.append(table);

    ImportCsvDialogue id(path, tables, this);
    if (id.exec() == QDialog::Accepted) {
        int num_failed = id.save();

        if (num_failed)
            QMessageBox::critical(this, tr("Import CSV - Leaklog"), tr("Failed to import %1 of %2 records.").arg(num_failed).arg(id.fileContent().count()));
        else
            QMessageBox::information(this, tr("Import CSV - Leaklog"), tr("Successfully imported %n record(s).", "", id.fileContent().count()));

        this->setWindowModified(true);
        refreshView();
    }
}

void MainWindow::addAssemblyRecordType()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    AssemblyRecordType record("");
    EditDialogue * md = new EditAssemblyRecordDialogue(&record, this);
    if (md->exec() == QDialog::Accepted) {
        this->setWindowModified(true);
        loadAssemblyRecordType(record.id().toInt(), true);
    }
    delete md;
}

void MainWindow::loadAssemblyRecordType(int assembly_record, bool refresh)
{
    if (assembly_record < 0) { return; }
    m_settings.setSelectedAssemblyRecordType(assembly_record);
    enableTools();
    if (refresh) {
        navigation->setView(Navigation::ListOfAssemblyRecordTypes);
    }
}

void MainWindow::editAssemblyRecordType()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!isAssemblyRecordTypeSelected()) { return; }
    QString old_id = selectedAssemblyRecordType();
    AssemblyRecordType record(old_id);
    EditDialogue * md = new EditAssemblyRecordDialogue(&record, this);
    if (md->exec() == QDialog::Accepted) {
        this->setWindowModified(true);
        if (old_id != record.id()) {
            MTSqlQuery update_ar_type;
            update_ar_type.prepare("UPDATE inspections SET ar_type = :new_id WHERE ar_type = :old_id");
            update_ar_type.bindValue(":old_id", old_id);
            update_ar_type.bindValue(":new_id", record.id());
            update_ar_type.exec();
        }
        loadAssemblyRecordType(record.id().toInt(), true);
    }
    delete md;
}

void MainWindow::removeAssemblyRecordType()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!isAssemblyRecordTypeSelected()) { return; }
    QString sel_record = selectedAssemblyRecordType();
    bool ok;
    QString confirmation = QInputDialog::getText(this, tr("Remove assembly record item - Leaklog"), tr("Are you sure you want to remove the selected assembly record type?\nTo remove all data about the record \"%1\" type REMOVE and confirm:").arg(sel_record), QLineEdit::Normal, "", &ok);
    if (!ok || confirmation != tr("REMOVE")) { return; }
    AssemblyRecordType record(sel_record);
    record.remove();
    m_settings.setSelectedAssemblyRecordType(-1);
    enableTools();
    this->setWindowModified(true);
    navigation->setView(Navigation::ListOfAssemblyRecordTypes);
}

void MainWindow::addAssemblyRecordItemType()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    AssemblyRecordItemType record("");
    EditDialogue * md = new EditDialogue(&record, this);
    if (md->exec() == QDialog::Accepted) {
        this->setWindowModified(true);
        loadAssemblyRecordItemType(record.id().toInt(), true);
    }
    delete md;
}

void MainWindow::loadAssemblyRecordItemType(int assembly_record_item, bool refresh)
{
    if (assembly_record_item < 0) { return; }
    m_settings.setSelectedAssemblyRecordItemType(assembly_record_item);
    enableTools();
    if (refresh) {
        navigation->setView(Navigation::ListOfAssemblyRecordItemTypes);
    }
}

void MainWindow::editAssemblyRecordItemType()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!isAssemblyRecordItemTypeSelected()) { return; }
    QString old_id = selectedAssemblyRecordItemType();
    AssemblyRecordItemType record(old_id);
    EditDialogue * md = new EditDialogue(&record, this);
    if (md->exec() == QDialog::Accepted) {
        this->setWindowModified(true);
        if (old_id != record.id()) {
            MTSqlQuery update_ar_items;
            update_ar_items.prepare(QString("UPDATE assembly_record_items SET item_type_id = :new_id WHERE item_type_id = :old_id AND source = %1")
                                    .arg(AssemblyRecordItem::AssemblyRecordItemTypes));
            update_ar_items.bindValue(":old_id", old_id);
            update_ar_items.bindValue(":new_id", record.id());
            update_ar_items.exec();
        }
        loadAssemblyRecordItemType(record.id().toInt(), true);
    }
    delete md;
}

void MainWindow::removeAssemblyRecordItemType()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!isAssemblyRecordItemTypeSelected()) { return; }
    QString sel_record = selectedAssemblyRecordItemType();
    bool ok;
    QString confirmation = QInputDialog::getText(this, tr("Remove assembly record item type - Leaklog"), tr("Are you sure you want to remove the selected assembly record item type?\nTo remove all data about the record item \"%1\" type REMOVE and confirm:").arg(sel_record), QLineEdit::Normal, "", &ok);
    if (!ok || confirmation != tr("REMOVE")) { return; }
    AssemblyRecordItemType record(sel_record);
    record.remove();
    m_settings.setSelectedAssemblyRecordItemType(-1);
    enableTools();
    this->setWindowModified(true);
    navigation->setView(Navigation::ListOfAssemblyRecordItemTypes);
}

void MainWindow::addAssemblyRecordItemCategory()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    AssemblyRecordItemCategory record("");
    EditDialogue * md = new EditDialogue(&record, this);
    if (md->exec() == QDialog::Accepted) {
        this->setWindowModified(true);
        loadAssemblyRecordItemCategory(record.id().toInt(), true);
    }
    delete md;
}

void MainWindow::loadAssemblyRecordItemCategory(int assembly_record_item_category, bool refresh)
{
    if (assembly_record_item_category < 0) { return; }
    m_settings.setSelectedAssemblyRecordItemCategory(assembly_record_item_category);
    enableTools();
    if (refresh) {
        navigation->setView(Navigation::ListOfAssemblyRecordItemCategories);
    }
}

void MainWindow::editAssemblyRecordItemCategory()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!isAssemblyRecordItemCategorySelected()) { return; }
    QString old_id = selectedAssemblyRecordItemCategory();
    AssemblyRecordItemCategory record(old_id);
    EditDialogue * md = new EditDialogue(&record, this);
    if (md->exec() == QDialog::Accepted) {
        this->setWindowModified(true);
        if (old_id != record.id()) {
            MTSqlQuery update_ar_item_types;
            update_ar_item_types.prepare("UPDATE assembly_record_item_types SET category_id = :new_id WHERE category_id = :old_id");
            update_ar_item_types.bindValue(":old_id", old_id);
            update_ar_item_types.bindValue(":new_id", record.id());
            update_ar_item_types.exec();
            MTSqlQuery update_ar_type_categories;
            update_ar_type_categories.prepare("UPDATE assembly_record_type_categories SET record_category_id = :new_id WHERE record_category_id = :old_id");
            update_ar_type_categories.bindValue(":old_id", old_id);
            update_ar_type_categories.bindValue(":new_id", record.id());
            update_ar_type_categories.exec();
            MTSqlQuery update_ar_items;
            update_ar_items.prepare("UPDATE assembly_record_items SET category_id = :new_id WHERE category_id = :old_id");
            update_ar_items.bindValue(":old_id", old_id);
            update_ar_items.bindValue(":new_id", record.id());
            update_ar_items.exec();
        }
        loadAssemblyRecordItemCategory(record.id().toInt(), true);
    }
    delete md;
}

void MainWindow::removeAssemblyRecordItemCategory()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!isAssemblyRecordItemCategorySelected()) { return; }
    QString sel_category = selectedAssemblyRecordItemCategory();
    bool ok;
    QString confirmation = QInputDialog::getText(this, tr("Remove assembly record item category - Leaklog"), tr("Are you sure you want to remove the selected assembly record item category?\nTo remove all data about the item category \"%1\" type REMOVE and confirm:").arg(sel_category), QLineEdit::Normal, "", &ok);
    if (!ok || confirmation != tr("REMOVE")) { return; }
    AssemblyRecordItemCategory category(sel_category);
    category.remove();
    m_settings.setSelectedAssemblyRecordItemCategory(-1);
    enableTools();
    this->setWindowModified(true);
    navigation->setView(Navigation::ListOfAssemblyRecordItemCategories);
}

void MainWindow::loadAssemblyRecord(const QString & inspection, bool refresh)
{
    if (!isCustomerSelected()) { return; }
    if (!isCircuitSelected()) { return; }
    if (inspection.isEmpty()) { return; }
    Inspection inspection_rec(selectedCustomer(), selectedCircuit(), inspection);
    m_settings.setSelectedInspection(inspection, !inspection_rec.value("arno").toString().isEmpty());
    m_settings.setSelectedInspectionIsRepair(inspection_rec.value("repair").toBool());
    enableTools();
    if (refresh) {
        navigation->setView(Navigation::AssemblyRecord);
    }
}

void MainWindow::addCircuitUnitType()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    CircuitUnitType unit_type("");
    EditDialogue * md = new EditDialogue(&unit_type, this);
    if (md->exec() == QDialog::Accepted) {
        this->setWindowModified(true);
        loadCircuitUnitType(unit_type.id().toInt(), true);
    }
    delete md;
}

void MainWindow::loadCircuitUnitType(int circuit_unit_type, bool refresh)
{
    if (circuit_unit_type < 0) { return; }
    m_settings.setSelectedCircuitUnitType(circuit_unit_type);
    enableTools();
    if (refresh) {
        navigation->setView(Navigation::ListOfCircuitUnitTypes);
    }
}

void MainWindow::editCircuitUnitType()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!isCircuitUnitTypeSelected()) { return; }
    QString old_id = selectedCircuitUnitType();
    CircuitUnitType record(old_id);
    EditDialogue * md = new EditDialogue(&record, this);
    if (md->exec() == QDialog::Accepted) {
        this->setWindowModified(true);
        if (old_id != record.id()) {
            MTSqlQuery update_circuit_units;
            update_circuit_units.prepare("UPDATE circuit_units SET unit_type_id = :new_id WHERE unit_type_id = :old_id");
            update_circuit_units.bindValue(":old_id", old_id);
            update_circuit_units.bindValue(":new_id", record.id());
            update_circuit_units.exec();
            MTSqlQuery update_ar_items;
            update_ar_items.prepare(QString("UPDATE assembly_record_items SET item_type_id = :new_id WHERE item_type_id = :old_id AND source = %1")
                                    .arg(AssemblyRecordItem::CircuitUnitTypes));
            update_ar_items.bindValue(":old_id", old_id);
            update_ar_items.bindValue(":new_id", record.id());
            update_ar_items.exec();
        }
        loadCircuitUnitType(record.id().toInt(), true);
    }
    delete md;
}

void MainWindow::removeCircuitUnitType()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!isCircuitUnitTypeSelected()) { return; }
    QString sel_unit_type = selectedCircuitUnitType();
    bool ok;
    QString confirmation = QInputDialog::getText(this, tr("Remove circuit unit type - Leaklog"), tr("Are you sure you want to remove the selected circuit unit type?\nTo remove all data about the unit type \"%1\" type REMOVE and confirm:").arg(sel_unit_type), QLineEdit::Normal, "", &ok);
    if (!ok || confirmation != tr("REMOVE")) { return; }
    CircuitUnitType unit_type(sel_unit_type);
    unit_type.remove();
    m_settings.setSelectedCircuitUnitType(-1);
    enableTools();
    this->setWindowModified(true);
    navigation->setView(Navigation::ListOfCircuitUnitTypes);
}

void MainWindow::addStyle()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!isOperationPermitted("add_style")) { return; }

    MTSqlQuery query("SELECT MAX(id) FROM styles");
    if (!query.last()) return;

    int id = query.value(0).toInt() + 1;

    Style record(QString::number(id));
    EditDialogue * md = new EditDialogue(&record, this);
    if (md->exec() == QDialog::Accepted) {
        QVariantMap attributes = record.list("id, name");
        QListWidgetItem * item = new QListWidgetItem;
        item->setText(attributes.value("name").toString());
        item->setData(Qt::UserRole, attributes.value("id"));
        lw_styles->addItem(item);
        this->setWindowModified(true);
        refreshView();
    }
    delete md;
}

void MainWindow::editStyle()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!lw_styles->currentIndex().isValid()) { return; }
    if (!isOperationPermitted("edit_style")) { return; }
    QListWidgetItem * item = lw_styles->currentItem();
    Style record(item->data(Qt::UserRole).toString());
    EditDialogue * md = new EditDialogue(&record, this);
    if (md->exec() == QDialog::Accepted) {
        QVariantMap attributes = record.list("id, name");
        item->setText(attributes.value("name").toString());
        item->setData(Qt::UserRole, attributes.value("id"));
        this->setWindowModified(true);
        refreshView();
    }
    delete md;
}

void MainWindow::removeStyle()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!lw_styles->currentIndex().isValid()) { return; }
    if (!isOperationPermitted("remove_style")) { return; }
    QListWidgetItem * item = lw_styles->currentItem();
    bool ok;
    QString confirmation = QInputDialog::getText(this, tr("Remove style - Leaklog"), tr("Are you sure you want to remove the selected style?\nTo remove the style \"%1\" type REMOVE and confirm:").arg(item->text()), QLineEdit::Normal, "", &ok);
    if (!ok || confirmation != tr("REMOVE")) { return; }
    Style record(item->data(Qt::UserRole).toString());
    record.remove();
    delete item;
    enableTools();
    this->setWindowModified(true);
    refreshView();
}
