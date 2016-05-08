/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2016 Matus & Michal Tomlein

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

#include <QSqlError>

using namespace Global;

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

static QMap<int, QString> migrateV1Inspectors(QSqlDatabase &database)
{
    QMap<int, QString> inspector_uuids;

    MTSqlQuery query(database);
    query.exec(renameV1TableQuery("inspectors"));
    query.exec(createTableQuery(Inspector::tableName(), Inspector::columns(), database));

    QStringList inspectors_columns = QStringList() << "uuid" << "certificate_number" << "certificate_country" << "person" << "mail" << "phone" << "list_price" << "acquisition_price" << "date_updated" << "updated_by";
    QString insert_inspector = insertQuery(Inspector::tableName(), inspectors_columns);

    MTSqlQuery inspectors("SELECT * FROM v1_inspectors", database);
    while (inspectors.next()) {
        MTSqlQuery inspector(database);
        inspector.prepare(insert_inspector);

        int pos = 0;
        foreach (const QString &column, inspectors_columns) {
            if (column == "uuid") {
                QString uuid = createUUID();
                inspector_uuids.insert(inspectors.intValue("id"), uuid);
                inspector.bindValue(pos, uuid);
            } else {
                inspector.bindValue(pos, inspectors.value(column));
            }
            pos++;
        }
        inspector.exec();
    }

    return inspector_uuids;
}

