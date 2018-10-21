/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2018 Matus & Michal Tomlein

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
#include "records.h"
#include "warnings.h"

#include <QSqlError>
#include <QUuid>

#include <functional>

using namespace Global;

static const QUuid migration_namespace("6f607242-def6-4c4c-a9a2-429ce4ea73d3");

static inline QString createUUIDv5(const QUuid &ns, const QString &name)
{
    return QUuid::createUuidV5(ns, name).toString().mid(1, 36);
}

static QString renameV1TableQuery(const QString &table_name)
{
    return QString("ALTER TABLE %1 RENAME TO v1_%1").arg(table_name);
}

static QString createTableQuery(const QString &table_name, const ColumnList &columns, QSqlDatabase &database)
{
    return QString("CREATE TABLE %1 (%2)").arg(table_name).arg(sqlStringForDatabaseType(columns.toString(), database));
}

static QString insertQuery(const QString &table_name, const QStringList &columns)
{
    return QString("INSERT INTO %1 (%2) VALUES (%3)").arg(table_name).arg(columns.join(", "))
           .arg(map(columns, [](const QString &column) { return ":" + column; }).join(", "));
}

static inline QString inspectorUUID(int id)
{
    return id ? createUUIDv5(migration_namespace, QString("inspector:%1").arg(id)) : QString();
}

static inline QString customerUUID(int id)
{
    return id ? createUUIDv5(migration_namespace, QString("customer:%1").arg(id)) : QString();
}

static inline QString personUUID(qlonglong id)
{
    return id ? createUUIDv5(migration_namespace, QString("person:%1").arg(id)) : QString();
}

static inline QString circuitUUID(int customer, int circuit)
{
    return customer && circuit ? createUUIDv5(migration_namespace, QString("customer:%1/circuit:%2").arg(customer).arg(circuit)) : QString();
}

static inline QString compressorUUID(qlonglong id)
{
    return id ? createUUIDv5(migration_namespace, QString("compressor:%1").arg(id)) : QString();
}

static inline QString inspectionUUID(int customer, int circuit, const QString &date)
{
    return customer && circuit && !date.isEmpty() ? createUUIDv5(migration_namespace, QString("customer:%1/circuit:%2/inspection:%3").arg(customer).arg(circuit).arg(date)) : QString();
}

static inline QString inspectionCompressorUUID(int customer, int circuit, const QString &date, qlonglong compressor)
{
    return customer && circuit && !date.isEmpty() && compressor ? createUUIDv5(migration_namespace, QString("customer:%1/circuit:%2/inspection:%3/compressor:%4").arg(customer).arg(circuit).arg(date).arg(compressor)) : QString();
}

static inline QString inspectionFileUUID(int customer, int circuit, const QString &date, const QString &file_uuid)
{
    return customer && circuit && !date.isEmpty() && !file_uuid.isEmpty() ? createUUIDv5(migration_namespace, QString("customer:%1/circuit:%2/inspection:%3/file:%4").arg(customer).arg(circuit).arg(date).arg(file_uuid)) : QString();
}

static inline QString repairUUID(const QString &date)
{
    return !date.isEmpty() ? createUUIDv5(migration_namespace, QString("repair:%1").arg(date)) : QString();
}

static inline QString serviceCompanyUUID(int id)
{
    return id ? createUUIDv5(migration_namespace, QString("servicecompany:%1").arg(id)) : QString();
}

static inline QString refrigerantRecordUUID(const QString &date)
{
    return !date.isEmpty() ? createUUIDv5(migration_namespace, QString("refrigerantrecord:%1").arg(date)) : QString();
}

static inline QString tableUUID(int uid)
{
    return uid ? Table::predefinedUUID(uid) : createUUID();
}

static inline QString warningUUID(int id)
{
    return id >= 1000 ? Warnings::predefinedUUID(id) : createUUID();
}

static inline QString assemblyRecordItemCategoryUUID(int id)
{
    return id >= 1000 ? AssemblyRecordItemCategory::predefinedUUID(id) : createUUID();
}

static void migrateV1Variables(QSqlDatabase &database)
{
    QUuid database_uuid = QUuid(DBInfo::databaseUUID(database));

    QString update_variable = "UPDATE variables SET uuid = :uuid, parent_uuid = :parent_uuid WHERE id = :id";
    int variables_table_id = JournalEntry::tableIDForName(VariableRecord::tableName());

    MTSqlQuery variables("SELECT parent_id, id FROM variables", database);
    while (variables.next()) {
        QString id = variables.stringValue("id");
        QString parent_id = variables.stringValue("parent_id");

        QString uuid = createUUIDv5(database_uuid, id);
        QString parent_uuid;
        if (!parent_id.isEmpty())
            parent_uuid = createUUIDv5(database_uuid, parent_id);

        MTSqlQuery variable(database);
        variable.prepare(update_variable);
        variable.bindValue(":uuid", uuid);
        variable.bindValue(":parent_uuid", parent_uuid);
        variable.bindValue(":id", id);
        variable.exec();

        journalInsertion(variables_table_id, uuid, database);
    }

    MTSqlQuery query(database);

    if (isDatabaseRemote(database)) {
        query.exec("ALTER TABLE variables DROP COLUMN parent_id");
        query.exec("ALTER TABLE variables ADD PRIMARY KEY (uuid)");
    }
}

static void migrateV1Inspectors(QSqlDatabase &database)
{
    MTSqlQuery query(database);
    query.exec(renameV1TableQuery("inspectors"));
    query.exec(createTableQuery(Inspector::tableName(), Inspector::columns(), database));

    QStringList inspectors_columns = QStringList() << "uuid" << "certificate_number" << "certificate_country" << "person" << "mail" << "phone" << "list_price" << "acquisition_price" << "date_updated" << "updated_by";
    QString insert_inspector = insertQuery(Inspector::tableName(), inspectors_columns);
    int inspectors_table_id = JournalEntry::tableIDForName(Inspector::tableName());

    MTSqlQuery inspectors("SELECT * FROM v1_inspectors", database);
    while (inspectors.next()) {
        int id = inspectors.intValue("id");
        QString uuid = inspectorUUID(id);

        MTSqlQuery inspector(database);
        inspector.prepare(insert_inspector);

        int pos = 0;
        foreach (const QString &column, inspectors_columns) {
            if (column == "uuid") {
                inspector.bindValue(pos, uuid);
            } else if (column == "certificate_number") {
                QString value = inspectors.value(column).toString();
                inspector.bindValue(pos, value.isEmpty() ? QString::number(id).rightJustified(4, '0') : value);
            } else {
                inspector.bindValue(pos, inspectors.value(column));
            }
            pos++;
        }
        inspector.exec();

        journalInsertion(inspectors_table_id, uuid, database);
    }
}

