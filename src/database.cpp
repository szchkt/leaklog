/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2020 Matus & Michal Tomlein

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

#include "mainwindow.h"
#include "migrations.h"
#include "global.h"
#include "variables.h"
#include "variableevaluation.h"
#include "warnings.h"
#include "editwarningdialogue.h"
#include "tabbededitdialogue.h"
#include "editcustomerdialogue.h"
#include "editinspectiondialogue.h"
#include "editassemblyrecorddialogue.h"
#include "editcircuitdialogue.h"
#include "editinspectordialogue.h"
#include "editdialoguetable.h"
#include "importdialogue.h"
#include "importcsvdialogue.h"
#include "records.h"
#include "removedialogue.h"
#include "mtlistwidget.h"
#include "mtaddress.h"
#include "mtvariant.h"
#include "undostack.h"
#include "sha256.h"
#include "syncengine.h"

#include <QBuffer>
#include <QMessageBox>
#include <QFile>
#include <QFileDialog>
#include <QInputDialog>
#include <QDialogButtonBox>
#include <QRadioButton>
#include <QSqlRecord>
#include <QSqlError>
#include <QCalendarWidget>
#include <QDateTime>
#include <QSettings>
#include <QTimer>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUuid>

#include <limits>

using namespace Global;

bool MainWindow::saveChangesBeforeProceeding(const QString &title, bool close_)
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

bool MainWindow::initDatabase(QSqlDatabase &database, bool transaction, bool save_on_upgrade)
{
    if (transaction) { database.transaction(); }
{ // (SCOPE)
    MTSqlQuery query(database);
    QStringList tables = database.tables();
    Variables *variables = NULL;
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
                    if (!variables)
                        variables = Variables::defaultVariables();
                    if (ok && (variables->variableForID(variableNames().key(v)).scope() & Variable::Compressor))
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
                    if (!variables)
                        variables = Variables::defaultVariables();
                    if (ok && (variables->variableForID(variableNames().key(v)).scope() & Variable::Compressor))
                        all_field_names << variableNames().key(v) + " " + variableTypeToSqlType(variableType(variableNames().key(v)));
                }
            }
            for (int f = 0; f < all_field_names.count(); ++f) {
                QString field_name = all_field_names.at(f).split(" ").first();
                if (!field_names.contains(field_name)) {
                    addColumn(all_field_names.at(f), databaseTables().key(i), database);
                }
            }
        }
    }
    delete variables;

    double v = DBInfo::valueForKey("db_version", QString(), database).toDouble();

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
        query.exec("UPDATE inspections SET nominal = 0 WHERE nominal IS NULL");
        query.exec("UPDATE inspections SET repair = 0 WHERE repair IS NULL");
    }

    if (v > 0 && v < 0.907) {
        query.exec("UPDATE inspections SET outside_interval = repair");
    }

    if (v < 0.908) {
        query.exec(QString("INSERT INTO assembly_record_item_categories (uuid, name, display_options, display_position) VALUES ('%1', '%2', 31, 0)").arg(INSPECTORS_CATEGORY_UUID).arg(tr("Inspectors")));
        query.exec(QString("INSERT INTO assembly_record_item_categories (uuid, name, display_options, display_position) VALUES ('%1', '%2', 31, 0)").arg(CIRCUIT_UNITS_CATEGORY_UUID).arg(tr("Circuit units")));
    }

    if (v > 0 && v < 0.9083) {
        MTSqlQuery files(database);
        files.exec("SELECT id, data FROM files");
        while (files.next()) {
            QByteArray png;
            if (isDatabaseRemote(database))
                png = QByteArray::fromBase64(files.value(1).toByteArray());
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

    if (v > 0 && v < 1.9) {
        query.exec("DROP INDEX IF EXISTS index_refrigerant_management_id");
        query.exec("DROP INDEX IF EXISTS index_circuits_id");
        query.exec("DROP INDEX IF EXISTS index_inspections_id");
        query.exec("DROP INDEX IF EXISTS index_repairs_id");
        query.exec("DROP INDEX IF EXISTS index_tables_id");
        query.exec("DROP INDEX IF EXISTS index_variables_id");
        query.exec("DROP INDEX IF EXISTS index_warnings_filters_parent");
        query.exec("DROP INDEX IF EXISTS index_warnings_conditions_parent");
        query.exec("DROP INDEX IF EXISTS index_compressors_id");
        query.exec("DROP INDEX IF EXISTS index_inspections_compressors_id");
        query.exec("DROP INDEX IF EXISTS index_inspection_images_parent");
        query.exec("DROP INDEX IF EXISTS index_circuit_units_id");
        query.exec("DROP INDEX IF EXISTS index_styles_id");

        progress_bar->setVisible(true);
        migrateV1Database(database, progress_bar);
        progress_bar->setVisible(false);
    }

    if (v < F_DB_VERSION) {
        bool remote = isDatabaseRemote(database);
        QString unique_index = remote ? "CREATE UNIQUE INDEX " : "CREATE UNIQUE INDEX IF NOT EXISTS ";
        QString index = remote ? "CREATE INDEX " : "CREATE INDEX IF NOT EXISTS ";

        if (v == 0) {
            query.exec(unique_index + "index_db_info_id ON db_info (id ASC)");
        }

        if (v < 0.9082) {
            query.exec(index + "index_assembly_record_items_parent ON assembly_record_items (arno ASC)");
        }

        if (v < 1.9) {
            query.exec(index + "index_assembly_record_items_ar_item_type_uuid ON assembly_record_items (ar_item_type_uuid ASC)");
            query.exec(index + "index_assembly_record_items_ar_item_category_uuid ON assembly_record_items (ar_item_category_uuid ASC)");
            query.exec(index + "index_assembly_record_item_types_ar_item_category_uuid ON assembly_record_item_types (ar_item_category_uuid ASC)");
            query.exec(index + "index_assembly_record_type_categories_ar_type_uuid ON assembly_record_type_categories (ar_type_uuid ASC)");
            query.exec(index + "index_assembly_record_type_categories_ar_item_category_uuid ON assembly_record_type_categories (ar_item_category_uuid ASC)");
            query.exec(index + "index_circuits_customer_uuid ON circuits (customer_uuid ASC)");
            query.exec(index + "index_circuit_units_circuit_uuid ON circuit_units (circuit_uuid ASC)");
            query.exec(index + "index_circuit_units_unit_type_uuid ON circuit_units (unit_type_uuid ASC)");
            query.exec(index + "index_compressors_circuit_uuid ON compressors (circuit_uuid ASC)");
            query.exec(index + "index_inspections_customer_uuid ON inspections (customer_uuid ASC)");
            query.exec(index + "index_inspections_circuit_uuid ON inspections (circuit_uuid ASC)");
            query.exec(index + "index_inspections_compressors_inspection_uuid ON inspections_compressors (inspection_uuid ASC)");
            query.exec(index + "index_inspections_compressors_compressor_uuid ON inspections_compressors (compressor_uuid ASC)");
            query.exec(index + "index_inspections_date ON inspections (date ASC)");
            query.exec(index + "index_inspections_files_inspection_uuid ON inspections_files (inspection_uuid ASC)");
            query.exec(index + "index_inspections_files_file_uuid ON inspections_files (file_uuid ASC)");
            query.exec(index + "index_journal_source_uuid_entry_id ON journal (source_uuid ASC, entry_id ASC)");
            query.exec(index + "index_persons_customer_uuid ON persons (customer_uuid ASC)");
            query.exec(index + "index_refrigerant_management_date ON refrigerant_management (date ASC)");
            query.exec(index + "index_repairs_customer_uuid ON repairs (customer_uuid ASC)");
            query.exec(index + "index_repairs_date ON repairs (date ASC)");
            query.exec(index + "index_repairs_inspector_uuid ON repairs (inspector_uuid ASC)");
            query.exec(index + "index_variables_id ON variables (id ASC)");
            query.exec(index + "index_warnings_conditions_warning_uuid ON warnings_conditions (warning_uuid ASC)");
            query.exec(index + "index_warnings_filters_warning_uuid ON warnings_filters (warning_uuid ASC)");
        }

        if (v < 2.1) {
            query.exec("DROP INDEX IF EXISTS index_journal_source_uuid_entry_id");
            query.exec("DELETE FROM journal WHERE id NOT IN (SELECT MIN(id) FROM journal GROUP BY source_uuid, entry_id)");
            query.exec(unique_index + "index_journal_source_uuid_version_entry_id ON journal (source_uuid ASC, version ASC, entry_id ASC)");

            QString service_company_uuid = MTSqlQuery("SELECT uuid FROM service_companies ORDER BY uuid LIMIT 1", database).nextValue().toString();
            if (!QUuid(service_company_uuid).isNull()) {
                query.exec(QString("UPDATE circuits SET service_company_uuid = '%1' WHERE service_company_uuid IS NULL").arg(service_company_uuid));
                query.exec(QString("UPDATE repairs SET service_company_uuid = '%1' WHERE service_company_uuid IS NULL").arg(service_company_uuid));
                query.exec(QString("UPDATE refrigerant_management SET service_company_uuid = '%1' WHERE service_company_uuid IS NULL").arg(service_company_uuid));
                query.exec(QString("UPDATE inspectors SET service_company_uuid = '%1' WHERE service_company_uuid IS NULL").arg(service_company_uuid));
            }
        }

        if (save_on_upgrade && !transaction && v > 0) {
            QMessageBox message(this);
            message.setWindowTitle(tr("Database upgraded - Leaklog"));
            message.setWindowModality(Qt::WindowModal);
            message.setWindowFlags(message.windowFlags() | Qt::Sheet);
            message.setIcon(QMessageBox::Information);
            message.setText(tr("The database has been upgraded to work with this version of Leaklog."
                               " It is recommended that you save the changes now."));
            message.setInformativeText(tr("Once saved, you will not be able to use this database with previous versions of Leaklog."
                                          " Do you want to save the changes?"));
            message.addButton(tr("&Save"), QMessageBox::AcceptRole);
            message.addButton(tr("&Later"), QMessageBox::RejectRole);
            switch (message.exec()) {
                case 0: // Save
                    saveDatabase(true, false);
                    break;
                case 1: // Later
                    return false;
            }
        }
    }
} // (SCOPE)
    if (transaction) { database.commit(); }
    return true;
}

void MainWindow::initTables(bool transaction)
{
    QSqlDatabase db = QSqlDatabase::database();
    if (transaction) { db.transaction(); }
{ // (SCOPE)
    bool update_tables = false;
    int tables_version = DBInfo::valueForKey("tables_version").toInt();
    if (tables_version < 2) {
        DBInfo::setValueForKey("tables_version", "2");
        update_tables = true;
    }

    Table leakages(LEAKAGES_TABLE_UUID);
    if (update_tables || !leakages.exists()) {
        leakages.setName(tr("Leakages"));
        leakages.setPosition(-90);
        leakages.setHighlightNominal(false);
        leakages.setVariables("vis_aur_chk;dir_leak_chk;refr_add_am;refr_add_per;refr_reco;oil_leak_am;inspector_uuid;person_uuid;risks;rmds;arno");
        leakages.setSummedVariables("vis_aur_chk;refr_add_am;refr_add_per;refr_reco;oil_leak_am");
        leakages.setScope(Variable::Inspection);
        leakages.save();
    }

    Table pressures_and_temperatures(PRESSURES_AND_TEMPERATURES_TABLE_UUID);
    if (update_tables || !pressures_and_temperatures.exists()) {
        pressures_and_temperatures.setName(tr("Pressures and temperatures"));
        pressures_and_temperatures.setPosition(-70);
        pressures_and_temperatures.setHighlightNominal(true);
        pressures_and_temperatures.setVariables("t_sec;p_0;t_0;delta_t_evap;t_evap_out;t_sh;p_c;t_c;delta_t_c;t_ev;t_sc;sftsw");
        pressures_and_temperatures.setSummedVariables("");
        pressures_and_temperatures.setScope(Variable::Inspection);
        pressures_and_temperatures.save();
    }

    Table compressors(COMPRESSORS_TABLE_UUID);
    if (update_tables || !compressors.exists()) {
        compressors.setName(tr("Compressors"));
        compressors.setPosition(-40);
        compressors.setHighlightNominal(true);
        compressors.setVariables("t_comp_in;t_comp_out;ep_comp;ec;ev;oil_shortage;noise_vibr_comp;comp_runtime");
        compressors.setSummedVariables("");
        compressors.setScope(Variable::Compressor);
        compressors.save();
    }

} // (SCOPE)
    if (transaction) { db.commit(); }
}

void MainWindow::newDatabase(const QString &uuid, const QString &name)
{
    if (saveChangesBeforeProceeding(tr("New database - Leaklog"), true)) { return; }
    QSettings settings("SZCHKT", "Leaklog");
    QDir dir(settings.value("file_dialog_dir", QDir::homePath()).toString());
    QString path = QFileDialog::getSaveFileName(this, tr("New database - Leaklog"),
                                                dir.absoluteFilePath(name.isEmpty() ? tr("untitled.lklg") : QString("%1.lklg").arg(name)),
                                                tr("Leaklog Database (*.lklg)"));
    if (path.isEmpty()) { return; }
    dir.setPath(path);
    dir.cdUp();
    settings.setValue("file_dialog_dir", dir.absolutePath());

    if (!path.endsWith(".lklg", Qt::CaseInsensitive)) { path.append(".lklg"); }
    QFile file(path); if (file.exists()) { file.remove(); }
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(path);
    if (!db.open()) {
        QMessageBox::critical(this, tr("New database - Leaklog"), tr("Cannot write file %1:\n%2.").arg(path).arg(db.lastError().text()));
        clearWindowTitle();
        return;
    }
    addRecent(path);
    initDatabase(db);
    initTables();
    if (uuid.isEmpty()) {
        ServiceCompany().save();
    }
    DBInfo::setValueForKey("db_version", QString::number(F_DB_VERSION));
    DBInfo::setValueForKey("min_leaklog_version", QString::number(F_DB_MIN_LEAKLOG_VERSION));
    DBInfo::setValueForKey("created_with", QString("Leaklog-%1").arg(F_LEAKLOG_VERSION));
    DBInfo::setValueForKey("date_created", QDateTime::currentDateTime().toString(DATE_TIME_FORMAT));
    if (!uuid.isEmpty()) {
        DBInfo::setValueForKey("database_uuid", uuid);
        DBInfo::setValueForKey("sync_server", "leaklog.org");
    }
    if (!name.isEmpty()) {
        DBInfo::setDatabaseName(name);
    }
    openDatabase(db, path, uuid.isEmpty());
}

void MainWindow::downloadDatabase()
{
    if (authenticator->token().isEmpty()) {
        logIn();
        return;
    }

    progress_bar->setRange(0, 0);
    progress_bar->setVisible(true);
    setEnabled(false);

    authenticator->getDatabases([this](bool success, const QJsonDocument &document) {
        setEnabled(true);
        progress_bar->setVisible(false);

        if (success) {
            QJsonArray databases = document.object().value("databases").toArray();
            MTDictionary items;

            foreach (const QJsonValue &value, databases) {
                QJsonObject database = value.toObject();
                QString uuid = database.value("uuid").toString();
                if (uuid.isEmpty())
                    continue;
                QString name = database.value("name").toString();
                if (name.isEmpty())
                    name = uuid;
                QString identifier = name;
                int i = 1;
                while (items.contains(identifier)) {
                    i++;
                    identifier = QString("%1 (%2)").arg(name).arg(i);
                }
                items.insert(identifier, uuid);
            }

            if (items.isEmpty()) {
                QMessageBox message(this);
                message.setWindowTitle(tr("Download database from Leaklog.org"));
                message.setWindowModality(Qt::WindowModal);
                message.setWindowFlags(message.windowFlags() | Qt::Sheet);
                message.setIcon(QMessageBox::Information);
                message.setText(tr("No databases found."));
                message.setInformativeText(tr("Create a new database or open an existing database, then sync with Leaklog.org."));
                message.addButton(tr("OK"), QMessageBox::AcceptRole);
                message.exec();
            } else {
                bool ok = false;
                QString identifier = QInputDialog::getItem(this, tr("Download database from Leaklog.org"), tr("Select a database to download:"), items.keys(), 0, false, &ok, Qt::Sheet);
                if (ok) {
                    newDatabase(items.value(identifier), identifier);
                }
            }
        } else {
            QMessageBox message(this);
            message.setWindowTitle(tr("Download database from Leaklog.org"));
            message.setWindowModality(Qt::WindowModal);
            message.setWindowFlags(message.windowFlags() | Qt::Sheet);
            message.setIcon(QMessageBox::Warning);
            message.setText(tr("Failed to connect to Leaklog.org."));
            message.addButton(tr("OK"), QMessageBox::AcceptRole);
            message.exec();
        }
    });
}

void MainWindow::openRecent(QListWidgetItem *item)
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
        openDatabase(db, s);
    } else {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName(s);
        openDatabase(db, s);
    }
}

void MainWindow::open()
{
    if (saveChangesBeforeProceeding(tr("Open database - Leaklog"), true)) { return; }
    QSettings settings("SZCHKT", "Leaklog");
    QDir dir(settings.value("file_dialog_dir", QDir::homePath()).toString());
    QString path = QFileDialog::getOpenFileName(this, tr("Open database - Leaklog"), dir.absolutePath(),
                                                tr("Leaklog Databases (*.lklg);;All files (*.*)"));
    if (path.isEmpty()) { return; }
    dir.setPath(path);
    dir.cdUp();
    settings.setValue("file_dialog_dir", dir.absolutePath());
    addRecent(path);
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(path);
    openDatabase(db, path);
}

void MainWindow::openRemote()
{
    if (saveChangesBeforeProceeding(tr("Open remote database - Leaklog"), true)) { return; }

    QDialog *d = new QDialog(this);
	d->setWindowTitle(tr("Open remote database - Leaklog"));
        QGridLayout *gl = new QGridLayout(d);
            QLabel *lbl_driver_ = new QLabel(tr("Driver:"), d);
            lbl_driver_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        gl->addWidget(lbl_driver_, 0, 0);
            QLabel *lbl_driver = new QLabel("PostgreSQL", d);
        gl->addWidget(lbl_driver, 0, 1);
            QLabel *lbl_server = new QLabel(tr("Server:"), d);
            lbl_server->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        gl->addWidget(lbl_server, 1, 0);
            QLineEdit *le_server = new QLineEdit(d);
        gl->addWidget(le_server, 1, 1);
            QLabel *lbl_port = new QLabel(tr("Port:"), d);
            lbl_port->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        gl->addWidget(lbl_port, 2, 0);
            QSpinBox *spb_port = new QSpinBox(d);
            spb_port->setRange(0, 999999);
            spb_port->setValue(0);
            spb_port->setSpecialValueText(tr("Default"));
        gl->addWidget(spb_port, 2, 1);
            QLabel *lbl_db_name = new QLabel(tr("Database name:"), d);
            lbl_db_name->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        gl->addWidget(lbl_db_name, 3, 0);
            QLineEdit *le_db_name = new QLineEdit(d);
        gl->addWidget(le_db_name, 3, 1);
            QLabel *lbl_user_name = new QLabel(tr("User name:"), d);
            lbl_user_name->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        gl->addWidget(lbl_user_name, 4, 0);
            QLineEdit *le_user_name = new QLineEdit(d);
        gl->addWidget(le_user_name, 4, 1);
            QLabel *lbl_password = new QLabel(tr("Password:"), d);
            lbl_password->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        gl->addWidget(lbl_password, 5, 0);
            QLineEdit *le_password = new QLineEdit(d);
            le_password->setEchoMode(QLineEdit::Password);
        gl->addWidget(le_password, 5, 1);
            QDialogButtonBox *bb = new QDialogButtonBox(d);
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

    QString connection_string = QString("db:QPSQL:%1@%2@%3:%4")
            .arg(le_user_name->text())
            .arg(le_db_name->text())
            .arg(le_server->text())
            .arg(port);

    if (db.open()) {
        addRecent(connection_string);
    }

    openDatabase(db, connection_string);
}