static QMap<int, QString> migrateV1Customers(QSqlDatabase &database)
{
    QMap<int, QString> customer_uuids;

    MTSqlQuery query(database);
    query.exec(renameV1TableQuery("customers"));
    query.exec(createTableQuery(Customer::tableName(), Customer::columns(), database));

    QStringList customers_columns = QStringList() << "uuid" << "id" << "company" << "address" << "mail" << "phone" << "operator_type" << "operator_id" << "operator_company" << "operator_address" << "operator_mail" << "operator_phone" << "date_updated" << "updated_by";
    QString insert_customer = insertQuery(Customer::tableName(), customers_columns);

    MTSqlQuery customers("SELECT * FROM v1_customers", database);
    while (customers.next()) {
        int operator_id = customers.intValue("operator_id");

        MTSqlQuery customer(database);
        customer.prepare(insert_customer);

        int pos = 0;
        foreach (const QString &column, customers_columns) {
            if (column == "uuid") {
                QString uuid = createUUID();
                customer_uuids.insert(customers.intValue("id"), uuid);
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
    }

    return customer_uuids;
}

static QMap<qlonglong, QString> migrateV1Persons(const QMap<int, QString> &customer_uuids, QSqlDatabase &database)
{
    QMap<qlonglong, QString> person_uuids;

    MTSqlQuery query(database);
    query.exec(renameV1TableQuery("persons"));
    query.exec(createTableQuery(Person::tableName(), Person::columns(), database));

    QStringList persons_columns = QStringList() << "uuid" << "customer_uuid" << "name" << "mail" << "phone" << "hidden" << "date_updated" << "updated_by";
    QString insert_person = insertQuery(Person::tableName(), persons_columns);

    MTSqlQuery persons("SELECT * FROM v1_persons", database);
    while (persons.next()) {
        MTSqlQuery person(database);
        person.prepare(insert_person);

        int pos = 0;
        foreach (const QString &column, persons_columns) {
            if (column == "uuid") {
                QString uuid = createUUID();
                person_uuids.insert(persons.longLongValue("id"), uuid);
                person.bindValue(pos, uuid);
            } else if (column == "customer_uuid") {
                person.bindValue(pos, customer_uuids.value(persons.intValue("company_id")));
            } else {
                person.bindValue(pos, persons.value(column));
            }
            pos++;
        }
        person.exec();
    }

    return person_uuids;
}

static QMap<QPair<int, int>, QString> migrateV1Circuits(const QMap<int, QString> &customer_uuids, QSqlDatabase &database)
{
    QMap<QPair<int, int>, QString> circuit_uuids;

    MTSqlQuery query(database);
    query.exec(renameV1TableQuery("circuits"));
    query.exec(createTableQuery(Circuit::tableName(), Circuit::columns(), database));

    QStringList columns = QStringList() << "uuid" << "customer_uuid" << "id" << "name" << "disused" << "operation" << "building" << "device" << "hermetic" << "manufacturer" << "type" << "sn" << "year" << "commissioning" << "decommissioning" << "field" << "refrigerant" << "refrigerant_amount" << "oil" << "oil_amount" << "leak_detector" << "runtime" << "utilisation" << "inspection_interval" << "date_updated" << "updated_by";
    QString insert_query = insertQuery(Circuit::tableName(), columns);

    MTSqlQuery circuits("SELECT * FROM v1_circuits", database);
    while (circuits.next()) {
        int parent = circuits.intValue("parent");
        int id = circuits.intValue("id");
        QString uuid = createUUID();
        circuit_uuids.insert(QPair<int, int>(parent, id), uuid);

        MTSqlQuery circuit(database);
        circuit.prepare(insert_query);

        int pos = 0;
        foreach (const QString &column, columns) {
            if (column == "uuid") {
                circuit.bindValue(pos, uuid);
            } else if (column == "customer_uuid") {
                circuit.bindValue(pos, customer_uuids.value(parent));
            } else {
                circuit.bindValue(pos, circuits.value(column));
            }
            pos++;
        }
        circuit.exec();
    }

    return circuit_uuids;
}

static QMap<qlonglong, QString> migrateV1Compressors(const QMap<QPair<int, int>, QString> &circuit_uuids, QSqlDatabase &database)
{
    QMap<qlonglong, QString> compressor_uuids;

    MTSqlQuery query(database);
    query.exec(renameV1TableQuery("compressors"));
    query.exec(createTableQuery(Compressor::tableName(), Compressor::columns(), database));

    QStringList columns = QStringList() << "uuid" << "circuit_uuid" << "name" << "manufacturer" << "type" << "sn" << "date_updated" << "updated_by";
    QString insert_compressor = insertQuery(Compressor::tableName(), columns);

    MTSqlQuery compressors("SELECT * FROM v1_compressors", database);
    while (compressors.next()) {
        MTSqlQuery compressor(database);
        compressor.prepare(insert_compressor);

        int pos = 0;
        foreach (const QString &column, columns) {
            if (column == "uuid") {
                QString uuid = createUUID();
                compressor_uuids.insert(compressors.longLongValue("id"), uuid);
                compressor.bindValue(pos, uuid);
            } else if (column == "circuit_uuid") {
                compressor.bindValue(pos, circuit_uuids.value(QPair<int, int>(compressors.intValue("customer_id"), compressors.intValue("circuit_id"))));
            } else {
                compressor.bindValue(pos, compressors.value(column));
            }
            pos++;
        }
        compressor.exec();
    }

    return compressor_uuids;
}

static QString inspectionTypeDataFromV1Data(Inspection::Type type, const QString &type_data, const QMap<int, QString> &customer_uuids)
{
    switch (type) {
        case Inspection::CircuitMovedType: {
            QStringList data = type_data.split(UNIT_SEPARATOR);
            if (data.count() > 2) {
                data[0] = customer_uuids.value(data[0].toInt());
                data[1] = customer_uuids.value(data[1].toInt());
            }
            return data.join(UNIT_SEPARATOR);
        }

        default:
            break;
    }

    return type_data;
}

static QMap<QPair<QPair<int, int>, QString>, QString> migrateV1Inspections(const QMap<int, QString> &customer_uuids,
                                                                           const QMap<QPair<int, int>, QString> &circuit_uuids,
                                                                           const QMap<int, QString> &inspector_uuids,
                                                                           const QMap<qlonglong, QString> &person_uuids,
                                                                           const QMap<int, QString> &assembly_record_type_uuids,
                                                                           QSqlDatabase &database)
{
    QMap<QPair<QPair<int, int>, QString>, QString> inspection_uuids;

    QString update_inspection = "UPDATE inspections SET uuid = :uuid, customer_uuid = :customer_uuid, circuit_uuid = :circuit_uuid, inspector_uuid = :inspector_uuid, person_uuid = :person_uuid, ar_type_uuid = :ar_type_uuid, inspection_type_data = :inspection_type_data WHERE customer = :customer AND circuit = :circuit AND date = :date";

    MTSqlQuery inspections("SELECT customer, circuit, date, inspector, operator, ar_type, inspection_type, inspection_type_data FROM inspections", database);
    while (inspections.next()) {
        int customer = inspections.intValue("customer");
        int circuit = inspections.intValue("circuit");
        QPair<int, int> customer_circuit(customer, circuit);
        QString date = inspections.stringValue("date");
        int inspector = inspections.intValue("inspector");
        qlonglong operator_id = inspections.longLongValue("operator");
        int ar_type = inspections.intValue("ar_type");
        Inspection::Type inspection_type = (Inspection::Type)inspections.intValue("inspection_type");
        QString inspection_type_data = inspections.stringValue("inspection_type_data");
        QString uuid = createUUID();
        inspection_uuids.insert(QPair<QPair<int, int>, QString>(customer_circuit, date), uuid);

        MTSqlQuery inspection(database);
        inspection.prepare(update_inspection);
        inspection.bindValue(":uuid", uuid);
        inspection.bindValue(":customer_uuid", customer_uuids.value(customer));
        inspection.bindValue(":circuit_uuid", circuit_uuids.value(customer_circuit));
        inspection.bindValue(":inspector_uuid", inspector_uuids.value(inspector));
        inspection.bindValue(":person_uuid", person_uuids.value(operator_id));
        inspection.bindValue(":ar_type_uuid", assembly_record_type_uuids.value(ar_type));
        inspection.bindValue(":inspection_type_data", inspectionTypeDataFromV1Data(inspection_type, inspection_type_data, customer_uuids));
        inspection.bindValue(":customer", customer);
        inspection.bindValue(":circuit", circuit);
        inspection.bindValue(":date", date);
        inspection.exec();
    }

    return inspection_uuids;
}

static void migrateV1InspectionCompressors(const QMap<QPair<QPair<int, int>, QString>, QString> &inspection_uuids,
                                           const QMap<qlonglong, QString> &compressor_uuids,
                                           QSqlDatabase &database)
{
    QString update_inspection_compressor = "UPDATE inspections_compressors SET uuid = :uuid, inspection_uuid = :inspection_uuid, compressor_uuid = :compressor_uuid WHERE id = :id";

    MTSqlQuery inspection_compressors("SELECT id, customer_id, circuit_id, date, compressor_id FROM inspections_compressors", database);
    while (inspection_compressors.next()) {
        int id = inspection_compressors.intValue("id");
        int customer_id = inspection_compressors.intValue("customer_id");
        int circuit_id = inspection_compressors.intValue("circuit_id");
        QPair<int, int> customer_circuit(customer_id, circuit_id);
        QString date = inspection_compressors.stringValue("date");
        qlonglong compressor_id = inspection_compressors.longLongValue("compressor_id");
        QString uuid = createUUID();

        MTSqlQuery inspection_compressor(database);
        inspection_compressor.prepare(update_inspection_compressor);
        inspection_compressor.bindValue(":uuid", uuid);
        inspection_compressor.bindValue(":inspection_uuid", inspection_uuids.value(QPair<QPair<int, int>, QString>(customer_circuit, date)));
        inspection_compressor.bindValue(":compressor_uuid", compressor_uuids.value(compressor_id));
        inspection_compressor.bindValue(":id", id);
        inspection_compressor.exec();
    }
}

static QMap<int, QString> migrateV1Files(QSqlDatabase &database)
{
    QMap<int, QString> file_uuids;

    QString update_file = "UPDATE files SET uuid = :uuid WHERE id = :id";

    MTSqlQuery files("SELECT id FROM files", database);
    while (files.next()) {
        int id = files.intValue("id");
        QString uuid = createUUID();
        file_uuids.insert(id, uuid);

        MTSqlQuery file(database);
        file.prepare(update_file);
        file.bindValue(":uuid", uuid);
        file.bindValue(":id", id);
        file.exec();
    }

    return file_uuids;
}

static void migrateV1InspectionImages(const QMap<QPair<QPair<int, int>, QString>, QString> &inspection_uuids,
                                      const QMap<int, QString> &file_uuids,
                                      QSqlDatabase &database)
{
    MTSqlQuery query(database);
    query.exec(renameV1TableQuery("inspection_images"));
    query.exec(createTableQuery(InspectionImage::tableName(), InspectionImage::columns(), database));

    QStringList columns = QStringList() << "uuid" << "inspection_uuid" << "file_uuid" << "description" << "date_updated" << "updated_by";
    QString insert_query = insertQuery(InspectionImage::tableName(), columns);

    MTSqlQuery inspection_images("SELECT * FROM v1_inspection_images", database);
    while (inspection_images.next()) {
        int customer = inspection_images.intValue("customer");
        int circuit = inspection_images.intValue("circuit");
        QPair<int, int> customer_circuit(customer, circuit);
        QString date = inspection_images.stringValue("date");
        int file_id = inspection_images.intValue("file_id");
        QString uuid = createUUID();

        MTSqlQuery inspection_image(database);
        inspection_image.prepare(insert_query);

        int pos = 0;
        foreach (const QString &column, columns) {
            if (column == "uuid") {
                inspection_image.bindValue(pos, uuid);
            } else if (column == "inspection_uuid") {
                inspection_image.bindValue(pos, inspection_uuids.value(QPair<QPair<int, int>, QString>(customer_circuit, date)));
            } else if (column == "file_uuid") {
                inspection_image.bindValue(pos, file_uuids.value(file_id));
            } else {
                inspection_image.bindValue(pos, inspection_images.value(column));
            }
            pos++;
        }
        inspection_image.exec();
    }
}

static void migrateV1Repairs(const QMap<int, QString> &customer_uuids,
                             const QMap<int, QString> &inspector_uuids,
                             QSqlDatabase &database)
{
    MTSqlQuery query(database);
    query.exec(renameV1TableQuery("repairs"));
    query.exec(createTableQuery(Repair::tableName(), Repair::columns(), database));

    QStringList columns = QStringList() << "uuid" << "customer_uuid" << "inspector_uuid" << "date" << "customer" << "device" << "field" << "refrigerant" << "refrigerant_amount" << "refr_add_am" << "refr_reco" << "arno" << "date_updated" << "updated_by";
    QString insert_query = insertQuery(Repair::tableName(), columns);

    MTSqlQuery repairs("SELECT * FROM v1_repairs", database);
    while (repairs.next()) {
        MTSqlQuery repair(database);
        repair.prepare(insert_query);

        int pos = 0;
        foreach (const QString &column, columns) {
            if (column == "uuid") {
                QString uuid = createUUID();
                repair.bindValue(pos, uuid);
            } else if (column == "customer_uuid") {
                repair.bindValue(pos, customer_uuids.value(repairs.intValue("parent")));
            } else if (column == "inspector_uuid") {
                repair.bindValue(pos, inspector_uuids.value(repairs.intValue("repairman")));
            } else {
                repair.bindValue(pos, repairs.value(column));
            }
            pos++;
        }
        repair.exec();
    }
}

static void migrateV1ServiceCompanies(const QMap<int, QString> &file_uuids, QSqlDatabase &database)
{
    QMap<int, QString> service_company_uuids;

    MTSqlQuery query(database);
    query.exec(renameV1TableQuery("service_companies"));
    query.exec(createTableQuery(ServiceCompany::tableName(), ServiceCompany::columns(), database));

    QStringList columns = QStringList() << "uuid" << "image_file_uuid" << "id" << "name" << "address" << "mail" << "phone" << "website" << "date_updated" << "updated_by";
    QString insert_query = insertQuery(ServiceCompany::tableName(), columns);

    MTSqlQuery service_companies("SELECT * FROM v1_service_companies", database);
    while (service_companies.next()) {
        MTSqlQuery service_company(database);
        service_company.prepare(insert_query);

        int pos = 0;
        foreach (const QString &column, columns) {
            if (column == "uuid") {
                QString uuid = createUUID();
                service_company_uuids.insert(service_companies.intValue("id"), uuid);
                service_company.bindValue(pos, uuid);
            } else if (column == "id") {
                QString id = service_companies.stringValue("id");
                service_company.bindValue(pos, formatCompanyID(id));
            } else if (column == "image_file_uuid") {
                service_company.bindValue(pos, file_uuids.value(service_companies.intValue("image")));
            } else {
                service_company.bindValue(pos, service_companies.value(column));
            }
            pos++;
        }
        service_company.exec();
    }

    DBInfo::setValueForKey("default_service_company_uuid", service_company_uuids.value(DBInfo::valueForKey("default_service_company", QString(), database).toInt()), database);
}

static void migrateV1Tables(QSqlDatabase &database)
{
    MTSqlQuery query(database);
    query.exec(renameV1TableQuery("tables"));
    query.exec(createTableQuery(Table::tableName(), Table::columns(), database));

    QStringList columns = QStringList() << "uuid" << "name" << "position" << "highlight_nominal" << "scope" << "variables" << "sum" << "avg" << "date_updated" << "updated_by";
    QString insert_query = insertQuery(Table::tableName(), columns);

    MTSqlQuery tables("SELECT * FROM v1_tables", database);
    while (tables.next()) {
        int uid = tables.intValue("uid");

        MTSqlQuery table(database);
        table.prepare(insert_query);

        int pos = 0;
        foreach (const QString &column, columns) {
            if (column == "uuid") {
                QString uuid;

                switch (uid) {
                    case 90:
                        uuid = LEAKAGES_TABLE_UUID;
                        break;

                    case 70:
                        uuid = PRESSURES_AND_TEMPERATURES_TABLE_UUID;
                        break;

                    case 40:
                        uuid = COMPRESSORS_TABLE_UUID;
                        break;

                    default:
                        uuid = createUUID();
                        break;
                }

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

    MTSqlQuery warnings("SELECT * FROM v1_warnings", database);
    while (warnings.next()) {
        MTSqlQuery warning(database);
        warning.prepare(insert_query);

        int pos = 0;
        foreach (const QString &column, columns) {
            if (column == "uuid") {
                QString uuid = createUUID();
                warning_uuids.insert(warnings.intValue("id"), uuid);
                warning.bindValue(pos, uuid);
            } else {
                warning.bindValue(pos, warnings.value(column));
            }
            pos++;
        }
        warning.exec();
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

    MTSqlQuery warning_filters("SELECT * FROM v1_warnings_filters", database);
    while (warning_filters.next()) {
        MTSqlQuery warning_filter(database);
        warning_filter.prepare(insert_query);

        int pos = 0;
        foreach (const QString &column, columns) {
            if (column == "uuid") {
                QString uuid = createUUID();
                warning_filter.bindValue(pos, uuid);
            } else if (column == "warning_uuid") {
                warning_filter.bindValue(pos, warning_uuids.value(warning_filters.intValue("parent")));
            } else {
                warning_filter.bindValue(pos, warning_filters.value(column));
            }
            pos++;
        }
        warning_filter.exec();
    }
}

static void migrateV1WarningConditions(const QMap<int, QString> &warning_uuids, QSqlDatabase &database)
{
    MTSqlQuery query(database);
    query.exec(renameV1TableQuery("warnings_conditions"));
    query.exec(createTableQuery(WarningCondition::tableName(), WarningCondition::columns(), database));

    QStringList columns = QStringList() << "uuid" << "warning_uuid" << "value_ins" << "function" << "value_nom";
    QString insert_query = insertQuery(WarningCondition::tableName(), columns);

    MTSqlQuery warning_conditions("SELECT * FROM v1_warnings_conditions", database);
    while (warning_conditions.next()) {
        MTSqlQuery warning_condition(database);
        warning_condition.prepare(insert_query);

        int pos = 0;
        foreach (const QString &column, columns) {
            if (column == "uuid") {
                QString uuid = createUUID();
                warning_condition.bindValue(pos, uuid);
            } else if (column == "warning_uuid") {
                warning_condition.bindValue(pos, warning_uuids.value(warning_conditions.intValue("parent")));
            } else {
                warning_condition.bindValue(pos, warning_conditions.value(column));
            }
            pos++;
        }
        warning_condition.exec();
    }
}

static void migrateV1RefrigerantManagement(QSqlDatabase &database)
{
    MTSqlQuery query(database);
    query.exec(renameV1TableQuery("refrigerant_management"));
    query.exec(createTableQuery(RefrigerantRecord::tableName(), RefrigerantRecord::columns(), database));

    QStringList columns = QStringList() << "uuid" << "date" << "partner" << "partner_id" << "refrigerant" << "purchased" << "purchased_reco" << "sold" << "sold_reco" << "refr_rege" << "refr_disp" << "leaked" << "leaked_reco" << "date_updated" << "updated_by";
    QString insert_query = insertQuery(RefrigerantRecord::tableName(), columns);

    MTSqlQuery refrigerant_management("SELECT * FROM v1_refrigerant_management", database);
    while (refrigerant_management.next()) {
        MTSqlQuery refrigerant_record(database);
        refrigerant_record.prepare(insert_query);

        int pos = 0;
        foreach (const QString &column, columns) {
            if (column == "uuid") {
                QString uuid = createUUID();
                refrigerant_record.bindValue(pos, uuid);
            } else if (column == "partner_id") {
                refrigerant_record.bindValue(pos, formatCompanyID(refrigerant_management.stringValue(column)));
            } else {
                refrigerant_record.bindValue(pos, refrigerant_management.value(column));
            }
            pos++;
        }
        refrigerant_record.exec();
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

    MTSqlQuery assembly_record_types("SELECT * FROM v1_assembly_record_types", database);
    while (assembly_record_types.next()) {
        MTSqlQuery assembly_record_type(database);
        assembly_record_type.prepare(insert_query);

        int pos = 0;
        foreach (const QString &column, columns) {
            if (column == "uuid") {
                QString uuid = createUUID();
                assembly_record_type_uuids.insert(assembly_record_types.intValue("id"), uuid);
                assembly_record_type.bindValue(pos, uuid);
            } else if (column == "style_uuid") {
                assembly_record_type.bindValue(pos, style_uuids.value(assembly_record_types.intValue("style")));
            } else {
                assembly_record_type.bindValue(pos, assembly_record_types.value(column));
            }
            pos++;
        }
        assembly_record_type.exec();
    }

    return assembly_record_type_uuids;
}

static QMap<int, QString> migrateV1AssemblyRecordItemCategories(QSqlDatabase &database)
{
    QMap<int, QString> assembly_record_item_category_uuids;

    MTSqlQuery query(database);
    query.exec(renameV1TableQuery("assembly_record_item_categories"));
    query.exec(createTableQuery(AssemblyRecordItemCategory::tableName(), AssemblyRecordItemCategory::columns(), database));

    QStringList columns = QStringList() << "uuid" << "predefined" << "name" << "display_options" << "display_position" << "date_updated" << "updated_by";
    QString insert_query = insertQuery(AssemblyRecordItemCategory::tableName(), columns);

    MTSqlQuery item_categories("SELECT * FROM v1_assembly_record_item_categories", database);
    while (item_categories.next()) {
        int id = item_categories.intValue("id");

        MTSqlQuery item_category(database);
        item_category.prepare(insert_query);

        int pos = 0;
        foreach (const QString &column, columns) {
            if (column == "uuid") {
                QString uuid;

                switch (id) {
                    case 1000:
                        uuid = INSPECTORS_CATEGORY_UUID;
                        break;

                    case 1001:
                        uuid = CIRCUIT_UNITS_CATEGORY_UUID;
                        break;

                    default:
                        uuid = createUUID();
                        break;
                }

                assembly_record_item_category_uuids.insert(id, uuid);
                item_category.bindValue(pos, uuid);
            } else if (column == "predefined") {
                item_category.bindValue(pos, id >= 1000);
            } else {
                item_category.bindValue(pos, item_categories.value(column));
            }
            pos++;
        }
        item_category.exec();
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

    MTSqlQuery item_types("SELECT * FROM v1_assembly_record_item_types", database);
    while (item_types.next()) {
        MTSqlQuery item_type(database);
        item_type.prepare(insert_query);

        int pos = 0;
        foreach (const QString &column, columns) {
            if (column == "uuid") {
                QString uuid = createUUID();
                assembly_record_item_type_uuids.insert(item_types.intValue("id"), uuid);
                item_type.bindValue(pos, uuid);
            } else if (column == "ar_item_category_uuid") {
                item_type.bindValue(pos, assembly_record_item_category_uuids.value(item_types.intValue("category_id")));
            } else {
                item_type.bindValue(pos, item_types.value(column));
            }
            pos++;
        }
        item_type.exec();
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

    MTSqlQuery type_categories("SELECT * FROM v1_assembly_record_type_categories", database);
    while (type_categories.next()) {
        MTSqlQuery type_category(database);
        type_category.prepare(insert_item_type);

        int pos = 0;
        foreach (const QString &column, columns) {
            if (column == "uuid") {
                QString uuid = createUUID();
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
    }
}

static void migrateV1AssemblyRecordItems(const QMap<int, QString> &assembly_record_item_category_uuids,
                                         const QMap<int, QString> &assembly_record_item_type_uuids,
                                         const QMap<int, QString> &circuit_unit_type_uuids,
                                         const QMap<int, QString> &inspector_uuids,
                                         QSqlDatabase &database)
{
    MTSqlQuery query(database);
    query.exec(renameV1TableQuery("assembly_record_items"));
    query.exec(createTableQuery(AssemblyRecordItem::tableName(), AssemblyRecordItem::columns(), database));

    QStringList columns = QStringList() << "uuid" << "ar_item_type_uuid" << "ar_item_category_uuid" << "arno" << "value" << "acquisition_price" << "list_price" << "source" << "name" << "unit" << "discount" << "date_updated" << "updated_by";
    QString insert_item = insertQuery(AssemblyRecordItem::tableName(), columns);

    MTSqlQuery items("SELECT * FROM v1_assembly_record_items", database);
    while (items.next()) {
        MTSqlQuery item(database);
        item.prepare(insert_item);

        int pos = 0;
        foreach (const QString &column, columns) {
            if (column == "uuid") {
                QString uuid = createUUID();
                item.bindValue(pos, uuid);
            } else if (column == "ar_item_type_uuid") {
                switch (items.intValue("source")) {
                    case AssemblyRecordItem::CircuitUnitTypes:
                        item.bindValue(pos, circuit_unit_type_uuids.value(items.intValue("item_type_id")));
                        break;

                    case AssemblyRecordItem::Inspectors:
                        item.bindValue(pos, inspector_uuids.value(items.intValue("item_type_id")));
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

    MTSqlQuery unit_types("SELECT * FROM v1_circuit_unit_types", database);
    while (unit_types.next()) {
        MTSqlQuery unit_type(database);
        unit_type.prepare(insert_query);

        int pos = 0;
        foreach (const QString &column, columns) {
            if (column == "uuid") {
                QString uuid = createUUID();
                circuit_unit_type_uuids.insert(unit_types.intValue("id"), uuid);
                unit_type.bindValue(pos, uuid);
            } else {
                unit_type.bindValue(pos, unit_types.value(column));
            }
            pos++;
        }
        unit_type.exec();
    }

    return circuit_unit_type_uuids;
}

static void migrateV1CircuitUnits(const QMap<QPair<int, int>, QString> &circuit_uuids,
                                  const QMap<int, QString> &circuit_unit_type_uuids,
                                  QSqlDatabase &database)
{
    MTSqlQuery query(database);
    query.exec(renameV1TableQuery("circuit_units"));
    query.exec(createTableQuery(CircuitUnit::tableName(), CircuitUnit::columns(), database));

    QStringList columns = QStringList() << "uuid" << "circuit_uuid" << "unit_type_uuid" << "sn" << "date_updated" << "updated_by";
    QString insert_query = insertQuery(CircuitUnit::tableName(), columns);

    MTSqlQuery units("SELECT * FROM v1_circuit_units", database);
    while (units.next()) {
        MTSqlQuery unit(database);
        unit.prepare(insert_query);

        int pos = 0;
        foreach (const QString &column, columns) {
            if (column == "uuid") {
                QString uuid = createUUID();
                unit.bindValue(pos, uuid);
            } else if (column == "circuit_uuid") {
                unit.bindValue(pos, circuit_uuids.value(QPair<int, int>(units.intValue("company_id"), units.intValue("circuit_id"))));
            } else if (column == "unit_type_uuid") {
                unit.bindValue(pos, circuit_unit_type_uuids.value(units.intValue("unit_type_id")));
            } else {
                unit.bindValue(pos, units.value(column));
            }
            pos++;
        }
        unit.exec();
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

    MTSqlQuery styles("SELECT * FROM v1_styles", database);
    while (styles.next()) {
        MTSqlQuery style(database);
        style.prepare(insert_query);

        int pos = 0;
        foreach (const QString &column, columns) {
            if (column == "uuid") {
                QString uuid = createUUID();
                style_uuids.insert(styles.intValue("id"), uuid);
                style.bindValue(pos, uuid);
            } else {
                style.bindValue(pos, styles.value(column));
            }
            pos++;
        }
        style.exec();
    }

    return style_uuids;
}

void migrateV1Database(QSqlDatabase &database)
{
    auto style_uuids = migrateV1Styles(database);

    auto assembly_record_type_uuids = migrateV1AssemblyRecordTypes(style_uuids, database);

    auto inspector_uuids = migrateV1Inspectors(database);

    auto customer_uuids = migrateV1Customers(database);

    auto person_uuids = migrateV1Persons(customer_uuids, database);

    auto circuit_uuids = migrateV1Circuits(customer_uuids, database);

    auto compressor_uuids = migrateV1Compressors(circuit_uuids, database);

    auto inspection_uuids = migrateV1Inspections(customer_uuids, circuit_uuids, inspector_uuids, person_uuids, assembly_record_type_uuids, database);

    migrateV1InspectionCompressors(inspection_uuids, compressor_uuids, database);

    auto file_uuids = migrateV1Files(database);

    migrateV1InspectionImages(inspection_uuids, file_uuids, database);

    migrateV1Repairs(customer_uuids, inspector_uuids, database);

    migrateV1ServiceCompanies(file_uuids, database);

    migrateV1Tables(database);

    auto warning_uuids = migrateV1Warnings(database);

    migrateV1WarningFilters(warning_uuids, database);

    migrateV1WarningConditions(warning_uuids, database);

    migrateV1RefrigerantManagement(database);

    auto circuit_unit_type_uuids = migrateV1CircuitUnitTypes(database);

    migrateV1CircuitUnits(circuit_uuids, circuit_unit_type_uuids, database);

    auto assembly_record_item_category_uuids = migrateV1AssemblyRecordItemCategories(database);

    auto assembly_record_item_type_uuids = migrateV1AssemblyRecordItemTypes(assembly_record_item_category_uuids, database);

    migrateV1AssemblyRecordTypeCategories(assembly_record_item_category_uuids, assembly_record_item_type_uuids, database);

    migrateV1AssemblyRecordItems(assembly_record_item_category_uuids, assembly_record_item_type_uuids, circuit_unit_type_uuids, inspector_uuids, database);
}
