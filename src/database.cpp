/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2017 Matus & Michal Tomlein

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
#include "editdialoguewithautoid.h"
#include "importdialogue.h"
#include "importcsvdialogue.h"
#include "records.h"
#include "removedialogue.h"
#include "mtlistwidget.h"
#include "mtaddress.h"
#include "mtvariant.h"
#include "undostack.h"
#include "sha256.h"

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

void MainWindow::initDatabase(QSqlDatabase &database, bool transaction, bool save_on_upgrade)
{
    if (transaction) { database.transaction(); }
{ // (SCOPE)
    MTSqlQuery query(database);
    QStringList tables = database.tables();
    Variables *variables = Variables::defaultVariables();
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
                query.bindValue(":date_updated", QDateTime::currentDateTime().toString(DATE_TIME_FORMAT));
                query.bindValue(":updated_by", currentUser());
                query.exec();
                query.prepare("UPDATE variables SET type = 'group', date_updated = :date_updated, updated_by = :updated_by WHERE id = :id");
                query.bindValue(":id", subvariables.value(0));
                query.bindValue(":date_updated", QDateTime::currentDateTime().toString(DATE_TIME_FORMAT));
                query.bindValue(":updated_by", currentUser());
                query.exec();
            }

            query.exec("DROP TABLE subvariables");
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
            }
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
    double v = DBInfo::valueForKey("db_version").toDouble();
    if (v > 0 && v < 0.909) {
        Table("", "Leakages").remove();
        Table("", "Pressures and temperatures").remove();
        Table("", "Electrical parameters").remove();
        Table("", "Compressors").remove();
    }
    if (v > 0 && v < 0.9082) {
        Table(tr("Leakages")).remove();
        Table(tr("Pressures and temperatures")).remove();
        Table(tr("Compressors")).remove();
    }
    int tables_version = DBInfo::valueForKey("tables_version").toInt();
    if (tables_version < 1) {
        DBInfo::setValueForKey("tables_version", "1");
        Table("", "90").remove();
        Table("", "70").remove();
        Table("", "40").remove();
    }
    QVariantMap set;
    Table leakages("", "90");
    if (!leakages.exists()) {
        set.insert("id", tr("Leakages"));
        set.insert("highlight_nominal", 0);
        set.insert("variables", "vis_aur_chk;dir_leak_chk;refr_add_am;refr_add_per;refr_reco;oil_leak_am;inspector;operator;risks;rmds;arno");
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
    QString path = QFileDialog::getSaveFileName(this, tr("New database - Leaklog"),
                                                QDir::home().absoluteFilePath(tr("untitled.lklg")),
                                                tr("Leaklog Database (*.lklg)"));
    if (path.isEmpty()) { return; }
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
    db.transaction();
    DBInfo::setValueForKey("created_with", QString("Leaklog-%1").arg(F_LEAKLOG_VERSION));
    DBInfo::setValueForKey("date_created", QDateTime::currentDateTime().toString(DATE_TIME_FORMAT));
    openDatabase(QString(), path);
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
        if (db.open()) {
            db.transaction();
            initDatabase(db, false);
        }
        openDatabase(QString(), s);
    } else {
        openDatabase(s, s);
    }
}

void MainWindow::open()
{
    if (saveChangesBeforeProceeding(tr("Open database - Leaklog"), true)) { return; }
    QString path = QFileDialog::getOpenFileName(this, tr("Open database - Leaklog"), QDir::homePath(),
                                                tr("Leaklog Databases (*.lklg);;All files (*.*)"));
    if (path.isEmpty()) { return; }
    addRecent(path);
    openDatabase(path, path);
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
        db.transaction();
        initDatabase(db, false);
    }

    openDatabase(QString(), connection_string);
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
            int min_diff = INT_MAX;
            int delete_index = 1;
            for (int i = 0; i + 3 < entries.count(); ++i) {
                int diff = dateForFileName(entries.at(i)).daysTo(dateForFileName(entries.at(i + 1)));
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

void MainWindow::openDatabase(QString path, const QString &connection_string)
{
    m_connection_string = connection_string;

    if (path.isEmpty()) {
        QSqlDatabase db = QSqlDatabase::database();
        path = db.databaseName();
        if (!db.isOpen()) {
            QMessageBox::critical(this, tr("Open database - Leaklog"), tr("Cannot open database %1:\n%2.").arg(path).arg(db.lastError().text()));
            clearWindowTitle();
            return;
        }
    } else {
        QFile file(path);
        if (!file.exists()) {
            QMessageBox::critical(this, tr("Open database - Leaklog"), tr("File %1 does not exist.").arg(path));
            clearWindowTitle();
            return;
        }

        backupDatabase(path);

        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName(path);
        if (!db.open()) {
            QMessageBox::critical(this, tr("Open database - Leaklog"), tr("Cannot read file %1:\n%2.").arg(path).arg(db.lastError().text()));
            clearWindowTitle();
            return;
        }
        db.transaction();
        initDatabase(db, false);
    }

    if (DBInfo::valueForKey("min_leaklog_version", DBInfo::valueForKey("db_version")).toDouble() > F_LEAKLOG_VERSION) {
        QMessageBox::warning(this, tr("Open database - Leaklog"), tr("A newer version of Leaklog is required to open this database."));
        closeDatabase(false);
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

    MTSqlQuery query("SELECT date FROM refrigerant_management WHERE purchased > 0 OR purchased_reco > 0");
    if (!query.next())
        QMessageBox::information(this, tr("Refrigerant management"), tr("You should add a record of purchase for every kind of refrigerant you have in store. You can do so by clicking the \"Add record of refrigerant management\" button."));
}

void MainWindow::loadDatabase(bool reload)
{
    if (reload) {
        trw_variables->clear();

        cb_table_edit->clear();

        lw_warnings->clear();

        lw_styles->clear();
    }

    loadVariables(trw_variables);

    QStringList tables;

    MTSqlQuery query("SELECT id FROM tables ORDER BY uid DESC, id ASC");
    while (query.next()) {
        cb_table_edit->addItem(query.value(0).toString());
        tables << query.value(0).toString();
    }

    emit tablesChanged(tables);

    // loadTable(cb_table_edit->currentText());

    Warnings warnings;
    while (warnings.next()) {
        QListWidgetItem *item = new QListWidgetItem;
        item->setText(warnings.value("description").toString().isEmpty() ? warnings.value("name").toString() : tr("%1 (%2)").arg(warnings.value("name").toString()).arg(warnings.value("description").toString()));
        item->setData(Qt::UserRole, warnings.value("id").toString());
        lw_warnings->addItem(item);
    }

    ListOfVariantMaps styles = Style().listAll("id, name");
    for (int i = 0; i < styles.count(); ++i) {
        QListWidgetItem *item = new QListWidgetItem;
        item->setText(styles.at(i).value("name").toString());
        item->setData(Qt::UserRole, styles.at(i).value("id"));
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
}

void MainWindow::saveAndCompact()
{
    saveDatabase(true);
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
        refreshView();
    }
}

void MainWindow::autosave()
{
    if (!isWindowModified())
        return;

    if (!QSqlDatabase::database().isOpen())
        return;

    QString autosave_mode = DBInfo::valueForKey("autosave");
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

    if (autosave_mode == "ask" || autosave_mode == "delayed")
        saveDatabase(false);
}

void MainWindow::closeDatabase(bool save)
{
    if (save && saveChangesBeforeProceeding(tr("Close database - Leaklog"), false))
        return;

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

void MainWindow::setDatabaseModified(bool modified)
{
    setWindowModified(modified);

    if (modified)
        emit databaseModified();

    if (QSqlDatabase::database().isOpen() && DBInfo::valueForKey("autosave") == "immediate") {
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
            QStringList tables;
            for (int i = 0; i < cb_table_edit->count(); ++i)
                tables << cb_table_edit->itemText(i);
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

bool MainWindow::canRemoveCircuit(const QString &customer_id, const QString &circuit_id)
{
    if (superuserModeEnabled())
        return true;

    MTSqlQuery query;
    query.prepare(QString("SELECT date FROM inspections"
                          " WHERE %1 AND ((refr_add_am IS NOT NULL AND CAST(refr_add_am AS NUMERIC) <> 0)"
                          " OR (refr_reco IS NOT NULL AND CAST(refr_reco AS NUMERIC) <> 0)) LIMIT 1")
                  .arg(circuit_id.isEmpty() ? "customer = :customer" : "customer = :customer AND circuit = :circuit"));
    query.bindValue(":customer", customer_id);
    if (!circuit_id.isEmpty())
        query.bindValue(":circuit", circuit_id);

    if (query.exec() && query.next()) {
        QMessageBox message(this);
        message.setWindowModality(Qt::WindowModal);
        message.setWindowFlags(message.windowFlags() | Qt::Sheet);
        message.setIcon(QMessageBox::Warning);
        message.setWindowTitle(circuit_id.isEmpty() ? tr("Remove customer - Leaklog") : tr("Remove circuit - Leaklog"));
        message.setText(circuit_id.isEmpty() ? tr("You cannot remove the selected customer.") : tr("You cannot remove the selected circuit."));
        message.setInformativeText(circuit_id.isEmpty() ? tr("Removing this customer would affect the store.") : tr("Removing this circuit would affect the store."));
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

    QSet<QString> predefined_refrigerants = listRefrigerants(false).toSet();
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

    QList<MTDictionary> updated_refrigerants = table->allValues();
    foreach (const MTDictionary &updated_refrigerant, updated_refrigerants) {
        QVariantMap refrigerant;
        refrigerant.insert("name", updated_refrigerant.value("name"));
        refrigerant.insert("gwp", updated_refrigerant.value("gwp"));
        refrigerants << refrigerant;
    }

    DBInfo::setRefrigerants(refrigerants);
    setDatabaseModified(true);
    refreshView();
}

void MainWindow::editServiceCompany()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!isOperationPermitted("edit_service_company")) { return; }
    ServiceCompany record(DBInfo::valueForKey("default_service_company"));
    UndoCommand command(m_undo_stack, tr("Edit service company information"));
    EditDialogue md(&record, m_undo_stack, this);
    if (md.exec() == QDialog::Accepted) {
        DBInfo::setValueForKey("default_service_company", record.id());
        setDatabaseModified(true);
        refreshView();
    }
}

void MainWindow::addRefrigerantRecord()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!isOperationPermitted("add_refrigerant_management")) { return; }
    editRefrigerantRecord("");
}

void MainWindow::editRefrigerantRecord(const QString &date)
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!date.isEmpty() && isRecordLocked(date)) { return; }
    RefrigerantRecord record(date);
    if (!isOperationPermitted("edit_refrigerant_management", record.stringValue("updated_by"))) { return; }
    UndoCommand command(m_undo_stack, date.isEmpty()
                        ? tr("Add record of refrigerant management")
                        : tr("Edit record of refrigerant management %1").arg(m_settings.formatDateTime(date)));
    EditDialogue md(&record, m_undo_stack, this);
    if (md.exec() == QDialog::Accepted) {
        QVariantMap attributes = record.list();
        if (attributes.value("purchased").toDouble() <= 0.0 && attributes.value("purchased_reco").toDouble() <= 0.0 &&
            attributes.value("sold").toDouble() <= 0.0 && attributes.value("sold_reco").toDouble() <= 0.0 &&
            attributes.value("refr_rege").toDouble() <= 0.0 && attributes.value("refr_disp").toDouble() <= 0.0 &&
            attributes.value("leaked").toDouble() <= 0.0 && attributes.value("leaked_reco").toDouble() <= 0.0) {
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
    Customer record("");
    UndoCommand command(m_undo_stack, tr("Add customer"));
    EditCustomerDialogue md(&record, m_undo_stack, this);
    if (md.exec() == QDialog::Accepted) {
        setDatabaseModified(true);
        m_tab->loadCustomer(record.id().toInt(), true);
    }
}

void MainWindow::editCustomer()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!m_tab->isCustomerSelected()) { return; }
    Customer record(m_tab->selectedCustomer());
    if (!isOperationPermitted("edit_customer", record.stringValue("updated_by"))) { return; }
    QString old_id = m_tab->selectedCustomer();
    QString old_company_name = record.stringValue("company");
    UndoCommand command(m_undo_stack, tr("Edit customer %1%2")
                        .arg(formatCompanyID(old_id))
                        .arg(old_company_name.isEmpty() ? QString() : QString(" (%1)").arg(old_company_name)));
    EditCustomerDialogue md(&record, m_undo_stack, this);
    if (md.exec() == QDialog::Accepted) {
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
            m_tab->loadCustomer(record.id().toInt(), false);
        } else if (old_company_name != company_name) {
            MTSqlQuery update_repairs;
            update_repairs.prepare("UPDATE repairs SET customer = :customer WHERE parent = :id");
            update_repairs.bindValue(":id", record.id());
            update_repairs.bindValue(":customer", company_name);
            update_repairs.exec();
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
    Customer record(m_tab->selectedCustomer());
    record.readValues();
    record.id().clear();
    QString company_name = record.stringValue("company");
    UndoCommand command(m_undo_stack, tr("Duplicate customer %1%2")
                        .arg(formatCompanyID(m_tab->selectedCustomer()))
                        .arg(company_name.isEmpty() ? QString() : QString(" (%1)").arg(company_name)));
    EditDialogue md(&record, m_undo_stack, this);
    if (md.exec() == QDialog::Accepted) {
        setDatabaseModified(true);
        m_tab->loadCustomer(record.id().toInt(), true);
    }
}

void MainWindow::removeCustomer()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!m_tab->isCustomerSelected()) { return; }
    if (!canRemoveCircuit(m_tab->selectedCustomer())) { return; }
    Customer record(m_tab->selectedCustomer());
    record.readValues("company, updated_by");
    if (!isOperationPermitted("remove_customer", record.stringValue("updated_by"))) { return; }
    if (RemoveDialogue::confirm(this, tr("Remove customer - Leaklog"),
                                tr("Are you sure you want to remove the selected customer?\nTo remove all data about the customer \"%1\" type REMOVE and confirm:")
                                .arg(m_tab->selectedCustomer())) != QDialog::Accepted)
        return;

    QString company_name = record.stringValue("company");
    UndoCommand command(m_undo_stack, tr("Remove customer %1%2")
                        .arg(formatCompanyID(m_tab->selectedCustomer()))
                        .arg(company_name.isEmpty() ? QString() : QString(" (%1)").arg(company_name)));
    m_undo_stack->savepoint();

    record.remove();
    Circuit circuits(m_tab->selectedCustomer(), "");
    circuits.remove();
    MTRecord inspections("inspections", "date", "", MTDictionary("customer", m_tab->selectedCustomer()));
    inspections.remove();
    MTRecord repairs("repairs", "date", "", MTDictionary("parent", m_tab->selectedCustomer()));
    repairs.remove();
    m_tab->setSelectedCustomer(-1);
    m_tab->setSelectedCircuit(-1);
    m_tab->setSelectedCompressor(-1);
    m_tab->clearSelectedInspection();
    m_tab->clearSelectedRepair();
    enableTools();
    setDatabaseModified(true);
    m_tab->setView(View::Customers);
}