static void migrateV1Customers(QSqlDatabase &database)
{
    MTSqlQuery query(database);
    query.exec(renameV1TableQuery("customers"));
    query.exec(createTableQuery(Customer::tableName(), Customer::columns(), database));

    QStringList customers_columns = QStringList() << "uuid" << "id" << "company" << "address" << "mail" << "phone" << "operator_type" << "operator_id" << "operator_company" << "operator_address" << "operator_mail" << "operator_phone" << "notes" << "date_updated" << "updated_by";
    QString insert_customer = insertQuery(Customer::tableName(), customers_columns);
    int customers_table_id = JournalEntry::tableIDForName(Customer::tableName());

    MTSqlQuery customers("SELECT * FROM v1_customers", database);
    while (customers.next()) {
        QString uuid = customerUUID(customers.intValue("id"));
        int operator_id = customers.intValue("operator_id");

        MTSqlQuery customer(database);
        customer.prepare(insert_customer);

        int pos = 0;
        foreach (const QString &column, customers_columns) {
            if (column == "uuid") {
                customer.bindValue(pos, uuid);
            } else if (column == "id") {
                QString id = customers.stringValue(column);
                customer.bindValue(pos, formatCompanyID(id));
            } else if (column == "operator_type") {
                switch (operator_id) {
                    case -1:
                    case 0:
                        customer.bindValue(pos, operator_id);
                        break;

                    default:
                        customer.bindValue(pos, 1);
                        break;
                }
            } else if (column == "operator_id") {
                switch (operator_id) {
                    case -1:
                    case 0:
                        customer.bindValue(pos, QString());
                        break;

                    default:
                        customer.bindValue(pos, formatCompanyID(customers.stringValue(column)));
                        break;
                }
            } else {
                customer.bindValue(pos, customers.value(column));
            }
            pos++;
        }
        customer.exec();

        journalInsertion(customers_table_id, uuid, database);
    }
}

static void migrateV1Persons(QSqlDatabase &database)
{
    MTSqlQuery query(database);
    query.exec(renameV1TableQuery("persons"));
    query.exec(createTableQuery(Person::tableName(), Person::columns(), database));

    QStringList persons_columns = QStringList() << "uuid" << "customer_uuid" << "name" << "mail" << "phone" << "hidden" << "date_updated" << "updated_by";
    QString insert_person = insertQuery(Person::tableName(), persons_columns);
    int persons_table_id = JournalEntry::tableIDForName(Person::tableName());

    MTSqlQuery persons("SELECT * FROM v1_persons", database);
    while (persons.next()) {
        QString uuid = personUUID(persons.longLongValue("id"));

        MTSqlQuery person(database);
        person.prepare(insert_person);

        int pos = 0;
        foreach (const QString &column, persons_columns) {
            if (column == "uuid") {
                person.bindValue(pos, uuid);
            } else if (column == "customer_uuid") {
                person.bindValue(pos, customerUUID(persons.intValue("company_id")));
            } else {
                person.bindValue(pos, persons.value(column));
            }
            pos++;
        }
        person.exec();

        journalInsertion(persons_table_id, uuid, database);
    }
}

static void migrateV1Circuits(QSqlDatabase &database, std::function<void(int, int)> progress)
{
    MTSqlQuery query(database);
    query.exec(renameV1TableQuery("circuits"));
    query.exec(createTableQuery(Circuit::tableName(), Circuit::columns(), database));

    QStringList columns = QStringList() << "uuid" << "customer_uuid" << "id" << "name" << "disused" << "operation" << "building" << "device" << "hermetic" << "manufacturer" << "type" << "sn" << "year" << "commissioning" << "decommissioning" << "field" << "refrigerant" << "refrigerant_amount" << "oil" << "oil_amount" << "leak_detector" << "runtime" << "utilisation" << "inspection_interval" << "notes" << "date_updated" << "updated_by";
    QString insert_query = insertQuery(Circuit::tableName(), columns);
    int circuits_table_id = JournalEntry::tableIDForName(Circuit::tableName());

    int current = 0;
    int total = MTSqlQuery("SELECT COUNT(*) FROM v1_circuits", database).nextValue().toInt();

    MTSqlQuery circuits("SELECT * FROM v1_circuits", database);
    while (circuits.next()) {
        int parent = circuits.intValue("parent");
        int id = circuits.intValue("id");
        QString uuid = circuitUUID(parent, id);

        MTSqlQuery circuit(database);
        circuit.prepare(insert_query);

        int pos = 0;
        foreach (const QString &column, columns) {
            if (column == "uuid") {
                circuit.bindValue(pos, uuid);
            } else if (column == "customer_uuid") {
                circuit.bindValue(pos, customerUUID(parent));
            } else {
                circuit.bindValue(pos, circuits.value(column));
            }
            pos++;
        }
        circuit.exec();

        journalInsertion(circuits_table_id, uuid, database);

        if (total > 0) {
            progress(current, total);
            current++;
        }
    }
}

