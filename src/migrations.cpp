/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2015 Matus & Michal Tomlein

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
        foreach (const QString &column, inspectors_columns) {
            if (column == "uuid") {
                QString uuid = createUUID();
                inspector_uuids.insert(inspectors.intValue("id"), uuid);
                inspector.bindValue("uuid", uuid);
            } else {
                inspector.bindValue(column, inspectors.value(column));
            }
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

    QStringList customers_columns = QStringList() << "uuid" << "id" << "company" << "address" << "mail" << "phone" << "operator_id" << "operator_company" << "operator_address" << "operator_mail" << "operator_phone" << "date_updated" << "updated_by";
    QString insert_customer = insertQuery(Customer::tableName(), customers_columns);

    MTSqlQuery customers("SELECT * FROM v1_customers", database);
    while (customers.next()) {
        MTSqlQuery customer(database);
        customer.prepare(insert_customer);
        foreach (const QString &column, customers_columns) {
            if (column == "uuid") {
                QString uuid = createUUID();
                customer_uuids.insert(customers.intValue("id"), uuid);
                customer.bindValue("uuid", uuid);
            } else if (column == "id") {
                QString id = customers.stringValue("id");
                customer.bindValue("id", formatCompanyID(id));
            } else {
                customer.bindValue(column, customers.value(column));
            }
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
        foreach (const QString &column, persons_columns) {
            if (column == "uuid") {
                QString uuid = createUUID();
                person_uuids.insert(persons.longLongValue("id"), uuid);
                person.bindValue("uuid", uuid);
            } else if (column == "customer_uuid") {
                person.bindValue(column, customer_uuids.value(persons.intValue("company_id")));
            } else {
                person.bindValue(column, persons.value(column));
            }
        }
        person.exec();
    }

    return person_uuids;
}

static QMap<QPair<int, int>, QString> migrateV1Circuits(const QMap<int, QString> &customer_uuids, QSqlDatabase &database)
{
    QMap<QPair<int, int>, QString> circuit_uuids;

    QString update_circuit = "UPDATE circuits SET uuid = :uuid, customer_uuid = :customer_uuid WHERE parent = :parent AND id = :id";

    MTSqlQuery circuits("SELECT parent, id FROM circuits", database);
    while (circuits.next()) {
        int parent = circuits.intValue("parent");
        int id = circuits.intValue("id");
        QString uuid = createUUID();
        circuit_uuids.insert(QPair<int, int>(parent, id), uuid);

        MTSqlQuery circuit(database);
        circuit.prepare(update_circuit);
        circuit.bindValue("uuid", uuid);
        circuit.bindValue("customer_uuid", customer_uuids.value(parent));
        circuit.bindValue("parent", parent);
        circuit.bindValue("id", id);
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
        foreach (const QString &column, columns) {
            if (column == "uuid") {
                QString uuid = createUUID();
                compressor_uuids.insert(compressors.longLongValue("id"), uuid);
                compressor.bindValue("uuid", uuid);
            } else if (column == "circuit_uuid") {
                compressor.bindValue(column, circuit_uuids.value(QPair<int, int>(compressors.intValue("customer_id"), compressors.intValue("circuit_id"))));
            } else {
                compressor.bindValue(column, compressors.value(column));
            }
        }
        compressor.exec();
    }

    return compressor_uuids;
}

static QMap<QPair<QPair<int, int>, QString>, QString> migrateV1Inspections(const QMap<int, QString> &customer_uuids,
                                                                           const QMap<QPair<int, int>, QString> &circuit_uuids,
                                                                           const QMap<int, QString> &inspector_uuids,
                                                                           const QMap<qlonglong, QString> &person_uuids,
                                                                           QSqlDatabase &database)
{
    QMap<QPair<QPair<int, int>, QString>, QString> inspection_uuids;

    QString update_inspection = "UPDATE inspections SET uuid = :uuid, customer_uuid = :customer_uuid, circuit_uuid = :circuit_uuid, inspector = :inspector, operator = :operator WHERE customer = :customer AND circuit = :circuit AND date = :date";

    MTSqlQuery inspections("SELECT customer, circuit, date, inspector, operator FROM inspections", database);
    while (inspections.next()) {
        int customer = inspections.intValue("customer");
        int circuit = inspections.intValue("circuit");
        QPair<int, int> customer_circuit(customer, circuit);
        QString date = inspections.stringValue("date");
        int inspector = inspections.intValue("inspector");
        qlonglong operator_id = inspections.longLongValue("operator");
        QString uuid = createUUID();
        inspection_uuids.insert(QPair<QPair<int, int>, QString>(customer_circuit, date), uuid);

        MTSqlQuery inspection(database);
        inspection.prepare(update_inspection);
        inspection.bindValue("uuid", uuid);
        inspection.bindValue("customer_uuid", customer_uuids.value(customer));
        inspection.bindValue("circuit_uuid", circuit_uuids.value(customer_circuit));
        inspection.bindValue("inspector", inspector_uuids.value(inspector));
        inspection.bindValue("operator", person_uuids.value(operator_id));
        inspection.bindValue("customer", customer);
        inspection.bindValue("circuit", circuit);
        inspection.bindValue("date", date);
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
        inspection_compressor.bindValue("uuid", uuid);
        inspection_compressor.bindValue("inspection_uuid", inspection_uuids.value(QPair<QPair<int, int>, QString>(customer_circuit, date)));
        inspection_compressor.bindValue("compressor_uuid", compressor_uuids.value(compressor_id));
        inspection_compressor.bindValue("id", id);
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
        file.bindValue("uuid", uuid);
        file.bindValue("id", id);
        file.exec();
    }

    return file_uuids;
}

static void migrateV1InspectionImages(const QMap<QPair<QPair<int, int>, QString>, QString> &inspection_uuids,
                                      const QMap<int, QString> &file_uuids,
                                      QSqlDatabase &database)
{
    QString update_inspection_image = "UPDATE inspection_images SET uuid = :uuid, inspection_uuid = :inspection_uuid, file_uuid = :file_uuid WHERE customer = :customer AND circuit = :circuit AND date = :date AND file_id = :file_id";

    MTSqlQuery inspection_images("SELECT customer, circuit, date, file_id FROM inspection_images", database);
    while (inspection_images.next()) {
        int customer = inspection_images.intValue("customer");
        int circuit = inspection_images.intValue("circuit");
        QPair<int, int> customer_circuit(customer, circuit);
        QString date = inspection_images.stringValue("date");
        int file_id = inspection_images.intValue("file_id");
        QString uuid = createUUID();

        MTSqlQuery inspection_image(database);
        inspection_image.prepare(update_inspection_image);
        inspection_image.bindValue("uuid", uuid);
        inspection_image.bindValue("inspection_uuid", inspection_uuids.value(QPair<QPair<int, int>, QString>(customer_circuit, date)));
        inspection_image.bindValue("file_uuid", file_uuids.value(file_id));
        inspection_image.bindValue("customer", customer);
        inspection_image.bindValue("circuit", circuit);
        inspection_image.bindValue("date", date);
        inspection_image.bindValue("file_id", file_id);
        inspection_image.exec();
    }
}

static void migrateV1Repairs(const QMap<int, QString> &customer_uuids,
                             const QMap<int, QString> &inspector_uuids,
                             QSqlDatabase &database)
{
    QString update_repair = "UPDATE repairs SET uuid = :uuid, customer_uuid = :customer_uuid, inspector_uuid = :inspector_uuid WHERE date = :date";

    MTSqlQuery repairs("SELECT date, parent, repairman FROM repairs", database);
    while (repairs.next()) {
        QString date = repairs.stringValue("date");
        int parent = repairs.intValue("parent");
        int repairman = repairs.intValue("repairman");
        QString uuid = createUUID();

        MTSqlQuery repair(database);
        repair.prepare(update_repair);
        repair.bindValue("uuid", uuid);
        repair.bindValue("customer_uuid", customer_uuids.value(parent));
        repair.bindValue("inspector_uuid", inspector_uuids.value(repairman));
        repair.bindValue("date", date);
        repair.exec();
    }
}

static void migrateV1ServiceCompanies(const QMap<int, QString> &file_uuids, QSqlDatabase &database)
{
    MTSqlQuery query(database);
    query.exec(renameV1TableQuery("service_companies"));
    query.exec(createTableQuery(ServiceCompany::tableName(), ServiceCompany::columns(), database));

    QStringList columns = QStringList() << "uuid" << "image_file_uuid" << "id" << "name" << "address" << "mail" << "phone" << "website" << "date_updated" << "updated_by";
    QString insert_query = insertQuery(ServiceCompany::tableName(), columns);

    MTSqlQuery service_companies("SELECT * FROM v1_service_companies", database);
    while (service_companies.next()) {
        MTSqlQuery service_company(database);
        service_company.prepare(insert_query);
        foreach (const QString &column, columns) {
            if (column == "uuid") {
                QString uuid = createUUID();
                service_company.bindValue("uuid", uuid);
            } else if (column == "id") {
                QString id = service_companies.stringValue("id");
                service_company.bindValue("id", formatCompanyID(id));
            } else if (column == "image_file_uuid") {
                service_company.bindValue(column, file_uuids.value(service_companies.intValue("image")));
            } else {
                service_company.bindValue(column, service_companies.value(column));
            }
        }
        service_company.exec();
    }
}

static void migrateV1Tables(QSqlDatabase &database)
{
    QString update_table = "UPDATE tables SET uuid = :uuid WHERE id = :id";

    MTSqlQuery tables("SELECT id FROM tables", database);
    while (tables.next()) {
        QString id = tables.stringValue("id");
        QString uuid = createUUID();

        MTSqlQuery table(database);
        table.prepare(update_table);
        table.bindValue("uuid", uuid);
        table.bindValue("id", id);
        table.exec();
    }
}

static QMap<int, QString> migrateV1Warnings(QSqlDatabase &database)
{
    QMap<int, QString> warning_uuids;

    QString update_warning = "UPDATE warnings SET uuid = :uuid WHERE id = :id";

    MTSqlQuery warnings("SELECT id FROM warnings", database);
    while (warnings.next()) {
        int id = warnings.intValue("id");
        QString uuid = createUUID();
        warning_uuids.insert(id, uuid);

        MTSqlQuery warning(database);
        warning.prepare(update_warning);
        warning.bindValue("uuid", uuid);
        warning.bindValue("id", id);
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
        foreach (const QString &column, columns) {
            if (column == "uuid") {
                QString uuid = createUUID();
                warning_filter.bindValue("uuid", uuid);
            } else if (column == "warning_uuid") {
                warning_filter.bindValue(column, warning_uuids.value(warning_filters.intValue("parent")));
            } else {
                warning_filter.bindValue(column, warning_filters.value(column));
            }
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
        foreach (const QString &column, columns) {
            if (column == "uuid") {
                QString uuid = createUUID();
                warning_condition.bindValue("uuid", uuid);
            } else if (column == "warning_uuid") {
                warning_condition.bindValue(column, warning_uuids.value(warning_conditions.intValue("parent")));
            } else {
                warning_condition.bindValue(column, warning_conditions.value(column));
            }
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
        foreach (const QString &column, columns) {
            if (column == "uuid") {
                QString uuid = createUUID();
                refrigerant_record.bindValue("uuid", uuid);
            } else if (column == "partner_id") {
                refrigerant_record.bindValue(column, formatCompanyID(refrigerant_management.stringValue(column)));
            } else {
                refrigerant_record.bindValue(column, refrigerant_management.value(column));
            }
        }
        refrigerant_record.exec();
    }
}

static QMap<int, QString> migrateV1AssemblyRecordTypes(QSqlDatabase &database)
{
    QMap<int, QString> assembly_record_type_uuids;

    MTSqlQuery query(database);
    query.exec(renameV1TableQuery("assembly_record_types"));
    query.exec(createTableQuery(AssemblyRecordType::tableName(), AssemblyRecordType::columns(), database));

    QStringList columns = QStringList() << "uuid" << "name" << "description" << "display_options" << "style" << "name_format" << "date_updated" << "updated_by";
    QString insert_query = insertQuery(AssemblyRecordType::tableName(), columns);

    MTSqlQuery assembly_record_types("SELECT * FROM v1_assembly_record_types", database);
    while (assembly_record_types.next()) {
        MTSqlQuery assembly_record_type(database);
        assembly_record_type.prepare(insert_query);
        foreach (const QString &column, columns) {
            if (column == "uuid") {
                QString uuid = createUUID();
                assembly_record_type_uuids.insert(assembly_record_types.intValue("id"), uuid);
                assembly_record_type.bindValue("uuid", uuid);
            } else {
                assembly_record_type.bindValue(column, assembly_record_types.value(column));
            }
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

    QStringList columns = QStringList() << "uuid" << "name" << "display_options" << "display_position" << "date_updated" << "updated_by";
    QString insert_query = insertQuery(AssemblyRecordItemCategory::tableName(), columns);

    MTSqlQuery item_categories("SELECT * FROM v1_assembly_record_item_categories", database);
    while (item_categories.next()) {
        MTSqlQuery item_category(database);
        item_category.prepare(insert_query);
        foreach (const QString &column, columns) {
            if (column == "uuid") {
                QString uuid = createUUID();
                assembly_record_item_category_uuids.insert(item_categories.intValue("id"), uuid);
                item_category.bindValue("uuid", uuid);
            } else {
                item_category.bindValue(column, item_categories.value(column));
            }
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

    QStringList columns = QStringList() << "uuid" << "item_category_uuid" << "name" << "acquisition_price" << "list_price" << "ean" << "unit" << "inspection_variable_id" << "value_data_type" << "discount" << "auto_show" << "date_updated" << "updated_by";
    QString insert_query = insertQuery(AssemblyRecordItemType::tableName(), columns);

    MTSqlQuery item_types("SELECT * FROM v1_assembly_record_item_types", database);
    while (item_types.next()) {
        MTSqlQuery item_type(database);
        item_type.prepare(insert_query);
        foreach (const QString &column, columns) {
            if (column == "uuid") {
                QString uuid = createUUID();
                assembly_record_item_type_uuids.insert(item_types.intValue("id"), uuid);
                item_type.bindValue("uuid", uuid);
            } else if (column == "item_category_uuid") {
                item_type.bindValue(column, assembly_record_item_category_uuids.value(item_types.intValue("category_id")));
            } else {
                item_type.bindValue(column, item_types.value(column));
            }
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

    QStringList columns = QStringList() << "uuid" << "item_type_uuid" << "item_category_uuid" << "position" << "date_updated" << "updated_by";
    QString insert_item_type = insertQuery(AssemblyRecordTypeCategory::tableName(), columns);

    MTSqlQuery type_categories("SELECT * FROM v1_assembly_record_type_categories", database);
    while (type_categories.next()) {
        MTSqlQuery type_category(database);
        type_category.prepare(insert_item_type);
        foreach (const QString &column, columns) {
            if (column == "uuid") {
                QString uuid = createUUID();
                type_category.bindValue("uuid", uuid);
            } else if (column == "item_type_uuid") {
                type_category.bindValue(column, assembly_record_item_type_uuids.value(type_categories.intValue("record_type_id")));
            } else if (column == "item_category_uuid") {
                type_category.bindValue(column, assembly_record_item_category_uuids.value(type_categories.intValue("record_category_id")));
            } else {
                type_category.bindValue(column, type_categories.value(column));
            }
        }
        type_category.exec();
    }
}

static void migrateV1AssemblyRecordItems(const QMap<int, QString> &assembly_record_item_category_uuids,
                                         const QMap<int, QString> &assembly_record_item_type_uuids,
                                         QSqlDatabase &database)
{
    MTSqlQuery query(database);
    query.exec(renameV1TableQuery("assembly_record_items"));
    query.exec(createTableQuery(AssemblyRecordItem::tableName(), AssemblyRecordItem::columns(), database));

    QStringList columns = QStringList() << "uuid" << "item_type_uuid" << "item_category_uuid" << "arno" << "value" << "acquisition_price" << "list_price" << "source" << "name" << "unit" << "discount" << "date_updated" << "updated_by";
    QString insert_item = insertQuery(AssemblyRecordItem::tableName(), columns);

    MTSqlQuery items("SELECT * FROM v1_assembly_record_items", database);
    while (items.next()) {
        MTSqlQuery item(database);
        item.prepare(insert_item);
        foreach (const QString &column, columns) {
            if (column == "uuid") {
                QString uuid = createUUID();
                item.bindValue("uuid", uuid);
            } else if (column == "item_type_uuid") {
                item.bindValue(column, assembly_record_item_type_uuids.value(items.intValue("item_type_id")));
            } else if (column == "item_category_uuid") {
                item.bindValue(column, assembly_record_item_category_uuids.value(items.intValue("category_id")));
            } else {
                item.bindValue(column, items.value(column));
            }
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
        foreach (const QString &column, columns) {
            if (column == "uuid") {
                QString uuid = createUUID();
                circuit_unit_type_uuids.insert(unit_types.intValue("id"), uuid);
                unit_type.bindValue("uuid", uuid);
            } else {
                unit_type.bindValue(column, unit_types.value(column));
            }
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
        foreach (const QString &column, columns) {
            if (column == "uuid") {
                QString uuid = createUUID();
                unit.bindValue("uuid", uuid);
            } else if (column == "circuit_uuid") {
                unit.bindValue(column, circuit_uuids.value(QPair<int, int>(units.intValue("company_id"), units.intValue("circuit_id"))));
            } else if (column == "unit_type_uuid") {
                unit.bindValue(column, circuit_unit_type_uuids.value(units.intValue("unit_type_id")));
            } else {
                unit.bindValue(column, units.value(column));
            }
        }
        unit.exec();
    }
}

static void migrateV1Styles(QSqlDatabase &database)
{
    QString update_styles = "UPDATE styles SET uuid = :uuid WHERE id = :id";

    MTSqlQuery styles("SELECT id FROM styles", database);
    while (styles.next()) {
        int id = styles.intValue("id");
        QString uuid = createUUID();

        MTSqlQuery style(database);
        style.prepare(update_styles);
        style.bindValue("uuid", uuid);
        style.bindValue("id", id);
        style.exec();
    }
}

void migrateV1Database(QSqlDatabase &database)
{
    auto inspector_uuids = migrateV1Inspectors(database);

    auto customer_uuids = migrateV1Customers(database);

    auto person_uuids = migrateV1Persons(customer_uuids, database);

    auto circuit_uuids = migrateV1Circuits(customer_uuids, database);

    auto compressor_uuids = migrateV1Compressors(circuit_uuids, database);

    auto inspection_uuids = migrateV1Inspections(customer_uuids, circuit_uuids, inspector_uuids, person_uuids, database);

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

    auto assembly_record_type_uuids = migrateV1AssemblyRecordTypes(database);

    auto assembly_record_item_category_uuids = migrateV1AssemblyRecordItemCategories(database);

    auto assembly_record_item_type_uuids = migrateV1AssemblyRecordItemTypes(assembly_record_item_category_uuids, database);

    migrateV1AssemblyRecordTypeCategories(assembly_record_item_category_uuids, assembly_record_item_type_uuids, database);

    migrateV1AssemblyRecordItems(assembly_record_item_category_uuids, assembly_record_item_type_uuids, database);

    auto circuit_unit_type_uuids = migrateV1CircuitUnitTypes(database);

    migrateV1CircuitUnits(circuit_uuids, circuit_unit_type_uuids, database);

    migrateV1Styles(database);
}