void MainWindow::decommissionAllCircuits()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!m_tab->isCustomerSelected()) { return; }
    if (!isOperationPermitted("decommission_circuit")) { return; }

    Customer customer(m_tab->selectedCustomer());
    customer.readValues("company");

    QDialog d(this);
    d.setWindowTitle(tr("Decommission all circuits - Leaklog"));
    QGridLayout *gl = new QGridLayout(&d);

    QLabel *lbl = new QLabel(tr("Decommission all circuits of:"), &d);
    lbl->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    gl->addWidget(lbl, 0, 0);

    gl->addWidget(new QLabel(customer.stringValue("company"), &d), 0, 1);

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
#if QT_VERSION < QT_VERSION_CHECK(4, 8, 0)
    date->calendarWidget()->setFirstDayOfWeek(Qt::Monday);
#else
    date->calendarWidget()->setFirstDayOfWeek(QLocale().firstDayOfWeek());
#endif
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

    QString company_name = customer.stringValue("company");
    UndoCommand command(m_undo_stack, tr("Decommission all circuits of customer %1%2")
                        .arg(formatCompanyID(m_tab->selectedCustomer()))
                        .arg(company_name.isEmpty() ? QString() : QString(" (%1)").arg(company_name)));
    m_undo_stack->savepoint();

    QVariantMap set;
    set.insert("disused", exclude_from_agenda->isChecked() ? Circuit::ExcludedFromAgenda : Circuit::Decommissioned);
    set.insert("decommissioning", date->date().toString(DATE_FORMAT));
    set.insert("decommissioning_reason", decommissioning_reason->text());

    Circuit circuits(customer.id(), QString());
    circuits.parents().insert("disused", QString::number(Circuit::Commissioned));
    circuits.update(set, false, true);

    setDatabaseModified(true);
    refreshView();
}

void MainWindow::addCircuit()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!m_tab->isCustomerSelected()) { return; }
    if (!isOperationPermitted("add_circuit")) { return; }
    Circuit record(m_tab->selectedCustomer(), "");
    UndoCommand command(m_undo_stack, tr("Add circuit"));
    EditCircuitDialogue md(&record, m_undo_stack, this);
    if (md.exec() == QDialog::Accepted) {
        setDatabaseModified(true);
        m_tab->loadCircuit(record.id().toInt(), true);
    }
}

void MainWindow::editCircuit()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!m_tab->isCustomerSelected()) { return; }
    if (!m_tab->isCircuitSelected()) { return; }
    Circuit record(m_tab->selectedCustomer(), m_tab->selectedCircuit());
    if (!isOperationPermitted("edit_circuit", record.stringValue("updated_by"))) { return; }
    QString company_name = Customer(m_tab->selectedCustomer()).stringValue("company");
    UndoCommand command(m_undo_stack, tr("Edit circuit %1 (%2)")
                        .arg(m_tab->selectedCircuit().rightJustified(5, '0'))
                        .arg(company_name.isEmpty() ? formatCompanyID(m_tab->selectedCustomer()) : company_name));
    EditCircuitDialogue md(&record, m_undo_stack, this);
    QString old_id = m_tab->selectedCircuit();
    if (md.exec() == QDialog::Accepted) {
        if (old_id != record.id()) {
            Circuit::cascadeIDChange(m_tab->selectedCustomer().toInt(), old_id.toInt(), record.id().toInt());
            m_tab->loadCircuit(record.id().toInt(), false);
        } else {
            enableTools();
        }
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
    Circuit record(m_tab->selectedCustomer(), m_tab->selectedCircuit());
    record.readValues();
    record.id().clear();
    QString company_name = Customer(m_tab->selectedCustomer()).stringValue("company");
    UndoCommand command(m_undo_stack, tr("Duplicate circuit %1 (%2)")
                        .arg(m_tab->selectedCircuit().rightJustified(5, '0'))
                        .arg(company_name.isEmpty() ? formatCompanyID(m_tab->selectedCustomer()) : company_name));
    EditDialogue md(&record, m_undo_stack, this);
    if (md.exec() == QDialog::Accepted) {
        ListOfVariantMaps compressors = Compressor(QString(),
                                                   MTDictionary(QStringList() << "customer_id" << "circuit_id",
                                                                QStringList() << m_tab->selectedCustomer()
                                                                              << m_tab->selectedCircuit())).listAll();

        qint64 next_id = qMax(Compressor().max("id") + (qint64)1, (qint64)QDateTime::currentDateTime().toTime_t());
        for (int i = 0; i < compressors.size(); ++i) {
            compressors[i].insert("id", next_id++);
            compressors[i].insert("circuit_id", record.id());
            Compressor().update(compressors[i]);
        }

        ListOfVariantMaps circuit_units = CircuitUnit(QString(),
                                                      MTDictionary(QStringList() << "company_id" << "circuit_id",
                                                                   QStringList() << m_tab->selectedCustomer()
                                                                                 << m_tab->selectedCircuit())).listAll();

        next_id = CircuitUnit().max("id");
        for (int i = 0; i < circuit_units.size(); ++i) {
            circuit_units[i].insert("id", ++next_id);
            circuit_units[i].insert("circuit_id", record.id());
            CircuitUnit().update(circuit_units[i]);
        }

        setDatabaseModified(true);
        m_tab->loadCircuit(record.id().toInt(), true);
    }
}

void MainWindow::duplicateAndDecommissionCircuit()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!m_tab->isCustomerSelected()) { return; }
    if (!m_tab->isCircuitSelected()) { return; }
    if (!isOperationPermitted("add_circuit")) { return; }
    if (!isOperationPermitted("decommission_circuit")) { return; }

    Circuit circuit(m_tab->selectedCustomer(), m_tab->selectedCircuit());
    QVariantMap attributes = circuit.list();

    QDialog d(this);
    d.setWindowTitle(tr("Duplicate and decommission - Leaklog"));
    QGridLayout *gl = new QGridLayout(&d);

    QLabel *lbl = new QLabel(tr("Duplicate and decommission circuit:"), &d);
    lbl->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    gl->addWidget(lbl, 0, 0);

    gl->addWidget(new QLabel(m_tab->selectedCircuit().rightJustified(5, '0'), &d), 0, 1);

    lbl = new QLabel(tr("%1:").arg(Circuit::attributes().value("decommissioning")), &d);
    lbl->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    gl->addWidget(lbl, 1, 0);

    QDateEdit *date = new QDateEdit(&d);
    date->setDisplayFormat(m_settings.dateFormatString());
    date->setDate(QDate::currentDate());
    date->setCalendarPopup(true);
    date->calendarWidget()->setLocale(QLocale());
#if QT_VERSION < QT_VERSION_CHECK(4, 8, 0)
    date->calendarWidget()->setFirstDayOfWeek(Qt::Monday);
#else
    date->calendarWidget()->setFirstDayOfWeek(QLocale().firstDayOfWeek());