static QDate dateForFileName(const QString &file_name)
{
    QStringList components = file_name.split('-');
    if (components.count() < 3)
        return QDate();

    QString year = components.at(components.count() - 3);
    year.remove(QRegExp("[^0-9]+"));
    QString month = components.at(components.count() - 2);
    month.remove(QRegExp("[^0-9]+"));
    QString day = components.at(components.count() - 1);
    day.remove(QRegExp("[^0-9]+"));

    return QDate(year.toInt(), month.toInt(), day.toInt());
}

void MainWindow::backupDatabase(const QString &path)
{
    QPair<bool, QDir> backup_dir = backupDirectoryForDatabasePath(path);

    QFileInfo db_info(path);
    QString backup_file_name = QString("%1-%2.lklg").arg(db_info.completeBaseName()).arg(QDate::currentDate().toString("yyyy-MM-dd"));

    if (backup_dir.first) {
        if (!backup_dir.second.exists(backup_file_name) && !QFile::copy(path, backup_dir.second.absoluteFilePath(backup_file_name))) {
            QMessageBox::warning(this, tr("Open database - Leaklog"), tr("Failed to create backup: %1").arg(tr("Failed to copy database.")));
        }

        QStringList entries = backup_dir.second.entryList(QStringList() << "*.lklg", QDir::Files, QDir::Name);
        while (entries.count() > 10) {
            qint64 min_diff = std::numeric_limits<qint64>::max();
            int delete_index = 1;
            for (int i = 0; i + 3 < entries.count(); ++i) {
                qint64 diff = dateForFileName(entries.at(i)).daysTo(dateForFileName(entries.at(i + 1)));
                if (min_diff > diff) {
                    min_diff = diff;
                    delete_index = i + 1;
                }
            }
            QString entry = entries.at(delete_index);
            qDebug().nospace() << "Deleting backup " << delete_index + 1 << " (" << entry << "), days since previous backup: " << min_diff;
            backup_dir.second.remove(entry);
            entries.removeAt(delete_index);
        }
    } else {
        QMessageBox::warning(this, tr("Open database - Leaklog"), tr("Failed to create backup: %1").arg(tr("Could not create backup folder.")));
    }
}

void MainWindow::openDatabase(QSqlDatabase &db, const QString &connection_string, bool show_warnings)
{
    m_connection_string = connection_string;

    QString path;

    if (isDatabaseRemote(db)) {
        path = db.databaseName();

        if (!db.isOpen() && !db.open()) {
            QMessageBox::critical(this, tr("Open database - Leaklog"), tr("Cannot open database %1:\n%2.").arg(path).arg(db.lastError().text()));
            clearWindowTitle();
            return;
        }
    } else {
        path = connection_string;

        QFile file(path);
        if (!file.exists()) {
            QMessageBox::critical(this, tr("Open database - Leaklog"), tr("File %1 does not exist.").arg(path));
            clearWindowTitle();
            return;
        }

        if (!db.isOpen()) {
            backupDatabase(path);

            if (!db.open()) {
                QMessageBox::critical(this, tr("Open database - Leaklog"), tr("Cannot read file %1:\n%2.").arg(path).arg(db.lastError().text()));
                clearWindowTitle();
                return;
            }
        }
    }

    db.transaction();

    if (!initDatabase(db, false)) {
        QMetaObject::invokeMethod(this, "closeDatabase", Qt::QueuedConnection, Q_ARG(bool, false));
        return;
    }

    if (DBInfo::valueForKey("min_leaklog_version", DBInfo::valueForKey("db_version")).toDouble() > F_LEAKLOG_VERSION) {
        QMessageBox::warning(this, tr("Open database - Leaklog"), tr("A newer version of Leaklog is required to open this database."));
        QMetaObject::invokeMethod(this, "closeDatabase", Qt::QueuedConnection, Q_ARG(bool, false));
        return;
    }

    QSettings settings("SZCHKT", "Leaklog");
    settings.beginGroup(QString("tabs/%1").arg(sha256(m_connection_string)));

    tabw_main->blockSignals(true);
    if (settings.childGroups().isEmpty()) {
        newTab();
    } else {
        foreach (const QString &index, settings.childGroups()) {
            newTab(false);
            settings.beginGroup(index);
            m_tab->restoreSettings(settings);
            settings.endGroup();
        }

        tabw_main->setCurrentIndex(settings.value("current_index").toInt());
    }
    tabw_main->blockSignals(false);
    tabChanged(tabw_main->currentIndex());

    settings.endGroup();

    initTables(false);

    loadDatabase(false);

    setWindowTitleWithRepresentedFilename(path);
    setDatabaseModified(false);

    QApplication::processEvents();

    setAllEnabled(true);
    enableTools();
    stw_main->setCurrentIndex(1);

    sync_engine = new SyncEngine(authenticator, this);
    connect(sync_engine, SIGNAL(syncStarted()), this, SLOT(syncStarted()));
    connect(sync_engine, SIGNAL(syncProgress(double)), this, SLOT(syncProgress(double)));
    connect(sync_engine, SIGNAL(syncFinished(bool, bool)), this, SLOT(syncFinished(bool, bool)));

    if (show_warnings) {
        MTSqlQuery query("SELECT date FROM refrigerant_management WHERE purchased > 0 OR purchased_reco > 0");
        if (!query.next()) {
            QMessageBox::information(this, tr("Refrigerant management"), tr("You should add a record of purchase for every kind of refrigerant you have in store. You can do so by clicking the \"Add record of refrigerant management\" button."));
        }
    }

    sync(false);
}

void MainWindow::loadDatabase(bool reload)
{
    loadServiceCompanies();

    trw_variables->clear();
    loadVariables(trw_variables);

    cb_table_edit->clear();
    MTDictionary tables;

    MTSqlQuery query("SELECT uuid, name FROM tables ORDER BY position ASC, name ASC");
    while (query.next()) {
        cb_table_edit->addItem(query.value(1).toString(), query.value(0));
        tables.setValue(query.value(0).toString(), query.value(1).toString());
    }

    emit tablesChanged(tables);

    lw_warnings->clear();
    Warnings warnings;
    while (warnings.next()) {
        QListWidgetItem *item = new QListWidgetItem;
        item->setText(warnings.value("description").toString().isEmpty() ? warnings.value("name").toString() : tr("%1 (%2)").arg(warnings.value("name").toString()).arg(warnings.value("description").toString()));
        item->setData(Qt::UserRole, warnings.value("uuid").toString());
        lw_warnings->addItem(item);
    }

    lw_styles->clear();
    ListOfVariantMaps styles = Style::query().listAll("uuid, name");
    for (int i = 0; i < styles.count(); ++i) {
        QListWidgetItem *item = new QListWidgetItem;
        item->setText(styles.at(i).value("name").toString());
        item->setData(Qt::UserRole, styles.at(i).value("uuid"));
        lw_styles->addItem(item);
    }

    updateLockButton();

    if (reload) {
        for (int i = 0; i < tabw_main->count(); ++i) {
            ViewTab *tab = qobject_cast<ViewTab *>(tabw_main->widget(i));
            tab->validateSelection();
            tab->refreshView();
        }

        enableTools();
    }
}

void MainWindow::save()
{
    saveDatabase(false);
    sync(false, false);
}

void MainWindow::saveAndCompact()
{
    saveDatabase(true);
    sync(false, false);
}

void MainWindow::saveDatabase(bool compact, bool update_ui)
{
    DBInfo::setValueForKey("saved_with", QString("Leaklog-%1").arg(F_LEAKLOG_VERSION));
    if (DBInfo::valueForKey("db_version").toDouble() < F_DB_VERSION)
        DBInfo::setValueForKey("db_version", QString::number(F_DB_VERSION));
    if (DBInfo::valueForKey("min_leaklog_version").toDouble() < F_DB_MIN_LEAKLOG_VERSION)
        DBInfo::setValueForKey("min_leaklog_version", QString::number(F_DB_MIN_LEAKLOG_VERSION));

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

    m_undo_stack->clear();
    setDatabaseModified(false);

    if (update_ui) {
        setWindowTitleWithRepresentedFilename(db.databaseName());
        loadDatabase(true);
    }
}

void MainWindow::autosave()
{
    if (!isWindowModified())
        return;

    if (!QSqlDatabase::database().isOpen())
        return;

    QString autosave_mode = DBInfo::autosaveMode();
    if (autosave_mode.isEmpty() || autosave_mode == "immediate")
        return;

    if (autosave_mode == "ask") {
        if (hasActiveModalWidget()) {
            QTimer::singleShot(10000, this, SLOT(autosave()));
            return;
        }

        QMessageBox message(this);
        message.setWindowTitle(tr("Database modified - Leaklog"));
        message.setWindowModality(Qt::WindowModal);
        message.setWindowFlags(message.windowFlags() | Qt::Sheet);
        message.setIcon(QMessageBox::Information);
        message.setText(tr("The database has been modified."));
        message.setInformativeText(tr("Do you want to save your changes?"));
        message.addButton(tr("&Save"), QMessageBox::AcceptRole);
        message.addButton(tr("&Later"), QMessageBox::RejectRole);
        message.setDefaultButton(QMessageBox::Cancel);
        switch (message.exec()) {
            case 0: // Save
                break;
            case 1: // Later
                return;
        }
    }

    if (autosave_mode == "ask" || autosave_mode == "delayed") {
        saveDatabase(false);
        sync(false, false);
    }
}

void MainWindow::closeDatabase(bool save)
{
    if (save && saveChangesBeforeProceeding(tr("Close database - Leaklog"), false))
        return;

    if (sync_engine) {
        delete sync_engine;
        sync_engine = NULL;
    }

    QSettings settings("SZCHKT", "Leaklog");
    settings.beginGroup(QString("tabs/%1").arg(sha256(m_connection_string)));
    settings.remove(QString());

    for (int i = 0; i < tabw_main->count(); ++i) {
        ViewTab *tab = qobject_cast<ViewTab *>(tabw_main->widget(i));
        settings.beginGroup(QString::number(i));
        tab->saveSettings(settings);
        settings.endGroup();
    }

    settings.setValue("current_index", tabw_main->currentIndex());
    settings.endGroup();

    m_undo_stack->clear();
    cb_table_edit->clear();
    trw_variables->clear();
    trw_table_variables->clear();
    lw_warnings->clear();

    tabw_main->blockSignals(true);
    while (tabw_main->count())
        delete tabw_main->widget(0);
    tabw_main->blockSignals(false);

    m_tab = NULL;

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

    m_connection_string.clear();

    stw_main->setCurrentIndex(0);
    clearWindowTitle();
    setDatabaseModified(false);
}

void MainWindow::autosync()
{
    if (sync_engine && sync_engine->error().isEmpty() && !isWindowModified() && !hasActiveModalWidget()) {
        sync(false);
    }
}

void MainWindow::sync(bool force, bool save)
{
    if (authenticator->token().isEmpty()) {
        if (force)
            logIn();
    } else if (sync_engine) {
        QString server = DBInfo::valueForKey("sync_server");
        if (server == "leaklog.org") {
            if (save) {
                saveDatabase(false, false);
            }
            sync_engine->sync(force);
        } else if (force || server.isNull() || !server.isEmpty()) {
            QMessageBox message(this);
            message.setWindowTitle(tr("Sync database - Leaklog"));
            message.setWindowModality(Qt::WindowModal);
            message.setWindowFlags(message.windowFlags() | Qt::Sheet);
            message.setIcon(QMessageBox::Information);
            message.setText(tr("Do you want to sync this database with Leaklog.org?"));
            message.setInformativeText(tr("By using this service, you agree to the <a href=\"https://leaklog.org/terms\">Terms of Service</a>."));
            message.setTextFormat(Qt::RichText);
            message.setTextInteractionFlags(Qt::LinksAccessibleByMouse);
            message.addButton(tr("&Sync"), QMessageBox::AcceptRole);
            message.addButton(tr("Do &Not Sync"), QMessageBox::RejectRole);
            switch (message.exec()) {
                case 0: // Sync
                    DBInfo::setValueForKey("sync_server", "leaklog.org");
                    saveDatabase(false, false);
                    sync_engine->sync(force);
                    break;
                case 1: // Do Not Sync
                    if (server.isNull()) {
                        DBInfo::setValueForKey("sync_server", "");
                        saveDatabase(false, false);
                    }
                    break;
            }
        }
    }
}

void MainWindow::syncStarted()
{
    progress_bar->setRange(0, 0);
    progress_bar->setVisible(true);

    if (isWindowModified()) {
        saveDatabase(false, false);
    }
}

void MainWindow::syncProgress(double progress)
{
    progress_bar->setRange(0, 100);
    progress_bar->setValue(progress * 100);
}

void MainWindow::syncFinished(bool success, bool changed)
{
    progress_bar->setRange(0, 0);
    progress_bar->setVisible(false);

    if (success) {
        saveDatabase(false, changed);
    } else {
        bool has_modal_widget = hasActiveModalWidget();

        if (!has_modal_widget) {
            QMessageBox message(this);
#ifdef Q_OS_MAC
            message.setWindowTitle(tr("Sync"));
#else
            message.setWindowTitle(tr("Sync - Leaklog"));
#endif
            message.setWindowModality(Qt::WindowModal);
            message.setWindowFlags(message.windowFlags() | Qt::Sheet);
            message.setIcon(QMessageBox::Warning);
            message.setText(tr("Failed to sync with the server."));
            message.setInformativeText(sync_engine->error());
            message.addButton(tr("OK"), QMessageBox::AcceptRole);
            message.exec();
        }

        if (sync_engine->action() == "login") {
            logoutFinished();

            if (!has_modal_widget) {
                logIn();
            }
        }
    }
}

void MainWindow::setDatabaseModified(bool modified)
{
    setWindowModified(modified);

    if (modified)
        emit databaseModified();

    if (!QSqlDatabase::database().isOpen() || DBInfo::autosaveMode() == "immediate") {
        actionSave->setVisible(false);
        actionSave_and_compact->setVisible(false);
        actionUndo->setVisible(false);
        tbtn_undo_action->setVisible(false);

        if (modified)
            saveDatabase(false, false);
    } else {
        actionSave->setVisible(true);
        actionSave_and_compact->setVisible(true);
        actionUndo->setVisible(true);
        tbtn_undo_action->setVisible(true);
        tbtn_undo_action->setEnabled(actionUndo->isEnabled());
    }
}

void MainWindow::newTab(bool init)
{
    ViewTab *viewtab = new ViewTab(this);
    tabw_main->addTab(viewtab, QString());
    m_tab = viewtab;
    m_tab->connectSlots(this);

    if (init) {
        if (cb_table_edit->count()) {
            MTDictionary tables;
            for (int i = 0; i < cb_table_edit->count(); ++i)
                tables.setValue(cb_table_edit->itemData(i).toString(), cb_table_edit->itemText(i));
            emit tablesChanged(tables);
        }

        m_tab->setView(View::Store);
        tabw_main->setCurrentWidget(m_tab);
    }
}

void MainWindow::closeTab()
{
    if (tabw_main->count() < 2)
        return;

    delete m_tab;
}

void MainWindow::closeTab(int index)
{
    if (tabw_main->count() < 2)
        return;

    delete tabw_main->widget(index);
}

void MainWindow::tabChanged(int index)
{
    if (index < 0) {
        m_tab = NULL;
    } else {
        m_tab = qobject_cast<ViewTab *>(tabw_main->widget(index));
        m_tab->refreshViewIfNeeded();
        enableTools();
    }
}

void MainWindow::tabTextChanged(QWidget *tab, const QString &text)
{
    tabw_main->setTabText(tabw_main->indexOf(tab), text);
}

bool MainWindow::isOperationPermitted(const QString &operation, const QString &record_owner)
{
    int permitted = DBInfo::isOperationPermitted(operation, record_owner);
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

bool MainWindow::canRemoveCircuit(const QString &customer_uuid, const QString &circuit_uuid)
{
    if (superuserModeEnabled())
        return true;

    MTSqlQuery query;
    query.prepare(QString("SELECT date FROM inspections"
                          " WHERE %1 AND ((refr_add_am IS NOT NULL AND CAST(refr_add_am AS NUMERIC) <> 0)"
                          " OR (refr_reco IS NOT NULL AND CAST(refr_reco AS NUMERIC) <> 0)) LIMIT 1")
                  .arg(circuit_uuid.isEmpty() ? "customer_uuid = :customer_uuid" : "customer_uuid = :customer_uuid AND circuit_uuid = :circuit_uuid"));
    query.bindValue(":customer_uuid", customer_uuid);
    if (!circuit_uuid.isEmpty())
        query.bindValue(":circuit_uuid", circuit_uuid);

    if (query.exec() && query.next()) {
        QMessageBox message(this);
        message.setWindowModality(Qt::WindowModal);
        message.setWindowFlags(message.windowFlags() | Qt::Sheet);
        message.setIcon(QMessageBox::Warning);
        message.setWindowTitle(circuit_uuid.isEmpty() ? tr("Remove customer - Leaklog") : tr("Remove circuit - Leaklog"));
        message.setText(circuit_uuid.isEmpty() ? tr("You cannot remove the selected customer.") : tr("You cannot remove the selected circuit."));
        message.setInformativeText(circuit_uuid.isEmpty() ? tr("Removing this customer would affect the store.") : tr("Removing this circuit would affect the store."));
        message.addButton(tr("OK"), QMessageBox::AcceptRole);
        message.exec();
        return false;
    }

    return true;
}

bool MainWindow::isRecordLocked(const QString &date)
{
    if (DBInfo::isRecordLocked(date)) {
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

void MainWindow::editRefrigerants()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!isOperationPermitted("edit_service_company")) { return; }

    QDialog d(this);
    d.setWindowTitle(tr("Edit refrigerants - Leaklog"));
    d.setWindowModality(Qt::WindowModal);
    d.setWindowFlags(d.windowFlags() | Qt::Sheet);
    QGridLayout *gl = new QGridLayout(&d);

    QList<EditDialogueTableCell *> cells;
    cells << new EditDialogueTableCell(tr("Refrigerant"), "name", Global::String);
    cells << new EditDialogueTableCell(tr("GWP"), "gwp", Global::Numeric);

    EditDialogueBasicTable *table = new EditDialogueBasicTable(tr("Refrigerants"), cells, this);
    gl->addWidget(table, 0, 0);

    QSet<QString> predefined_refrigerants = refrigerantSet(false);
    QSet<QString> used_refrigerants;
    MTSqlQuery query("SELECT refrigerant FROM circuits UNION SELECT refrigerant FROM repairs UNION SELECT refrigerant FROM refrigerant_management");
    while (query.next()) {
        used_refrigerants << query.value(0).toString();
    }

    QList<QVariantMap> refrigerants = DBInfo::refrigerants();
    foreach (const QVariantMap &refrigerant, refrigerants) {
        QString refrigerant_name = refrigerant.value("name").toString();
        bool editable = predefined_refrigerants.contains(refrigerant_name) || !used_refrigerants.contains(refrigerant_name);

        QMap<QString, EditDialogueTableCell *> row;
        row.insert("name", new EditDialogueTableCell(refrigerant_name, "name", editable ? Global::String : -1));
        row.insert("gwp", new EditDialogueTableCell(refrigerant.value("gwp"), "gwp", Global::Numeric));

        table->addRow(row, true, editable ? EditDialogueTable::Removable : EditDialogueTable::Default);
    }

    QDialogButtonBox *bb = new QDialogButtonBox(&d);
    bb->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    bb->button(QDialogButtonBox::Ok)->setText(tr("Save"));
    bb->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));
    bb->button(QDialogButtonBox::Cancel)->setFocus();
    QObject::connect(bb, SIGNAL(accepted()), &d, SLOT(accept()));
    QObject::connect(bb, SIGNAL(rejected()), &d, SLOT(reject()));
    gl->addWidget(bb, 1, 0);

    if (d.exec() != QDialog::Accepted)
        return;

    refrigerants.clear();

    QList<QVariantMap> updated_refrigerants = table->allValues();
    foreach (const QVariantMap &updated_refrigerant, updated_refrigerants) {
        QVariantMap refrigerant;
        refrigerant.insert("name", updated_refrigerant.value("name"));
        refrigerant.insert("gwp", updated_refrigerant.value("gwp"));
        refrigerants << refrigerant;
    }

    DBInfo::setRefrigerants(refrigerants);
    setDatabaseModified(true);
    refreshView();
}