static void migrateV1Compressors(QSqlDatabase &database, std::function<void(int, int)> progress)
{
    MTSqlQuery query(database);
    query.exec(renameV1TableQuery("compressors"));
    query.exec(createTableQuery(Compressor::tableName(), Compressor::columns(), database));

    QStringList columns = QStringList() << "uuid" << "circuit_uuid" << "name" << "manufacturer" << "type" << "sn" << "date_updated" << "updated_by";
    QString insert_compressor = insertQuery(Compressor::tableName(), columns);
    int compressors_table_id = JournalEntry::tableIDForName(Compressor::tableName());

    int current = 0;
    int total = MTSqlQuery("SELECT COUNT(*) FROM v1_compressors", database).nextValue().toInt();

    MTSqlQuery compressors("SELECT * FROM v1_compressors", database);
    while (compressors.next()) {
        QString uuid = compressorUUID(compressors.longLongValue("id"));

        MTSqlQuery compressor(database);
        compressor.prepare(insert_compressor);

        int pos = 0;
        foreach (const QString &column, columns) {
            if (column == "uuid") {
                compressor.bindValue(pos, uuid);
            } else if (column == "circuit_uuid") {
                compressor.bindValue(pos, circuitUUID(compressors.intValue("customer_id"), compressors.intValue("circuit_id")));
            } else {
                compressor.bindValue(pos, compressors.value(column));
            }
            pos++;
        }
        compressor.exec();

        journalInsertion(compressors_table_id, uuid, database);

        if (total > 0) {
            progress(current, total);
            current++;
        }
    }
}

static QString inspectionTypeDataFromV1Data(Inspection::Type type, const QString &type_data)
{
    switch (type) {
        case Inspection::CircuitMoved: {
            QStringList data = type_data.split(UNIT_SEPARATOR);
            if (data.count() > 2) {
                data[0] = customerUUID(data[0].toInt());
                data[1] = customerUUID(data[1].toInt());
            }
            return data.join(UNIT_SEPARATOR);
        }

        default:
            break;
    }

    return type_data;
}

static void migrateV1Inspections(const QMap<int, QString> &assembly_record_type_uuids, QSqlDatabase &database, std::function<void(int, int)> progress)
{
    QString update_inspection = "UPDATE inspections SET uuid = :uuid, customer_uuid = :customer_uuid, circuit_uuid = :circuit_uuid, inspector_uuid = :inspector_uuid, person_uuid = :person_uuid, ar_type_uuid = :ar_type_uuid, inspection_type = :inspection_type, inspection_type_data = :inspection_type_data WHERE customer = :customer AND circuit = :circuit AND date = :date";
    int inspections_table_id = JournalEntry::tableIDForName(Inspection::tableName());

    int current = 0;
    int total = MTSqlQuery("SELECT COUNT(*) FROM inspections", database).nextValue().toInt();

    MTSqlQuery inspections("SELECT customer, circuit, date, nominal, repair, inspector, operator, ar_type, inspection_type, inspection_type_data FROM inspections", database);
    while (inspections.next()) {
        int customer = inspections.intValue("customer");
        int circuit = inspections.intValue("circuit");
        QString date = inspections.stringValue("date");
        QString uuid = inspectionUUID(customer, circuit, date);
        int inspector = inspections.intValue("inspector");
        qlonglong operator_id = inspections.longLongValue("operator");
        int ar_type = inspections.intValue("ar_type");
        Inspection::Type inspection_type = (Inspection::Type)(inspections.intValue("inspection_type") * -1);
        QString inspection_type_data = inspections.stringValue("inspection_type_data");

        if (inspections.intValue("nominal")) {
            inspection_type = Inspection::NominalInspection;
        } else switch (inspections.intValue("repair")) {
            case 1:
                inspection_type = Inspection::Repair;
                break;
            case 2:
                inspection_type = Inspection::InspectionAfterRepair;
                break;
        }

        MTSqlQuery inspection(database);
        inspection.prepare(update_inspection);
        inspection.bindValue(":uuid", uuid);
        inspection.bindValue(":customer_uuid", customerUUID(customer));
        inspection.bindValue(":circuit_uuid", circuitUUID(customer, circuit));
        inspection.bindValue(":inspector_uuid", inspectorUUID(inspector));
        inspection.bindValue(":person_uuid", personUUID(operator_id));
        inspection.bindValue(":ar_type_uuid", assembly_record_type_uuids.value(ar_type));
        inspection.bindValue(":inspection_type", (int)inspection_type);
        inspection.bindValue(":inspection_type_data", inspectionTypeDataFromV1Data(inspection_type, inspection_type_data));
        inspection.bindValue(":customer", customer);
        inspection.bindValue(":circuit", circuit);
        inspection.bindValue(":date", date);
        inspection.exec();

        journalInsertion(inspections_table_id, uuid, database);

        if (total > 0) {
            progress(current, total);
            current++;
        }
    }

    MTSqlQuery query(database);

    if (isDatabaseRemote(database)) {
        query.exec("ALTER TABLE inspections DROP COLUMN customer");
        query.exec("ALTER TABLE inspections DROP COLUMN circuit");
        query.exec("ALTER TABLE inspections DROP COLUMN inspector");
        query.exec("ALTER TABLE inspections DROP COLUMN operator");
        query.exec("ALTER TABLE inspections DROP COLUMN ar_type");
        query.exec("ALTER TABLE inspections ADD PRIMARY KEY (uuid)");
    } else {
        query.exec("CREATE INDEX IF NOT EXISTS index_inspections_uuid ON inspections (uuid ASC)");
    }
}