#endif
    gl->addWidget(date, 1, 1);

    QRadioButton *set_original_id = new QRadioButton(tr("Change ID of the original to:"), &d);
    set_original_id->setChecked(true);
    gl->addWidget(set_original_id, 2, 0);

    QSpinBox *new_id = new QSpinBox(&d);
    new_id->setRange(1, 99999);
    new_id->setValue(Circuit(m_tab->selectedCustomer(), QString()).max("id") + 1);
    gl->addWidget(new_id, 2, 1, 2, 1);

    QRadioButton *set_duplicate_id = new QRadioButton(tr("Choose a new ID for the duplicate:"), &d);
    gl->addWidget(set_duplicate_id, 3, 0);

    QStringList refrigerants = listRefrigerants();

    lbl = new QLabel(tr("Previous refrigerant:"), &d);
    lbl->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    gl->addWidget(lbl, 4, 0);

    QComboBox *old_refrigerant = new QComboBox(&d);
    old_refrigerant->addItems(refrigerants);
    old_refrigerant->setCurrentIndex(refrigerants.indexOf(attributes.value("refrigerant").toString()));
    gl->addWidget(old_refrigerant, 4, 1);

    lbl = new QLabel(tr("New refrigerant:"), &d);
    lbl->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    gl->addWidget(lbl, 5, 0);

    QComboBox *new_refrigerant = new QComboBox(&d);
    new_refrigerant->addItems(refrigerants);
    new_refrigerant->setCurrentIndex(refrigerants.indexOf(attributes.value("refrigerant").toString()));
    gl->addWidget(new_refrigerant, 5, 1);

    lbl = new QLabel(QApplication::translate("EditDialogue", "This ID is not available. Please choose a different ID."), &d);
    QFont bold;
    bold.setBold(true);
    lbl->setFont(bold);
    lbl->setWordWrap(true);
    lbl->setAlignment(Qt::AlignCenter);
    lbl->setVisible(false);
    gl->addWidget(lbl, 6, 0, 1, 2);

    QDialogButtonBox *bb = new QDialogButtonBox(&d);
    bb->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    bb->button(QDialogButtonBox::Ok)->setText(tr("Duplicate and Decommission"));
    bb->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));
    bb->button(QDialogButtonBox::Cancel)->setFocus();
    QObject::connect(bb, SIGNAL(accepted()), &d, SLOT(accept()));
    QObject::connect(bb, SIGNAL(rejected()), &d, SLOT(reject()));
    gl->addWidget(bb, 7, 0, 1, 2);

    int id = 0;
    do {
        if (id)
            lbl->setVisible(true);

        if (d.exec() != QDialog::Accepted)
            return;

        id = new_id->value();
    } while (Circuit(m_tab->selectedCustomer(), QString::number(id)).exists());

    QString company_name = Customer(m_tab->selectedCustomer()).stringValue("company");
    UndoCommand command(m_undo_stack, tr("Duplicate and decommission circuit %1 (%2)")
                        .arg(m_tab->selectedCircuit().rightJustified(5, '0'))
                        .arg(company_name.isEmpty() ? formatCompanyID(m_tab->selectedCustomer()) : company_name));
    m_undo_stack->savepoint();

    ListOfVariantMaps compressors = Compressor(QString(),
                                               MTDictionary(QStringList() << "customer_id" << "circuit_id",
                                                            QStringList() << m_tab->selectedCustomer()
                                                                          << m_tab->selectedCircuit())).listAll();

    ListOfVariantMaps circuit_units = CircuitUnit(QString(),
                                                  MTDictionary(QStringList() << "company_id" << "circuit_id",
                                                               QStringList() << m_tab->selectedCustomer()
                                                                             << m_tab->selectedCircuit())).listAll();

    QVariantMap set;
    set.insert("disused", Circuit::Decommissioned);
    set.insert("decommissioning", date->date().toString(DATE_FORMAT));
    set.insert("refrigerant", old_refrigerant->currentText());
    circuit.update(set);

    int duplicate_id;

    if (set_original_id->isChecked()) {
        duplicate_id = attributes.value("id").toInt();
        circuit.update("id", id);
        Circuit::cascadeIDChange(m_tab->selectedCustomer().toInt(), duplicate_id, id, -1, true);
    } else {
        duplicate_id = id;
        attributes.insert("id", id);
    }

    attributes.insert("refrigerant", new_refrigerant->currentText());
    Circuit().update(attributes);

    qint64 next_id = qMax(Compressor().max("id") + (qint64)1, (qint64)QDateTime::currentDateTime().toTime_t());
    for (int i = 0; i < compressors.size(); ++i) {
        compressors[i].insert("id", next_id++);
        compressors[i].insert("circuit_id", duplicate_id);
        Compressor().update(compressors[i]);
    }

    next_id = CircuitUnit().max("id");
    for (int i = 0; i < circuit_units.size(); ++i) {
        circuit_units[i].insert("id", ++next_id);
        circuit_units[i].insert("circuit_id", duplicate_id);
        CircuitUnit().update(circuit_units[i]);
    }

    setDatabaseModified(true);
    m_tab->loadCircuit(duplicate_id, true);
}

void MainWindow::moveCircuit()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!m_tab->isCustomerSelected()) { return; }
    if (!m_tab->isCircuitSelected()) { return; }
    if (!isOperationPermitted("add_circuit")) { return; }

    Circuit circuit(m_tab->selectedCustomer(), m_tab->selectedCircuit());
    QVariantMap attributes = circuit.list();

    if (!isOperationPermitted("remove_circuit", attributes.value("updated_by").toString())) { return; }

    QDialog d(this);
    d.setWindowTitle(tr("Move circuit to another customer - Leaklog"));
    QGridLayout *gl = new QGridLayout(&d);

    QLabel *lbl = new QLabel(tr("Move circuit:"), &d);
    lbl->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    gl->addWidget(lbl, 0, 0);

    gl->addWidget(new QLabel(m_tab->selectedCircuit().rightJustified(5, '0'), &d), 0, 1);

    lbl = new QLabel(tr("To customer:"), &d);
    lbl->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    gl->addWidget(lbl, 1, 0);

    QComboBox *cb_customer = new QComboBox(&d);

    int selected_customer = m_tab->selectedCustomer().toInt();
    ListOfVariantMaps customers = Customer("").listAll("id, company", "company");
    foreach (const QVariantMap &customer, customers) {
        QVariant id = customer.value("id");
        if (id.toInt() != selected_customer)
            cb_customer->addItem(customer.value("company").toString(), id);
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
    gl->addWidget(spb_circuit_id, 3, 1);

    QLabel *lbl_id_taken = new QLabel(QApplication::translate("EditDialogue", "This ID is not available. Please choose a different ID."), &d);
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
#if QT_VERSION < QT_VERSION_CHECK(4, 8, 0)
    date->calendarWidget()->setFirstDayOfWeek(Qt::Monday);
#else
    date->calendarWidget()->setFirstDayOfWeek(QLocale().firstDayOfWeek());
#endif
    gl->addWidget(date, 5, 1);

    QLabel *lbl_date_taken = new QLabel(QApplication::translate("EditDialogue", "This date is not available. Please choose a different date."), &d);
    lbl_date_taken->setFont(bold);
    lbl_date_taken->setWordWrap(true);
    lbl_date_taken->setAlignment(Qt::AlignCenter);
    lbl_date_taken->setVisible(false);
    gl->addWidget(lbl_date_taken, 6, 0, 1, 2);

    QDialogButtonBox *bb = new QDialogButtonBox(&d);
    bb->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    bb->button(QDialogButtonBox::Ok)->setText(tr("Move"));
    bb->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));
    bb->button(QDialogButtonBox::Cancel)->setFocus();
    QObject::connect(bb, SIGNAL(accepted()), &d, SLOT(accept()));
    QObject::connect(bb, SIGNAL(rejected()), &d, SLOT(reject()));
    gl->addWidget(bb, 7, 0, 1, 2);

    cb_customer->setCurrentIndex(-1);

    int customer_id = 0;
    int circuit_id = 0;
    QString inspection_date;
    bool inspection_date_taken = false;
    do {
        lbl_id_taken->setVisible(customer_id > 0 && circuit_id != 0 && !inspection_date_taken);
        lbl_select_customer->setVisible(customer_id < 0);
        lbl_date_taken->setVisible(inspection_date_taken);

        if (d.exec() != QDialog::Accepted)
            return;

        customer_id = cb_customer->currentIndex() < 0 ? -1 : cb_customer->itemData(cb_customer->currentIndex()).toInt();
        circuit_id = spb_circuit_id->value();
        inspection_date = date->dateTime().toString(DATE_TIME_FORMAT);
        inspection_date_taken = false;
    } while (customer_id < 0 || Circuit(QString::number(customer_id), QString::number(circuit_id)).exists() ||
             (inspection_date_taken = Inspection(m_tab->selectedCustomer(),
                                                 m_tab->selectedCircuit(),
                                                 inspection_date).exists()));

    QString company_name = Customer(m_tab->selectedCustomer()).stringValue("company");
    QString new_company_name = Customer(QString::number(customer_id)).stringValue("company");
    UndoCommand command(m_undo_stack, tr("Move circuit %1 (%2) to customer %3")
                        .arg(m_tab->selectedCircuit().rightJustified(5, '0'))
                        .arg(company_name.isEmpty() ? formatCompanyID(m_tab->selectedCustomer()) : company_name)
                        .arg(new_company_name.isEmpty() ? formatCompanyID(customer_id) : new_company_name));
    m_undo_stack->savepoint();

    QVariantMap set;
    set.insert("parent", customer_id);
    set.insert("id", circuit_id);
    circuit.update(set);
    Circuit::cascadeIDChange(selected_customer, m_tab->selectedCircuit().toInt(), circuit_id, customer_id);

    QVariantMap inspection;
    inspection.insert("customer", customer_id);
    inspection.insert("circuit", circuit_id);
    inspection.insert("date", inspection_date);
    inspection.insert("nominal", 0);
    inspection.insert("repair", 0);
    inspection.insert("outside_interval", 1);
    inspection.insert("inspection_type", Inspection::CircuitMovedType);
    QStringList data;
    data << m_tab->selectedCustomer();
    data << QString::number(customer_id);
    data << company_name.remove(UNIT_SEPARATOR);
    data << new_company_name.remove(UNIT_SEPARATOR);
    inspection.insert("inspection_type_data", data.join(UNIT_SEPARATOR));
    inspection.insert("rmds", QApplication::translate("Inspection", "Circuit moved from customer %1 to %2.")
                      .arg(QString("%1 (%2)").arg(company_name).arg(selected_customer, 8, 10, QChar('0')))
                      .arg(QString("%1 (%2)").arg(new_company_name).arg(customer_id, 8, 10, QChar('0')))
                      .append("\n\n")
                      .append(tr("DO NOT EDIT THIS INSPECTION: If you can read this message, you are using an older version of Leaklog than the one used to create this inspection. Changes made to this inspection will not be visible in newer versions of Leaklog.")));
    Inspection().update(inspection);

    setDatabaseModified(true);
    m_tab->setSelectedCustomer(customer_id);
    m_tab->loadCircuit(circuit_id, true);
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

    int customer_id = customer_index < 0 ? 0 : cb_customer->itemData(customer_index).toInt();

    if (customer_id && Circuit(QString::number(customer_id), m_tab->selectedCircuit()).exists()) {
        spb_circuit_id->setValue(Circuit(QString::number(customer_id), QString()).max("id") + 1);
    } else {
        spb_circuit_id->setValue(m_tab->selectedCircuit().toInt());
    }
}