void MainWindow::loadServiceCompanies()
{
    if (!QSqlDatabase::database().isOpen()) { return; }

    emit serviceCompaniesChanged();
}

void MainWindow::addServiceCompany()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!isOperationPermitted("add_service_company")) { return; }
    ServiceCompany record("");
    UndoCommand command(m_undo_stack, tr("Add service company"));
    EditDialogue md(&record, m_undo_stack, this);
    if (md.exec() == QDialog::Accepted) {
        setDatabaseModified(true);
        loadServiceCompanies();
        refreshView();
    }
}

void MainWindow::editServiceCompany(const QString &uuid)
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (uuid.isEmpty() && !m_tab->isServiceCompanySelected()) { return; }
    if (!isOperationPermitted("edit_service_company")) { return; }
    ServiceCompany record(uuid.isEmpty() ? m_tab->selectedServiceCompanyUUID() : uuid);
    UndoCommand command(m_undo_stack, tr("Edit service company"));
    EditDialogue md(&record, m_undo_stack, this);
    if (md.exec() == QDialog::Accepted) {
        setDatabaseModified(true);
        loadServiceCompanies();
        refreshView();
    }
}

void MainWindow::removeServiceCompany()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!m_tab->isServiceCompanySelected()) { return; }
    if (!isOperationPermitted("remove_service_company")) { return; }

    if (Circuit::query({{"service_company_uuid", m_tab->selectedServiceCompanyUUID()}}).exists() ||
        Repair::query({{"service_company_uuid", m_tab->selectedServiceCompanyUUID()}}).exists() ||
        RefrigerantRecord::query({{"service_company_uuid", m_tab->selectedServiceCompanyUUID()}}).exists() ||
        Inspector::query({{"service_company_uuid", m_tab->selectedServiceCompanyUUID()}}).exists()) {
        QMessageBox message(this);
        message.setWindowModality(Qt::WindowModal);
        message.setWindowFlags(message.windowFlags() | Qt::Sheet);
        message.setIcon(QMessageBox::Warning);
        message.setWindowTitle(tr("Remove service company - Leaklog"));
        message.setText(tr("You cannot remove the selected service company."));
        message.setInformativeText(tr("Removing this service company would affect the store."));
        message.addButton(tr("OK"), QMessageBox::AcceptRole);
        message.exec();
        return;
    }

    ServiceCompany record(m_tab->selectedServiceCompanyUUID());

    if (RemoveDialogue::confirm(this, tr("Remove service company - Leaklog"),
                                tr("Are you sure you want to remove the selected service company?\nTo remove all data about the service company \"%1\" type REMOVE and confirm:")
                                .arg(record.name())) != QDialog::Accepted)
        return;

    QString name = record.name();
    UndoCommand command(m_undo_stack, tr("Remove service company %1%2")
                        .arg(record.companyID())
                        .arg(name.isEmpty() ? QString() : QString(" (%1)").arg(name)));
    m_undo_stack->savepoint();

    record.remove();

    loadServiceCompanies();
    enableTools();
    setDatabaseModified(true);
    refreshView();
}

void MainWindow::addRefrigerantRecord()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!isOperationPermitted("add_refrigerant_management")) { return; }
    editRefrigerantRecord("");
}

void MainWindow::editRefrigerantRecord(const QString &uuid)
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    RefrigerantRecord record(uuid);
    bool exists = record.exists();
    if (exists && isRecordLocked(record.date())) { return; }
    if (!isOperationPermitted("edit_refrigerant_management", record.updatedBy())) { return; }
    if (!exists) {
        if (m_tab->isServiceCompanySelected()) {
            record.setValue("service_company_uuid", m_tab->selectedServiceCompanyUUID());
        }
    }
    UndoCommand command(m_undo_stack, uuid.isEmpty()
                        ? tr("Add record of refrigerant management")
                        : tr("Edit record of refrigerant management %1").arg(m_settings.formatDateTime(record.date())));
    EditDialogue md(&record, m_undo_stack, this);
    if (md.exec() == QDialog::Accepted) {
        if (record.purchased() <= 0.0 && record.purchasedRecovered() <= 0.0 &&
            record.sold() <= 0.0 && record.soldRecovered() <= 0.0 &&
            record.regenerated() <= 0.0 && record.disposedOf() <= 0.0 &&
            record.leaked() <= 0.0 && record.leakedRecovered() <= 0.0) {
            record.remove();
        }
        setDatabaseModified(true);
        refreshView();
    }
}

void MainWindow::addCustomer()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!isOperationPermitted("add_customer")) { return; }
    Customer record;
    UndoCommand command(m_undo_stack, tr("Add customer"));
    EditCustomerDialogue md(&record, m_undo_stack, this);
    if (md.exec() == QDialog::Accepted) {
        setDatabaseModified(true);
        m_tab->loadCustomer(record.uuid(), true);
    }
}

void MainWindow::editCustomer()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!m_tab->isCustomerSelected()) { return; }
    Customer record(m_tab->selectedCustomerUUID());
    if (!isOperationPermitted("edit_customer", record.updatedBy())) { return; }
    QString old_company_name = record.companyName();
    UndoCommand command(m_undo_stack, tr("Edit customer %1%2")
                        .arg(record.companyID())
                        .arg(old_company_name.isEmpty() ? QString() : QString(" (%1)").arg(old_company_name)));
    EditCustomerDialogue md(&record, m_undo_stack, this);
    if (md.exec() == QDialog::Accepted) {
        QString company_name = record.companyName();
        if (old_company_name != company_name) {
            auto repairs = record.repairs().all();
            foreach (auto repair, repairs) {
                repair.setCustomer(company_name);
                repair.save();
            }
            enableTools();
        } else {
            enableTools();
        }
        refreshView();
        setDatabaseModified(true);
    }
}

void MainWindow::duplicateCustomer()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!m_tab->isCustomerSelected()) { return; }
    if (!isOperationPermitted("add_customer")) { return; }
    Customer record(m_tab->selectedCustomerUUID());
    record.duplicate();
    QString company_name = record.companyName();
    UndoCommand command(m_undo_stack, tr("Duplicate customer %1%2")
                        .arg(record.companyID())
                        .arg(company_name.isEmpty() ? QString() : QString(" (%1)").arg(company_name)));
    EditDialogue md(&record, m_undo_stack, this);
    if (md.exec() == QDialog::Accepted) {
        setDatabaseModified(true);
        m_tab->loadCustomer(record.uuid(), true);
    }
}

void MainWindow::removeCustomer()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!m_tab->isCustomerSelected()) { return; }
    if (!canRemoveCircuit(m_tab->selectedCustomerUUID())) { return; }
    Customer record(m_tab->selectedCustomerUUID());
    if (!isOperationPermitted("remove_customer", record.updatedBy())) { return; }
    if (RemoveDialogue::confirm(this, tr("Remove customer - Leaklog"),
                                tr("Are you sure you want to remove the selected customer?\nTo remove all data about the customer \"%1\" type REMOVE and confirm:")
                                .arg(record.companyID())) != QDialog::Accepted)
        return;

    QString company_name = record.companyName();
    UndoCommand command(m_undo_stack, tr("Remove customer %1%2")
                        .arg(record.companyID())
                        .arg(company_name.isEmpty() ? QString() : QString(" (%1)").arg(company_name)));
    m_undo_stack->savepoint();

    record.remove();

    m_tab->clearSelectedCustomer();
    enableTools();
    setDatabaseModified(true);
    m_tab->setView(View::Customers);
}

void MainWindow::starCustomer(const QString &uuid)
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    Customer record(uuid);
    if (!isOperationPermitted("edit_customer", record.updatedBy())) { return; }
    QString company_name = record.companyName();
    UndoCommand command(m_undo_stack, tr("Edit customer %1%2")
                        .arg(record.companyID())
                        .arg(company_name.isEmpty() ? QString() : QString(" (%1)").arg(company_name)));
    record.setStarred(!record.isStarred());
    record.save();
    refreshView();
    setDatabaseModified(true);
}

void MainWindow::decommissionAllCircuits()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!m_tab->isCustomerSelected()) { return; }
    if (!isOperationPermitted("decommission_circuit")) { return; }

    Customer customer(m_tab->selectedCustomerUUID());

    QDialog d(this);
    d.setWindowTitle(tr("Decommission all circuits - Leaklog"));
    QGridLayout *gl = new QGridLayout(&d);

    QLabel *lbl = new QLabel(tr("Decommission all circuits of:"), &d);
    lbl->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    gl->addWidget(lbl, 0, 0);

    gl->addWidget(new QLabel(customer.companyName(), &d), 0, 1);

    QCheckBox *exclude_from_agenda = new QCheckBox(tr("Exclude from Agenda only"), &d);
    gl->addWidget(exclude_from_agenda, 1, 1);

    lbl = new QLabel(tr("%1:").arg(Circuit::attributes().value("decommissioning")), &d);
    lbl->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    gl->addWidget(lbl, 2, 0);

    QDateEdit *date = new QDateEdit(&d);
    date->setDisplayFormat(m_settings.dateFormatString());
    date->setDate(QDate::currentDate());
    date->setCalendarPopup(true);
    date->calendarWidget()->setLocale(QLocale());
    date->calendarWidget()->setFirstDayOfWeek(QLocale().firstDayOfWeek());
    gl->addWidget(date, 2, 1);

    lbl = new QLabel(tr("%1:").arg(Circuit::attributes().value("decommissioning_reason")), &d);
    lbl->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    gl->addWidget(lbl, 3, 0);

    QLineEdit *decommissioning_reason = new QLineEdit(&d);
    gl->addWidget(decommissioning_reason, 3, 1);

    QDialogButtonBox *bb = new QDialogButtonBox(&d);
    bb->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    bb->button(QDialogButtonBox::Ok)->setText(tr("Decommission"));
    bb->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));
    bb->button(QDialogButtonBox::Cancel)->setFocus();
    QObject::connect(bb, SIGNAL(accepted()), &d, SLOT(accept()));
    QObject::connect(bb, SIGNAL(rejected()), &d, SLOT(reject()));
    gl->addWidget(bb, 4, 0, 1, 2);

    if (d.exec() != QDialog::Accepted) return;

    QString company_name = customer.companyName();
    UndoCommand command(m_undo_stack, tr("Decommission all circuits of customer %1%2")
                        .arg(customer.companyID())
                        .arg(company_name.isEmpty() ? QString() : QString(" (%1)").arg(company_name)));
    m_undo_stack->savepoint();

    QString decommissioning = date->date().toString(DATE_FORMAT);

    auto circuits = customer.circuits().where(QString("disused = %1").arg(Circuit::Commissioned));
    foreach (auto circuit, circuits) {
        circuit.setStatus(exclude_from_agenda->isChecked() ? Circuit::ExcludedFromAgenda : Circuit::Decommissioned);
        circuit.setDateOfDecommissioning(decommissioning);
        circuit.setReasonForDecommissioning(decommissioning_reason->text());
        circuit.save();
    }

    setDatabaseModified(true);
    refreshView();
}

void MainWindow::addCircuit()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!m_tab->isCustomerSelected()) { return; }
    if (!isOperationPermitted("add_circuit")) { return; }
    Circuit record;
    record.setCustomerUUID(m_tab->selectedCustomerUUID());
    if (m_tab->isServiceCompanySelected()) {
        record.setValue("service_company_uuid", m_tab->selectedServiceCompanyUUID());
    }
    UndoCommand command(m_undo_stack, tr("Add circuit"));
    EditCircuitDialogue md(&record, m_undo_stack, this);
    if (md.exec() == QDialog::Accepted) {
        setDatabaseModified(true);
        m_tab->loadCircuit(record.uuid(), true);
    }
}

void MainWindow::editCircuit()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!m_tab->isCustomerSelected()) { return; }
    if (!m_tab->isCircuitSelected()) { return; }
    Circuit record(m_tab->selectedCircuitUUID());
    if (!isOperationPermitted("edit_circuit", record.updatedBy())) { return; }
    Customer customer(m_tab->selectedCustomerUUID());
    QString company_name = customer.companyName();
    UndoCommand command(m_undo_stack, tr("Edit circuit %1 (%2)")
                        .arg(record.circuitID())
                        .arg(company_name.isEmpty() ? customer.companyID() : company_name));
    EditCircuitDialogue md(&record, m_undo_stack, this);
    if (md.exec() == QDialog::Accepted) {
        enableTools();
        refreshView();
        setDatabaseModified(true);
    }
}

void MainWindow::duplicateCircuit()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!m_tab->isCustomerSelected()) { return; }
    if (!m_tab->isCircuitSelected()) { return; }
    if (!isOperationPermitted("add_circuit")) { return; }
    Circuit record(m_tab->selectedCircuitUUID());
    record.duplicate();
    Customer customer(m_tab->selectedCustomerUUID());
    QString company_name = customer.companyName();
    UndoCommand command(m_undo_stack, tr("Duplicate circuit %1 (%2)")
                        .arg(record.circuitID())
                        .arg(company_name.isEmpty() ? customer.companyID() : company_name));
    EditDialogue md(&record, m_undo_stack, this);
    if (md.exec() == QDialog::Accepted) {
        ListOfVariantMaps compressors = Compressor::query({{"circuit_uuid", m_tab->selectedCircuitUUID()}}).listAll();

        for (int i = 0; i < compressors.size(); ++i) {
            compressors[i].insert("uuid", createUUID());
            compressors[i].insert("circuit_uuid", record.uuid());
            Compressor().update(compressors[i]);
        }

        ListOfVariantMaps circuit_units = CircuitUnit::query({{"circuit_uuid", m_tab->selectedCircuitUUID()}}).listAll();

        for (int i = 0; i < circuit_units.size(); ++i) {
            circuit_units[i].insert("uuid", createUUID());
            circuit_units[i].insert("circuit_uuid", record.uuid());
            CircuitUnit().update(circuit_units[i]);
        }

        setDatabaseModified(true);
        m_tab->loadCircuit(record.uuid(), true);
    }
}

void MainWindow::duplicateAndDecommissionCircuit()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!m_tab->isCustomerSelected()) { return; }
    if (!m_tab->isCircuitSelected()) { return; }
    if (!isOperationPermitted("add_circuit")) { return; }
    if (!isOperationPermitted("decommission_circuit")) { return; }

    Circuit circuit(m_tab->selectedCircuitUUID());
    QVariantMap attributes = circuit.savedValues();

    QDialog d(this);
    d.setWindowTitle(tr("Duplicate and decommission - Leaklog"));
    QGridLayout *gl = new QGridLayout(&d);

    QLabel *lbl = new QLabel(tr("Duplicate and decommission circuit:"), &d);
    lbl->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    gl->addWidget(lbl, 0, 0);

    gl->addWidget(new QLabel(circuit.circuitID(), &d), 0, 1);

    lbl = new QLabel(tr("%1:").arg(Circuit::attributes().value("decommissioning")), &d);
    lbl->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    gl->addWidget(lbl, 1, 0);

    QDateEdit *date = new QDateEdit(&d);
    date->setDisplayFormat(m_settings.dateFormatString());
    date->setDate(QDate::currentDate());
    date->setCalendarPopup(true);
    date->calendarWidget()->setLocale(QLocale());
    date->calendarWidget()->setFirstDayOfWeek(QLocale().firstDayOfWeek());
    gl->addWidget(date, 1, 1);

    QRadioButton *keep_id = new QRadioButton(tr("Keep ID"), &d);
    keep_id->setChecked(true);
    gl->addWidget(keep_id, 2, 0);

    QRadioButton *set_original_id = new QRadioButton(tr("Change ID of the original to:"), &d);
    gl->addWidget(set_original_id, 3, 0);

    QSpinBox *new_id = new QSpinBox(&d);
    new_id->setRange(1, 99999);
    new_id->setValue((int)Circuit::query({{"customer_uuid", m_tab->selectedCustomerUUID()}}).max("id") + 1);
    gl->addWidget(new_id, 3, 1, 2, 1);

    QRadioButton *set_duplicate_id = new QRadioButton(tr("Choose a new ID for the duplicate:"), &d);
    gl->addWidget(set_duplicate_id, 4, 0);

    QStringList refrigerants = listRefrigerants();

    lbl = new QLabel(tr("Previous refrigerant:"), &d);
    lbl->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    gl->addWidget(lbl, 5, 0);

    QComboBox *old_refrigerant = new QComboBox(&d);
    old_refrigerant->addItems(refrigerants);
    old_refrigerant->setCurrentIndex(refrigerants.indexOf(circuit.refrigerant()));
    gl->addWidget(old_refrigerant, 5, 1);

    lbl = new QLabel(tr("New refrigerant:"), &d);
    lbl->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    gl->addWidget(lbl, 6, 0);

    QComboBox *new_refrigerant = new QComboBox(&d);
    new_refrigerant->addItems(refrigerants);
    new_refrigerant->setCurrentIndex(refrigerants.indexOf(circuit.refrigerant()));
    gl->addWidget(new_refrigerant, 6, 1);

    lbl = new QLabel(QApplication::translate("Circuit", "This ID is not available. Please choose a different ID."), &d);
    QFont bold;
    bold.setBold(true);
    lbl->setFont(bold);
    lbl->setWordWrap(true);
    lbl->setAlignment(Qt::AlignCenter);
    lbl->setVisible(false);
    gl->addWidget(lbl, 7, 0, 1, 2);

    QDialogButtonBox *bb = new QDialogButtonBox(&d);
    bb->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    bb->button(QDialogButtonBox::Ok)->setText(tr("Duplicate and Decommission"));
    bb->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));
    bb->button(QDialogButtonBox::Cancel)->setFocus();
    QObject::connect(bb, SIGNAL(accepted()), &d, SLOT(accept()));
    QObject::connect(bb, SIGNAL(rejected()), &d, SLOT(reject()));
    gl->addWidget(bb, 8, 0, 1, 2);

    int id = 0;
    do {
        if (id)
            lbl->setVisible(true);

        if (d.exec() != QDialog::Accepted)
            return;

        if (keep_id->isChecked()) {
            id = circuit.intValue("id");
            break;
        }

        id = new_id->value();
    } while (Circuit::query({{"customer_uuid", m_tab->selectedCustomerUUID()}, {"id", QString::number(id)}}).exists());

    Customer customer(m_tab->selectedCustomerUUID());
    QString company_name = customer.companyName();
    UndoCommand command(m_undo_stack, tr("Duplicate and decommission circuit %1 (%2)")
                        .arg(circuit.circuitID())
                        .arg(company_name.isEmpty() ? customer.companyID() : company_name));
    m_undo_stack->savepoint();

    QString date_string = date->date().toString(DATE_FORMAT);

    circuit.setStatus(Circuit::Decommissioned);
    circuit.setDateOfDecommissioning(date_string);
    circuit.setRefrigerant(old_refrigerant->currentText());

    if (set_original_id->isChecked()) {
        circuit.setCircuitID(id);
    } else {
        attributes.insert("id", id);
    }

    circuit.save();

    attributes.insert("refrigerant", new_refrigerant->currentText());
    attributes.insert("refrigerant_amount", 0.0);
    attributes.insert("disused", Circuit::Commissioned);
    attributes.insert("commissioning", date_string);
    attributes.remove("decommissioning");
    attributes.insert("uuid", createUUID());
    Circuit duplicate;
    duplicate.update(attributes);

    ListOfVariantMaps compressors = Compressor::query({{"circuit_uuid", m_tab->selectedCircuitUUID()}}).listAll();
    for (int i = 0; i < compressors.size(); ++i) {
        compressors[i].insert("uuid", createUUID());
        compressors[i].insert("circuit_uuid", duplicate.uuid());
        Compressor().update(compressors[i]);
    }

    ListOfVariantMaps circuit_units = CircuitUnit::query({{"circuit_uuid", m_tab->selectedCircuitUUID()}}).listAll();
    for (int i = 0; i < circuit_units.size(); ++i) {
        circuit_units[i].insert("uuid", createUUID());
        circuit_units[i].insert("circuit_uuid", duplicate.uuid());
        CircuitUnit().update(circuit_units[i]);
    }

    setDatabaseModified(true);
    m_tab->loadCircuit(duplicate.uuid(), true);
}