static void migrateV1InspectionCompressors(QSqlDatabase &database, std::function<void(int, int)> progress)
{
    QString update_inspection_compressor = "UPDATE inspections_compressors SET uuid = :uuid, inspection_uuid = :inspection_uuid, compressor_uuid = :compressor_uuid WHERE id = :id";
    int inspections_compressors_table_id = JournalEntry::tableIDForName(InspectionCompressor::tableName());

    int current = 0;
    int total = MTSqlQuery("SELECT COUNT(*) FROM inspections_compressors", database).nextValue().toInt();

    MTSqlQuery inspection_compressors("SELECT id, customer_id, circuit_id, date, compressor_id FROM inspections_compressors", database);
    while (inspection_compressors.next()) {
        int id = inspection_compressors.intValue("id");
        int customer_id = inspection_compressors.intValue("customer_id");
        int circuit_id = inspection_compressors.intValue("circuit_id");
        QString date = inspection_compressors.stringValue("date");
        qlonglong compressor_id = inspection_compressors.longLongValue("compressor_id");
        QString uuid = inspectionCompressorUUID(customer_id, circuit_id, date, compressor_id);

        MTSqlQuery inspection_compressor(database);
        inspection_compressor.prepare(update_inspection_compressor);
        inspection_compressor.bindValue(":uuid", uuid);
        inspection_compressor.bindValue(":inspection_uuid", inspectionUUID(customer_id, circuit_id, date));
        inspection_compressor.bindValue(":compressor_uuid", compressorUUID(compressor_id));
        inspection_compressor.bindValue(":id", id);
        inspection_compressor.exec();

        journalInsertion(inspections_compressors_table_id, uuid, database);

        if (total > 0) {
            progress(current, total);
            current++;
        }
    }

    MTSqlQuery query(database);

    if (isDatabaseRemote(database)) {
        query.exec("ALTER TABLE inspections_compressors DROP COLUMN id");
        query.exec("ALTER TABLE inspections_compressors DROP COLUMN customer_id");
        query.exec("ALTER TABLE inspections_compressors DROP COLUMN circuit_id");
        query.exec("ALTER TABLE inspections_compressors DROP COLUMN date");
        query.exec("ALTER TABLE inspections_compressors DROP COLUMN compressor_id");
        query.exec("ALTER TABLE inspections_compressors ADD PRIMARY KEY (uuid)");
    } else {
        query.exec("CREATE INDEX IF NOT EXISTS index_inspections_compressors_uuid ON inspections_compressors (uuid ASC)");
    }
}

static QMap<int, QString> migrateV1Files(QSqlDatabase &database)
{
    QMap<int, QString> file_uuids;

    QString update_file = "UPDATE files SET uuid = :uuid WHERE id = :id";
    int files_table_id = JournalEntry::tableIDForName(File::tableName());

    MTSqlQuery files("SELECT id, name FROM files", database);
    while (files.next()) {
        int id = files.intValue("id");
        QString uuid = createUUIDv5(migration_namespace, QString("file:%1[%2]").arg(id).arg(files.stringValue("name")));
        file_uuids.insert(id, uuid);

        MTSqlQuery file(database);
        file.prepare(update_file);
        file.bindValue(":uuid", uuid);
        file.bindValue(":id", id);
        file.exec();

        journalInsertion(files_table_id, uuid, database);
    }

    MTSqlQuery query(database);

    if (isDatabaseRemote(database)) {
        query.exec("ALTER TABLE files DROP COLUMN id");
        query.exec("ALTER TABLE files ADD PRIMARY KEY (uuid)");
        query.exec("UPDATE files SET data = decode(convert_from(data, 'UTF8'), 'base64')");
    } else {
        query.exec("CREATE INDEX IF NOT EXISTS index_files_uuid ON files (uuid ASC)");
    }

    return file_uuids;
}

static void migrateV1InspectionFiles(const QMap<int, QString> &file_uuids, QSqlDatabase &database)
{
    MTSqlQuery query(database);
    query.exec(renameV1TableQuery("inspection_images"));
    query.exec(createTableQuery(InspectionFile::tableName(), InspectionFile::columns(), database));

    QStringList columns = QStringList() << "uuid" << "inspection_uuid" << "file_uuid" << "description" << "date_updated" << "updated_by";
    QString insert_query = insertQuery(InspectionFile::tableName(), columns);
    int inspections_files_table_id = JournalEntry::tableIDForName(InspectionFile::tableName());

    MTSqlQuery inspection_images("SELECT * FROM v1_inspection_images", database);
    while (inspection_images.next()) {
        int customer = inspection_images.intValue("customer");
        int circuit = inspection_images.intValue("circuit");
        QString date = inspection_images.stringValue("date");
        int file_id = inspection_images.intValue("file_id");
        QString file_uuid = file_uuids.value(file_id);
        QString uuid = inspectionFileUUID(customer, circuit, date, file_uuid);

        MTSqlQuery inspection_file(database);
        inspection_file.prepare(insert_query);

        int pos = 0;
        foreach (const QString &column, columns) {
            if (column == "uuid") {
                inspection_file.bindValue(pos, uuid);
            } else if (column == "inspection_uuid") {
                inspection_file.bindValue(pos, inspectionUUID(customer, circuit, date));
            } else if (column == "file_uuid") {
                inspection_file.bindValue(pos, file_uuid);
            } else {
                inspection_file.bindValue(pos, inspection_images.value(column));
            }
            pos++;
        }
        inspection_file.exec();

        journalInsertion(inspections_files_table_id, uuid, database);
    }
}

static void migrateV1Repairs(QSqlDatabase &database)
{
    MTSqlQuery query(database);
    query.exec(renameV1TableQuery("repairs"));
    query.exec(createTableQuery(Repair::tableName(), Repair::columns(), database));

    QStringList columns = QStringList() << "uuid" << "customer_uuid" << "inspector_uuid" << "date" << "customer" << "device" << "field" << "refrigerant" << "refrigerant_amount" << "refr_add_am" << "refr_reco" << "arno" << "date_updated" << "updated_by";
    QString insert_query = insertQuery(Repair::tableName(), columns);
    int repairs_table_id = JournalEntry::tableIDForName(Repair::tableName());

    MTSqlQuery repairs("SELECT * FROM v1_repairs", database);
    while (repairs.next()) {
        QString uuid = repairUUID(repairs.stringValue("date"));

        MTSqlQuery repair(database);
        repair.prepare(insert_query);

        int pos = 0;
        foreach (const QString &column, columns) {
            if (column == "uuid") {
                repair.bindValue(pos, uuid);
            } else if (column == "customer_uuid") {
                repair.bindValue(pos, customerUUID(repairs.intValue("parent")));
            } else if (column == "inspector_uuid") {
                repair.bindValue(pos, inspectorUUID(repairs.intValue("repairman")));
            } else {
                repair.bindValue(pos, repairs.value(column));
            }
            pos++;
        }
        repair.exec();

        journalInsertion(repairs_table_id, uuid, database);
    }
}