void MainWindow::removeCircuit()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!m_tab->isCustomerSelected()) { return; }
    if (!m_tab->isCircuitSelected()) { return; }
    if (!canRemoveCircuit(m_tab->selectedCustomer(), m_tab->selectedCircuit())) { return; }
    Circuit record(m_tab->selectedCustomer(), m_tab->selectedCircuit());
    if (!isOperationPermitted("remove_circuit", record.stringValue("updated_by"))) { return; }
    if (RemoveDialogue::confirm(this, tr("Remove circuit - Leaklog"),
                                tr("Are you sure you want to remove the selected circuit?\nTo remove all data about the circuit \"%1\" type REMOVE and confirm:")
                                .arg(m_tab->selectedCircuit())) != QDialog::Accepted)
        return;

    QString company_name = Customer(m_tab->selectedCustomer()).stringValue("company");
    UndoCommand command(m_undo_stack, tr("Remove circuit %1 (%2)")
                        .arg(m_tab->selectedCircuit().rightJustified(5, '0'))
                        .arg(company_name.isEmpty() ? formatCompanyID(m_tab->selectedCustomer()) : company_name));
    m_undo_stack->savepoint();

    record.remove();
    MTDictionary parents(QStringList() << "customer_id" << "circuit_id",
                         QStringList() << m_tab->selectedCustomer() << m_tab->selectedCircuit());
    Compressor("", parents).remove();
    CircuitUnit("", MTDictionary(QStringList() << "company_id" << "circuit_id",
                                 QStringList() << m_tab->selectedCustomer() << m_tab->selectedCircuit())).remove();
    Inspection(m_tab->selectedCustomer(), m_tab->selectedCircuit(), "").remove();
    InspectionsCompressor("", parents).remove();
    MTRecord("inspection_images", "", "", MTDictionary(QStringList() << "customer" << "circuit",
                                                       QStringList() << m_tab->selectedCustomer()
                                                                     << m_tab->selectedCircuit())).remove();
    m_tab->clearSelectedCircuit();
    enableTools();
    setDatabaseModified(true);
    m_tab->setView(View::Circuits);
}

void MainWindow::addInspection()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!m_tab->isCustomerSelected()) { return; }
    if (!m_tab->isCircuitSelected()) { return; }
    if (!isOperationPermitted("add_inspection")) { return; }
    Inspection record(m_tab->selectedCustomer(), m_tab->selectedCircuit(), "");
    if (m_tab->isInspectorSelected()) {
        record.setValue("inspector", m_tab->selectedInspector());
    }
    UndoCommand command(m_undo_stack, tr("Add inspection"));
    EditInspectionDialogue md(&record, m_undo_stack, this);
    if (md.exec() == QDialog::Accepted) {
        setDatabaseModified(true);
        m_tab->loadInspection(record.id(), true);
    }
}

void MainWindow::editInspection()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!m_tab->isCustomerSelected()) { return; }
    if (!m_tab->isCircuitSelected()) { return; }
    if (!m_tab->isInspectionSelected()) { return; }
    Inspection record(m_tab->selectedCustomer(), m_tab->selectedCircuit(), m_tab->selectedInspection());
    if (!isOperationPermitted("edit_inspection", record.stringValue("updated_by"))) { return; }
    if (isRecordLocked(m_tab->selectedInspection())) { return; }
    QString company_name = Customer(m_tab->selectedCustomer()).stringValue("company");
    UndoCommand command(m_undo_stack, tr("Edit inspection %1 (%2, circuit %3)")
                        .arg(m_settings.formatDateTime(m_tab->selectedInspection()))
                        .arg(company_name.isEmpty() ? formatCompanyID(m_tab->selectedCustomer()) : company_name)
                        .arg(m_tab->selectedCircuit().rightJustified(5, '0')));
    EditInspectionDialogue md(&record, m_undo_stack, this);
    if (md.exec() == QDialog::Accepted) {
        setDatabaseModified(true);
        m_tab->loadInspection(record.id(), false);
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
    Inspection record(m_tab->selectedCustomer(), m_tab->selectedCircuit(), m_tab->selectedInspection());
    record.readValues();
    record.id().clear();
    QString company_name = Customer(m_tab->selectedCustomer()).stringValue("company");
    UndoCommand command(m_undo_stack, tr("Duplicate inspection %1 (%2, circuit %3)")
                        .arg(m_settings.formatDateTime(m_tab->selectedInspection()))
                        .arg(company_name.isEmpty() ? formatCompanyID(m_tab->selectedCustomer()) : company_name)
                        .arg(m_tab->selectedCircuit().rightJustified(5, '0')));
    EditInspectionDialogue md(&record, m_undo_stack, this, m_tab->selectedInspection());
    if (md.exec() == QDialog::Accepted) {
        setDatabaseModified(true);
        m_tab->loadInspection(record.id(), true);
    }
}

void MainWindow::removeInspection(const QString &date)
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!m_tab->isCustomerSelected()) { return; }
    if (!m_tab->isCircuitSelected()) { return; }
    QString inspection_date = date;
    if (inspection_date.isEmpty()) {
        if (!m_tab->isInspectionSelected()) { return; }

        inspection_date = m_tab->selectedInspection();
    }
    Inspection record(m_tab->selectedCustomer(), m_tab->selectedCircuit(), inspection_date);
    if (!isOperationPermitted("remove_inspection", record.stringValue("updated_by"))) { return; }
    if (isRecordLocked(inspection_date)) { return; }
    if (RemoveDialogue::confirm(this, tr("Remove inspection - Leaklog"),
                                tr("Are you sure you want to remove the selected inspection?\nTo remove all data about the inspection \"%1\" type REMOVE and confirm:")
                                .arg(m_settings.formatDateTime(inspection_date))) != QDialog::Accepted)
        return;

    QString company_name = Customer(m_tab->selectedCustomer()).stringValue("company");
    UndoCommand command(m_undo_stack, tr("Remove inspection %1 (%2, circuit %3)")
                        .arg(m_settings.formatDateTime(inspection_date))
                        .arg(company_name.isEmpty() ? formatCompanyID(m_tab->selectedCustomer()) : company_name)
                        .arg(m_tab->selectedCircuit().rightJustified(5, '0')));
    m_undo_stack->savepoint();

    record.remove();
    MTDictionary parents(QStringList() << "customer_id" << "circuit_id" << "date",
                         QStringList() << m_tab->selectedCustomer()
                                       << m_tab->selectedCircuit()
                                       << inspection_date);
    InspectionsCompressor("", parents).remove();
    InspectionImage(m_tab->selectedCustomer(), m_tab->selectedCircuit(), inspection_date).remove();
    if (m_tab->selectedInspection() == inspection_date)
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

    Circuit circuit_record(m_tab->selectedCustomer(), m_tab->selectedCircuit());
    circuit_record.addJoin("LEFT JOIN (SELECT customer, circuit, MAX(date) AS date FROM inspections"
                           " WHERE outside_interval = 0 GROUP BY customer, circuit) AS ins"
                           " ON ins.customer = circuits.parent AND ins.circuit = circuits.id");
    MTSqlQuery circuit_query = circuit_record.select("circuits.hermetic, circuits.leak_detector, circuits.inspection_interval, "
                                                     "COALESCE(ins.date, circuits.commissioning) AS last_regular_inspection, "
                                                     "circuits.refrigerant, " + circuitRefrigerantAmountQuery());
    circuit_query.setForwardOnly(true);
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

    QDialog d(this);
    d.setWindowTitle(tr("Skip inspection - Leaklog"));
    QGridLayout *gl = new QGridLayout(&d);

    QLabel *lbl = new QLabel(tr("Skip inspection of circuit:"), &d);
    lbl->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    gl->addWidget(lbl, 0, 0);

    gl->addWidget(new QLabel(m_tab->selectedCircuit().rightJustified(5, '0'), &d), 0, 1);

    lbl = new QLabel(tr("Date of skipped inspection:"), &d);
    lbl->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    gl->addWidget(lbl, 1, 0);

    QDateTimeEdit *date = new QDateTimeEdit(&d);
    date->setDisplayFormat(m_settings.dateTimeFormatString());
    date->setDateTime(next_regular_inspection_date);
    date->setCalendarPopup(true);
    date->calendarWidget()->setLocale(QLocale());
#if QT_VERSION < QT_VERSION_CHECK(4, 8, 0)
    date->calendarWidget()->setFirstDayOfWeek(Qt::Monday);
#else
    date->calendarWidget()->setFirstDayOfWeek(QLocale().firstDayOfWeek());
#endif
    gl->addWidget(date, 1, 1);

    QLabel *lbl_date_taken = new QLabel(QApplication::translate("EditDialogue", "This date is not available. Please choose a different date."), &d);
    QFont bold;
    bold.setBold(true);
    lbl_date_taken->setFont(bold);
    lbl_date_taken->setWordWrap(true);
    lbl_date_taken->setAlignment(Qt::AlignCenter);
    lbl_date_taken->setVisible(false);
    gl->addWidget(lbl_date_taken, 2, 0, 1, 2);

    lbl = new QLabel(tr("Reason:"), &d);
    lbl->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    gl->addWidget(lbl, 3, 0);

    QLineEdit *reason = new QLineEdit(&d);
    reason->setText(QApplication::translate("Inspection", "Inspection carried out by another service company."));
    reason->setMinimumWidth(360);
    gl->addWidget(reason, 3, 1);

    QDialogButtonBox *bb = new QDialogButtonBox(&d);
    bb->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    bb->button(QDialogButtonBox::Ok)->setText(tr("Skip"));
    bb->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));
    bb->button(QDialogButtonBox::Cancel)->setFocus();
    QObject::connect(bb, SIGNAL(accepted()), &d, SLOT(accept()));
    QObject::connect(bb, SIGNAL(rejected()), &d, SLOT(reject()));
    gl->addWidget(bb, 4, 0, 1, 2);

    QString inspection_date;
    do {
        lbl_date_taken->setVisible(!inspection_date.isEmpty());

        if (d.exec() != QDialog::Accepted)
            return;

        inspection_date = date->dateTime().toString(DATE_TIME_FORMAT);
    } while (Inspection(m_tab->selectedCustomer(), m_tab->selectedCircuit(), inspection_date).exists());

    QString company_name = Customer(m_tab->selectedCustomer()).stringValue("company");
    UndoCommand command(m_undo_stack, tr("Skip inspection of circuit %1 (%2)")
                        .arg(m_tab->selectedCircuit().rightJustified(5, '0'))
                        .arg(company_name.isEmpty() ? formatCompanyID(m_tab->selectedCustomer()) : company_name));
    m_undo_stack->savepoint();

    QVariantMap inspection;
    inspection.insert("customer", m_tab->selectedCustomer().toInt());
    inspection.insert("circuit", m_tab->selectedCircuit().toInt());
    inspection.insert("date", inspection_date);
    inspection.insert("nominal", 0);
    inspection.insert("repair", 0);
    inspection.insert("outside_interval", 0);
    inspection.insert("inspection_type", Inspection::InspectionSkippedType);
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
    Repair record("");
    if (m_tab->isCustomerSelected()) {
        record.setValue("parent", m_tab->selectedCustomer());
        record.setValue("customer", Customer(m_tab->selectedCustomer()).stringValue("company"));
    }
    if (m_tab->isInspectorSelected()) {
        record.setValue("repairman", m_tab->selectedInspector());
    }
    UndoCommand command(m_undo_stack, tr("Add repair"));
    EditDialogue md(&record, m_undo_stack, this);
    if (md.exec() == QDialog::Accepted) {
        setDatabaseModified(true);
        m_tab->loadRepair(record.id(), true);
    }
}