void MainWindow::moveCircuit()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!m_tab->isCustomerSelected()) { return; }
    if (!m_tab->isCircuitSelected()) { return; }
    if (!isOperationPermitted("add_circuit")) { return; }

    Circuit circuit(m_tab->selectedCircuitUUID());
    QVariantMap attributes = circuit.savedValues();

    if (!isOperationPermitted("remove_circuit", circuit.updatedBy())) { return; }

    QDialog d(this);
    d.setWindowTitle(tr("Move circuit to another customer - Leaklog"));
    QGridLayout *gl = new QGridLayout(&d);

    QLabel *lbl = new QLabel(tr("Move circuit:"), &d);
    lbl->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    gl->addWidget(lbl, 0, 0);

    gl->addWidget(new QLabel(circuit.circuitID(), &d), 0, 1);

    lbl = new QLabel(tr("To customer:"), &d);
    lbl->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    gl->addWidget(lbl, 1, 0);

    QComboBox *cb_customer = new QComboBox(&d);

    QString selected_customer_uuid = m_tab->selectedCustomerUUID();
    ListOfVariantMaps customers = Customer::query().listAll("uuid, company", "company");
    foreach (const QVariantMap &customer, customers) {
        QVariant uuid = customer.value("uuid");
        if (uuid.toString() != selected_customer_uuid)
            cb_customer->addItem(customer.value("company").toString(), uuid);
    }

    QObject::connect(cb_customer, SIGNAL(currentIndexChanged(int)), this, SLOT(customerChangedInMoveCircuitDialogue(int)));
    gl->addWidget(cb_customer, 1, 1);

    QLabel *lbl_select_customer = new QLabel(QApplication::translate("EditDialogue", "Select a customer first."), &d);
    QFont bold;
    bold.setBold(true);
    lbl_select_customer->setFont(bold);
    lbl_select_customer->setWordWrap(true);
    lbl_select_customer->setAlignment(Qt::AlignCenter);
    lbl_select_customer->setVisible(false);
    gl->addWidget(lbl_select_customer, 2, 0, 1, 2);

    lbl = new QLabel(tr("Change circuit ID to:"), &d);
    lbl->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    gl->addWidget(lbl, 3, 0);

    QSpinBox *spb_circuit_id = new QSpinBox(&d);
    spb_circuit_id->setObjectName("spb_circuit_id");
    spb_circuit_id->setRange(1, 99999);
    spb_circuit_id->setValue(circuit.circuitID().toInt());
    gl->addWidget(spb_circuit_id, 3, 1);

    QLabel *lbl_id_taken = new QLabel(QApplication::translate("Circuit", "This ID is not available. Please choose a different ID."), &d);
    lbl_id_taken->setFont(bold);
    lbl_id_taken->setWordWrap(true);
    lbl_id_taken->setAlignment(Qt::AlignCenter);
    lbl_id_taken->setVisible(false);
    gl->addWidget(lbl_id_taken, 4, 0, 1, 2);

    lbl = new QLabel(tr("Date:"), &d);
    lbl->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    gl->addWidget(lbl, 5, 0);

    QDateTimeEdit *date = new QDateTimeEdit(&d);
    date->setDisplayFormat(m_settings.dateTimeFormatString());
    date->setDateTime(QDateTime::currentDateTime());
    date->setCalendarPopup(true);
    date->calendarWidget()->setLocale(QLocale());
    date->calendarWidget()->setFirstDayOfWeek(QLocale().firstDayOfWeek());
    gl->addWidget(date, 5, 1);

    QDialogButtonBox *bb = new QDialogButtonBox(&d);
    bb->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    bb->button(QDialogButtonBox::Ok)->setText(tr("Move"));
    bb->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));
    bb->button(QDialogButtonBox::Cancel)->setFocus();
    QObject::connect(bb, SIGNAL(accepted()), &d, SLOT(accept()));
    QObject::connect(bb, SIGNAL(rejected()), &d, SLOT(reject()));
    gl->addWidget(bb, 6, 0, 1, 2);

    cb_customer->setCurrentIndex(-1);

    QString customer_uuid = "";
    int circuit_id = 0;
    QString inspection_date;
    do {
        lbl_id_taken->setVisible(!customer_uuid.isEmpty() && circuit_id != 0);
        lbl_select_customer->setVisible(customer_uuid.isNull());

        if (d.exec() != QDialog::Accepted)
            return;

        customer_uuid = cb_customer->currentData().toString();
        circuit_id = spb_circuit_id->value();
        inspection_date = date->dateTime().toString(DATE_TIME_FORMAT);
    } while (customer_uuid.isEmpty() || Circuit::query({{"customer_uuid", customer_uuid}, {"id", QString::number(circuit_id)}}).exists());

    Customer customer(selected_customer_uuid);
    QString company_name = customer.companyName();
    Customer new_customer(customer_uuid);
    QString new_company_name = new_customer.companyName();
    UndoCommand command(m_undo_stack, tr("Move circuit %1 (%2) to customer %3")
                        .arg(circuit.circuitID())
                        .arg(company_name.isEmpty() ? customer.companyID() : company_name)
                        .arg(new_company_name.isEmpty() ? new_customer.companyID() : new_company_name));
    m_undo_stack->savepoint();

    circuit.setCustomerUUID(customer_uuid);
    circuit.setCircuitID(circuit_id);
    circuit.save();

    circuit.inspections().each([=](Inspection &inspection) {
        inspection.setCustomerUUID(customer_uuid);
        inspection.save();
    });

    QVariantMap inspection;
    inspection.insert("customer_uuid", customer_uuid);
    inspection.insert("circuit_uuid", circuit.uuid());
    inspection.insert("date", inspection_date);
    inspection.insert("inspection_type", Inspection::CircuitMoved);
    inspection.insert("outside_interval", 1);
    QStringList data;
    data << selected_customer_uuid;
    data << customer_uuid;
    data << company_name.remove(UNIT_SEPARATOR);
    data << new_company_name.remove(UNIT_SEPARATOR);
    inspection.insert("inspection_type_data", data.join(UNIT_SEPARATOR));
    inspection.insert("rmds", QApplication::translate("Inspection", "Circuit moved from customer %1 to %2.")
                      .arg(QString("%1 (%2)").arg(company_name).arg(customer.companyID()))
                      .arg(QString("%1 (%2)").arg(new_company_name).arg(new_customer.companyID()))
                      .append("\n\n")
                      .append(tr("DO NOT EDIT THIS INSPECTION: If you can read this message, you are using an older version of Leaklog than the one used to create this inspection. Changes made to this inspection will not be visible in newer versions of Leaklog.")));
    Inspection().update(inspection);

    setDatabaseModified(true);
    m_tab->setSelectedCustomerUUID(customer_uuid);
    m_tab->loadCircuit(circuit.uuid(), true);
}

void MainWindow::customerChangedInMoveCircuitDialogue(int customer_index)
{
    QComboBox *cb_customer = qobject_cast<QComboBox *>(sender());
    if (!cb_customer)
        return;

    QDialog *d = qobject_cast<QDialog *>(cb_customer->parent());
    if (!d)
        return;

    QSpinBox *spb_circuit_id = d->findChild<QSpinBox *>("spb_circuit_id");
    if (!spb_circuit_id)
        return;

    QString customer_uuid = customer_index < 0 ? QString() : cb_customer->itemData(customer_index).toString();
    QString circuit_id = QString::number(spb_circuit_id->value());

    if (!customer_uuid.isEmpty() && Circuit::query({{"customer_uuid", customer_uuid}, {"id", circuit_id}}).exists()) {
        spb_circuit_id->setValue((int)Circuit::query({{"customer_uuid", customer_uuid}}).max("id") + 1);
    }
}

void MainWindow::removeCircuit()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!m_tab->isCustomerSelected()) { return; }
    if (!m_tab->isCircuitSelected()) { return; }
    if (!canRemoveCircuit(m_tab->selectedCustomerUUID(), m_tab->selectedCircuitUUID())) { return; }
    Circuit record(m_tab->selectedCircuitUUID());
    if (!isOperationPermitted("remove_circuit", record.updatedBy())) { return; }
    if (RemoveDialogue::confirm(this, tr("Remove circuit - Leaklog"),
                                tr("Are you sure you want to remove the selected circuit?\nTo remove all data about the circuit \"%1\" type REMOVE and confirm:")
                                .arg(record.circuitID())) != QDialog::Accepted)
        return;

    Customer customer(m_tab->selectedCustomerUUID());
    QString company_name = customer.companyName();
    UndoCommand command(m_undo_stack, tr("Remove circuit %1 (%2)")
                        .arg(record.circuitID())
                        .arg(company_name.isEmpty() ? customer.companyID() : company_name));
    m_undo_stack->savepoint();

    record.remove();

    m_tab->clearSelectedCircuit();
    enableTools();
    setDatabaseModified(true);
    m_tab->setView(View::Circuits);
}

void MainWindow::starCircuit(const QString &customer_uuid, const QString &uuid)
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    Circuit record(uuid);
    if (!isOperationPermitted("edit_circuit", record.updatedBy())) { return; }
    Customer customer(customer_uuid);
    QString company_name = customer.companyName();
    UndoCommand command(m_undo_stack, tr("Edit circuit %1 (%2)")
                        .arg(record.circuitID())
                        .arg(company_name.isEmpty() ? customer.companyID() : company_name));
    record.setStarred(!record.isStarred());
    record.save();
    refreshView();
    setDatabaseModified(true);
}

void MainWindow::addInspection()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!m_tab->isCustomerSelected()) { return; }
    if (!m_tab->isCircuitSelected()) { return; }
    if (!isOperationPermitted("add_inspection")) { return; }
    Inspection record;
    record.setValue("customer_uuid", m_tab->selectedCustomerUUID());
    record.setValue("circuit_uuid", m_tab->selectedCircuitUUID());
    if (m_tab->isInspectorSelected()) {
        record.setValue("inspector_uuid", m_tab->selectedInspectorUUID());
    }
    UndoCommand command(m_undo_stack, tr("Add inspection"));
    EditInspectionDialogue md(&record, m_undo_stack, this);
    if (md.exec() == QDialog::Accepted) {
        setDatabaseModified(true);
        m_tab->loadInspection(record.uuid(), true);
    }
}

void MainWindow::editInspection()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!m_tab->isCustomerSelected()) { return; }
    if (!m_tab->isCircuitSelected()) { return; }
    if (!m_tab->isInspectionSelected()) { return; }
    Inspection record(m_tab->selectedInspectionUUID());
    if (!isOperationPermitted("edit_inspection", record.updatedBy())) { return; }
    if (isRecordLocked(record.date())) { return; }
    Customer customer(m_tab->selectedCustomerUUID());
    QString company_name = customer.companyName();
    UndoCommand command(m_undo_stack, tr("Edit inspection %1 (%2, circuit %3)")
                        .arg(m_settings.formatDateTime(record.date()))
                        .arg(company_name.isEmpty() ? customer.companyID() : company_name)
                        .arg(record.circuit().circuitID()));
    EditInspectionDialogue md(&record, m_undo_stack, this);
    if (md.exec() == QDialog::Accepted) {
        setDatabaseModified(true);
        m_tab->loadInspection(record.uuid(), false);
        refreshView();
    }
}

void MainWindow::duplicateInspection()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!m_tab->isCustomerSelected()) { return; }
    if (!m_tab->isCircuitSelected()) { return; }
    if (!m_tab->isInspectionSelected()) { return; }
    if (!isOperationPermitted("add_inspection")) { return; }
    Inspection record(m_tab->selectedInspectionUUID());
    record.duplicate();
    Customer customer(m_tab->selectedCustomerUUID());
    QString company_name = customer.companyName();
    UndoCommand command(m_undo_stack, tr("Duplicate inspection %1 (%2, circuit %3)")
                        .arg(m_settings.formatDateTime(record.date()))
                        .arg(company_name.isEmpty() ? customer.companyID() : company_name)
                        .arg(record.circuit().circuitID()));
    EditInspectionDialogue md(&record, m_undo_stack, this, m_tab->selectedInspectionUUID());
    if (md.exec() == QDialog::Accepted) {
        setDatabaseModified(true);
        m_tab->loadInspection(record.uuid(), true);
    }
}

void MainWindow::removeInspection(const QString &uuid)
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!m_tab->isCustomerSelected()) { return; }
    if (!m_tab->isCircuitSelected()) { return; }
    QString inspection_uuid = uuid;
    if (inspection_uuid.isEmpty()) {
        if (!m_tab->isInspectionSelected()) { return; }

        inspection_uuid = m_tab->selectedInspectionUUID();
    }
    Inspection record(inspection_uuid);
    if (!isOperationPermitted("remove_inspection", record.updatedBy())) { return; }
    if (isRecordLocked(record.date())) { return; }
    if (RemoveDialogue::confirm(this, tr("Remove inspection - Leaklog"),
                                tr("Are you sure you want to remove the selected inspection?\nTo remove all data about the inspection \"%1\" type REMOVE and confirm:")
                                .arg(m_settings.formatDateTime(record.date()))) != QDialog::Accepted)
        return;

    Customer customer(m_tab->selectedCustomerUUID());
    QString company_name = customer.companyName();
    UndoCommand command(m_undo_stack, tr("Remove inspection %1 (%2, circuit %3)")
                        .arg(m_settings.formatDateTime(record.date()))
                        .arg(company_name.isEmpty() ? customer.companyID() : company_name)
                        .arg(record.circuit().circuitID()));
    m_undo_stack->savepoint();

    record.remove();
    if (m_tab->selectedInspectionUUID() == inspection_uuid)
        m_tab->clearSelectedInspection();
    enableTools();
    setDatabaseModified(true);
    m_tab->setView(View::Inspections);
}

void MainWindow::skipInspection()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!m_tab->isCustomerSelected()) { return; }
    if (!m_tab->isCircuitSelected()) { return; }
    if (!isOperationPermitted("add_inspection")) { return; }

    QDateTime next_regular_inspection_date = QDateTime::currentDateTime();

    MTQuery circuit_record = Circuit::query({{"uuid", m_tab->selectedCircuitUUID()}});
    circuit_record.addJoin("LEFT JOIN (SELECT circuit_uuid, MAX(date) AS date FROM inspections"
                           " WHERE outside_interval = 0 GROUP BY circuit_uuid) AS ins"
                           " ON ins.circuit_uuid = circuits.uuid");
    MTSqlQuery circuit_query = circuit_record.select("circuits.hermetic, circuits.leak_detector, circuits.inspection_interval, "
                                                     "COALESCE(ins.date, circuits.commissioning) AS last_regular_inspection, "
                                                     "circuits.refrigerant, " + circuitRefrigerantAmountQuery());
    circuit_query.exec();
    if (circuit_query.next()) {
        int inspection_interval = Warnings::circuitInspectionInterval(circuit_query.stringValue("refrigerant"),
                                                                      circuit_query.doubleValue("refrigerant_amount"),
                                                                      m_tab->toolBarStack()->isCO2EquivalentChecked(),
                                                                      circuit_query.intValue("hermetic"),
                                                                      circuit_query.intValue("leak_detector"),
                                                                      circuit_query.intValue("inspection_interval"));
        if (inspection_interval) {
            QString last_regular_inspection_date = circuit_query.stringValue("last_regular_inspection");
            if (!last_regular_inspection_date.isEmpty()) {
                next_regular_inspection_date = QDateTime::fromString(last_regular_inspection_date.split("-").first(),
                                                                     DATE_FORMAT).addDays(inspection_interval);
            }
        }
    }

    Circuit circuit(m_tab->selectedCircuitUUID());

    QDialog d(this);
    d.setWindowTitle(tr("Skip inspection - Leaklog"));
    QGridLayout *gl = new QGridLayout(&d);

    QLabel *lbl = new QLabel(tr("Skip inspection of circuit:"), &d);
    lbl->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    gl->addWidget(lbl, 0, 0);

    gl->addWidget(new QLabel(circuit.circuitID(), &d), 0, 1);

    lbl = new QLabel(tr("Date of skipped inspection:"), &d);
    lbl->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    gl->addWidget(lbl, 1, 0);

    QDateTimeEdit *date = new QDateTimeEdit(&d);
    date->setDisplayFormat(m_settings.dateTimeFormatString());
    date->setDateTime(next_regular_inspection_date);
    date->setCalendarPopup(true);
    date->calendarWidget()->setLocale(QLocale());
    date->calendarWidget()->setFirstDayOfWeek(QLocale().firstDayOfWeek());
    gl->addWidget(date, 1, 1);

    lbl = new QLabel(tr("Reason:"), &d);
    lbl->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    gl->addWidget(lbl, 2, 0);

    QLineEdit *reason = new QLineEdit(&d);
    reason->setText(QApplication::translate("Inspection", "Inspection carried out by another service company."));
    reason->setMinimumWidth(360);
    gl->addWidget(reason, 2, 1);

    QDialogButtonBox *bb = new QDialogButtonBox(&d);
    bb->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    bb->button(QDialogButtonBox::Ok)->setText(tr("Skip"));
    bb->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));
    bb->button(QDialogButtonBox::Cancel)->setFocus();
    QObject::connect(bb, SIGNAL(accepted()), &d, SLOT(accept()));
    QObject::connect(bb, SIGNAL(rejected()), &d, SLOT(reject()));
    gl->addWidget(bb, 3, 0, 1, 2);

    if (d.exec() != QDialog::Accepted)
        return;

    QString inspection_date = date->dateTime().toString(DATE_TIME_FORMAT);

    Customer customer(m_tab->selectedCustomerUUID());
    QString company_name = customer.companyName();
    UndoCommand command(m_undo_stack, tr("Skip inspection of circuit %1 (%2)")
                        .arg(circuit.circuitID())
                        .arg(company_name.isEmpty() ? customer.companyID() : company_name));
    m_undo_stack->savepoint();

    QVariantMap inspection;
    inspection.insert("customer_uuid", m_tab->selectedCustomerUUID());
    inspection.insert("circuit_uuid", m_tab->selectedCircuitUUID());
    inspection.insert("date", inspection_date);
    inspection.insert("inspection_type", Inspection::SkippedInspection);
    inspection.insert("outside_interval", 0);
    QStringList data;
    data << reason->text().remove(UNIT_SEPARATOR);
    inspection.insert("inspection_type_data", data.join(UNIT_SEPARATOR));
    inspection.insert("rmds", reason->text().append("\n\n")
                      .append(tr("DO NOT EDIT THIS INSPECTION: If you can read this message, you are using an older version of Leaklog than the one used to create this inspection. Changes made to this inspection will not be visible in newer versions of Leaklog.")));
    Inspection().update(inspection);

    setDatabaseModified(true);
    m_tab->setView(View::Inspections);
}