static void migrateV1ServiceCompanies(const QMap<int, QString> &file_uuids, QSqlDatabase &database)
{
    MTSqlQuery query(database);
    query.exec(renameV1TableQuery("service_companies"));
    query.exec(createTableQuery(ServiceCompany::tableName(), ServiceCompany::columns(), database));

    QStringList columns = QStringList() << "uuid" << "image_file_uuid" << "id" << "name" << "address" << "mail" << "phone" << "website" << "date_updated" << "updated_by";
    QString insert_query = insertQuery(ServiceCompany::tableName(), columns);
    int service_companies_table_id = JournalEntry::tableIDForName(ServiceCompany::tableName());

    MTSqlQuery service_companies("SELECT * FROM v1_service_companies", database);
    while (service_companies.next()) {
        QString uuid = serviceCompanyUUID(service_companies.intValue("id"));

        MTSqlQuery service_company(database);
        service_company.prepare(insert_query);

        int pos = 0;
        foreach (const QString &column, columns) {
            if (column == "uuid") {
                service_company.bindValue(pos, uuid);
            } else if (column == "id") {
                service_company.bindValue(pos, formatCompanyID(service_companies.stringValue("id")));
            } else if (column == "image_file_uuid") {
                service_company.bindValue(pos, file_uuids.value(service_companies.intValue("image")));
            } else {
                service_company.bindValue(pos, service_companies.value(column));
            }
            pos++;
        }
        service_company.exec();

        journalInsertion(service_companies_table_id, uuid, database);
    }
}

static void migrateV1Tables(QSqlDatabase &database)
{
    MTSqlQuery query(database);
    query.exec(renameV1TableQuery("tables"));
    query.exec(createTableQuery(Table::tableName(), Table::columns(), database));

    QStringList columns = QStringList() << "uuid" << "name" << "position" << "highlight_nominal" << "scope" << "variables" << "sum" << "avg" << "date_updated" << "updated_by";
    QString insert_query = insertQuery(Table::tableName(), columns);
    int tables_table_id = JournalEntry::tableIDForName(Table::tableName());

    MTSqlQuery tables("SELECT * FROM v1_tables", database);
    while (tables.next()) {
        int uid = tables.intValue("uid");
        QString uuid = tableUUID(uid);

        MTSqlQuery table(database);
        table.prepare(insert_query);

        int pos = 0;
        foreach (const QString &column, columns) {
            if (column == "uuid") {
                table.bindValue(pos, uuid);
            } else if (column == "name") {
                table.bindValue(pos, tables.stringValue("id"));
            } else if (column == "position") {
                table.bindValue(pos, -uid);
            } else {
                table.bindValue(pos, tables.value(column));
            }
            pos++;
        }
        table.exec();

        journalInsertion(tables_table_id, uuid, database);
    }
}

static QMap<int, QString> migrateV1Warnings(QSqlDatabase &database)
{
    QMap<int, QString> warning_uuids;

    MTSqlQuery query(database);
    query.exec(renameV1TableQuery("warnings"));
    query.exec(createTableQuery(WarningRecord::tableName(), WarningRecord::columns(), database));

    QStringList columns = QStringList() << "uuid" << "scope" << "enabled" << "name" << "description" << "delay" << "date_updated" << "updated_by";
    QString insert_query = insertQuery(WarningRecord::tableName(), columns);
    int warnings_table_id = JournalEntry::tableIDForName(WarningRecord::tableName());

    MTSqlQuery warnings("SELECT * FROM v1_warnings", database);
    while (warnings.next()) {
        int id = warnings.intValue("id");
        QString uuid = warningUUID(id);
        warning_uuids.insert(id, uuid);

        MTSqlQuery warning(database);
        warning.prepare(insert_query);

        int pos = 0;
        foreach (const QString &column, columns) {
            if (column == "uuid") {
                warning.bindValue(pos, uuid);
            } else {
                warning.bindValue(pos, warnings.value(column));
            }
            pos++;
        }
        warning.exec();

        journalInsertion(warnings_table_id, uuid, database);
    }

    return warning_uuids;
}

static void migrateV1WarningFilters(const QMap<int, QString> &warning_uuids, QSqlDatabase &database)
{
    MTSqlQuery query(database);
    query.exec(renameV1TableQuery("warnings_filters"));
    query.exec(createTableQuery(WarningFilter::tableName(), WarningFilter::columns(), database));

    QStringList columns = QStringList() << "uuid" << "warning_uuid" << "circuit_attribute" << "function" << "value";
    QString insert_query = insertQuery(WarningFilter::tableName(), columns);
    int warnings_filters_table_id = JournalEntry::tableIDForName(WarningFilter::tableName());

    MTSqlQuery warning_filters("SELECT * FROM v1_warnings_filters", database);
    while (warning_filters.next()) {
        QString uuid = createUUID();

        MTSqlQuery warning_filter(database);
        warning_filter.prepare(insert_query);

        int pos = 0;
        foreach (const QString &column, columns) {
            if (column == "uuid") {
                warning_filter.bindValue(pos, uuid);
            } else if (column == "warning_uuid") {
                warning_filter.bindValue(pos, warning_uuids.value(warning_filters.intValue("parent")));
            } else {
                warning_filter.bindValue(pos, warning_filters.value(column));
            }
            pos++;
        }
        warning_filter.exec();

        journalInsertion(warnings_filters_table_id, uuid, database);
    }
}

static void migrateV1WarningConditions(const QMap<int, QString> &warning_uuids, QSqlDatabase &database)
{
    MTSqlQuery query(database);
    query.exec(renameV1TableQuery("warnings_conditions"));
    query.exec(createTableQuery(WarningCondition::tableName(), WarningCondition::columns(), database));

    QStringList columns = QStringList() << "uuid" << "warning_uuid" << "value_ins" << "function" << "value_nom";
    QString insert_query = insertQuery(WarningCondition::tableName(), columns);
    int warnings_conditions_table_id = JournalEntry::tableIDForName(WarningCondition::tableName());

    MTSqlQuery warning_conditions("SELECT * FROM v1_warnings_conditions", database);
    while (warning_conditions.next()) {
        QString uuid = createUUID();

        MTSqlQuery warning_condition(database);
        warning_condition.prepare(insert_query);

        int pos = 0;
        foreach (const QString &column, columns) {
            if (column == "uuid") {
                warning_condition.bindValue(pos, uuid);
            } else if (column == "warning_uuid") {
                warning_condition.bindValue(pos, warning_uuids.value(warning_conditions.intValue("parent")));
            } else {
                warning_condition.bindValue(pos, warning_conditions.value(column));
            }
            pos++;
        }
        warning_condition.exec();

        journalInsertion(warnings_conditions_table_id, uuid, database);
    }
}