void MainWindow::editRepair()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!m_tab->isRepairSelected()) { return; }
    Repair record(m_tab->selectedRepair());
    record.readValues();
    if (!isOperationPermitted("edit_repair", record.stringValue("updated_by"))) { return; }
    if (isRecordLocked(m_tab->selectedRepair())) { return; }
    QString company_name = record.stringValue("customer");
    UndoCommand command(m_undo_stack, tr("Edit repair %1%2")
                        .arg(m_settings.formatDateTime(m_tab->selectedRepair()))
                        .arg(company_name.isEmpty() ? "" : QString(" (%1)").arg(company_name)));
    EditDialogue md(&record, m_undo_stack, this);
    if (md.exec() == QDialog::Accepted) {
        setDatabaseModified(true);
        m_tab->loadRepair(record.id(), true);
    }
}

void MainWindow::duplicateRepair()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!m_tab->isRepairSelected()) { return; }
    if (!isOperationPermitted("add_repair")) { return; }
    Repair record(m_tab->selectedRepair());
    record.readValues();
    record.id().clear();
    if (!record.stringValue("parent").isEmpty()) {
        record.parents().insert("parent", record.stringValue("parent"));
    }
    QString company_name = record.stringValue("customer");
    UndoCommand command(m_undo_stack, tr("Duplicate repair %1%2")
                        .arg(m_settings.formatDateTime(m_tab->selectedRepair()))
                        .arg(company_name.isEmpty() ? "" : QString(" (%1)").arg(company_name)));
    EditDialogue md(&record, m_undo_stack, this);
    if (md.exec() == QDialog::Accepted) {
        setDatabaseModified(true);
        m_tab->loadRepair(record.id(), true);
    }
}

void MainWindow::removeRepair()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!m_tab->isRepairSelected()) { return; }
    QString repair = m_tab->selectedRepair();
    Repair record(repair);
    record.readValues("updated_by, customer");
    if (!isOperationPermitted("remove_repair", record.stringValue("updated_by"))) { return; }
    if (isRecordLocked(repair)) { return; }
    if (RemoveDialogue::confirm(this, tr("Remove repair - Leaklog"),
                                tr("Are you sure you want to remove the selected repair?\nTo remove all data about the repair \"%1\" type REMOVE and confirm:")
                                .arg(m_settings.formatDateTime(repair))) != QDialog::Accepted)
        return;

    QString company_name = record.stringValue("customer");
    UndoCommand command(m_undo_stack, tr("Remove repair %1%2")
                        .arg(m_settings.formatDateTime(repair))
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
        QTreeWidgetItem *item = variable_items.value(variables.id(), NULL);
        if (!item) {
            if (variables.parentID().isEmpty())
                item = new QTreeWidgetItem(trw);
            else
                item = new QTreeWidgetItem;
            variable_items.insert(variables.id(), item);
        }

        if (!variables.parentID().isEmpty()) {
            QTreeWidgetItem *parent_item = variable_items.value(variables.parentID(), NULL);
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
    UndoCommand command(m_undo_stack, tr("Add variable"));
    EditDialogue md(&record, m_undo_stack, this);
    if (md.exec() == QDialog::Accepted) {
        QVariantMap attributes = record.list("name, unit, scope, tolerance");
        QTreeWidgetItem *item = NULL;
        if (subvar) {
            item = new QTreeWidgetItem(trw_variables->currentItem());
            VariableRecord(parent_id).update("type", "group");
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
    VariableRecord record(id);
    UndoCommand command(m_undo_stack, tr("Edit variable %1").arg(id));
    EditDialogue md(&record, m_undo_stack, this);
    if (md.exec() == QDialog::Accepted) {
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

            MTSqlQuery update_subvariables;
            update_subvariables.prepare("UPDATE variables SET parent_id = :new_id WHERE parent_id = :old_id");
            update_subvariables.bindValue(":old_id", id);
            update_subvariables.bindValue(":new_id", record.id());
            update_subvariables.exec();
        }
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
    if (RemoveDialogue::confirm(this, subvar ? tr("Remove subvariable - Leaklog") : tr("Remove variable - Leaklog"),
                                tr("Are you sure you want to remove the selected variable?\nTo remove the variable \"%1\" type REMOVE and confirm:")
                                .arg(id)) != QDialog::Accepted)
        return;

    UndoCommand command(m_undo_stack, tr("Remove variable %1").arg(id));
    m_undo_stack->savepoint();

    MTRecord("variables", "id", "", MTDictionary("parent_id", id)).remove();
    MTRecord("variables", "id", id, MTDictionary()).remove();
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
        emit tableAdded(-1, record.id());
        cb_table_edit->addItem(record.id());
        setDatabaseModified(true);
        refreshView();
    }
}

void MainWindow::editTable()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (cb_table_edit->currentIndex() < 0) { return; }
    if (!isOperationPermitted("edit_table")) { return; }
    Table record(cb_table_edit->currentText());
    UndoCommand command(m_undo_stack, tr("Edit table %1").arg(cb_table_edit->currentText()));
    EditDialogue md(&record, m_undo_stack, this);
    if (md.exec() == QDialog::Accepted) {
        int i = cb_table_edit->currentIndex();
        emit tableRemoved(cb_table_edit->currentText());
        cb_table_edit->removeItem(i);
        emit tableAdded(i, record.id());
        cb_table_edit->insertItem(i, record.id());
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

    emit tableRemoved(cb_table_edit->currentText());
    Table record(cb_table_edit->currentText());
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
    Table record(cb_table_edit->currentText());
    QVariantMap attributes = record.list("variables, sum, avg");
    QStringList variables = attributes.value("variables").toString().split(";", QString::SkipEmptyParts);
    QStringList sum = attributes.value("sum").toString().split(";", QString::SkipEmptyParts);
    QStringList avg = attributes.value("avg").toString().split(";", QString::SkipEmptyParts);
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
    if (!isOperationPermitted("edit_table")) { loadTable(cb_table_edit->currentText()); return; }
    UndoCommand command(m_undo_stack, tr("Edit table %1").arg(cb_table_edit->currentText()));
    m_undo_stack->savepoint();
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

        Table record(cb_table_edit->currentText());
        QStringList variables = record.stringValue("variables").split(";", QString::SkipEmptyParts);
        variables << lw->currentItem()->data(Qt::UserRole).toString();
        record.update("variables", variables.join(";"));
        loadTable(cb_table_edit->currentText());
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

    Table record(cb_table_edit->currentText());
    QStringList variables = record.stringValue("variables").split(";", QString::SkipEmptyParts);
    QString variable = variables.takeAt(i);
    if (up) {
        if (i != 0) { i--; } else { i = variables.count(); }
    } else {
        if (i != variables.count()) { i++; } else { i = 0; }
    }
    variables.insert(i, variable);
    record.update("variables", variables.join(";"));
    loadTable(cb_table_edit->currentText());
    trw_table_variables->setCurrentItem(trw_table_variables->topLevelItem(i));
    setDatabaseModified(true);
    refreshView();
}

void MainWindow::addWarning()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!isOperationPermitted("add_warning")) { return; }
    WarningRecord record("");
    UndoCommand command(m_undo_stack, tr("Add warning"));
    EditWarningDialogue md(&record, m_undo_stack, this);
    if (md.exec() == QDialog::Accepted) {
        QVariantMap attributes = record.list("name, description");
        QString name = attributes.value("name").toString();
        QString description = attributes.value("description").toString();
        QListWidgetItem *item = new QListWidgetItem;
        item->setText(description.isEmpty() ? name : tr("%1 (%2)").arg(name).arg(description));
        item->setData(Qt::UserRole, record.id());
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
        QVariantMap attributes = record.list("name, description");
        QString name = attributes.value("name").toString();
        QString description = attributes.value("description").toString();
        item->setText(description.isEmpty() ? name : tr("%1 (%2)").arg(name).arg(description));
        item->setData(Qt::UserRole, record.id());
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
    MTRecord filters("warnings_filters", "", "", MTDictionary("parent", item->data(Qt::UserRole).toString()));
    filters.remove();
    MTRecord conditions("warnings_conditions", "", "", MTDictionary("parent", item->data(Qt::UserRole).toString()));
    conditions.remove();
    delete item;
    enableTools();
    setDatabaseModified(true);
    refreshView();
}

void MainWindow::addInspector()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!isOperationPermitted("add_inspector")) { return; }
    Inspector record("");
    UndoCommand command(m_undo_stack, tr("Add inspector"));
    EditInspectorDialogue md(&record, m_undo_stack, this);
    if (md.exec() == QDialog::Accepted) {
        setDatabaseModified(true);
        m_tab->loadInspector(record.id().toInt(), true);
    }
}

void MainWindow::editInspector()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!m_tab->isInspectorSelected()) { return; }
    if (!isOperationPermitted("edit_inspector")) { return; }
    QString old_id = m_tab->selectedInspector();
    Inspector record(old_id);
    UndoCommand command(m_undo_stack, tr("Edit inspector %1").arg(record.stringValue("person")));
    EditInspectorDialogue md(&record, m_undo_stack, this);
    if (md.exec() == QDialog::Accepted) {
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
            m_tab->loadInspector(record.id().toInt(), false);
        } else {
            enableTools();
        }
        refreshView();
        setDatabaseModified(true);
    }
}

void MainWindow::removeInspector()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!m_tab->isInspectorSelected()) { return; }
    if (!isOperationPermitted("remove_inspector")) { return; }
    if (RemoveDialogue::confirm(this, tr("Remove inspector - Leaklog"),
                                tr("Are you sure you want to remove the selected inspector?\nTo remove all data about the inspector \"%1\" type REMOVE and confirm:")
                                .arg(m_tab->selectedInspector())) != QDialog::Accepted)
        return;

    Inspector record(m_tab->selectedInspector());

    UndoCommand command(m_undo_stack, tr("Remove inspector %1").arg(record.stringValue("person")));
    m_undo_stack->savepoint();

    record.remove();
    m_tab->clearSelectedInspector();
    enableTools();
    setDatabaseModified(true);
    m_tab->setView(View::Inspectors);
}

void MainWindow::exportCustomerData()
{
    exportData("customer");
}

void MainWindow::exportCircuitData()
{
    if (!m_tab->isCircuitSelected()) { return; }
    exportData("circuit");
}

void MainWindow::exportInspectionData()
{
    if (!m_tab->isCircuitSelected()) { return; }
    if (!m_tab->isInspectionSelected()) { return; }
    exportData("inspection");
}