void MainWindow::addRepair()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!isOperationPermitted("add_repair")) { return; }
    Repair record;
    if (m_tab->isCustomerSelected()) {
        record.setValue("customer_uuid", m_tab->selectedCustomerUUID());
        record.setValue("customer", Customer(m_tab->selectedCustomerUUID()).stringValue("company"));
    }
    if (m_tab->isInspectorSelected()) {
        record.setValue("inspector_uuid", m_tab->selectedInspectorUUID());
    }
    if (m_tab->isServiceCompanySelected()) {
        record.setValue("service_company_uuid", m_tab->selectedServiceCompanyUUID());
    }
    UndoCommand command(m_undo_stack, tr("Add repair"));
    EditDialogue md(&record, m_undo_stack, this);
    if (md.exec() == QDialog::Accepted) {
        setDatabaseModified(true);
        m_tab->loadRepair(record.uuid(), true);
    }
}

void MainWindow::editRepair()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!m_tab->isRepairSelected()) { return; }
    Repair record(m_tab->selectedRepairUUID());
    if (!isOperationPermitted("edit_repair", record.updatedBy())) { return; }
    if (isRecordLocked(record.date())) { return; }
    QString company_name = record.customer();
    UndoCommand command(m_undo_stack, tr("Edit repair %1%2")
                        .arg(m_settings.formatDateTime(record.date()))
                        .arg(company_name.isEmpty() ? "" : QString(" (%1)").arg(company_name)));
    EditDialogue md(&record, m_undo_stack, this);
    if (md.exec() == QDialog::Accepted) {
        setDatabaseModified(true);
        m_tab->loadRepair(record.uuid(), true);
    }
}

void MainWindow::duplicateRepair()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!m_tab->isRepairSelected()) { return; }
    if (!isOperationPermitted("add_repair")) { return; }
    Repair record(m_tab->selectedRepairUUID());
    record.duplicate();
    if (!record.customerUUID().isEmpty()) {
        record.setCustomerUUID(record.customerUUID());
    }
    QString company_name = record.customer();
    UndoCommand command(m_undo_stack, tr("Duplicate repair %1%2")
                        .arg(m_settings.formatDateTime(record.date()))
                        .arg(company_name.isEmpty() ? "" : QString(" (%1)").arg(company_name)));
    EditDialogue md(&record, m_undo_stack, this);
    if (md.exec() == QDialog::Accepted) {
        setDatabaseModified(true);
        m_tab->loadRepair(record.uuid(), true);
    }
}

void MainWindow::removeRepair()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!m_tab->isRepairSelected()) { return; }
    Repair record(m_tab->selectedRepairUUID());
    if (!isOperationPermitted("remove_repair", record.updatedBy())) { return; }
    if (isRecordLocked(record.date())) { return; }
    if (RemoveDialogue::confirm(this, tr("Remove repair - Leaklog"),
                                tr("Are you sure you want to remove the selected repair?\nTo remove all data about the repair \"%1\" type REMOVE and confirm:")
                                .arg(m_settings.formatDateTime(record.date()))) != QDialog::Accepted)
        return;

    QString company_name = record.customer();
    UndoCommand command(m_undo_stack, tr("Remove repair %1%2")
                        .arg(m_settings.formatDateTime(record.date()))
                        .arg(company_name.isEmpty() ? "" : QString(" (%1)").arg(company_name)));
    m_undo_stack->savepoint();

    record.remove();
    m_tab->clearSelectedRepair();
    enableTools();
    setDatabaseModified(true);
    m_tab->setView(View::Repairs);
}

void MainWindow::loadVariables(QTreeWidget *trw, QSqlDatabase database)
{
    Variables variables(database);
    QMap<QString, QTreeWidgetItem *> variable_items;
    while (variables.next()) {
        QTreeWidgetItem *item = variable_items.value(variables.uuid(), NULL);
        if (!item) {
            if (variables.parentUUID().isEmpty())
                item = new QTreeWidgetItem(trw);
            else
                item = new QTreeWidgetItem;
            variable_items.insert(variables.uuid(), item);
        }

        if (!variables.parentUUID().isEmpty()) {
            QTreeWidgetItem *parent_item = variable_items.value(variables.parentUUID(), NULL);
            if (!parent_item) {
                parent_item = new QTreeWidgetItem(trw);
                variable_items.insert(variables.parentUUID(), parent_item);
            }
            parent_item->addChild(item);
        }

        item->setText(0, variables.name());
        item->setText(1, variables.id());
        item->setData(1, Qt::UserRole, variables.uuid());
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
    QString parent_uuid;
    if (subvar) {
        if (trw_variables->currentItem()->parent() != NULL)
            return;
        parent_uuid = trw_variables->currentItem()->data(1, Qt::UserRole).toString();
    }
    VariableRecord record;
    record.setParentUUID(parent_uuid);
    UndoCommand command(m_undo_stack, tr("Add variable"));
    EditDialogue md(&record, m_undo_stack, this);
    if (md.exec() == QDialog::Accepted) {
        QTreeWidgetItem *item = NULL;
        if (subvar) {
            item = new QTreeWidgetItem(trw_variables->currentItem());
            VariableRecord(parent_uuid).update("type", "group");
        } else {
            item = new QTreeWidgetItem(trw_variables);
        }
        item->setText(0, record.name());
        item->setText(1, record.variableID());
        item->setData(1, Qt::UserRole, record.uuid());
        item->setText(2, record.unit());
        item->setText(3, record.stringValue("tolerance"));

        int scope = record.scope();
        if (scope & Variable::Inspection)
            addColumn(record.variableID(), "inspections", db);
        if (scope & Variable::Compressor)
            addColumn(record.variableID(), "inspections_compressors", db);
        setDatabaseModified(true);
        refreshView();
    }
}

void MainWindow::editVariable()
{
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) { return; }
    if (!trw_variables->currentIndex().isValid()) { return; }
    if (!isOperationPermitted("edit_variable")) { return; }

    QTreeWidgetItem *item = trw_variables->currentItem();
    QString id = item->text(1);
    VariableRecord record(item->data(1, Qt::UserRole).toString(), {{"id", id}});
    UndoCommand command(m_undo_stack, tr("Edit variable %1").arg(id));
    EditDialogue md(&record, m_undo_stack, this);
    if (md.exec() == QDialog::Accepted) {
        item->setText(0, record.name());
        item->setText(2, record.unit());
        item->setText(3, record.stringValue("tolerance"));

        setDatabaseModified(true);
        refreshView();
    }
}

void MainWindow::removeVariable()
{
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) { return; }
    if (!trw_variables->currentIndex().isValid()) { return; }
    if (!isOperationPermitted("remove_variable")) { return; }
    QTreeWidgetItem *item = trw_variables->currentItem();
    if (variableNames().contains(item->text(1))) { return; }
    bool subvar = item->parent() != NULL;
    QString id = item->text(1);
    QString uuid = item->data(1, Qt::UserRole).toString();
    if (RemoveDialogue::confirm(this, subvar ? tr("Remove subvariable - Leaklog") : tr("Remove variable - Leaklog"),
                                tr("Are you sure you want to remove the selected variable?\nTo remove the variable \"%1\" type REMOVE and confirm:")
                                .arg(id)) != QDialog::Accepted)
        return;

    UndoCommand command(m_undo_stack, tr("Remove variable %1").arg(id));
    m_undo_stack->savepoint();

    VariableRecord::query({{"parent_uuid", uuid}}).removeAll();
    MTRecord("variables", uuid).remove();
    delete item;
    dropColumn(id, "inspections", db);
    dropColumn(id, "inspections_compressors", db);
    enableTools();
    setDatabaseModified(true);
    refreshView();
}

void MainWindow::addTable()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!isOperationPermitted("add_table")) { return; }
    Table record("");
    UndoCommand command(m_undo_stack, tr("Add table"));
    EditDialogue md(&record, m_undo_stack, this);
    if (md.exec() == QDialog::Accepted) {
        emit tableAdded(-1, record.uuid(), record.name());
        cb_table_edit->addItem(record.name(), record.uuid());
        setDatabaseModified(true);
        refreshView();
    }
}

void MainWindow::editTable()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (cb_table_edit->currentIndex() < 0) { return; }
    if (!isOperationPermitted("edit_table")) { return; }
    Table record(cb_table_edit->currentData().toString());
    UndoCommand command(m_undo_stack, tr("Edit table %1").arg(cb_table_edit->currentText()));
    EditDialogue md(&record, m_undo_stack, this);
    if (md.exec() == QDialog::Accepted) {
        int i = cb_table_edit->currentIndex();
        emit tableRemoved(record.uuid());
        cb_table_edit->removeItem(i);
        emit tableAdded(i, record.uuid(), record.name());
        cb_table_edit->insertItem(i, record.name(), record.uuid());
        cb_table_edit->setCurrentIndex(i);
        setDatabaseModified(true);
        refreshView();
    }
}

void MainWindow::removeTable()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (cb_table_edit->currentIndex() < 0) { return; }
    if (!isOperationPermitted("remove_table")) { return; }
    if (RemoveDialogue::confirm(this, tr("Remove table - Leaklog"),
                                tr("Are you sure you want to remove the selected table?\nTo remove the table \"%1\" type REMOVE and confirm:")
                                .arg(cb_table_edit->currentText())) != QDialog::Accepted)
        return;

    UndoCommand command(m_undo_stack, tr("Remove table %1").arg(cb_table_edit->currentText()));
    m_undo_stack->savepoint();

    QString uuid = cb_table_edit->currentData().toString();
    emit tableRemoved(uuid);
    Table record(uuid);
    record.remove();
    int i = cb_table_edit->currentIndex();
    cb_table_edit->removeItem(i);
    enableTools();
    setDatabaseModified(true);
    refreshView();
}

void MainWindow::loadTable(const QString &)
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (cb_table_edit->currentIndex() < 0) { enableTools(); return; }
    trw_table_variables->clear();
    Table record(cb_table_edit->currentData().toString());
    QStringList variables = record.variables();
    QStringList sum = record.summedVariables();
    QStringList avg = record.averagedVariables();
    for (int i = 0; i < variables.count(); ++i) {
        Variable variable(variables.at(i));
        QTreeWidgetItem *item = new QTreeWidgetItem(trw_table_variables);
        if (variable.next()) { item->setText(0, variable.name()); }
        item->setText(1, variables.at(i));
        QComboBox *cb_foot = new QComboBox;
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
    QString uuid = cb_table_edit->currentData().toString();
    if (!isOperationPermitted("edit_table")) { loadTable(uuid); return; }
    UndoCommand command(m_undo_stack, tr("Edit table %1").arg(cb_table_edit->currentText()));
    m_undo_stack->savepoint();
    Table table(uuid);
    QStringList variables, sum, avg; QString value;
    for (int i = 0; i < trw_table_variables->topLevelItemCount(); ++i) {
        variables << trw_table_variables->topLevelItem(i)->text(1);
        QString value = ((QComboBox *)trw_table_variables->itemWidget(trw_table_variables->topLevelItem(i), 2))->currentText();
        if (value == tr("Sum")) { sum << trw_table_variables->topLevelItem(i)->text(1); }
        else if (value == tr("Average")) { avg << trw_table_variables->topLevelItem(i)->text(1); }
    }
    table.setVariables(variables);
    table.setSummedVariables(sum);
    table.setAveragedVariables(avg);
    table.save();
    setDatabaseModified(true);
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
    QDialog d(this);
    d.setWindowTitle(tr("Add existing variable - Leaklog"));
    d.setMinimumSize(QSize(300, 350));
        QVBoxLayout *vl = new QVBoxLayout(&d);
        vl->setMargin(6); vl->setSpacing(6);
            QHBoxLayout *hl = new QHBoxLayout;
            hl->setMargin(0); hl->setSpacing(6);
                QLabel *lbl = new QLabel(tr("Search:"), &d);
                SearchLineEdit *sle = new SearchLineEdit(&d);
            hl->addWidget(lbl);
            hl->addWidget(sle);
        vl->addLayout(hl);
            MTListWidget *lw = new MTListWidget(&d);
            QObject::connect(lw, SIGNAL(itemDoubleClicked(QListWidgetItem *)), &d, SLOT(accept()));
            QObject::connect(sle, SIGNAL(textChanged(QLineEdit *, const QString &)), lw, SLOT(filterItems(QLineEdit *, const QString &)));
        vl->addWidget(lw);
            QDialogButtonBox *bb = new QDialogButtonBox(&d);
            bb->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
            QObject::connect(bb, SIGNAL(accepted()), &d, SLOT(accept()));
            QObject::connect(bb, SIGNAL(rejected()), &d, SLOT(reject()));
        vl->addWidget(bb);
    Variable variable;
    QString id, name; QStringList ids;
    while (variable.next()) {
        id = variable.id();
        if (ids.contains(id)) { continue; }
        ids << id;
        name = variable.name();
        if (!used_ids.contains(id)) {
            QListWidgetItem *item = new QListWidgetItem;
            item->setText(name.isEmpty() ? id : tr("%1 (%2)").arg(id).arg(name));
            item->setData(Qt::UserRole, id);
            lw->addItem(item);
        }
    }
    if (d.exec() == QDialog::Accepted && lw->currentIndex().isValid()) {
        UndoCommand command(m_undo_stack, tr("Add variable %1 to table %2")
                            .arg(lw->currentItem()->text())
                            .arg(cb_table_edit->currentText()));
        m_undo_stack->savepoint();

        QString uuid = cb_table_edit->currentData().toString();
        Table table(uuid);
        QStringList variables = table.variables();
        variables << lw->currentItem()->data(Qt::UserRole).toString();
        table.setVariables(variables);
        table.save();
        loadTable(uuid);
        setDatabaseModified(true);
        refreshView();
    }
}

void MainWindow::removeTableVariable()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (cb_table_edit->currentIndex() < 0) { return; }
    if (!trw_table_variables->currentIndex().isValid()) { return; }
    if (!isOperationPermitted("edit_table")) { return; }
    QTreeWidgetItem *item = trw_table_variables->currentItem();
    switch (QMessageBox::information(this, tr("Remove variable - Leaklog"), tr("Are you sure you want to remove the variable \"%1\" from the selected table?").arg(item->text(1)), tr("Remove"), tr("Cancel"), 0, 1)) {
        case 0: // Remove
            break;
        case 1: // Cancel
            return; break;
    }

    UndoCommand command(m_undo_stack, tr("Remove variable %1 (%2) from table %3")
                        .arg(item->text(1))
                        .arg(item->text(0))
                        .arg(cb_table_edit->currentText()));
    m_undo_stack->savepoint();

    QString uuid = cb_table_edit->currentData().toString();
    Table table(uuid);

    QStringList variables = table.variables();
    variables.removeAll(item->text(1));
    table.setVariables(variables);

    QStringList sum = table.summedVariables();
    sum.removeAll(item->text(1));
    table.setSummedVariables(sum);

    QStringList avg = table.averagedVariables();
    avg.removeAll(item->text(1));
    table.setAveragedVariables(avg);

    table.save();

    loadTable(uuid);
    setDatabaseModified(true);
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

    UndoCommand command(m_undo_stack, (up ? tr("Move variable %1 (%2) up in table %3") : tr("Move variable %1 (%2) down in table %3"))
                        .arg(trw_table_variables->currentItem()->text(1))
                        .arg(trw_table_variables->currentItem()->text(0))
                        .arg(cb_table_edit->currentText()));
    m_undo_stack->savepoint();

    QString uuid = cb_table_edit->currentData().toString();
    Table table(uuid);
    QStringList variables = table.stringValue("variables").split(";", QString::SkipEmptyParts);
    QString variable = variables.takeAt(i);
    if (up) {
        if (i != 0) { i--; } else { i = variables.count(); }
    } else {
        if (i != variables.count()) { i++; } else { i = 0; }
    }
    variables.insert(i, variable);
    table.setVariables(variables);
    table.save();
    loadTable(uuid);
    trw_table_variables->setCurrentItem(trw_table_variables->topLevelItem(i));
    setDatabaseModified(true);
    refreshView();
}

void MainWindow::addWarning()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!isOperationPermitted("add_warning")) { return; }
    WarningRecord record;
    UndoCommand command(m_undo_stack, tr("Add warning"));
    EditWarningDialogue md(&record, m_undo_stack, this);
    if (md.exec() == QDialog::Accepted) {
        QString name = record.name();
        QString description = record.description();
        QListWidgetItem *item = new QListWidgetItem;
        item->setText(description.isEmpty() ? name : tr("%1 (%2)").arg(name).arg(description));
        item->setData(Qt::UserRole, record.uuid());
        lw_warnings->addItem(item);
        setDatabaseModified(true);
        refreshView();
    }
}

void MainWindow::editWarning()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!lw_warnings->currentIndex().isValid()) { return; }
    if (!isOperationPermitted("edit_warning")) { return; }
    QListWidgetItem *item = lw_warnings->currentItem();
    WarningRecord record(item->data(Qt::UserRole).toString());
    UndoCommand command(m_undo_stack, tr("Edit warning %1").arg(record.stringValue("name")));
    EditWarningDialogue md(&record, m_undo_stack, this);
    if (md.exec() == QDialog::Accepted) {
        QString name = record.name();
        QString description = record.description();
        item->setText(description.isEmpty() ? name : tr("%1 (%2)").arg(name).arg(description));
        item->setData(Qt::UserRole, record.uuid());
        setDatabaseModified(true);
        refreshView();
    }
}