static void migrateV1RefrigerantManagement(QSqlDatabase &database)
{
    MTSqlQuery query(database);
    query.exec(renameV1TableQuery("refrigerant_management"));
    query.exec(createTableQuery(RefrigerantRecord::tableName(), RefrigerantRecord::columns(), database));

    QStringList columns = QStringList() << "uuid" << "date" << "partner" << "partner_id" << "refrigerant" << "batch_number" << "purchased" << "purchased_reco" << "sold" << "sold_reco" << "refr_rege" << "refr_disp" << "leaked" << "leaked_reco" << "notes" << "date_updated" << "updated_by";
    QString insert_query = insertQuery(RefrigerantRecord::tableName(), columns);
    int refrigerant_management_table_id = JournalEntry::tableIDForName(RefrigerantRecord::tableName());

    MTSqlQuery refrigerant_management("SELECT * FROM v1_refrigerant_management", database);
    while (refrigerant_management.next()) {
        QString uuid = refrigerantRecordUUID(refrigerant_management.stringValue("date"));

        MTSqlQuery refrigerant_record(database);
        refrigerant_record.prepare(insert_query);

        int pos = 0;
        foreach (const QString &column, columns) {
            if (column == "uuid") {
                refrigerant_record.bindValue(pos, uuid);
            } else if (column == "partner_id") {
                refrigerant_record.bindValue(pos, formatCompanyID(refrigerant_management.stringValue(column)));
            } else {
                refrigerant_record.bindValue(pos, refrigerant_management.value(column));
            }
            pos++;
        }
        refrigerant_record.exec();

        journalInsertion(refrigerant_management_table_id, uuid, database);
    }
}

static QMap<int, QString> migrateV1AssemblyRecordTypes(const QMap<int, QString> &style_uuids, QSqlDatabase &database)
{
    QMap<int, QString> assembly_record_type_uuids;

    MTSqlQuery query(database);
    query.exec(renameV1TableQuery("assembly_record_types"));
    query.exec(createTableQuery(AssemblyRecordType::tableName(), AssemblyRecordType::columns(), database));

    QStringList columns = QStringList() << "uuid" << "style_uuid" << "name" << "description" << "display_options" << "name_format" << "date_updated" << "updated_by";
    QString insert_query = insertQuery(AssemblyRecordType::tableName(), columns);
    int assembly_record_types_table_id = JournalEntry::tableIDForName(AssemblyRecordType::tableName());

    MTSqlQuery assembly_record_types("SELECT * FROM v1_assembly_record_types", database);
    while (assembly_record_types.next()) {
        QString uuid = createUUID();
        assembly_record_type_uuids.insert(assembly_record_types.intValue("id"), uuid);

        MTSqlQuery assembly_record_type(database);
        assembly_record_type.prepare(insert_query);

        int pos = 0;
        foreach (const QString &column, columns) {
            if (column == "uuid") {
                assembly_record_type.bindValue(pos, uuid);
            } else if (column == "style_uuid") {
                assembly_record_type.bindValue(pos, style_uuids.value(assembly_record_types.intValue("style")));
            } else {
                assembly_record_type.bindValue(pos, assembly_record_types.value(column));
            }
            pos++;
        }
        assembly_record_type.exec();

        journalInsertion(assembly_record_types_table_id, uuid, database);
    }

    return assembly_record_type_uuids;
}

static QMap<int, QString> migrateV1AssemblyRecordItemCategories(QSqlDatabase &database)
{
    QMap<int, QString> assembly_record_item_category_uuids;

    MTSqlQuery query(database);
    query.exec(renameV1TableQuery("assembly_record_item_categories"));
    query.exec(createTableQuery(AssemblyRecordItemCategory::tableName(), AssemblyRecordItemCategory::columns(), database));

    QStringList columns = QStringList() << "uuid" << "name" << "display_options" << "display_position" << "date_updated" << "updated_by";
    QString insert_query = insertQuery(AssemblyRecordItemCategory::tableName(), columns);
    int assembly_record_item_categories_table_id = JournalEntry::tableIDForName(AssemblyRecordItemCategory::tableName());

    MTSqlQuery item_categories("SELECT * FROM v1_assembly_record_item_categories", database);
    while (item_categories.next()) {
        int id = item_categories.intValue("id");
        QString uuid = assemblyRecordItemCategoryUUID(id);
        assembly_record_item_category_uuids.insert(id, uuid);

        MTSqlQuery item_category(database);
        item_category.prepare(insert_query);

        int pos = 0;
        foreach (const QString &column, columns) {
            if (column == "uuid") {
                item_category.bindValue(pos, uuid);
            } else {
                item_category.bindValue(pos, item_categories.value(column));
            }
            pos++;
        }
        item_category.exec();

        journalInsertion(assembly_record_item_categories_table_id, uuid, database);
    }

    return assembly_record_item_category_uuids;
}