void MainWindow::exportData(const QString &type)
{
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) { return; }
    if (!m_tab->isCustomerSelected()) { return; }

    QString title;
    if (type == "customer") { title = tr("Export customer data - Leaklog"); }
    else if (type == "circuit") { title = tr("Export circuit data - Leaklog"); }
    else if (type == "inspection") { title = tr("Export inspection data - Leaklog"); }
    else { title = tr("Export data - Leaklog"); }

    QString path = QFileDialog::getSaveFileName(this, title,
                                                QDir::home().absoluteFilePath(tr("untitled.lklg")),
                                                tr("Leaklog Database (*.lklg)"));
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

        copyTable("service_companies", db, data);
        copyTable("refrigerant_management", db, data);
        copyTable("inspectors", db, data);
        copyTable("variables", db, data);
        copyTable("tables", db, data);
        copyTable("assembly_record_types", db, data);
        copyTable("assembly_record_item_types", db, data);
        copyTable("assembly_record_type_categories", db, data);
        copyTable("assembly_record_item_categories", db, data);
        copyTable("circuit_unit_types", db, data);
        copyTable("customers", db, data, QString("id = %1").arg(m_tab->selectedCustomer()));

        if (type == "customer") {
            copyTable("repairs", db, data, QString("parent IS NOT NULL AND parent = %1").arg(m_tab->selectedCustomer()));
            copyTable("circuits", db, data, QString("parent = %1").arg(m_tab->selectedCustomer()));
            copyTable("compressors", db, data, QString("customer_id = %1").arg(m_tab->selectedCustomer()));
            copyTable("circuit_units", db, data, QString("company_id = %1").arg(m_tab->selectedCustomer()));
            copyTable("inspections", db, data, QString("customer = %1").arg(m_tab->selectedCustomer()));
            copyTable("inspections_compressors", db, data, QString("customer_id = %1").arg(m_tab->selectedCustomer()));
            copyTable("inspection_images", db, data, QString("customer = %1").arg(m_tab->selectedCustomer()));
            copyTable("files", db, data, QString("id IN (SELECT file_id FROM inspection_images WHERE customer = %1)"
                                                 " OR id IN (SELECT image FROM service_companies)")
                      .arg(m_tab->selectedCustomer()));
            copyTable("assembly_record_items", db, data, QString("arno IN (SELECT arno FROM inspections WHERE customer = %1)")
                      .arg(m_tab->selectedCustomer()));
        } else if (type == "circuit") {
            copyTable("circuits", db, data, QString("parent = %1 AND id = %2").arg(m_tab->selectedCustomer()).arg(m_tab->selectedCircuit()));
            copyTable("compressors", db, data, QString("customer_id = %1 AND circuit_id = %2").arg(m_tab->selectedCustomer()).arg(m_tab->selectedCircuit()));
            copyTable("circuit_units", db, data, QString("company_id = %1 AND circuit_id = %2").arg(m_tab->selectedCustomer()).arg(m_tab->selectedCircuit()));
            copyTable("inspections", db, data, QString("customer = %1 AND circuit = %2").arg(m_tab->selectedCustomer()).arg(m_tab->selectedCircuit()));
            copyTable("inspections_compressors", db, data, QString("customer_id = %1 AND circuit_id = %2").arg(m_tab->selectedCustomer()).arg(m_tab->selectedCircuit()));
            copyTable("inspection_images", db, data, QString("customer = %1 AND circuit = %2").arg(m_tab->selectedCustomer()).arg(m_tab->selectedCircuit()));
            copyTable("files", db, data, QString("id IN (SELECT file_id FROM inspection_images WHERE customer = %1 AND circuit = %2)"
                                                 " OR id IN (SELECT image FROM service_companies)")
                      .arg(m_tab->selectedCustomer()).arg(m_tab->selectedCircuit()));
            copyTable("assembly_record_items", db, data, QString("arno IN (SELECT arno FROM inspections WHERE customer = %1 AND circuit = %2)")
                      .arg(m_tab->selectedCustomer()).arg(m_tab->selectedCircuit()));
        } else if (type == "inspection") {
            copyTable("circuits", db, data, QString("parent = %1 AND id = %2").arg(m_tab->selectedCustomer()).arg(m_tab->selectedCircuit()));
            copyTable("compressors", db, data, QString("customer_id = %1 AND circuit_id = %2").arg(m_tab->selectedCustomer()).arg(m_tab->selectedCircuit()));
            copyTable("circuit_units", db, data, QString("company_id = %1 AND circuit_id = %2").arg(m_tab->selectedCustomer()).arg(m_tab->selectedCircuit()));
            copyTable("inspections", db, data, QString("customer = %1 AND circuit = %2 AND date = '%3'")
                      .arg(m_tab->selectedCustomer()).arg(m_tab->selectedCircuit()).arg(m_tab->selectedInspection()));
            copyTable("inspections_compressors", db, data, QString("customer_id = %1 AND circuit_id = %2 AND date = '%3'")
                      .arg(m_tab->selectedCustomer()).arg(m_tab->selectedCircuit()).arg(m_tab->selectedInspection()));
            copyTable("inspection_images", db, data, QString("customer = %1 AND circuit = %2 AND date = '%3'")
                      .arg(m_tab->selectedCustomer()).arg(m_tab->selectedCircuit()).arg(m_tab->selectedInspection()));
            copyTable("files", db, data, QString("id IN (SELECT file_id FROM inspection_images WHERE customer = %1 AND circuit = %2 AND date = '%3')"
                                                 " OR id IN (SELECT image FROM service_companies)")
                      .arg(m_tab->selectedCustomer()).arg(m_tab->selectedCircuit()).arg(m_tab->selectedInspection()));
            copyTable("assembly_record_items", db, data, QString("arno IN (SELECT arno FROM inspections WHERE customer = %1 AND circuit = %2 AND date = '%3')")
                      .arg(m_tab->selectedCustomer()).arg(m_tab->selectedCircuit()).arg(m_tab->selectedInspection()));
        }

        DBInfo::setValueForKey("default_service_company", DBInfo::valueForKey("default_service_company"), data);

        data.commit();
        data.close();
    } // END EXPORT (SCOPE)

    QSqlDatabase::removeDatabase("exportData");
}