void MainWindow::removeWarning()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!lw_warnings->currentIndex().isValid()) { return; }
    if (!isOperationPermitted("remove_warning")) { return; }
    QListWidgetItem *item = lw_warnings->currentItem();
    if (RemoveDialogue::confirm(this, tr("Remove warning - Leaklog"),
                                tr("Are you sure you want to remove the selected warning?\nTo remove the warning \"%1\" type REMOVE and confirm:")
                                .arg(item->text())) != QDialog::Accepted)
        return;

    UndoCommand command(m_undo_stack, tr("Remove warning %1").arg(item->text()));
    m_undo_stack->savepoint();

    WarningRecord record(item->data(Qt::UserRole).toString());
    record.remove();
    delete item;
    enableTools();
    setDatabaseModified(true);
    refreshView();
}

void MainWindow::addInspector()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!isOperationPermitted("add_inspector")) { return; }
    Inspector record;
    if (m_tab->isServiceCompanySelected()) {
        record.setValue("service_company_uuid", m_tab->selectedServiceCompanyUUID());
    }
    UndoCommand command(m_undo_stack, tr("Add inspector"));
    EditInspectorDialogue md(&record, m_undo_stack, this);
    if (md.exec() == QDialog::Accepted) {
        setDatabaseModified(true);
        m_tab->loadInspector(record.uuid(), true);
    }
}

void MainWindow::editInspector()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!m_tab->isInspectorSelected()) { return; }
    if (!isOperationPermitted("edit_inspector")) { return; }
    Inspector record(m_tab->selectedInspectorUUID());
    UndoCommand command(m_undo_stack, tr("Edit inspector %1").arg(record.personName()));
    EditInspectorDialogue md(&record, m_undo_stack, this);
    if (md.exec() == QDialog::Accepted) {
        enableTools();
        refreshView();
        setDatabaseModified(true);
    }
}

void MainWindow::removeInspector()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!m_tab->isInspectorSelected()) { return; }
    if (!isOperationPermitted("remove_inspector")) { return; }

    Inspector record(m_tab->selectedInspectorUUID());

    if (RemoveDialogue::confirm(this, tr("Remove inspector - Leaklog"),
                                tr("Are you sure you want to remove the selected inspector?\nTo remove all data about the inspector \"%1\" type REMOVE and confirm:")
                                .arg(record.personName())) != QDialog::Accepted)
        return;

    UndoCommand command(m_undo_stack, tr("Remove inspector %1").arg(record.personName()));
    m_undo_stack->savepoint();

    record.remove();
    m_tab->clearSelectedInspector();
    enableTools();
    setDatabaseModified(true);
    m_tab->setView(View::Inspectors);
}

void MainWindow::importData()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!isOperationPermitted("import_data")) { return; }
    QSettings settings("SZCHKT", "Leaklog");
    QDir dir(settings.value("file_dialog_dir", QDir::homePath()).toString());
    QString path = QFileDialog::getOpenFileName(this, tr("Import data - Leaklog"), dir.absolutePath(),
                                                tr("Leaklog Databases (*.lklg);;All files (*.*)"));
    if (path.isEmpty()) { return; }
    dir.setPath(path);
    dir.cdUp();
    settings.setValue("file_dialog_dir", dir.absolutePath());