static QMap<int, QString> migrateV1AssemblyRecordItemTypes(const QMap<int, QString> &assembly_record_item_category_uuids, QSqlDatabase &database)
{
    QMap<int, QString> assembly_record_item_type_uuids;

    MTSqlQuery query(database);
    query.exec(renameV1TableQuery("assembly_record_item_types"));
    query.exec(createTableQuery(AssemblyRecordItemType::tableName(), AssemblyRecordItemType::columns(), database));

    QStringList columns = QStringList() << "uuid" << "ar_item_category_uuid" << "name" << "acquisition_price" << "list_price" << "ean" << "unit" << "inspection_variable_id" << "value_data_type" << "discount" << "auto_show" << "date_updated" << "updated_by";
    QString insert_query = insertQuery(AssemblyRecordItemType::tableName(), columns);
    int assembly_record_item_types_table_id = JournalEntry::tableIDForName(AssemblyRecordItemType::tableName());

    MTSqlQuery item_types("SELECT * FROM v1_assembly_record_item_types", database);
    while (item_types.next()) {
        QString uuid = createUUID();
        assembly_record_item_type_uuids.insert(item_types.intValue("id"), uuid);

        MTSqlQuery item_type(database);
        item_type.prepare(insert_query);

        int pos = 0;
        foreach (const QString &column, columns) {
            if (column == "uuid") {
                item_type.bindValue(pos, uuid);
            } else if (column == "ar_item_category_uuid") {
                item_type.bindValue(pos, assembly_record_item_category_uuids.value(item_types.intValue("category_id")));
            } else {
                item_type.bindValue(pos, item_types.value(column));
            }
            pos++;
        }
        item_type.exec();

        journalInsertion(assembly_record_item_types_table_id, uuid, database);
    }

    return assembly_record_item_type_uuids;
}

static void migrateV1AssemblyRecordTypeCategories(const QMap<int, QString> &assembly_record_item_category_uuids,
                                                  const QMap<int, QString> &assembly_record_item_type_uuids,
                                                  QSqlDatabase &database)
{
    MTSqlQuery query(database);
    query.exec(renameV1TableQuery("assembly_record_type_categories"));
    query.exec(createTableQuery(AssemblyRecordTypeCategory::tableName(), AssemblyRecordTypeCategory::columns(), database));

    QStringList columns = QStringList() << "uuid" << "ar_type_uuid" << "ar_item_category_uuid" << "position" << "date_updated" << "updated_by";
    QString insert_item_type = insertQuery(AssemblyRecordTypeCategory::tableName(), columns);
    int assembly_record_type_categories_table_id = JournalEntry::tableIDForName(AssemblyRecordTypeCategory::tableName());

    MTSqlQuery type_categories("SELECT * FROM v1_assembly_record_type_categories", database);
    while (type_categories.next()) {
        QString uuid = createUUID();

        MTSqlQuery type_category(database);
        type_category.prepare(insert_item_type);

        int pos = 0;
        foreach (const QString &column, columns) {
            if (column == "uuid") {
                type_category.bindValue(pos, uuid);
            } else if (column == "ar_type_uuid") {
                type_category.bindValue(pos, assembly_record_item_type_uuids.value(type_categories.intValue("record_type_id")));
            } else if (column == "ar_item_category_uuid") {
                type_category.bindValue(pos, assembly_record_item_category_uuids.value(type_categories.intValue("record_category_id")));
            } else {
                type_category.bindValue(pos, type_categories.value(column));
            }
            pos++;
        }
        type_category.exec();

        journalInsertion(assembly_record_type_categories_table_id, uuid, database);
    }
}

static void migrateV1AssemblyRecordItems(const QMap<int, QString> &assembly_record_item_category_uuids,
                                         const QMap<int, QString> &assembly_record_item_type_uuids,
                                         const QMap<int, QString> &circuit_unit_type_uuids,
                                         QSqlDatabase &database)
{
    MTSqlQuery query(database);
    query.exec(renameV1TableQuery("assembly_record_items"));
    query.exec(createTableQuery(AssemblyRecordItem::tableName(), AssemblyRecordItem::columns(), database));

    QStringList columns = QStringList() << "uuid" << "ar_item_type_uuid" << "ar_item_category_uuid" << "arno" << "value" << "acquisition_price" << "list_price" << "source" << "name" << "unit" << "discount" << "date_updated" << "updated_by";
    QString insert_item = insertQuery(AssemblyRecordItem::tableName(), columns);
    int assembly_record_items_table_id = JournalEntry::tableIDForName(AssemblyRecordItem::tableName());

    MTSqlQuery items("SELECT * FROM v1_assembly_record_items", database);
    while (items.next()) {
        QString uuid = createUUID();

        MTSqlQuery item(database);
        item.prepare(insert_item);

        int pos = 0;
        foreach (const QString &column, columns) {
            if (column == "uuid") {
                item.bindValue(pos, uuid);
            } else if (column == "ar_item_type_uuid") {
                switch (items.intValue("source")) {
                    case AssemblyRecordItem::CircuitUnitTypes:
                        item.bindValue(pos, circuit_unit_type_uuids.value(items.intValue("item_type_id")));
                        break;

                    case AssemblyRecordItem::Inspectors:
                        item.bindValue(pos, inspectorUUID(items.intValue("item_type_id")));
                        break;

                    default:
                        item.bindValue(pos, assembly_record_item_type_uuids.value(items.intValue("item_type_id")));
                        break;
                }
            } else if (column == "ar_item_category_uuid") {
                item.bindValue(pos, assembly_record_item_category_uuids.value(items.intValue("category_id")));
            } else {
                item.bindValue(pos, items.value(column));
            }
            pos++;
        }
        item.exec();

        journalInsertion(assembly_record_items_table_id, uuid, database);
    }
}

static QMap<int, QString> migrateV1CircuitUnitTypes(QSqlDatabase &database)
{
    QMap<int, QString> circuit_unit_type_uuids;

    MTSqlQuery query(database);
    query.exec(renameV1TableQuery("circuit_unit_types"));
    query.exec(createTableQuery(CircuitUnitType::tableName(), CircuitUnitType::columns(), database));

    QStringList columns = QStringList() << "uuid" << "manufacturer" << "type" << "refrigerant" << "refrigerant_amount" << "acquisition_price" << "list_price" << "location" << "unit" << "oil" << "oil_amount" << "output" << "output_unit" << "output_t0_tc" << "notes" << "discount" << "date_updated" << "updated_by";
    QString insert_query = insertQuery(CircuitUnitType::tableName(), columns);
    int circuit_unit_types_table_id = JournalEntry::tableIDForName(CircuitUnitType::tableName());

    MTSqlQuery unit_types("SELECT * FROM v1_circuit_unit_types", database);
    while (unit_types.next()) {
        QString uuid = createUUID();
        circuit_unit_type_uuids.insert(unit_types.intValue("id"), uuid);

        MTSqlQuery unit_type(database);
        unit_type.prepare(insert_query);

        int pos = 0;
        foreach (const QString &column, columns) {
            if (column == "uuid") {
                unit_type.bindValue(pos, uuid);
            } else {
                unit_type.bindValue(pos, unit_types.value(column));
            }
            pos++;
        }
        unit_type.exec();

        journalInsertion(circuit_unit_types_table_id, uuid, database);
    }

    return circuit_unit_type_uuids;
}