void MainWindow::importData()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!isOperationPermitted("import_data")) { return; }
    QString path = QFileDialog::getOpenFileName(this, tr("Import data - Leaklog"), QDir::homePath(),
                                                tr("Leaklog Databases (*.lklg);;All files (*.*)"));
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
    initDatabase(data, false, false);
    ImportDialogue *id = new ImportDialogue(this);
    QVariantMap attributes;
    QVariant attribute;
    bool modified, attribute_modified;

    // Customers
    query.exec("SELECT * FROM customers ORDER BY company");
    while (query.next()) {
        QString company_id_justified = formatCompanyID(query.stringValue("id"));
        Customer customer(query.stringValue("id"));
        if (customer.exists()) {
            attributes = customer.list();
            modified = false;

            MTDictionary columns(true);
            columns.insert(company_id_justified, "0");

            for (int i = 1; i < Customer::attributes().count(); ++i) {
                attribute = query.value(Customer::attributes().key(i));
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
                        columns.insert(formatCompanyID(attribute), attribute_modified ? "1" : "0");
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
                item->setCheckState(0, query.stringValue("date_updated") > attributes.value("date_updated").toString() ? Qt::Checked : Qt::Unchecked);
                if (item->checkState(0) == Qt::Unchecked)
                    item->setToolTip(0, tr("This record is older than the current record in your database"));
                item->setData(0, Qt::UserRole, query.value("id"));
            }
        } else {
            QTreeWidgetItem *item = new QTreeWidgetItem(id->newCustomers());
            item->setText(0, company_id_justified);
            for (int i = 1; i < Customer::attributes().count(); ++i) {
                item->setText(i, MTVariant(query.value(Customer::attributes().key(i)),
                                           Customer::attributes().key(i)).toString());
            }
            item->setCheckState(0, Qt::Checked);
            item->setData(0, Qt::UserRole, query.value("id"));
        }
    }

    // Contact persons
    query.exec("SELECT persons.*, customers.company FROM persons LEFT JOIN customers ON persons.company_id = customers.id"
               " ORDER BY customers.company, persons.name");
    while (query.next()) {
        Person person(query.stringValue("id"));
        if (person.exists()) {
            attributes = person.list();
            modified = false;

            MTDictionary columns(true);
            columns.insert(query.stringValue("company"), query.value("company_id") != attributes.value("company_id") ? "1" : "0");

            for (int i = 2; i < Person::attributes().count(); ++i) {
                attribute = query.value(Person::attributes().key(i));
                attribute_modified = attribute != attributes.value(Person::attributes().key(i));
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
                item->setCheckState(0, query.stringValue("date_updated") > attributes.value("date_updated").toString() ? Qt::Checked : Qt::Unchecked);
                if (item->checkState(0) == Qt::Unchecked)
                    item->setToolTip(0, tr("This record is older than the current record in your database"));
                item->setData(0, Qt::UserRole, query.value("id"));
            }
        } else {
            QTreeWidgetItem *item = new QTreeWidgetItem(id->newPersons());
            item->setText(0, query.stringValue("company"));
            for (int i = 2; i < Person::attributes().count(); ++i) {
                item->setText(i - 1, query.stringValue(Person::attributes().key(i)));
            }
            item->setCheckState(0, Qt::Checked);
            item->setData(0, Qt::UserRole, query.value("id"));
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
                       " LEFT JOIN customers ON circuits.parent = customers.id"
                       " LEFT JOIN compressors ON compressors.customer_id = circuits.parent AND compressors.circuit_id = circuits.id"
                       " ORDER BY customers.company, circuits.id, compressors.id").arg(compressors_columns.join(", ")));
    QString last_id;
    QTreeWidgetItem *last_item = NULL;
    while (query.next()) {
        QString current_id = QString("%1:%2").arg(query.stringValue("parent")).arg(query.stringValue("id"));
        if (last_id != current_id) {
            if (last_item && last_item->treeWidget() == NULL)
                delete last_item;
            last_id = current_id;

            QString circuit_id_justified = query.stringValue("id").rightJustified(5, '0');
            Circuit circuit(query.stringValue("parent"), query.stringValue("id"));
            if (circuit.exists()) {
                attributes = circuit.list();
                modified = false;

                MTDictionary columns(true);
                columns.insert(query.stringValue("company"), "0");
                columns.insert(circuit_id_justified, "0");

                for (int i = 1; i < Circuit::attributes().count(); ++i) {
                    attribute = query.value(Circuit::attributes().key(i));
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
                last_item->setCheckState(0, query.stringValue("date_updated") > attributes.value("date_updated").toString() ? Qt::Checked : Qt::Unchecked);
                if (last_item->checkState(0) == Qt::Unchecked)
                    last_item->setToolTip(0, tr("This record is older than the current record in your database"));
                last_item->setData(0, Qt::UserRole, query.value("parent"));
                last_item->setData(1, Qt::UserRole, query.value("id"));
            } else {
                last_item = new QTreeWidgetItem(id->newCircuits());
                last_item->setText(0, query.stringValue("company"));
                last_item->setText(1, circuit_id_justified);
                for (int i = 1; i < Circuit::attributes().count(); ++i) {
                    last_item->setText(i + 1, query.stringValue(Circuit::attributes().key(i)));
                }
                last_item->setCheckState(0, Qt::Checked);
                last_item->setData(0, Qt::UserRole, query.value("parent"));
                last_item->setData(1, Qt::UserRole, query.value("id"));
            }
        }

        if (last_item && query.longLongValue("compressor_id")) {
            Compressor compressor(query.stringValue("compressor_id"));
            if (compressor.exists()) {
                attributes = compressor.list();
                modified = false;

                MTDictionary columns(true);
                QMapIterator<int, QString> i(compressor_attributes);
                while (i.hasNext()) { i.next();
                    attribute = query.value("compressor_" + i.value());
                    attribute_modified = attribute != attributes.value(i.value());
                    if (attribute_modified) {
                        modified = true;
                        shown_sections << i.key();
                    }
                    columns.insert(attribute.toString(), attribute_modified ? "1" : "0");
                }

                if (modified && query.stringValue("compressor_date_updated") > attributes.value("date_updated").toString()) {
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
                    item->setData(0, Qt::UserRole, query.value("compressor_id"));
                    if (last_item->treeWidget() == NULL)
                        id->modifiedCircuits()->addTopLevelItem(last_item);
                }
            } else {
                QTreeWidgetItem *item = new QTreeWidgetItem(last_item);
                item->setData(0, Qt::UserRole, query.value("compressor_id"));
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
    variable_names.insert("nominal", QApplication::translate("Inspection", "Nominal"));
    variable_names.insert("repair", QApplication::translate("Inspection", "Repair"));
    QStringList compressor_variable_names;
    Variables variables(data);
    QMap<QString, QTreeWidgetItem *> variable_items;
    while (variables.next()) {
        QTreeWidgetItem *item = variable_items.value(variables.id(), NULL);
        if (!item) {
            if (variables.parentID().isEmpty())
                item = new QTreeWidgetItem(id->variables());
            else
                item = new QTreeWidgetItem;
            variable_items.insert(variables.id(), item);
        }

        if (!variables.parentID().isEmpty()) {
            QTreeWidgetItem *parent_item = variable_items.value(variables.parentID(), NULL);
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

    QStringList inspections_compressors_columns = InspectionsCompressor::columns().toStringList([](const Column &column) {
        return QString("inspections_compressors.%1 AS compressor_%1").arg(column.name());
    });
    foreach (const QString &variable_name, compressor_variable_names)
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
                .arg(query.stringValue("customer"))
                .arg(query.stringValue("circuit"))
                .arg(query.stringValue("date"));
        if (last_id != current_id) {
            if (last_item && last_item->treeWidget() == NULL)
                delete last_item;
            last_id = current_id;

            QString circuit_id_justified = query.stringValue("circuit").rightJustified(5, '0');
            record_locked = DBInfo::isRecordLocked(query.stringValue("date"));
            Inspection inspection(query.stringValue("customer"), query.stringValue("circuit"), query.stringValue("date"));
            if (inspection.exists()) {
                attributes = inspection.list();
                modified = false;

                MTDictionary columns(true);
                columns.insert(query.stringValue("company"), "0");
                columns.insert(circuit_id_justified, "0");
                columns.insert(query.stringValue("date"), "0");

                for (int i = 0; i < variable_names.count(); ++i) {
                    attribute = query.value(variable_names.key(i));
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
                last_item->setCheckState(0, query.stringValue("date_updated") > attributes.value("date_updated").toString() ? Qt::Checked : Qt::Unchecked);
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
                last_item->setData(0, Qt::UserRole, query.value("customer"));
                last_item->setData(1, Qt::UserRole, query.value("circuit"));
                last_item->setData(2, Qt::UserRole, query.value("date"));
            }
        }

        if (last_item && query.intValue("compressor_id")) {
            MTDictionary inspections_compressor_parents(QStringList() << "customer_id" << "circuit_id" << "date" << "compressor_id",
                                                        QStringList() << query.stringValue("customer") << query.stringValue("circuit")
                                                        << query.stringValue("date") << query.stringValue("compressor_compressor_id"));
            InspectionsCompressor inspections_compressor(QString(), inspections_compressor_parents);
            if (inspections_compressor.exists()) {
                attributes = inspections_compressor.list();
                modified = false;

                MTDictionary columns(true);
                columns.insert(QString(), "0");
                columns.insert(query.stringValue("compressor_name"), "0");
                columns.insert(QString(), "0");

                for (int i = 0; i < variable_names.count(); ++i) {
                    if (!query.record().contains("compressor_" + variable_names.key(i))) {
                        columns.insert(QString(), "0");
                        continue;
                    }
                    attribute = query.value("compressor_" + variable_names.key(i));
                    attribute_modified = attribute != attributes.value(variable_names.key(i));
                    if (attribute_modified) {
                        modified = true;
                        shown_sections << i + 3;
                    }
                    columns.insert(attribute.toString(), attribute_modified ? "1" : "0");
                }

                if (modified && query.stringValue("compressor_date_updated") > attributes.value("date_updated").toString()) {
                    QTreeWidgetItem *item = new QTreeWidgetItem(last_item, columns.keys());
                    for (int i = 0; i < columns.count(); ++i) {
                        if (columns.value(i).toInt()) {
                            item->setBackground(i, QBrush(Qt::darkMagenta));
                            item->setForeground(i, QBrush(Qt::white));
                        }
                    }
                    item->setData(0, Qt::UserRole, query.value("compressor_compressor_id"));
                    if (last_item->treeWidget() == NULL)
                        id->modifiedInspections()->addTopLevelItem(last_item);
                }
            } else {
                QTreeWidgetItem *item = new QTreeWidgetItem(last_item);
                item->setData(0, Qt::UserRole, query.value("compressor_compressor_id"));
                item->setText(1, query.stringValue("compressor_name"));
                for (int i = 0; i < variable_names.count(); ++i) {
                    if (!query.record().contains("compressor_" + variable_names.key(i)))
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
        Repair repair(query.stringValue("date"));
        if (repair.exists()) {
            attributes = repair.list();
            modified = false;

            MTDictionary columns(true);
            columns.insert(query.stringValue("date"), "0");

            for (int i = 1; i < Repair::attributes().count(); ++i) {
                attribute = query.value(Repair::attributes().key(i));
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
                item->setCheckState(0, query.stringValue("date_updated") > attributes.value("date_updated").toString() ? Qt::Checked : Qt::Unchecked);
                if (item->checkState(0) == Qt::Unchecked)
                    item->setToolTip(0, tr("This record is older than the current record in your database"));
            }
        } else {
            item = new QTreeWidgetItem(id->newRepairs());
            item->setText(0, query.stringValue("date"));
            for (int i = 1; i < Repair::attributes().count(); ++i) {
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
                item->setData(0, Qt::UserRole, query.value("date"));
            }
        }
    }

    // Refrigerant management
    query.exec("SELECT * FROM refrigerant_management ORDER BY date");
    while (query.next()) {
        record_locked = DBInfo::isRecordLocked(query.stringValue("date"));
        QTreeWidgetItem *item = NULL;
        RefrigerantRecord record(query.stringValue("date"));
        if (record.exists()) {
            attributes = record.list();
            modified = false;

            MTDictionary columns(true);
            columns.insert(query.stringValue("date"), "0");

            for (int i = 1; i < RefrigerantRecord::attributes().count(); ++i) {
                attribute = query.value(RefrigerantRecord::attributes().key(i));
                attribute_modified = attribute != attributes.value(RefrigerantRecord::attributes().key(i));
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
                item->setCheckState(0, query.stringValue("date_updated") > attributes.value("date_updated").toString() ? Qt::Checked : Qt::Unchecked);
                if (item->checkState(0) == Qt::Unchecked)
                    item->setToolTip(0, tr("This record is older than the current record in your database"));
            }
        } else {
            item = new QTreeWidgetItem(id->newRefrigerantManagement());
            item->setText(0, query.stringValue("date"));
            for (int i = 1; i < RefrigerantRecord::attributes().count(); ++i) {
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
                item->setData(0, Qt::UserRole, query.value("date"));
            }
        }
    }

    // Inspectors
    query.exec("SELECT * FROM inspectors ORDER BY person");
    while (query.next()) {
        Inspector inspector(query.stringValue("id"));
        if (inspector.exists()) {
            attributes = inspector.list();
            modified = false;

            MTDictionary columns(true);
            columns.insert(query.stringValue("id").rightJustified(4, '0'), "0");

            for (int i = 1; i < Inspector::attributes().count(); ++i) {
                attribute = query.value(Inspector::attributes().key(i));
                attribute_modified = attribute != attributes.value(Inspector::attributes().key(i));
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
                item->setCheckState(0, query.stringValue("date_updated") > attributes.value("date_updated").toString() ? Qt::Checked : Qt::Unchecked);
                if (item->checkState(0) == Qt::Unchecked)
                    item->setToolTip(0, tr("This record is older than the current record in your database"));
                item->setData(0, Qt::UserRole, query.value("id"));
            }
        } else {
            QTreeWidgetItem *item = new QTreeWidgetItem(id->newInspectors());
            item->setText(0, query.stringValue("id").rightJustified(4, '0'));
            for (int i = 1; i < Inspector::attributes().count(); ++i) {
                item->setText(i, query.stringValue(Inspector::attributes().key(i)));
            }
            item->setCheckState(0, Qt::Checked);
            item->setData(0, Qt::UserRole, query.value("id"));
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
        fields = Person::columns().columnNameSet();
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
        fields = Circuit::columns().columnNameSet();
        QSet<QString> compressors_fields = Compressor::columns().columnNameSet();
        for (int w = 0; w < 2; ++w) {
            for (int cc = 0; cc < trw[w]->topLevelItemCount(); ++cc) {
                QTreeWidgetItem *item = trw[w]->topLevelItem(cc);
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
                VariableRecord record(item->text(1));
                Variable variable(item->text(1));
                if (!variable.next()) {
                    new_item = new QTreeWidgetItem(trw_variables);
                    new_item->setText(0, item->text(0));
                    new_item->setText(1, item->text(1));
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
                    VariableRecord record(subitem->text(1));
                    Variable subvariable(subitem->text(1));
                    if (new_item != NULL && !subvariable.next()) {
                        QTreeWidgetItem *new_subitem = new QTreeWidgetItem(new_item);
                        new_subitem->setText(0, subitem->text(0));
                        new_subitem->setText(1, subitem->text(1));
                        new_subitem->setText(2, subitem->text(2));
                        new_subitem->setText(3, subitem->text(7));
                    }
                    set.clear();
                    set.insert("name", subitem->text(0));
                    set.insert("parent_id", item->text(1));
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
        inspections_skip_columns << "customer" << "circuit" << "inspection_type";

        // Import inspections
        trw[0] = id->newInspections();
        trw[1] = id->modifiedInspections();
        QSet<QString> inspections_compressors_fields = InspectionsCompressor::columns().columnNameSet();
        inspections_compressors_fields.unite(QSet<QString>::fromList(compressor_variable_names));
        for (int w = 0; w < 2; ++w) {
            for (int i = 0, j = 0; i < trw[w]->topLevelItemCount(); ++i) {
                QTreeWidgetItem *item = trw[w]->topLevelItem(i);
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
                    set.insert("inspection_type", query.value(query.record().indexOf("inspection_type")).toInt()); // NOT NULL

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
                        InspectionsCompressor inspections_compressor(QString(), inspections_compressor_parents);
                        inspections_compressor.update(set, false, inspections_compressor.exists());
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
        fields = RefrigerantRecord::columns().columnNameSet();
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
                RefrigerantRecord(r_date).update(set);
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
    QString path = QFileDialog::getOpenFileName(this, tr("Import CSV - Leaklog"), QDir::homePath(),
                                                tr("CSV files (*.csv);;All files (*.*)"));
    if (path.isEmpty()) { return; }

    QString string_value;
    QStringList refrigerants(listRefrigerants());

    QList<ImportDialogueTable *> tables;
    ImportDialogueTable *table = new ImportDialogueTable(tr("Customers"), "customers");
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

    ImportDialogueTable *circuits_table = new ImportDialogueTable(tr("Circuits"), "circuits");
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
    ImportDialogueTableColumn *col = circuits_table->addColumn(tr("Status"), "disused", ImportDialogueTableColumn::Select);
    col->addSelectValue(QString::number(Circuit::Commissioned), QApplication::translate("Circuit", "Commissioned"));
    col->addSelectValue(QString::number(Circuit::ExcludedFromAgenda), QApplication::translate("Circuit", "Excluded from Agenda"));
    col->addSelectValue(QString::number(Circuit::Decommissioned), QApplication::translate("Circuit", "Decommissioned"));
    circuits_table->addColumn(tr("Hermetically sealed"), "hermetic", ImportDialogueTableColumn::Boolean);
    circuits_table->addColumn(tr("Fixed leakage detector installed"), "leak_detector", ImportDialogueTableColumn::Boolean);
    circuits_table->addColumn(tr("Year of purchase"), "year", ImportDialogueTableColumn::Integer);
    circuits_table->addColumn(tr("Date of commissioning"), "commissioning", ImportDialogueTableColumn::Date);
    col = circuits_table->addColumn(tr("Field of application"), "field", ImportDialogueTableColumn::Select);
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
    col->addSelectValue(tr("integer"), QString::number(Global::Integer));
    col->addSelectValue(tr("decimal"), QString::number(Global::Numeric));
    col->addSelectValue(tr("short text"), QString::number(Global::String));
    col->addSelectValue(tr("long text"), QString::number(Global::Text));
    col->addSelectValue(tr("boolean"), QString::number(Global::Boolean));
    table->addColumn(tr("Automatically add to assembly record"), "auto_show", ImportDialogueTableColumn::Boolean);
    table->addForeignKeyColumn(tr("Category ID"), "category_id", "id", "assembly_record_item_categories");
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
        m_tab->loadAssemblyRecordType(record.id().toInt(), true);
    }
}

void MainWindow::editAssemblyRecordType()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!m_tab->isAssemblyRecordTypeSelected()) { return; }
    QString old_id = m_tab->selectedAssemblyRecordType();
    AssemblyRecordType record(old_id);
    UndoCommand command(m_undo_stack, tr("Edit assembly record type %1").arg(record.stringValue("name")));
    EditAssemblyRecordDialogue md(&record, m_undo_stack, this);
    if (md.exec() == QDialog::Accepted) {
        if (old_id != record.id()) {
            MTSqlQuery update_ar_type;
            update_ar_type.prepare("UPDATE inspections SET ar_type = :new_id WHERE ar_type = :old_id");
            update_ar_type.bindValue(":old_id", old_id);
            update_ar_type.bindValue(":new_id", record.id());
            update_ar_type.exec();
        }
        setDatabaseModified(true);
        m_tab->loadAssemblyRecordType(record.id().toInt(), false);
        refreshView();
    }
}

void MainWindow::removeAssemblyRecordType()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!m_tab->isAssemblyRecordTypeSelected()) { return; }
    QString sel_record = m_tab->selectedAssemblyRecordType();
    if (RemoveDialogue::confirm(this, tr("Remove assembly record type - Leaklog"),
                                tr("Are you sure you want to remove the selected assembly record type?\nTo remove all data about the record \"%1\" type REMOVE and confirm:")
                                .arg(sel_record)) != QDialog::Accepted)
        return;

    AssemblyRecordType record(sel_record);

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
    if (!m_tab->isAssemblyRecordItemCategorySelected() || m_tab->selectedAssemblyRecordItemCategory().toInt() >= 1000) { return; }
    AssemblyRecordItemType record("");
    record.setValue("category_id", m_tab->selectedAssemblyRecordItemCategory().toInt());
    UndoCommand command(m_undo_stack, tr("Add assembly record item type"));
    EditDialogueWithAutoId md(&record, m_undo_stack, this);
    if (md.exec() == QDialog::Accepted) {
        setDatabaseModified(true);
        m_tab->loadAssemblyRecordItemType(record.id().toInt(), true);
    }
}

void MainWindow::editAssemblyRecordItemType()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!m_tab->isAssemblyRecordItemTypeSelected()) { return; }
    QString old_id = m_tab->selectedAssemblyRecordItemType();
    AssemblyRecordItemType record(old_id);
    UndoCommand command(m_undo_stack, tr("Edit assembly record item type %1").arg(record.stringValue("name")));
    EditDialogueWithAutoId md(&record, m_undo_stack, this);
    if (md.exec() == QDialog::Accepted) {
        if (old_id != record.id()) {
            MTSqlQuery update_ar_items;
            update_ar_items.prepare(QString("UPDATE assembly_record_items SET item_type_id = :new_id WHERE item_type_id = :old_id AND source = %1")
                                    .arg(AssemblyRecordItem::AssemblyRecordItemTypes));
            update_ar_items.bindValue(":old_id", old_id);
            update_ar_items.bindValue(":new_id", record.id());
            update_ar_items.exec();
        }
        setDatabaseModified(true);
        m_tab->loadAssemblyRecordItemType(record.id().toInt(), false);
        refreshView();
    }
}

void MainWindow::removeAssemblyRecordItemType()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!m_tab->isAssemblyRecordItemTypeSelected()) { return; }
    QString sel_record = m_tab->selectedAssemblyRecordItemType();
    if (RemoveDialogue::confirm(this, tr("Remove assembly record item type - Leaklog"),
                                tr("Are you sure you want to remove the selected assembly record item type?\nTo remove all data about the record item \"%1\" type REMOVE and confirm:")
                                .arg(sel_record)) != QDialog::Accepted)
        return;

    AssemblyRecordItemType record(sel_record);

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
    AssemblyRecordItemCategory record("");
    UndoCommand command(m_undo_stack, tr("Add assembly record item category"));
    EditDialogueWithAutoId md(&record, m_undo_stack, this, 1000);
    if (md.exec() == QDialog::Accepted) {
        setDatabaseModified(true);
        m_tab->loadAssemblyRecordItemCategory(record.id().toInt(), true);
    }
}

void MainWindow::editAssemblyRecordItemCategory()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!m_tab->isAssemblyRecordItemCategorySelected()) { return; }
    QString old_id = m_tab->selectedAssemblyRecordItemCategory();
    AssemblyRecordItemCategory record(old_id);
    UndoCommand command(m_undo_stack, tr("Edit assembly record item category %1").arg(record.stringValue("name")));
    EditDialogueWithAutoId md(&record, m_undo_stack, this, 1000);
    if (md.exec() == QDialog::Accepted) {
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
        setDatabaseModified(true);
        m_tab->loadAssemblyRecordItemCategory(record.id().toInt(), false);
        refreshView();
    }
}

void MainWindow::removeAssemblyRecordItemCategory()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!m_tab->isAssemblyRecordItemCategorySelected()) { return; }
    QString sel_category = m_tab->selectedAssemblyRecordItemCategory();
    if (RemoveDialogue::confirm(this, tr("Remove assembly record item category - Leaklog"),
                                tr("Are you sure you want to remove the selected assembly record item category?\nTo remove all data about the item category \"%1\" type REMOVE and confirm:")
                                .arg(sel_category)) != QDialog::Accepted)
        return;

    AssemblyRecordItemCategory category(sel_category);

    UndoCommand command(m_undo_stack, tr("Remove assembly record item category %1").arg(category.stringValue("name")));
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
    CircuitUnitType unit_type("");
    UndoCommand command(m_undo_stack, tr("Add circuit unit type"));
    EditDialogueWithAutoId md(&unit_type, m_undo_stack, this);
    if (md.exec() == QDialog::Accepted) {
        setDatabaseModified(true);
        m_tab->loadCircuitUnitType(unit_type.id().toInt(), true);
    }
}

void MainWindow::editCircuitUnitType()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!m_tab->isCircuitUnitTypeSelected()) { return; }
    QString old_id = m_tab->selectedCircuitUnitType();
    CircuitUnitType record(old_id);
    UndoCommand command(m_undo_stack, tr("Edit circuit unit type %1").arg(record.stringValue("type")));
    EditDialogueWithAutoId md(&record, m_undo_stack, this);
    if (md.exec() == QDialog::Accepted) {
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
        setDatabaseModified(true);
        m_tab->loadCircuitUnitType(record.id().toInt(), false);
        refreshView();
    }
}