{ // BEGIN IMPORT (SCOPE)
    QSqlDatabase data = QSqlDatabase::addDatabase("QSQLITE", "importData");
    data.setDatabaseName(path);
    if (!data.open()) {
        QMessageBox::critical(this, tr("Import data - Leaklog"), tr("Cannot read file %1:\n%2.").arg(path).arg(data.lastError().text()));
        return;
    }
    data.transaction();
    MTSqlQuery query(data);
    initDatabase(data, false, false);
    ImportDialogue *id = new ImportDialogue(this);

    // Customers
    query.exec("SELECT * FROM customers ORDER BY company");
    while (query.next()) {
        QString customer_uuid = query.stringValue("uuid");
        Customer customer(customer_uuid);
        if (customer.exists()) {
            bool modified = false;

            MTDictionary columns(true);

            for (int i = 0; i < Customer::attributes().count(); ++i) {
                QVariant attribute = query.value(Customer::attributes().key(i));
                bool attribute_modified = attribute != customer.value(Customer::attributes().key(i));
                if (attribute_modified) modified = true;

                if (Customer::attributes().key(i) == "operator_id") {
                    switch (attribute.toInt()) {
                    case -1:
                        columns.insert(QApplication::translate("Customer", "Service company"), attribute_modified ? "1" : "0");
                        break;
                    case 0:
                        columns.insert(QApplication::translate("Customer", "Customer"), attribute_modified ? "1" : "0");
                    default:
                        columns.insert(attribute.toString(), attribute_modified ? "1" : "0");
                    }
                } else {
                    columns.insert(MTVariant(attribute, Customer::attributes().key(i)).toString(),
                        attribute_modified ? "1" : "0");
                }
            }

            if (modified) {
                QTreeWidgetItem *item = new QTreeWidgetItem(id->modifiedCustomers(), columns.keys());
                for (int i = 0; i < columns.count(); ++i) {
                    if (columns.value(i).toInt()) {
                        item->setBackground(i, QBrush(Qt::darkMagenta));
                        item->setForeground(i, QBrush(Qt::white));
                    }
                }
                item->setCheckState(0, query.stringValue("date_updated") > customer.dateUpdated() ? Qt::Checked : Qt::Unchecked);
                if (item->checkState(0) == Qt::Unchecked)
                    item->setToolTip(0, tr("This record is older than the current record in your database"));
                item->setData(0, Qt::UserRole, customer_uuid);
            }
        } else {
            QTreeWidgetItem *item = new QTreeWidgetItem(id->newCustomers());
            for (int i = 0; i < Customer::attributes().count(); ++i) {
                item->setText(i, MTVariant(query.value(Customer::attributes().key(i)),
                                           Customer::attributes().key(i)).toString());
            }
            item->setCheckState(0, Qt::Checked);
            item->setData(0, Qt::UserRole, customer_uuid);
        }
    }

    // Contact persons
    query.exec("SELECT persons.*, customers.company FROM persons LEFT JOIN customers ON persons.customer_uuid = customers.uuid"
               " ORDER BY customers.company, persons.name");
    while (query.next()) {
        QString person_uuid = query.stringValue("uuid");
        Person person(person_uuid);
        if (person.exists()) {
            bool modified = false;

            MTDictionary columns(true);
            columns.insert(query.stringValue("company"), query.value("customer_uuid") != person.customerUUID() ? "1" : "0");

            for (int i = 0; i < Person::attributes().count(); ++i) {
                QVariant attribute = query.value(Person::attributes().key(i));
                bool attribute_modified = attribute != person.value(Person::attributes().key(i));
                if (attribute_modified) modified = true;
                columns.insert(attribute.toString(), attribute_modified ? "1" : "0");
            }

            if (modified) {
                QTreeWidgetItem *item = new QTreeWidgetItem(id->modifiedPersons(), columns.keys());
                for (int i = 0; i < columns.count(); ++i) {
                    if (columns.value(i).toInt()) {
                        item->setBackground(i, QBrush(Qt::darkMagenta));
                        item->setForeground(i, QBrush(Qt::white));
                    }
                }
                item->setCheckState(0, query.stringValue("date_updated") > person.stringValue("date_updated") ? Qt::Checked : Qt::Unchecked);
                if (item->checkState(0) == Qt::Unchecked)
                    item->setToolTip(0, tr("This record is older than the current record in your database"));
                item->setData(0, Qt::UserRole, person_uuid);
            }
        } else {
            QTreeWidgetItem *item = new QTreeWidgetItem(id->newPersons());
            item->setText(0, query.stringValue("company"));
            for (int i = 0; i < Person::attributes().count(); ++i) {
                item->setText(i + 1, query.stringValue(Person::attributes().key(i)));
            }
            item->setCheckState(0, Qt::Checked);
            item->setData(0, Qt::UserRole, person_uuid);
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
    QStringList compressors_columns = Compressor::columns().toStringList([](const Column &column) {
        return QString("compressors.%1 AS compressor_%1").arg(column.name());
    });
    query.exec(QString("SELECT customers.company, circuits.*, %1 FROM circuits"
                       " LEFT JOIN customers ON circuits.customer_uuid = customers.uuid"
                       " LEFT JOIN compressors ON compressors.circuit_uuid = circuits.uuid"
                       " ORDER BY customers.company, circuits.id, compressors.name").arg(compressors_columns.join(", ")));
    QString last_uuid;
    QTreeWidgetItem *last_item = NULL;
    while (query.next()) {
        QString circuit_uuid = query.stringValue("uuid");
        if (last_uuid != circuit_uuid) {
            if (last_item && last_item->treeWidget() == NULL)
                delete last_item;
            last_uuid = circuit_uuid;

            QString circuit_id_justified = query.stringValue("id").rightJustified(5, '0');
            Circuit circuit(circuit_uuid);
            if (circuit.exists()) {
                bool modified = false;

                MTDictionary columns(true);
                columns.insert(query.stringValue("company"), query.value("customer_uuid") != circuit.customerUUID() ? "1" : "0");
                columns.insert(circuit_id_justified, query.value("id") != circuit.value("id") ? "1" : "0");

                for (int i = 1; i < Circuit::attributes().count(); ++i) {
                    QVariant attribute = query.value(Circuit::attributes().key(i));
                    bool attribute_modified = attribute != circuit.value(Circuit::attributes().key(i));
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
                last_item->setCheckState(0, query.stringValue("date_updated") > circuit.dateUpdated() ? Qt::Checked : Qt::Unchecked);
                if (last_item->checkState(0) == Qt::Unchecked)
                    last_item->setToolTip(0, tr("This record is older than the current record in your database"));
            } else {
                last_item = new QTreeWidgetItem(id->newCircuits());
                last_item->setText(0, query.stringValue("company"));
                last_item->setText(1, circuit_id_justified);
                for (int i = 1; i < Circuit::attributes().count(); ++i) {
                    last_item->setText(i + 1, query.stringValue(Circuit::attributes().key(i)));
                }
                last_item->setCheckState(0, Qt::Checked);
            }

            last_item->setData(0, Qt::UserRole, circuit_uuid);
            last_item->setData(1, Qt::UserRole, query.value("customer_uuid"));
        }

        QString compressor_uuid = query.stringValue("compressor_uuid");
        if (last_item && !compressor_uuid.isEmpty()) {
            Compressor compressor(compressor_uuid);
            if (compressor.exists()) {
                bool modified = false;

                MTDictionary columns(true);
                QMapIterator<int, QString> i(compressor_attributes);
                while (i.hasNext()) { i.next();
                    QVariant attribute = query.value("compressor_" + i.value());
                    bool attribute_modified = attribute != compressor.value(i.value());
                    if (attribute_modified) {
                        modified = true;
                        shown_sections << i.key();
                    }
                    columns.insert(attribute.toString(), attribute_modified ? "1" : "0");
                }

                if (modified && query.stringValue("compressor_date_updated") > compressor.stringValue("date_updated")) {
                    QTreeWidgetItem *item = new QTreeWidgetItem(last_item);
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
                    item->setData(0, Qt::UserRole, compressor_uuid);
                    if (last_item->treeWidget() == NULL)
                        id->modifiedCircuits()->addTopLevelItem(last_item);
                }
            } else {
                QTreeWidgetItem *item = new QTreeWidgetItem(last_item);
                item->setData(0, Qt::UserRole, compressor_uuid);
                QMapIterator<int, QString> i(compressor_attributes);
                while (i.hasNext()) { i.next();
                    item->setText(i.key(), query.stringValue("compressor_" + i.value()));
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
    QStringList variable_scopes;
    variable_scopes << QApplication::translate("VariableRecord", "Inspection");
    variable_scopes << QApplication::translate("VariableRecord", "Compressor");
    MTDictionary variable_names;
    variable_names.insert("inspection_type", QApplication::translate("Inspection", "Type"));
    QStringList compressor_variable_names;
    Variables variables(data);
    QMap<QString, QTreeWidgetItem *> variable_items;
    while (variables.next()) {
        QTreeWidgetItem *item = variable_items.value(variables.uuid(), NULL);
        if (!item) {
            if (variables.parentUUID().isEmpty())
                item = new QTreeWidgetItem(id->variables());
            else
                item = new QTreeWidgetItem;
            variable_items.insert(variables.uuid(), item);
        }

        if (!variables.parentUUID().isEmpty()) {
            QTreeWidgetItem *parent_item = variable_items.value(variables.parentUUID(), NULL);
            if (!parent_item) {
                parent_item = new QTreeWidgetItem(id->variables());
                variable_items.insert(variables.parentUUID(), parent_item);
            }
            parent_item->addChild(item);
            parent_item->setExpanded(true);

            variable_names.remove(variables.parentUUID());
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
        item->setData(1, Qt::UserRole, variables.uuid());
        item->setText(2, variables.unit());
        item->setText(3, variableTypes().value(variables.type()));
        item->setData(3, Qt::UserRole, variables.type());
        item->setText(4, variable_scopes.value(variables.scope() - Variable::Inspection));
        item->setData(4, Qt::UserRole, variables.scope());
        item->setText(5, variables.valueExpression());
        item->setText(6, variables.compareNom() ? tr("Yes") : tr("No"));
        item->setData(6, Qt::UserRole, variables.compareNom());
        item->setText(7, variables.value(Variable::Tolerance).toString());
        item->setText(8, variables.colBg());

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
            if (variable.scope() != variables.scope()) {
                overwrite = true;
                item->setBackground(4, QBrush(Qt::darkMagenta));
                item->setForeground(4, QBrush(Qt::white));
            }
            if (variable.valueExpression() != variables.valueExpression()) {
                overwrite = true;
                item->setBackground(5, QBrush(Qt::darkMagenta));
                item->setForeground(5, QBrush(Qt::white));
            }
            if ((variable.compareNom() > 0) != (variables.compareNom() > 0)) {
                overwrite = true;
                item->setBackground(6, QBrush(Qt::darkMagenta));
                item->setForeground(6, QBrush(Qt::white));
            }
            if (variable.tolerance() != variables.tolerance()) {
                overwrite = true;
                item->setBackground(7, QBrush(Qt::darkMagenta));
                item->setForeground(7, QBrush(Qt::white));
            }
            if (variable.colBg() != variables.colBg()) {
                overwrite = true;
                item->setBackground(8, QBrush(Qt::darkMagenta));
                item->setForeground(8, QBrush(Qt::white));
            }
        }
        QComboBox *cb_action = new QComboBox;
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
        id->variables()->setItemWidget(item, 9, cb_action);
    }

    // Inspections
    bool record_locked;
    QTreeWidget *trw[] = { id->newInspections(), id->modifiedInspections() };
    for (int w = 0; w < 2; ++w) {
        trw[w]->setColumnCount(variable_names.count() + 3);
        trw[w]->setHeaderItem(new QTreeWidgetItem(QStringList() << tr("Customer") << tr("Circuit") << tr("Date") << variable_names.values()));
    }
    shown_sections.clear();
    shown_sections << 0 << 1 << 2;

    QStringList inspections_columns = Inspection::columns().toStringList([](const Column &column) {
        return QString("inspections.%1").arg(column.name());
    });
    foreach (const QString &variable_name, variable_names.keys())
        inspections_columns << QString("inspections.%1").arg(variable_name);

    QStringList inspections_compressors_columns = InspectionCompressor::columns().toStringList([](const Column &column) {
        return QString("inspections_compressors.%1 AS compressor_%1").arg(column.name());
    });
    foreach (const QString &variable_name, compressor_variable_names)
        inspections_compressors_columns << QString("inspections_compressors.%1 AS compressor_%1").arg(variable_name);

    query.exec(QString("SELECT customers.company, %1, %2, circuits.id AS circuit_id, compressors.name AS compressor_name"
                       " FROM inspections LEFT JOIN customers ON inspections.customer_uuid = customers.uuid"
                       " LEFT JOIN circuits ON inspections.circuit_uuid = circuits.uuid"
                       " LEFT JOIN inspections_compressors ON inspections_compressors.inspection_uuid = inspections.uuid"
                       " LEFT JOIN compressors ON inspections_compressors.compressor_uuid = compressors.uuid"
                       " ORDER BY inspections.date, compressors.name")
               .arg(inspections_columns.join(", "))
               .arg(inspections_compressors_columns.join(", ")));
    last_uuid.clear();
    last_item = NULL;
    QSqlRecord record = query.record();
    while (query.next()) {
        QString inspection_uuid = query.stringValue("uuid");
        if (last_uuid != inspection_uuid) {
            if (last_item && last_item->treeWidget() == NULL)
                delete last_item;
            last_uuid = inspection_uuid;

            QString circuit_id_justified = query.stringValue("circuit_id").rightJustified(5, '0');
            record_locked = DBInfo::isRecordLocked(query.stringValue("date"));
            Inspection inspection(inspection_uuid);
            if (inspection.exists()) {
                bool modified = false;

                MTDictionary columns(true);
                columns.insert(query.stringValue("company"), query.value("customer_uuid") != inspection.customerUUID() ? "1" : "0");
                columns.insert(circuit_id_justified, query.value("circuit_uuid") != inspection.circuitUUID() ? "1" : "0");
                columns.insert(query.stringValue("date"), query.value("date") != inspection.date() ? "1" : "0");

                for (int i = 0; i < variable_names.count(); ++i) {
                    QVariant attribute = query.value(variable_names.key(i));
                    bool attribute_modified = attribute != inspection.value(variable_names.key(i));
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
                last_item->setCheckState(0, query.stringValue("date_updated") > inspection.dateUpdated() ? Qt::Checked : Qt::Unchecked);
                if (last_item->checkState(0) == Qt::Unchecked)
                    last_item->setToolTip(0, tr("This record is older than the current record in your database"));
            } else {
                last_item = new QTreeWidgetItem(id->newInspections());
                last_item->setText(0, query.stringValue("company"));
                last_item->setText(1, circuit_id_justified);
                last_item->setText(2, query.stringValue("date"));
                for (int i = 0; i < variable_names.count(); ++i) {
                    last_item->setText(i + 3, query.stringValue(variable_names.key(i)));
                }
                last_item->setCheckState(0, Qt::Checked);
            }

            if (record_locked) {
                last_item->setCheckState(0, Qt::Unchecked);
                last_item->setDisabled(true);
                last_item->setIcon(2, QIcon(":/images/images/locked16.png"));
            } else {
                last_item->setData(0, Qt::UserRole, inspection_uuid);
                last_item->setData(1, Qt::UserRole, query.value("customer_uuid"));
                last_item->setData(2, Qt::UserRole, query.value("circuit_uuid"));
            }
        }

        QString compressor_uuid = query.stringValue("compressor_uuid");
        if (last_item && !compressor_uuid.isEmpty()) {
            InspectionCompressor inspection_compressor(compressor_uuid);
            if (inspection_compressor.exists()) {
                bool modified = false;

                MTDictionary columns(true);
                columns.insert(QString(), "0");
                columns.insert(query.stringValue("compressor_name"), "0");
                columns.insert(QString(), "0");

                for (int i = 0; i < variable_names.count(); ++i) {
                    if (!record.contains("compressor_" + variable_names.key(i))) {
                        columns.insert(QString(), "0");
                        continue;
                    }
                    QVariant attribute = query.value("compressor_" + variable_names.key(i));
                    bool attribute_modified = attribute != inspection_compressor.value(variable_names.key(i));
                    if (attribute_modified) {
                        modified = true;
                        shown_sections << i + 3;
                    }
                    columns.insert(attribute.toString(), attribute_modified ? "1" : "0");
                }

                if (modified && query.stringValue("compressor_date_updated") > inspection_compressor.stringValue("date_updated")) {
                    QTreeWidgetItem *item = new QTreeWidgetItem(last_item, columns.keys());
                    for (int i = 0; i < columns.count(); ++i) {
                        if (columns.value(i).toInt()) {
                            item->setBackground(i, QBrush(Qt::darkMagenta));
                            item->setForeground(i, QBrush(Qt::white));
                        }
                    }
                    item->setData(0, Qt::UserRole, compressor_uuid);
                    if (last_item->treeWidget() == NULL)
                        id->modifiedInspections()->addTopLevelItem(last_item);
                }
            } else {
                QTreeWidgetItem *item = new QTreeWidgetItem(last_item);
                item->setData(0, Qt::UserRole, compressor_uuid);
                item->setText(1, query.stringValue("compressor_name"));
                for (int i = 0; i < variable_names.count(); ++i) {
                    if (!record.contains("compressor_" + variable_names.key(i)))
                        continue;
                    shown_sections << i + 3;
                    item->setText(i + 3, query.stringValue("compressor_" + variable_names.key(i)));
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
        record_locked = DBInfo::isRecordLocked(query.stringValue("date"));
        QTreeWidgetItem *item = NULL;
        QString repair_uuid = query.stringValue("uuid");
        Repair repair(repair_uuid);
        if (repair.exists()) {
            bool modified = false;

            MTDictionary columns(true);

            for (int i = 0; i < Repair::attributes().count(); ++i) {
                QVariant attribute = query.value(Repair::attributes().key(i));
                bool attribute_modified = attribute != repair.value(Repair::attributes().key(i));
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
                item->setCheckState(0, query.stringValue("date_updated") > repair.dateUpdated() ? Qt::Checked : Qt::Unchecked);
                if (item->checkState(0) == Qt::Unchecked)
                    item->setToolTip(0, tr("This record is older than the current record in your database"));
            }
        } else {
            item = new QTreeWidgetItem(id->newRepairs());
            for (int i = 0; i < Repair::attributes().count(); ++i) {
                item->setText(i, query.stringValue(Repair::attributes().key(i)));
            }
            item->setCheckState(0, Qt::Checked);
        }

        if (item) {
            if (record_locked) {
                item->setCheckState(0, Qt::Unchecked);
                item->setDisabled(true);
                item->setIcon(0, QIcon(":/images/images/locked16.png"));
            } else {
                item->setData(0, Qt::UserRole, repair_uuid);
            }
        }
    }

    // Refrigerant management
    query.exec("SELECT * FROM refrigerant_management ORDER BY date");
    while (query.next()) {
        record_locked = DBInfo::isRecordLocked(query.stringValue("date"));
        QTreeWidgetItem *item = NULL;
        QString record_uuid = query.stringValue("uuid");
        RefrigerantRecord record(record_uuid);
        if (record.exists()) {
            bool modified = false;

            MTDictionary columns(true);

            for (int i = 0; i < RefrigerantRecord::attributes().count(); ++i) {
                QVariant attribute = query.value(RefrigerantRecord::attributes().key(i));
                bool attribute_modified = attribute != record.value(RefrigerantRecord::attributes().key(i));
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
                item->setCheckState(0, query.stringValue("date_updated") > record.dateUpdated() ? Qt::Checked : Qt::Unchecked);
                if (item->checkState(0) == Qt::Unchecked)
                    item->setToolTip(0, tr("This record is older than the current record in your database"));
            }
        } else {
            item = new QTreeWidgetItem(id->newRefrigerantManagement());
            for (int i = 0; i < RefrigerantRecord::attributes().count(); ++i) {
                item->setText(i, query.stringValue(RefrigerantRecord::attributes().key(i)));
            }
            item->setCheckState(0, Qt::Checked);
        }

        if (item) {
            if (record_locked) {
                item->setCheckState(0, Qt::Unchecked);
                item->setDisabled(true);
                item->setIcon(0, QIcon(":/images/images/locked16.png"));
            } else {
                item->setData(0, Qt::UserRole, record_uuid);
            }
        }
    }

    // Inspectors
    query.exec("SELECT * FROM inspectors ORDER BY person");
    while (query.next()) {
        QString inspector_uuid = query.stringValue("uuid");
        Inspector inspector(inspector_uuid);
        if (inspector.exists()) {
            bool modified = false;

            MTDictionary columns(true);

            for (int i = 0; i < Inspector::attributes().count(); ++i) {
                QVariant attribute = query.value(Inspector::attributes().key(i));
                bool attribute_modified = attribute != inspector.value(Inspector::attributes().key(i));
                if (attribute_modified) modified = true;
                columns.insert(attribute.toString(), attribute_modified ? "1" : "0");
            }

            if (modified) {
                QTreeWidgetItem *item = new QTreeWidgetItem(id->modifiedInspectors(), columns.keys());
                for (int i = 0; i < columns.count(); ++i) {
                    if (columns.value(i).toInt()) {
                        item->setBackground(i, QBrush(Qt::darkMagenta));
                        item->setForeground(i, QBrush(Qt::white));
                    }
                }
                item->setCheckState(0, query.stringValue("date_updated") > inspector.dateUpdated() ? Qt::Checked : Qt::Unchecked);
                if (item->checkState(0) == Qt::Unchecked)
                    item->setToolTip(0, tr("This record is older than the current record in your database"));
                item->setData(0, Qt::UserRole, inspector_uuid);
            }
        } else {
            QTreeWidgetItem *item = new QTreeWidgetItem(id->newInspectors());
            for (int i = 0; i < Inspector::attributes().count(); ++i) {
                item->setText(i, query.stringValue(Inspector::attributes().key(i)));
            }
            item->setCheckState(0, Qt::Checked);
            item->setData(0, Qt::UserRole, inspector_uuid);
        }
    }

    if (id->exec() == QDialog::Accepted) { // BEGIN IMPORT
        UndoCommand command(m_undo_stack, tr("Import database %1").arg(QFileInfo(path).completeBaseName()));
        m_undo_stack->savepoint();

        QVariantMap set;
        QSet<QString> fields;

        // Import customers
        trw[0] = id->newCustomers();
        trw[1] = id->modifiedCustomers();
        fields = Customer::columns().columnNameSet();
        for (int w = 0; w < 2; ++w) {
            for (int c = 0; c < trw[w]->topLevelItemCount(); ++c) {
                if (trw[w]->topLevelItem(c)->checkState(0) == Qt::Unchecked) { continue; }
                set.clear();
                QString customer_uuid = trw[w]->topLevelItem(c)->data(0, Qt::UserRole).toString();
                query.prepare("SELECT * FROM customers WHERE uuid = :uuid");
                query.bindValue(":uuid", customer_uuid);
                query.exec();
                QSqlRecord record = query.record();
                if (query.next()) {
                    for (int f = 0; f < record.count(); ++f) {
                        if (fields.contains(record.fieldName(f)))
                            set.insert(record.fieldName(f), query.value(f));
                    }
                    Customer(customer_uuid).update(set);
                }
            }
        }

        // Import contact persons
        trw[0] = id->newPersons();
        trw[1] = id->modifiedPersons();
        fields = Person::columns().columnNameSet();
        for (int w = 0; w < 2; ++w) {
            for (int p = 0; p < trw[w]->topLevelItemCount(); ++p) {
                if (trw[w]->topLevelItem(p)->checkState(0) == Qt::Unchecked) { continue; }
                set.clear();
                QString person_uuid = trw[w]->topLevelItem(p)->data(0, Qt::UserRole).toString();
                query.prepare("SELECT * FROM persons WHERE uuid = :uuid");
                query.bindValue(":uuid", person_uuid);
                query.exec();
                QSqlRecord record = query.record();
                if (query.next()) {
                    for (int f = 0; f < record.count(); ++f) {
                        if (fields.contains(record.fieldName(f)))
                            set.insert(record.fieldName(f), query.value(f));
                    }
                    Person(person_uuid).update(set);
                }
            }
        }

        // Import circuits
        trw[0] = id->newCircuits();
        trw[1] = id->modifiedCircuits();
        fields = Circuit::columns().columnNameSet();
        QSet<QString> compressors_fields = Compressor::columns().columnNameSet();
        for (int w = 0; w < 2; ++w) {
            for (int cc = 0; cc < trw[w]->topLevelItemCount(); ++cc) {
                QTreeWidgetItem *item = trw[w]->topLevelItem(cc);
                if (item->checkState(0) == Qt::Unchecked) { continue; }
                set.clear();
                QString circuit_uuid = item->data(0, Qt::UserRole).toString();
                query.prepare("SELECT * FROM circuits WHERE uuid = :uuid");
                query.bindValue(":uuid", circuit_uuid);
                query.exec();
                QSqlRecord record = query.record();
                if (query.next()) {
                    for (int f = 0; f < record.count(); ++f) {
                        if (fields.contains(record.fieldName(f)))
                            set.insert(record.fieldName(f), query.value(f));
                    }
                    Circuit(circuit_uuid).update(set);
                }

                for (int c = 0; c < item->childCount(); ++c) {
                    set.clear();
                    QString compressor_uuid = item->child(c)->data(0, Qt::UserRole).toString();
                    query.prepare("SELECT * FROM compressors WHERE uuid = :uuid");
                    query.bindValue(":uuid", compressor_uuid);
                    query.exec();
                    QSqlRecord record = query.record();
                    if (query.next()) {
                        for (int f = 0; f < record.count(); ++f) {
                            if (compressors_fields.contains(record.fieldName(f)))
                                set.insert(record.fieldName(f), query.value(f));
                        }
                        Compressor(compressor_uuid).update(set);
                    }
                }
            }
        }

        // Import variables
        QStringList inspections_skip_columns; bool skip_parent = false;
        QString current_text; QTreeWidgetItem *item = NULL; QTreeWidgetItem *subitem = NULL;
        QTreeWidgetItem *new_item = NULL;
        for (int v = 0; v < id->variables()->topLevelItemCount(); ++v) {
            item = id->variables()->topLevelItem(v);
            skip_parent = false; new_item = NULL;
            current_text = ((QComboBox *)id->variables()->itemWidget(item, 9))->currentText();
            if (current_text == tr("Do not import")) {
                inspections_skip_columns << item->text(1);
                skip_parent = true;
            } else if (current_text == tr("Import") || current_text == tr("Overwrite and import")) {
                VariableRecord record(item->data(1, Qt::UserRole).toString());
                Variable variable(item->text(1));
                if (!variable.next()) {
                    new_item = new QTreeWidgetItem(trw_variables);
                    new_item->setText(0, item->text(0));
                    new_item->setText(1, item->text(1));
                    new_item->setData(1, Qt::UserRole, item->data(1, Qt::UserRole));
                    new_item->setText(2, item->text(2));
                    new_item->setText(3, item->text(7));
                }
                set.clear();
                set.insert("name", item->text(0));
                set.insert("id", item->text(1));
                set.insert("unit", item->text(2));
                set.insert("type", item->data(3, Qt::UserRole));
                set.insert("scope", item->data(4, Qt::UserRole));
                set.insert("value", item->text(5));
                set.insert("compare_nom", item->data(6, Qt::UserRole));
                set.insert("tolerance", item->text(7));
                set.insert("col_bg", item->text(8));
                record.update(set);
            }
            for (int sv = 0; sv < item->childCount(); ++sv) {
                subitem = item->child(sv);
                current_text = ((QComboBox *)id->variables()->itemWidget(subitem, 9))->currentText();
                if (skip_parent || current_text == tr("Do not import")) {
                    inspections_skip_columns << subitem->text(1);
                } else if (current_text == tr("Import") || current_text == tr("Overwrite and import")) {
                    VariableRecord record(subitem->data(1, Qt::UserRole).toString());
                    Variable subvariable(subitem->text(1));
                    if (new_item != NULL && !subvariable.next()) {
                        QTreeWidgetItem *new_subitem = new QTreeWidgetItem(new_item);
                        new_subitem->setText(0, subitem->text(0));
                        new_subitem->setText(1, subitem->text(1));
                        new_subitem->setData(1, Qt::UserRole, subitem->data(1, Qt::UserRole));
                        new_subitem->setText(2, subitem->text(2));
                        new_subitem->setText(3, subitem->text(7));
                    }
                    set.clear();
                    set.insert("name", subitem->text(0));
                    set.insert("parent_uuid", item->data(1, Qt::UserRole).toString());
                    set.insert("id", subitem->text(1));
                    set.insert("unit", subitem->text(2));
                    set.insert("type", subitem->data(3, Qt::UserRole));
                    set.insert("scope", subitem->data(4, Qt::UserRole));
                    set.insert("value", subitem->text(5));
                    set.insert("compare_nom", subitem->data(6, Qt::UserRole));
                    set.insert("tolerance", subitem->text(7));
                    record.update(set);
                }
            }
        }
        inspections_skip_columns << "inspection_type";

        // Import inspections
        trw[0] = id->newInspections();
        trw[1] = id->modifiedInspections();
        QSet<QString> inspections_compressors_fields = InspectionCompressor::columns().columnNameSet();
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
        inspections_compressors_fields.unite(QSet<QString>::fromList(compressor_variable_names));
#else
        inspections_compressors_fields.unite(QSet<QString>(compressor_variable_names.begin(), compressor_variable_names.end()));
#endif
        for (int w = 0; w < 2; ++w) {
            for (int i = 0, j = 0; i < trw[w]->topLevelItemCount(); ++i) {
                QTreeWidgetItem *item = trw[w]->topLevelItem(i);
                if (item->checkState(0) == Qt::Unchecked) { continue; }
                set.clear();
                QString inspection_uuid = item->data(0, Qt::UserRole).toString();
                query.prepare("SELECT * FROM inspections WHERE uuid = :uuid");
                query.bindValue(":uuid", inspection_uuid);
                query.exec();
                QSqlRecord record = query.record();
                if (query.next()) {
                    set.insert("inspection_type", query.value(record.indexOf("inspection_type")).toInt()); // NOT NULL

                    for (int f = 0; f < record.count(); ++f) {
                        if (!inspections_skip_columns.contains(record.fieldName(f))) {
                            set.insert(record.fieldName(f), query.value(f));
                        }
                    }
                }
                Inspection(inspection_uuid).update(set, j == 0);
                j++;

                for (int c = 0; c < item->childCount(); ++c) {
                    set.clear();
                    QString compressor_uuid = item->child(c)->data(0, Qt::UserRole).toString();
                    query.prepare("SELECT * FROM inspections_compressors WHERE uuid = :uuid");
                    query.bindValue(":uuid", compressor_uuid);
                    query.exec();
                    QSqlRecord record = query.record();
                    if (query.next()) {
                        for (int f = 0; f < record.count(); ++f) {
                            if (inspections_compressors_fields.contains(record.fieldName(f))
                                && !inspections_skip_columns.contains(record.fieldName(f)))
                                set.insert(record.fieldName(f), query.value(f));
                        }
                        InspectionCompressor(compressor_uuid).update(set);
                    }
                }
            }
        }

        // Import repairs
        trw[0] = id->newRepairs();
        trw[1] = id->modifiedRepairs();
        fields = Repair::columns().columnNameSet();
        for (int w = 0; w < 2; ++w) {
            for (int i = 0; i < trw[w]->topLevelItemCount(); ++i) {
                if (trw[w]->topLevelItem(i)->checkState(0) == Qt::Unchecked) { continue; }
                set.clear();
                QString repair_uuid = trw[w]->topLevelItem(i)->data(0, Qt::UserRole).toString();
                query.prepare("SELECT * FROM repairs WHERE uuid = :uuid");
                query.bindValue(":uuid", repair_uuid);
                query.exec();
                QSqlRecord record = query.record();
                if (query.next()) {
                    for (int f = 0; f < record.count(); ++f) {
                        if (fields.contains(record.fieldName(f)))
                            set.insert(record.fieldName(f), query.value(f));
                    }
                }
                Repair(repair_uuid).update(set);
            }
        }

        // Import refrigerant management
        trw[0] = id->newRefrigerantManagement();
        trw[1] = id->modifiedRefrigerantManagement();
        fields = RefrigerantRecord::columns().columnNameSet();
        for (int w = 0; w < 2; ++w) {
            for (int i = 0; i < trw[w]->topLevelItemCount(); ++i) {
                if (trw[w]->topLevelItem(i)->checkState(0) == Qt::Unchecked) { continue; }
                set.clear();
                QString record_uuid = trw[w]->topLevelItem(i)->data(0, Qt::UserRole).toString();
                query.prepare("SELECT * FROM refrigerant_management WHERE uuid = :uuid");
                query.bindValue(":uuid", record_uuid);
                query.exec();
                QSqlRecord record = query.record();
                if (query.next()) {
                    for (int f = 0; f < record.count(); ++f) {
                        if (fields.contains(record.fieldName(f)))
                            set.insert(record.fieldName(f), query.value(f));
                    }
                }
                RefrigerantRecord(record_uuid).update(set);
            }
        }

        // Import inspectors
        trw[0] = id->newInspectors();
        trw[1] = id->modifiedInspectors();
        fields = Inspector::columns().columnNameSet();
        for (int w = 0; w < 2; ++w) {
            for (int i = 0; i < trw[w]->topLevelItemCount(); ++i) {
                if (trw[w]->topLevelItem(i)->checkState(0) == Qt::Unchecked) { continue; }
                set.clear();
                QString inspector_uuid = trw[w]->topLevelItem(i)->data(0, Qt::UserRole).toString();
                query.prepare("SELECT * FROM inspectors WHERE uuid = :uuid");
                query.bindValue(":uuid", inspector_uuid);
                query.exec();
                QSqlRecord record = query.record();
                if (query.next()) {
                    for (int f = 0; f < record.count(); ++f) {
                        if (fields.contains(record.fieldName(f)))
                            set.insert(record.fieldName(f), query.value(f));
                    }
                }
                Inspector(inspector_uuid).update(set);
            }
        }
        setDatabaseModified(true);
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
    QSettings settings("SZCHKT", "Leaklog");
    QDir dir(settings.value("file_dialog_dir", QDir::homePath()).toString());
    QString path = QFileDialog::getOpenFileName(this, tr("Import CSV - Leaklog"), dir.absolutePath(),
                                                tr("CSV files (*.csv);;All files (*.*)"));
    if (path.isEmpty()) { return; }
    dir.setPath(path);
    dir.cdUp();
    settings.setValue("file_dialog_dir", dir.absolutePath());

    QString string_value;
    QStringList refrigerants(listRefrigerants());

    QList<ImportDialogueTable *> tables;
    ImportDialogueTable *table = new ImportDialogueTable(tr("Customers"), "customers");
    table->addColumn(QApplication::translate("Customer", "ID"), "id", ImportDialogueTableColumn::ID);
    table->addColumn(QApplication::translate("Customer", "Company"), "company", ImportDialogueTableColumn::Text);
    table->addColumn(QApplication::translate("Customer", "E-mail"), "mail", ImportDialogueTableColumn::Text);
    table->addColumn(QApplication::translate("Customer", "Phone"), "phone", ImportDialogueTableColumn::Text);
    table->addColumn(tr("Street"), "street", ImportDialogueTableColumn::AddressStreet);
    table->addColumn(tr("City"), "city", ImportDialogueTableColumn::AddressCity);
    table->addColumn(tr("Postal code"), "postal_code", ImportDialogueTableColumn::AddressPostalCode);
    table->addColumn(tr("Notes"), "notes", ImportDialogueTableColumn::Text);
    tables.append(table);

    table = table->addChildTableTemplate(tr("Contact persons"), "persons", "customer_uuid");
    table->addColumn(tr("Contact person name"), "name", ImportDialogueTableColumn::Text);
    table->addColumn(tr("Contact person e-mail"), "mail", ImportDialogueTableColumn::Text);
    table->addColumn(tr("Contact person phone"), "phone", ImportDialogueTableColumn::Text);

    ImportDialogueTable *circuits_table = new ImportDialogueTable(tr("Circuits"), "circuits");
    circuits_table->addForeignKeyColumn(tr("Customer ID"), "customer_uuid", "customers", "id");
    circuits_table->addColumn(QApplication::translate("Circuit", "ID"), "id", ImportDialogueTableColumn::Integer);
    circuits_table->addColumn(QApplication::translate("Circuit", "Circuit name"), "name", ImportDialogueTableColumn::Text);
    circuits_table->addColumn(QApplication::translate("Circuit", "Place of operation"), "operation", ImportDialogueTableColumn::Text);
    circuits_table->addColumn(QApplication::translate("Circuit", "Building"), "building", ImportDialogueTableColumn::Text);
    circuits_table->addColumn(QApplication::translate("Circuit", "Device"), "device", ImportDialogueTableColumn::Text);
    circuits_table->addColumn(QApplication::translate("Circuit", "Manufacturer"), "manufacturer", ImportDialogueTableColumn::Text);
    circuits_table->addColumn(QApplication::translate("Circuit", "Type"), "type", ImportDialogueTableColumn::Text);
    circuits_table->addColumn(QApplication::translate("Circuit", "Serial number"), "sn", ImportDialogueTableColumn::Text);
    ImportDialogueTableColumn *col = circuits_table->addColumn(QApplication::translate("Circuit", "Field of application"), "field", ImportDialogueTableColumn::Select);
    for (int n = attributeValues().indexOfKey("field") + 1; n < attributeValues().count() && attributeValues().key(n).startsWith("field::"); ++n) {
        string_value = attributeValues().key(n).mid(attributeValues().key(n).lastIndexOf(':') + 1);
        col->addSelectValue(string_value, string_value);
        col->addSelectValue(attributeValues().value(n).toLower(), string_value);
    }
    col = circuits_table->addColumn(QApplication::translate("Circuit", "Refrigerant"), "refrigerant", ImportDialogueTableColumn::Select);
    foreach (string_value, refrigerants)
        col->addSelectValue(string_value.toLower(), string_value);
    circuits_table->addColumn(QApplication::translate("Circuit", "Refrigerant amount"), "refrigerant_amount", ImportDialogueTableColumn::Numeric);
    col = circuits_table->addColumn(QApplication::translate("Circuit", "Oil"), "oil", ImportDialogueTableColumn::Select);
    for (int n = attributeValues().indexOfKey("oil") + 1; n < attributeValues().count() && attributeValues().key(n).startsWith("oil::"); ++n) {
        string_value = attributeValues().key(n).mid(attributeValues().key(n).lastIndexOf(':') + 1);
        col->addSelectValue(string_value, string_value);
        col->addSelectValue(attributeValues().value(n).toLower(), string_value);
    }
    circuits_table->addColumn(QApplication::translate("Circuit", "Oil amount"), "oil_amount", ImportDialogueTableColumn::Numeric);
    circuits_table->addColumn(QApplication::translate("Circuit", "Hermetically sealed"), "hermetic", ImportDialogueTableColumn::Boolean);
    circuits_table->addColumn(QApplication::translate("Circuit", "Fixed leakage detector installed"), "leak_detector", ImportDialogueTableColumn::Boolean);
    circuits_table->addColumn(QApplication::translate("Circuit", "Year of purchase"), "year", ImportDialogueTableColumn::Integer);
    circuits_table->addColumn(QApplication::translate("Circuit", "Date of commissioning"), "commissioning", ImportDialogueTableColumn::Date);
    col = circuits_table->addColumn(QApplication::translate("Circuit", "Status"), "disused", ImportDialogueTableColumn::Select);
    col->addSelectValue(QString::number(Circuit::Commissioned), QApplication::translate("Circuit", "Commissioned"));
    col->addSelectValue(QString::number(Circuit::ExcludedFromAgenda), QApplication::translate("Circuit", "Excluded from Agenda"));
    col->addSelectValue(QString::number(Circuit::Decommissioned), QApplication::translate("Circuit", "Decommissioned"));
    circuits_table->addColumn(QApplication::translate("Circuit", "Run-time per day"), "runtime", ImportDialogueTableColumn::Numeric);
    circuits_table->addColumn(QApplication::translate("Circuit", "Rate of utilisation"), "utilisation", ImportDialogueTableColumn::Numeric);
    circuits_table->addColumn(tr("Notes"), "notes", ImportDialogueTableColumn::Text);
    tables.append(circuits_table);

    table = circuits_table->addChildTableTemplate(tr("Compressors"), "compressors", "circuit_uuid");
    table->addColumn(QApplication::translate("Compressor", "Compressor name"), "name", ImportDialogueTableColumn::Text);
    table->addColumn(QApplication::translate("Compressor", "Manufacturer"), "manufacturer", ImportDialogueTableColumn::Text);
    table->addColumn(QApplication::translate("Compressor", "Type"), "type", ImportDialogueTableColumn::Text);
    table->addColumn(QApplication::translate("Compressor", "Serial number"), "sn", ImportDialogueTableColumn::Text);

    table = new ImportDialogueTable(tr("Circuit unit types"), "circuit_unit_types");
    table->addColumn(QApplication::translate("CircuitUnitType", "Manufacturer"), "manufacturer", ImportDialogueTableColumn::Text);
    table->addColumn(QApplication::translate("CircuitUnitType", "Type"), "type", ImportDialogueTableColumn::Text);
    col = table->addColumn(QApplication::translate("CircuitUnitType", "Refrigerant"), "refrigerant", ImportDialogueTableColumn::Select);
    foreach (string_value, refrigerants)
        col->addSelectValue(string_value.toLower(), string_value);
    table->addColumn(QApplication::translate("CircuitUnitType", "Refrigerant amount"), "refrigerant_amount", ImportDialogueTableColumn::Numeric);
    col = table->addColumn(QApplication::translate("CircuitUnitType", "Oil"), "oil", ImportDialogueTableColumn::Select);
    for (int n = attributeValues().indexOfKey("oil") + 1; n < attributeValues().count() && attributeValues().key(n).startsWith("oil::"); ++n) {
        string_value = attributeValues().key(n).mid(attributeValues().key(n).lastIndexOf(':') + 1);
        col->addSelectValue(string_value, string_value);
        col->addSelectValue(attributeValues().value(n).toLower(), string_value);
    }
    table->addColumn(QApplication::translate("CircuitUnitType", "Oil amount"), "oil_amount", ImportDialogueTableColumn::Numeric);
    table->addColumn(QApplication::translate("CircuitUnitType", "Acquisition price"), "acquisition_price", ImportDialogueTableColumn::Numeric);
    table->addColumn(QApplication::translate("CircuitUnitType", "List price"), "list_price", ImportDialogueTableColumn::Numeric);
    table->addColumn(QApplication::translate("CircuitUnitType", "Discount"), "discount", ImportDialogueTableColumn::Numeric);
    col = table->addColumn(QApplication::translate("CircuitUnitType", "Location"), "location", ImportDialogueTableColumn::Select);
    col->addSelectValue("external", QString::number(CircuitUnitType::External));
    col->addSelectValue("internal", QString::number(CircuitUnitType::Internal));
    table->addColumn(QApplication::translate("CircuitUnitType", "Output"), "output", ImportDialogueTableColumn::Numeric);
    col = table->addColumn(tr("Output unit"), "output_unit", ImportDialogueTableColumn::Select);
    col->addSelectValue("kW", "kW");
    col->addSelectValue("m3", "m3");
    table->addColumn(QApplication::translate("CircuitUnitType", "Output at t0/tc"), "output_t0_tc", ImportDialogueTableColumn::Numeric);
    table->addColumn(QApplication::translate("CircuitUnitType", "Notes"), "notes", ImportDialogueTableColumn::Text);
    tables.append(table);

    ImportCsvDialogue id(path, tables, this);
    if (id.exec() == QDialog::Accepted) {
        UndoCommand command(m_undo_stack, tr("Import CSV %1").arg(QFileInfo(path).completeBaseName()));
        m_undo_stack->savepoint();

        int num_failed = id.save();

        if (num_failed)
            QMessageBox::critical(this, tr("Import CSV - Leaklog"), tr("Failed to import %1 of %2 records.").arg(num_failed).arg(id.fileContent().count()));
        else
            QMessageBox::information(this, tr("Import CSV - Leaklog"), tr("Successfully imported %n record(s).", "", id.fileContent().count()));

        setDatabaseModified(true);
        refreshView();
    }
}

void MainWindow::addAssemblyRecordType()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    AssemblyRecordType record("");
    UndoCommand command(m_undo_stack, tr("Add assembly record type"));
    EditAssemblyRecordDialogue md(&record, m_undo_stack, this);
    if (md.exec() == QDialog::Accepted) {
        setDatabaseModified(true);
        m_tab->loadAssemblyRecordType(record.uuid(), true);
    }
}

void MainWindow::editAssemblyRecordType()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!m_tab->isAssemblyRecordTypeSelected()) { return; }
    AssemblyRecordType record(m_tab->selectedAssemblyRecordTypeUUID());
    UndoCommand command(m_undo_stack, tr("Edit assembly record type %1").arg(record.stringValue("name")));
    EditAssemblyRecordDialogue md(&record, m_undo_stack, this);
    if (md.exec() == QDialog::Accepted) {
        setDatabaseModified(true);
        m_tab->loadAssemblyRecordType(record.uuid(), false);
        refreshView();
    }
}

void MainWindow::removeAssemblyRecordType()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!m_tab->isAssemblyRecordTypeSelected()) { return; }
    AssemblyRecordType record(m_tab->selectedAssemblyRecordTypeUUID());
    if (RemoveDialogue::confirm(this, tr("Remove assembly record type - Leaklog"),
                                tr("Are you sure you want to remove the selected assembly record type?\nTo remove all data about the record \"%1\" type REMOVE and confirm:")
                                .arg(record.name())) != QDialog::Accepted)
        return;

    UndoCommand command(m_undo_stack, tr("Remove assembly record type %1").arg(record.stringValue("name")));
    m_undo_stack->savepoint();

    record.remove();
    m_tab->clearSelectedAssemblyRecordType();
    enableTools();
    setDatabaseModified(true);
    m_tab->setView(View::AssemblyRecordTypes);
}

void MainWindow::addAssemblyRecordItemType()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!m_tab->isAssemblyRecordItemCategorySelected()) { return; }
    if (AssemblyRecordItemCategory::isPredefined(m_tab->selectedAssemblyRecordItemCategoryUUID())) { return; }
    AssemblyRecordItemType record;
    record.setValue("ar_item_category_uuid", m_tab->selectedAssemblyRecordItemCategoryUUID());
    UndoCommand command(m_undo_stack, tr("Add assembly record item type"));
    EditDialogue md(&record, m_undo_stack, this);
    if (md.exec() == QDialog::Accepted) {
        setDatabaseModified(true);
        m_tab->loadAssemblyRecordItemType(record.uuid(), true);
    }
}

void MainWindow::editAssemblyRecordItemType()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!m_tab->isAssemblyRecordItemTypeSelected()) { return; }
    AssemblyRecordItemType record(m_tab->selectedAssemblyRecordItemTypeUUID());
    UndoCommand command(m_undo_stack, tr("Edit assembly record item type %1").arg(record.stringValue("name")));
    EditDialogue md(&record, m_undo_stack, this);
    if (md.exec() == QDialog::Accepted) {
        setDatabaseModified(true);
        m_tab->loadAssemblyRecordItemType(record.uuid(), false);
        refreshView();
    }
}

void MainWindow::removeAssemblyRecordItemType()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!m_tab->isAssemblyRecordItemTypeSelected()) { return; }
    AssemblyRecordItemType record(m_tab->selectedAssemblyRecordItemTypeUUID());
    if (RemoveDialogue::confirm(this, tr("Remove assembly record item type - Leaklog"),
                                tr("Are you sure you want to remove the selected assembly record item type?\nTo remove all data about the record item \"%1\" type REMOVE and confirm:")
                                .arg(record.name())) != QDialog::Accepted)
        return;

    UndoCommand command(m_undo_stack, tr("Remove assembly record item type %1").arg(record.stringValue("name")));
    m_undo_stack->savepoint();

    record.remove();
    m_tab->clearSelectedAssemblyRecordItemType();
    enableTools();
    setDatabaseModified(true);
    m_tab->setView(View::AssemblyRecordItems);
}

void MainWindow::addAssemblyRecordItemCategory()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    AssemblyRecordItemCategory record;
    UndoCommand command(m_undo_stack, tr("Add assembly record item category"));
    EditDialogue md(&record, m_undo_stack, this);
    if (md.exec() == QDialog::Accepted) {
        setDatabaseModified(true);
        m_tab->loadAssemblyRecordItemCategory(record.uuid(), true);
    }
}