static void migrateV1CircuitUnits(const QMap<int, QString> &circuit_unit_type_uuids,
                                  QSqlDatabase &database)
{
    MTSqlQuery query(database);
    query.exec(renameV1TableQuery("circuit_units"));
    query.exec(createTableQuery(CircuitUnit::tableName(), CircuitUnit::columns(), database));

    QStringList columns = QStringList() << "uuid" << "circuit_uuid" << "unit_type_uuid" << "sn" << "date_updated" << "updated_by";
    QString insert_query = insertQuery(CircuitUnit::tableName(), columns);
    int circuit_units_table_id = JournalEntry::tableIDForName(CircuitUnit::tableName());

    MTSqlQuery units("SELECT * FROM v1_circuit_units", database);
    while (units.next()) {
        QString uuid = createUUID();

        MTSqlQuery unit(database);
        unit.prepare(insert_query);

        int pos = 0;
        foreach (const QString &column, columns) {
            if (column == "uuid") {
                unit.bindValue(pos, uuid);
            } else if (column == "circuit_uuid") {
                unit.bindValue(pos, circuitUUID(units.intValue("company_id"), units.intValue("circuit_id")));
            } else if (column == "unit_type_uuid") {
                unit.bindValue(pos, circuit_unit_type_uuids.value(units.intValue("unit_type_id")));
            } else {
                unit.bindValue(pos, units.value(column));
            }
            pos++;
        }
        unit.exec();

        journalInsertion(circuit_units_table_id, uuid, database);
    }
}

static QMap<int, QString> migrateV1Styles(QSqlDatabase &database)
{
    QMap<int, QString> style_uuids;

    MTSqlQuery query(database);
    query.exec(renameV1TableQuery("styles"));
    query.exec(createTableQuery(Style::tableName(), Style::columns(), database));

    QStringList columns = QStringList() << "uuid" << "name" << "content" << "div_tables" << "date_updated" << "updated_by";
    QString insert_query = insertQuery(Style::tableName(), columns);
    int styles_table_id = JournalEntry::tableIDForName(Style::tableName());

    MTSqlQuery styles("SELECT * FROM v1_styles", database);
    while (styles.next()) {
        QString uuid = createUUID();
        style_uuids.insert(styles.intValue("id"), uuid);

        MTSqlQuery style(database);
        style.prepare(insert_query);

        int pos = 0;
        foreach (const QString &column, columns) {
            if (column == "uuid") {
                style.bindValue(pos, uuid);
            } else {
                style.bindValue(pos, styles.value(column));
            }
            pos++;
        }
        style.exec();

        journalInsertion(styles_table_id, uuid, database);
    }

    return style_uuids;
}

void migrateV1Database(QSqlDatabase &database, QProgressBar *progress)
{
    std::function<void(int)> setProgress = [progress](int value) {
        progress->setValue(value);
        QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    };

    int last = 0;
    std::function<std::function<void(int, int)>(int)> progressUpTo = [progress, setProgress, &last](int max) {
        last = progress->value();
        int min = last;
        return [setProgress, min, max, &last](int current, int total) {
            int value = min + (current / (double)total) * (max - min);
            if (last < value) {
                last = value;
                setProgress(value);
            }
        };
    };

    progress->setRange(0, 100);
    setProgress(0);

    migrateV1Variables(database);
    setProgress(1);

    auto style_uuids = migrateV1Styles(database);
    setProgress(2);

    auto assembly_record_type_uuids = migrateV1AssemblyRecordTypes(style_uuids, database);
    setProgress(3);

    migrateV1Inspectors(database);
    setProgress(4);

    migrateV1Customers(database);
    setProgress(5);

    migrateV1Persons(database);
    setProgress(6);

    migrateV1Circuits(database, progressUpTo(18));
    setProgress(18);

    migrateV1Compressors(database, progressUpTo(30));
    setProgress(30);

    migrateV1Inspections(assembly_record_type_uuids, database, progressUpTo(60));
    setProgress(60);

    migrateV1InspectionCompressors(database, progressUpTo(85));
    setProgress(85);

    auto file_uuids = migrateV1Files(database);
    setProgress(86);

    migrateV1InspectionFiles(file_uuids, database);
    setProgress(87);

    migrateV1Repairs(database);
    setProgress(88);

    migrateV1ServiceCompanies(file_uuids, database);
    setProgress(89);

    migrateV1Tables(database);
    setProgress(90);

    auto warning_uuids = migrateV1Warnings(database);
    setProgress(91);

    migrateV1WarningFilters(warning_uuids, database);
    setProgress(92);

    migrateV1WarningConditions(warning_uuids, database);
    setProgress(93);

    migrateV1RefrigerantManagement(database);
    setProgress(94);

    auto circuit_unit_type_uuids = migrateV1CircuitUnitTypes(database);
    setProgress(95);

    migrateV1CircuitUnits(circuit_unit_type_uuids, database);
    setProgress(96);

    auto assembly_record_item_category_uuids = migrateV1AssemblyRecordItemCategories(database);
    setProgress(97);

    auto assembly_record_item_type_uuids = migrateV1AssemblyRecordItemTypes(assembly_record_item_category_uuids, database);
    setProgress(98);

    migrateV1AssemblyRecordTypeCategories(assembly_record_item_category_uuids, assembly_record_item_type_uuids, database);
    setProgress(99);

    migrateV1AssemblyRecordItems(assembly_record_item_category_uuids, assembly_record_item_type_uuids, circuit_unit_type_uuids, database);
    setProgress(100);
}