void MainWindow::removeCircuitUnitType()
{
    if (!QSqlDatabase::database().isOpen()) { return; }
    if (!m_tab->isCircuitUnitTypeSelected()) { return; }
    QString sel_unit_type = m_tab->selectedCircuitUnitType();
    if (RemoveDialogue::confirm(this, tr("Remove circuit unit type - Leaklog"),
                                tr("Are you sure you want to remove the selected circuit unit type?\nTo remove all data about the unit type \"%1\" type REMOVE and confirm:")
                                .arg(sel_unit_type)) != QDialog::Accepted)
        return;

    CircuitUnitType unit_type(sel_unit_type);

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

    MTSqlQuery query("SELECT MAX(id) FROM styles");
    if (!query.last()) return;

    int id = query.value(0).toInt() + 1;

    Style record(QString::number(id));
    UndoCommand command(m_undo_stack, tr("Add style"));
    EditDialogue md(&record, m_undo_stack, this);
    if (md.exec() == QDialog::Accepted) {
        QVariantMap attributes = record.list("id, name");
        QListWidgetItem *item = new QListWidgetItem;
        item->setText(attributes.value("name").toString());
        item->setData(Qt::UserRole, attributes.value("id"));
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
    UndoCommand command(m_undo_stack, tr("Edit style %1").arg(record.stringValue("name")));
    EditDialogue md(&record, m_undo_stack, this);
    if (md.exec() == QDialog::Accepted) {
        QVariantMap attributes = record.list("id, name");
        item->setText(attributes.value("name").toString());
        item->setData(Qt::UserRole, attributes.value("id"));
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

    if ((actionMost_recent_first->isChecked() && column_name == "date") ||
        column_name == "date_updated")
        order = Qt::DescendingOrder;

    return QString("%1 %2").arg(column).arg(order == Qt::AscendingOrder ? "ASC" : "DESC");
}