void MainWindow::editAssemblyRecordItemCategory()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!m_tab->isAssemblyRecordItemCategorySelected()) { return; }
    AssemblyRecordItemCategory record(m_tab->selectedAssemblyRecordItemCategoryUUID());
    UndoCommand command(m_undo_stack, tr("Edit assembly record item category %1").arg(record.name()));
    EditDialogue md(&record, m_undo_stack, this);
    if (md.exec() == QDialog::Accepted) {
        setDatabaseModified(true);
        m_tab->loadAssemblyRecordItemCategory(record.uuid(), false);
        refreshView();
    }
}

void MainWindow::removeAssemblyRecordItemCategory()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!m_tab->isAssemblyRecordItemCategorySelected()) { return; }

    AssemblyRecordItemCategory category(m_tab->selectedAssemblyRecordItemCategoryUUID());

    if (RemoveDialogue::confirm(this, tr("Remove assembly record item category - Leaklog"),
                                tr("Are you sure you want to remove the selected assembly record item category?\nTo remove all data about the item category \"%1\" type REMOVE and confirm:")
                                .arg(category.name())) != QDialog::Accepted)
        return;

    UndoCommand command(m_undo_stack, tr("Remove assembly record item category %1").arg(category.name()));
    m_undo_stack->savepoint();

    category.remove();
    m_tab->clearSelectedAssemblyRecordItemCategory();
    enableTools();
    setDatabaseModified(true);
    m_tab->setView(View::AssemblyRecordItems);
}

void MainWindow::addCircuitUnitType()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    CircuitUnitType unit_type;
    UndoCommand command(m_undo_stack, tr("Add circuit unit type"));
    EditDialogue md(&unit_type, m_undo_stack, this);
    if (md.exec() == QDialog::Accepted) {
        setDatabaseModified(true);
        m_tab->loadCircuitUnitType(unit_type.uuid(), true);
    }
}

void MainWindow::editCircuitUnitType()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!m_tab->isCircuitUnitTypeSelected()) { return; }
    CircuitUnitType record(m_tab->selectedCircuitUnitTypeUUID());
    UndoCommand command(m_undo_stack, tr("Edit circuit unit type %1").arg(record.stringValue("type")));
    EditDialogue md(&record, m_undo_stack, this);
    if (md.exec() == QDialog::Accepted) {
        setDatabaseModified(true);
        m_tab->loadCircuitUnitType(record.uuid(), false);
        refreshView();
    }
}

void MainWindow::removeCircuitUnitType()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!m_tab->isCircuitUnitTypeSelected()) { return; }

    CircuitUnitType unit_type(m_tab->selectedCircuitUnitTypeUUID());

    if (RemoveDialogue::confirm(this, tr("Remove circuit unit type - Leaklog"),
                                tr("Are you sure you want to remove the selected circuit unit type?\nTo remove all data about the unit type \"%1\" type REMOVE and confirm:")
                                .arg(unit_type.stringValue("type"))) != QDialog::Accepted)
        return;

    UndoCommand command(m_undo_stack, tr("Remove circuit unit type %1").arg(unit_type.stringValue("type")));
    m_undo_stack->savepoint();

    unit_type.remove();
    m_tab->clearSelectedCircuitUnitType();
    enableTools();
    setDatabaseModified(true);
    m_tab->setView(View::CircuitUnitTypes);
}

void MainWindow::addStyle()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!isOperationPermitted("add_style")) { return; }

    Style record;
    UndoCommand command(m_undo_stack, tr("Add style"));
    EditDialogue md(&record, m_undo_stack, this);
    if (md.exec() == QDialog::Accepted) {
        QListWidgetItem *item = new QListWidgetItem;
        item->setText(record.name());
        item->setData(Qt::UserRole, record.uuid());
        lw_styles->addItem(item);
        setDatabaseModified(true);
        refreshView();
    }
}

void MainWindow::editStyle()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!lw_styles->currentIndex().isValid()) { return; }
    if (!isOperationPermitted("edit_style")) { return; }
    QListWidgetItem *item = lw_styles->currentItem();
    Style record(item->data(Qt::UserRole).toString());
    UndoCommand command(m_undo_stack, tr("Edit style %1").arg(record.name()));
    EditDialogue md(&record, m_undo_stack, this);
    if (md.exec() == QDialog::Accepted) {
        item->setText(record.name());
        setDatabaseModified(true);
        refreshView();
    }
}

void MainWindow::removeStyle()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!lw_styles->currentIndex().isValid()) { return; }
    if (!isOperationPermitted("remove_style")) { return; }
    QListWidgetItem *item = lw_styles->currentItem();
    if (RemoveDialogue::confirm(this, tr("Remove style - Leaklog"),
                                tr("Are you sure you want to remove the selected style?\nTo remove the style \"%1\" type REMOVE and confirm:")
                                .arg(item->text())) != QDialog::Accepted)
        return;

    Style record(item->data(Qt::UserRole).toString());

    UndoCommand command(m_undo_stack, tr("Remove style %1").arg(record.stringValue("name")));
    m_undo_stack->savepoint();

    record.remove();
    delete item;
    enableTools();
    setDatabaseModified(true);
    refreshView();
}

QString MainWindow::appendDefaultOrderToColumn(const QString &column)
{
    if (column.isEmpty())
        return column;

    QString column_name = column.split('.').last();

    int order = Qt::AscendingOrder;

    if ((actionMost_recent_first->isChecked() && (column_name.contains("date") || column_name.contains("commissioning"))) ||
        column_name.endsWith("_count"))
        order = Qt::DescendingOrder;

    return QString("%1 %2").arg(column).arg(order == Qt::AscendingOrder ? "ASC" : "DESC");
}
