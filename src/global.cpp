/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2011 Matus & Michal Tomlein

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

#include "fparser/fparser.hh"
#include "refrigerants.h"
#include "global.h"
#include "variables.h"

#include <QApplication>
#include <QSet>
#include <QVariant>
#include <QColor>
#include <QDate>
#include <QNetworkRequest>
#include <QSqlRecord>
#include <QSqlField>
#include <QSqlError>

#include <cmath>

QString Global::escapeString(QString s, bool escape_backslash, bool insert_linebreaks)
{
    if (escape_backslash) s.replace("\\", "\\\\");
    s.replace("&", "&amp;");
    s.replace("\"", "&quot;");
    s.replace("'", "&#039;");
    s.replace("<", "&lt;");
    s.replace(">", "&gt;");
    if (insert_linebreaks) s.replace("\n", "<br>");
    return s;
}

QString Global::upArrow() { return QString::fromUtf8("\342\206\221"); }

QString Global::downArrow() { return QString::fromUtf8("\342\206\223"); }

QString Global::rightTriangle() { return QString::fromUtf8("\342\226\270"); }

QString Global::degreeSign() { return QString::fromUtf8("\302\260"); }

QString Global::delta() { return QString::fromUtf8("\316\224"); }

QColor Global::textColourForBaseColour(const QColor & c)
{
    if ((((c.red() * 299.0) + (c.green() * 587.0) + (c.blue() * 114.0)) / 1000.0) > 125.0 &&
        (c.red() + c.green() + c.blue()) > 500) {
        return QColor(Qt::black);
    } else {
        return QColor(Qt::white);
    }
}

QString Global::variantTypeToSqlType(int type)
{
    switch (type) {
        case QVariant::Int: return "INTEGER"; break;
        case QVariant::Double: return "NUMERIC"; break;
        default: break;
    }
    return "TEXT";
}

QString Global::variableTypeToSqlType(const QString & type)
{
    if (type == "float") return "NUMERIC";
    else if (type == "int" || type == "bool") return "INTEGER";
    return "TEXT";
}

MTDictionary Global::getTableFieldNames(const QString & table, QSqlDatabase * database)
{
    MTDictionary field_names;
    QSqlQuery query(*database);
    query.exec("SELECT * FROM " + table);
    for (int i = 0; i < query.record().count(); ++i) {
        field_names.insert(query.record().fieldName(i), variantTypeToSqlType(query.record().field(i).type()));
    }
    return field_names;
}

void Global::copyTable(const QString & table, QSqlDatabase * from, QSqlDatabase * to, const QString & filter)
{
    QSqlQuery select(*from);
    select.exec("SELECT * FROM " + table + QString(filter.isEmpty() ? "" : (" WHERE " + filter)));
    if (select.next() && select.record().count()) {
        QString copy("INSERT INTO " + table + " (");
        MTDictionary field_names = getTableFieldNames(table, to);
        QString field_name;
        for (int i = 0; i < select.record().count(); ++i) {
            field_name = select.record().fieldName(i);
            copy.append(i == 0 ? "" : ", ");
            copy.append(field_name);
            if (!field_names.contains(field_name)) {
                addColumn(field_name + " " + variantTypeToSqlType(select.record().field(i).type()), table, to);
            }
        }
        copy.append(") VALUES (");
        for (int i = 0; i < select.record().count(); ++i) {
            copy.append(i == 0 ? ":" : ", :");
            copy.append(select.record().fieldName(i));
        }
        copy.append(")");
        do {
            QSqlQuery insert(*to);
            insert.prepare(copy);
            for (int i = 0; i < select.record().count(); ++i) {
                insert.bindValue(":" + select.record().fieldName(i), select.value(i));
            }
            insert.exec();
        } while (select.next());
    }
}

void Global::addColumn(const QString & column, const QString & table, QSqlDatabase * database)
{
    QString col = column;
    if (column.split(" ").count() < 2) { col.append(" TEXT"); }
    QSqlQuery add_column(*database);
    add_column.exec("ALTER TABLE " + table + " ADD COLUMN " + col);
}

void Global::renameColumn(const QString & column, const QString & new_name, const QString & table, QSqlDatabase * database)
{
    if (isDatabaseRemote(database)) {
        QSqlQuery rename_column(*database);
        rename_column.exec("ALTER TABLE " + table + " RENAME COLUMN " + column + " TO " + new_name);
    } else {
        MTDictionary all_field_names = getTableFieldNames(table, database);
        QString column_type;
        if (all_field_names.contains(column)) {
            column_type = " " + all_field_names.value(column);
            all_field_names.remove(column);
        }
        QString fields; QString field_names = all_field_names.keys().join(", ");
        for (int i = 0; i < all_field_names.count(); ++i) {
            if (i) { fields.append(", "); }
            fields.append(all_field_names.key(i) + " " + all_field_names.value(i));
        }
        QSqlQuery query(*database);
        query.exec(QString("CREATE TEMPORARY TABLE _tmp (%1, _tmpcol%2)").arg(fields).arg(column_type));
        query.exec(QString("INSERT INTO _tmp SELECT %1, %2 FROM %3").arg(field_names).arg(column).arg(table));
        query.exec(QString("DROP TABLE %1").arg(table));
        query.exec(QString("CREATE TABLE %1 (%2, %3%4)").arg(table).arg(fields).arg(new_name).arg(column_type));
        query.exec(QString("INSERT INTO %1 SELECT %2, _tmpcol FROM _tmp").arg(table).arg(field_names));
        query.exec(QString("DROP TABLE _tmp"));
    }
}

void Global::dropColumn(const QString & column, const QString & table, QSqlDatabase * database)
{
    if (isDatabaseRemote(database)) {
        QSqlQuery drop_column(*database);
        drop_column.exec("ALTER TABLE " + table + " DROP COLUMN " + column);
    } else {
        MTDictionary all_field_names = getTableFieldNames(table, database);
        all_field_names.remove(column);
        QString fields; QString field_names = all_field_names.keys().join(", ");
        for (int i = 0; i < all_field_names.count(); ++i) {
            if (i) { fields.append(", "); }
            fields.append(all_field_names.key(i) + " " + all_field_names.value(i));
        }
        QSqlQuery query(*database);
        query.exec(QString("CREATE TEMPORARY TABLE _tmp (%1)").arg(fields));
        query.exec(QString("INSERT INTO _tmp SELECT %1 FROM %2").arg(field_names).arg(table));
        query.exec(QString("DROP TABLE %1").arg(table));
        query.exec(QString("CREATE TABLE %1 (%2)").arg(table).arg(fields));
        query.exec(QString("INSERT INTO %1 SELECT %2 FROM _tmp").arg(table).arg(field_names));
        query.exec(QString("DROP TABLE _tmp"));
    }
}

QString Global::DBInfoValueForKey(const QString & key, const QString & default_value)
{
    QSqlQuery query(QString("SELECT value FROM db_info WHERE id = '%1'").arg(key));
    if (!query.next())
        return default_value;
    return query.value(0).toString();
}

QSqlError Global::setDBInfoValueForKey(const QString & key, const QString & value)
{
    QSqlQuery query(QString("SELECT value FROM db_info WHERE id = '%1'").arg(key));
    if (query.next())
        return QSqlQuery(QString("UPDATE db_info SET value = '%1' WHERE id = '%2'").arg(value).arg(key)).lastError();
    return QSqlQuery(QString("INSERT INTO db_info (id, value) VALUES ('%1', '%2')").arg(key).arg(value)).lastError();
}

QString Global::currentUser(QSqlDatabase * database)
{
    return database ?
            database->userName() :
            QSqlDatabase::database().userName();
}

bool Global::isCurrentUserAdmin()
{
    QString current_user = currentUser();
    return DBInfoValueForKey("admin", current_user) == current_user;
}

bool Global::isDatabaseRemote(QSqlDatabase * database)
{
    return database ?
            !database->driverName().contains("SQLITE") :
            !QSqlDatabase::database().driverName().contains("SQLITE");
}

int Global::isDatabaseLocked()
{
    QString locked = DBInfoValueForKey("locked");
    if (locked == "true")
        return 1;
    if (locked == "auto")
        return 2;
    return 0;
}

QString Global::lockDate()
{
    return DBInfoValueForKey("locked") == "auto" ?
            QDate::currentDate().addDays(-DBInfoValueForKey("autolock_days").toInt()).toString("yyyy.MM.dd") :
            DBInfoValueForKey("lock_date");
}

bool Global::isRecordLocked(const QString & date)
{
    if (isDatabaseLocked())
        return date < lockDate();
    return false;
}

bool Global::isOwnerPermissionApplicable(const QString & permission)
{
    return (isDatabaseRemote() &&
            (permission.startsWith("edit_") ||
             permission.startsWith("remove_")) &&
            (permission.endsWith("_refrigerant_management") ||
             permission.endsWith("_customer") ||
             permission.endsWith("_circuit") ||
             permission.endsWith("_inspection") ||
             permission.endsWith("_repair")));
}

int Global::isOperationPermitted(const QString & operation, const QString & record_owner)
{
    if (!isDatabaseLocked())
        return 4;
    if (isDatabaseRemote() && isCurrentUserAdmin())
        return 3;
    QString permission = DBInfoValueForKey(operation + "_permitted", "true");
    if (permission == "true")
        return 1;
    if (permission == "owner")
        return record_owner == currentUser() ? 2 : -2;
    return -1;
}

double Global::getCircuitRefrigerantAmount(const QString & customer_id, const QString & circuit_id, double refrigerant_amount)
{
    MTDictionary parents("customer", customer_id);
    parents.insert("circuit", circuit_id);
    parents.insert("nominal", "1");
    return refrigerant_amount + MTRecord("inspections", "date", "", parents).value("refr_add_am", 0.0).toDouble();
}

QMap<QString, MTDictionary> Global::parsed_expressions;

MTDictionary Global::parseExpression(const QString & exp, QStringList & used_ids)
{
    MTDictionary dict_exp(true);
    if (!exp.isEmpty() && !parsed_expressions.contains(exp)) {
        QStringList circuit_attributes; circuit_attributes << "refrigerant_amount" << "oil_amount";
        QStringList functions; functions << "sum" << "p_to_t";
        for (int i = 0; i < circuit_attributes.count(); ++i) {
            if (!used_ids.contains(circuit_attributes.at(i))) { used_ids << circuit_attributes.at(i); }
        }
        for (int i = 0; i < functions.count(); ++i) {
            if (!used_ids.contains(functions.at(i))) { used_ids << functions.at(i); }
        }
        QSet<int> matched;
        for (int i = 0; i < used_ids.count(); ++i) {
            QRegExp expression(QString("\\b%1\\b").arg(used_ids.at(i)));
            int index = exp.indexOf(expression);
            while (index >= 0) {
                int length = expression.matchedLength();
                if (!matched.contains(index)) {
                    for (int j = index; j < index + length; j++) { matched << j; }
                }
                index = exp.indexOf(expression, index + length);
            }
        }
        QString id_, f_; bool last_id = false; int last_f = 0;
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
                    if (!functions.contains(id_)) {
                        if (circuit_attributes.contains(id_)) {
                            dict_exp.insert(id_, "circuit_attribute");
                        } else {
                            dict_exp.insert(id_, last_f ? functions.at(last_f - 1) : "id");
                        }
                    }
                    last_f = functions.indexOf(id_) + 1;
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
            if (circuit_attributes.contains(id_)) {
                dict_exp.insert(id_, "circuit_attribute");
            } else {
                dict_exp.insert(id_, last_f ? functions.at(last_f - 1) : "id");
            }
        }
        parsed_expressions.insert(exp, dict_exp);
    } else {
        return parsed_expressions.value(exp, MTDictionary(true));
    }
    return dict_exp;
}

Refrigerants refrigerants;

double Global::evaluateExpression(QVariantMap & inspection, const MTDictionary & expression, const QString & customer_id, const QString & circuit_id, bool * ok, bool * null_var)
{
    if (null_var) *null_var = false;
    QString inspection_date = inspection.value("date").toString();
    FunctionParser fparser;
    const QString sum_query("SELECT %1 FROM inspections WHERE date LIKE '%2%' AND customer = :customer_id AND circuit = :circuit_id AND (nominal <> 1 OR nominal IS NULL)");
    MTRecord circuit("circuits", "id", circuit_id, MTDictionary("parent", customer_id));
    QVariantMap circuit_attributes = circuit.list();
    QString value;
    for (int i = 0; i < expression.count(); ++i) {
        if (expression.value(i) == "id") {
            if (null_var && inspection.value(expression.key(i)).isNull()) *null_var = true;
            value.append(QString::number(inspection.value(expression.key(i)).toDouble()));
        } else if (expression.value(i) == "sum") {
            double v = 0.0;
            if (inspection.value("nominal").toInt()) {
                if (null_var && inspection.value(expression.key(i)).isNull()) *null_var = true;
                v += inspection.value(expression.key(i)).toDouble();
            } else {
                QSqlQuery sum_ins;
                sum_ins.prepare(sum_query.arg(expression.key(i)).arg(inspection_date.left(4)));
                sum_ins.bindValue(":customer_id", customer_id);
                sum_ins.bindValue(":circuit_id", circuit_id);
                if (sum_ins.exec()) {
                    while (sum_ins.next()) {
                        v += sum_ins.value(0).toDouble();
                    }
                }
            }
            value.append(QString::number(v));
        } else if (expression.value(i) == "circuit_attribute") {
            double attribute = circuit_attributes.value(expression.key(i)).toDouble();
            if (expression.key(i) == "refrigerant_amount" && !inspection.value("nominal").toInt())
                attribute = getCircuitRefrigerantAmount(customer_id, circuit_id, attribute);
            value.append(QString::number(attribute));
        } else if (expression.value(i) == "p_to_t") {
            if (null_var && inspection.value(expression.key(i)).isNull()) *null_var = true;
            MTRecord circuit("circuits", "id", circuit_id, MTDictionary("parent", customer_id));
            QString refrigerant = circuit.stringValue("refrigerant");
            value.append(QString::number(refrigerants.pressureToTemperature(refrigerant, round(inspection.value(expression.key(i)).toDouble() * 10.0) / 10.0 + 1.0)));
        } else {
            value.append(expression.key(i));
        }
    }
    if (fparser.Parse(value.toStdString(), "") >= 0) {
        if (ok) *ok = false;
        return 0;
    }
    if (ok) *ok = true;
    long double result = fparser.Eval(NULL);
    if (round(result) == result) return (double)result;
    return (double)(round(result * REAL_NUMBER_PRECISION_EXP) / REAL_NUMBER_PRECISION_EXP);
}

QString Global::compareValues(double value1, double value2, double tolerance, const QString & bg_class)
{
    if (value1 < value2) {
        return "<div style=\"float: left; font-size: large;" + QString(value2 - value1 > tolerance ? "color: #FF0000; " : "") + "\">" + upArrow() + "</div>&nbsp;%1";
    } else if (value1 > value2) {
        return "<div style=\"float: left; font-size: large;" + QString(value1 - value2 > tolerance ? "color: #FF0000; " : "") + "\">" + downArrow() + "</div>&nbsp;%1";
    } else {
        return "%1";
    }
}

QString Global::toolTipLink(const QString & type, const QString & text, const QString & l1, const QString & l2, const QString & l3, bool modify_allowed)
{
    QString link = "<a ";
    if (modify_allowed)
        link += "onmouseover=\"Tip('<a href=&quot;%1&quot;>"
            + QApplication::translate("MainWindow", "View")
            + "</a> | <a href=&quot;%1/modify&quot;>"
            + QApplication::translate("MainWindow", "Modify")
            + "</a>', STICKY, true, CLICKCLOSE, true)\" onmouseout=\"UnTip()\" ";
    link += "href=\"%1\">" + text + "</a>";
    QString href; QStringList typelist = type.split("/");
    if (typelist.count() > 0) href.append(typelist.at(0) + ":" + l1);
    if (typelist.count() > 1) href.append("/" + typelist.at(1) + ":" + l2);
    if (typelist.count() > 2) href.append("/" + typelist.at(2) + ":" + l3);
    return link.arg(href);
}

class DatabaseTables
{
public:
    DatabaseTables() {
        dict.insert("service_companies", "id INTEGER PRIMARY KEY, name TEXT, address TEXT, mail TEXT, phone TEXT, website TEXT, image INTEGER, date_updated TEXT, updated_by TEXT");
        dict.insert("customers", "id INTEGER PRIMARY KEY, company TEXT, address TEXT, mail TEXT, phone TEXT, date_updated TEXT, updated_by TEXT");
        dict.insert("persons", "id INTEGER PRIMARY KEY, company_id INTEGER, name TEXT, mail TEXT, phone TEXT, date_updated TEXT, updated_by TEXT");
        dict.insert("circuits", "parent INTEGER, id INTEGER, name TEXT, disused INTEGER, operation TEXT, building TEXT, device TEXT, hermetic INTEGER, manufacturer TEXT, type TEXT, sn TEXT, year INTEGER, commissioning TEXT, decommissioning TEXT, field TEXT, refrigerant TEXT, refrigerant_amount NUMERIC, oil TEXT, oil_amount NUMERIC, leak_detector INTEGER, runtime NUMERIC, utilisation NUMERIC, inspection_interval INTEGER, date_updated TEXT, updated_by TEXT");
        dict.insert("inspections", "customer INTEGER, circuit INTEGER, date TEXT, nominal INTEGER, repair INTEGER, outside_interval INTEGER, date_updated TEXT, updated_by TEXT");
        dict.insert("inspection_images", "customer INTEGER, circuit INTEGER, date TEXT, description TEXT, file_id INTEGER, date_updated TEXT, updated_by TEXT");
        dict.insert("repairs", "date TEXT, parent INTEGER, customer TEXT, device TEXT, field TEXT, refrigerant TEXT, refrigerant_amount NUMERIC, refr_add_am NUMERIC, refr_reco NUMERIC, repairman TEXT, arno TEXT, date_updated TEXT, updated_by TEXT");
        dict.insert("inspectors", "id INTEGER PRIMARY KEY, person TEXT, mail TEXT, phone TEXT, list_price NUMERIC, acquisition_price NUMERIC, date_updated TEXT, updated_by TEXT");
        dict.insert("variables", "id TEXT, name TEXT, type TEXT, unit TEXT, value TEXT, compare_nom INTEGER, tolerance NUMERIC, col_bg TEXT, date_updated TEXT, updated_by TEXT");
        dict.insert("subvariables", "parent TEXT, id TEXT, name TEXT, type TEXT, unit TEXT, value TEXT, compare_nom INTEGER, tolerance NUMERIC, date_updated TEXT, updated_by TEXT");
        dict.insert("tables", "uid TEXT, id TEXT, highlight_nominal INTEGER, variables TEXT, sum TEXT, avg TEXT, date_updated TEXT, updated_by TEXT");
        dict.insert("warnings", "id INTEGER PRIMARY KEY, enabled INTEGER, name TEXT, description TEXT, delay INTEGER, date_updated TEXT, updated_by TEXT");
        dict.insert("warnings_filters", "parent INTEGER, circuit_attribute TEXT, function TEXT, value TEXT");
        dict.insert("warnings_conditions", "parent INTEGER, value_ins TEXT, function TEXT, value_nom TEXT");
        dict.insert("refrigerant_management", "date TEXT, partner TEXT, partner_id INTEGER, refrigerant TEXT, purchased NUMERIC, purchased_reco NUMERIC, sold NUMERIC, sold_reco NUMERIC, refr_rege NUMERIC, refr_disp NUMERIC, leaked NUMERIC, leaked_reco NUMERIC, date_updated TEXT, updated_by TEXT");
        dict.insert("assembly_record_types", "id INTEGER PRIMARY KEY, name TEXT, description TEXT, display_options INTEGER, style INTEGER, date_updated TEXT, updated_by TEXT");
        dict.insert("assembly_record_item_types", "id INTEGER PRIMARY KEY, name TEXT, acquisition_price NUMERIC, list_price NUMERIC, ean INTEGER, unit TEXT, category_id INTEGER, inspection_variable_id TEXT, value_data_type INTEGER, discount NUMERIC, auto_show INTEGER, date_updated TEXT, updated_by TEXT");
        dict.insert("assembly_record_type_categories", "record_type_id INTEGER, record_category_id INTEGER, position INTEGER, date_updated TEXT, updated_by TEXT");
        dict.insert("assembly_record_item_categories", "id INTEGER PRIMARY KEY, name TEXT, display_options INTEGER, display_position INTEGER, date_updated TEXT, updated_by TEXT");
        dict.insert("assembly_record_items", "arno TEXT, item_type_id INTEGER, value TEXT, acquisition_price REAL, list_price REAL, source INTEGER, name TEXT, category_id INTEGER, unit TEXT, discount NUMERIC, date_updated TEXT, updated_by TEXT");
        dict.insert("files", "id INTEGER PRIMARY KEY, name TEXT, data BYTEA, date_updated TEXT, updated_by TEXT");
        dict.insert("circuit_unit_types", "id INTEGER PRIMARY KEY, manufacturer TEXT, type TEXT, refrigerant TEXT, refrigerant_amount NUMERIC, acquisition_price NUMERIC, list_price NUMERIC, location INTEGER, unit TEXT, oil TEXT, oil_amount NUMERIC, output NUMERIC, output_unit TEXT, output_t0_tc NUMERIC, notes TEXT, discount NUMERIC, date_updated TEXT, updated_by TEXT");
        dict.insert("circuit_units", "id INTEGER PRIMARY KEY, company_id INTEGER, circuit_id INTEGER, unit_type_id INTEGER, sn TEXT, date_updated TEXT, updated_by TEXT");
        dict.insert("db_info", "id TEXT, value TEXT");
        dict.insert("styles", "id INTEGER, name TEXT, content TEXT, date_updated TEXT, div_tables INTEGER, updated_by TEXT");
    }

    MTDictionary dict;
};

const MTDictionary & Global::databaseTables()
{
    static DatabaseTables dict;
    return dict.dict;
}

class VariableTypes
{
public:
    VariableTypes() {
        dict.insert("int", QApplication::translate("VariableTypes", "Integer"));
        dict.insert("float", QApplication::translate("VariableTypes", "Real number"));
        dict.insert("string", QApplication::translate("VariableTypes", "String"));
        dict.insert("text", QApplication::translate("VariableTypes", "Text"));
        dict.insert("bool", QApplication::translate("VariableTypes", "Boolean"));
    }

    MTDictionary dict;
};

const MTDictionary & Global::variableTypes()
{
    static VariableTypes dict;
    return dict.dict;
}

class VariableNames
{
public:
    VariableNames() {
        dict.insert("t_sec", QApplication::translate("VariableNames", "Temperature sec. medium"));
        dict.insert("t_sec_evap_in", QApplication::translate("VariableNames", "evap. in"));
        dict.insert("t_sec_cond_in", QApplication::translate("VariableNames", "cond. in"));
        dict.insert("p_0", QApplication::translate("VariableNames", "Pressure evaporating"));
        dict.insert("p_c", QApplication::translate("VariableNames", "Pressure condensing"));
        dict.insert("t_0", QApplication::translate("VariableNames", "Temperature evaporating"));
        dict.insert("t_c", QApplication::translate("VariableNames", "Temperature condensing"));
        dict.insert("t_ev", QApplication::translate("VariableNames", "Temperature EV"));
        dict.insert("t_evap_out", QApplication::translate("VariableNames", "Temperature evap. out"));
        dict.insert("t_comp_in", QApplication::translate("VariableNames", "Temperature comp. in"));
        dict.insert("t_sc", QApplication::translate("VariableNames", "Subcooling"));
        dict.insert("t_sh", QApplication::translate("VariableNames", "Superheating"));
        dict.insert("t_sh_evap", QApplication::translate("VariableNames", "evap."));
        dict.insert("t_sh_comp", QApplication::translate("VariableNames", "comp."));
        dict.insert("t_comp_out", QApplication::translate("VariableNames", "Temperature discharge"));
        dict.insert("delta_t_evap", QApplication::translate("VariableNames", "%1T (evaporating)", 0, QApplication::UnicodeUTF8).arg(Global::delta()));
        dict.insert("delta_t_c", QApplication::translate("VariableNames", "%1T (condensing)", 0, QApplication::UnicodeUTF8).arg(Global::delta()));
        dict.insert("ep_comp", QApplication::translate("VariableNames", "Comp. el. power input"));
        dict.insert("ec", QApplication::translate("VariableNames", "Electric current"));
        dict.insert("ec_l1", QApplication::translate("VariableNames", "L1"));
        dict.insert("ec_l2", QApplication::translate("VariableNames", "L2"));
        dict.insert("ec_l3", QApplication::translate("VariableNames", "L3"));
        dict.insert("ev", QApplication::translate("VariableNames", "Electric voltage"));
        dict.insert("ev_l1", QApplication::translate("VariableNames", "L1"));
        dict.insert("ev_l2", QApplication::translate("VariableNames", "L2"));
        dict.insert("ev_l3", QApplication::translate("VariableNames", "L3"));
        dict.insert("ppsw", QApplication::translate("VariableNames", "Pneumatic pressure switches"));
        dict.insert("ppsw_hip", QApplication::translate("VariableNames", "HiP"));
        dict.insert("ppsw_lop", QApplication::translate("VariableNames", "LoP"));
        dict.insert("ppsw_diff", QApplication::translate("VariableNames", "Diff"));
        dict.insert("sftsw", QApplication::translate("VariableNames", "Safety switch"));
        dict.insert("rmds", QApplication::translate("VariableNames", "Remedies"));
        dict.insert("arno", QApplication::translate("VariableNames", "Assembly record No."));
        dict.insert("ar_type", QApplication::translate("VariableNames", "Assembly record type"));
        dict.insert("vis_aur_chk", QApplication::translate("VariableNames", "Visual and aural check"));
        dict.insert("corr_def", QApplication::translate("VariableNames", "Corr/Def"));
        dict.insert("noise_vibr", QApplication::translate("VariableNames", "Noise/Vibr"));
        dict.insert("bbl_lvl", QApplication::translate("VariableNames", "Bubble/Level"));
        dict.insert("oil_leak_am", QApplication::translate("VariableNames", "Oil leak"));
        dict.insert("dir_leak_chk", QApplication::translate("VariableNames", "Direct leak check (location)"));
        dict.insert("el_detect", QApplication::translate("VariableNames", "Electronic detection"));
        dict.insert("uv_detect", QApplication::translate("VariableNames", "UV detection"));
        dict.insert("bbl_detect", QApplication::translate("VariableNames", "Bubble detection"));
        dict.insert("refr_add_am", QApplication::translate("VariableNames", "Refrigerant addition"));
        dict.insert("refr_add_per", QApplication::translate("VariableNames", "Annual leakage"));
        dict.insert("refr_reco", QApplication::translate("VariableNames", "Refrigerant recovery"));
        dict.insert("inspector", QApplication::translate("VariableNames", "Inspector"));
        dict.insert("operator", QApplication::translate("VariableNames", "Operator"));
    }

    MTDictionary dict;
};

const MTDictionary & Global::variableNames()
{
    static VariableNames dict;
    return dict.dict;
}

class VariableType
{
public:
    VariableType() {
        dict.insert("t_sec_evap_in", "float");
        dict.insert("t_sec_cond_in", "float");
        dict.insert("p_0", "float");
        dict.insert("p_c", "float");
        dict.insert("t_0", "float");
        dict.insert("t_c", "float");
        dict.insert("t_ev", "float");
        dict.insert("t_evap_out", "float");
        dict.insert("t_comp_in", "float");
        dict.insert("t_sc", "float");
        dict.insert("t_sh_evap", "float");
        dict.insert("t_sh_comp", "float");
        dict.insert("t_comp_out", "float");
        dict.insert("delta_t_evap", "float");
        dict.insert("delta_t_c", "float");
        dict.insert("ep_comp", "float");
        dict.insert("ec_l1", "float");
        dict.insert("ec_l2", "float");
        dict.insert("ec_l3", "float");
        dict.insert("ev_l1", "float");
        dict.insert("ev_l2", "float");
        dict.insert("ev_l3", "float");
        dict.insert("ppsw_hip", "float");
        dict.insert("ppsw_lop", "float");
        dict.insert("ppsw_diff", "float");
        dict.insert("sftsw", "float");
        dict.insert("rmds", "text");
        dict.insert("arno", "string");
        dict.insert("ar_type", "integer");
        dict.insert("corr_def", "bool");
        dict.insert("noise_vibr", "bool");
        dict.insert("bbl_lvl", "bool");
        dict.insert("oil_leak_am", "float");
        dict.insert("el_detect", "bool");
        dict.insert("uv_detect", "bool");
        dict.insert("bbl_detect", "bool");
        dict.insert("refr_add_am", "float");
        dict.insert("refr_add_per", "float");
        dict.insert("refr_reco", "float");
        dict.insert("inspector", "string");
        dict.insert("operator", "string");
    }

    QHash<QString, QString> dict;
};

const QString Global::variableType(const QString & id, bool * ok)
{
    static VariableType types;
    if (ok) *ok = types.dict.contains(id);
    return types.dict.value(id, "string");
}

class FieldsOfApplication
{
public:
    FieldsOfApplication(): dict(true) {
        dict.insert(QApplication::translate("FieldsOfApplication", "Refrigeration"), "refrigeration");
        dict.insert(QApplication::translate("FieldsOfApplication", "Transportation"), "transportation");
        dict.insert(QApplication::translate("FieldsOfApplication", "Air conditioning"), "airconditioning");
        dict.insert(QApplication::translate("FieldsOfApplication", "Heat pumps"), "heatpumps");
        // OBSOLETE
        dict.insert(QApplication::translate("FieldsOfApplication", "Air conditioning"), "car");
        dict.insert(QApplication::translate("FieldsOfApplication", "Air conditioning"), "home");
        dict.insert(QApplication::translate("FieldsOfApplication", "Air conditioning"), "commercial");
        dict.insert(QApplication::translate("FieldsOfApplication", "Refrigeration"), "industrial");
        dict.insert(QApplication::translate("FieldsOfApplication", "Air conditioning"), "agricultural");
        dict.insert(QApplication::translate("FieldsOfApplication", "Refrigeration"), "other");
        dict.insert(QApplication::translate("FieldsOfApplication", "Air conditioning"), "lowrise");
        dict.insert(QApplication::translate("FieldsOfApplication", "Air conditioning"), "highrise");
        dict.insert(QApplication::translate("FieldsOfApplication", "Air conditioning"), "institutional");
    }

    MTDictionary dict;
};

const MTDictionary & Global::fieldsOfApplication()
{
    static FieldsOfApplication dict;
    return dict.dict;
}

class Oils
{
public:
    Oils() {
        dict.insert(QApplication::translate("Oils", "MO (Mineral oil)"), "mo");
        dict.insert(QApplication::translate("Oils", "AB (Alkylbenzene oil)"), "ab");
        dict.insert(QApplication::translate("Oils", "POE (Polyolester oil)"), "poe");
        dict.insert(QApplication::translate("Oils", "PAO (Polyalphaolefin oil)"), "pao");
        dict.insert(QApplication::translate("Oils", "PVE (Polyvinylether oil)"), "pve");
        dict.insert(QApplication::translate("Oils", "PAG (Polyglycol oil)"), "pag");
    }

    MTDictionary dict;
};

const MTDictionary & Global::oils()
{
    static Oils dict;
    return dict.dict;
}

class AttributeValues
{
public:
    AttributeValues() {
        dict.insert("field", QApplication::translate("FieldsOfApplication", "Field of application"));
        dict.insert("field::refrigeration", QApplication::translate("FieldsOfApplication", "Refrigeration"));
        dict.insert("field::transportation", QApplication::translate("FieldsOfApplication", "Transportation"));
        dict.insert("field::airconditioning", QApplication::translate("FieldsOfApplication", "Air conditioning"));
        dict.insert("field::heatpumps", QApplication::translate("FieldsOfApplication", "Heat pumps"));
        dict.insert("oil", QApplication::translate("Oils", "Oil"));
        dict.insert("oil::mo", QApplication::translate("Oils", "MO (Mineral oil)"));
        dict.insert("oil::ab", QApplication::translate("Oils", "AB (Alkylbenzene oil)"));
        dict.insert("oil::poe", QApplication::translate("Oils", "POE (Polyolester oil)"));
        dict.insert("oil::pao", QApplication::translate("Oils", "PAO (Polyalphaolefin oil)"));
        dict.insert("oil::pve", QApplication::translate("Oils", "PVE (Polyvinylether oil)"));
        dict.insert("oil::pag", QApplication::translate("Oils", "PAG (Polyglycol oil)"));
        dict.insert("refrigerant", QApplication::translate("Circuit", "Refrigerant"));
        QStringList list_refrigerants = Global::listRefrigerantsToString().split(";");
        for (int i = 0; i < list_refrigerants.count(); ++i) {
            dict.insert(QString("refrigerant::%1").arg(list_refrigerants.at(i)), list_refrigerants.at(i));
        }
        // OBSOLETE
        dict.insert("field::car", QApplication::translate("FieldsOfApplication", "Air conditioning"));
        dict.insert("field::home", QApplication::translate("FieldsOfApplication", "Air conditioning"));
        dict.insert("field::commercial", QApplication::translate("FieldsOfApplication", "Air conditioning"));
        dict.insert("field::industrial", QApplication::translate("FieldsOfApplication", "Refrigeration"));
        dict.insert("field::agricultural", QApplication::translate("FieldsOfApplication", "Air conditioning"));
        dict.insert("field::other", QApplication::translate("FieldsOfApplication", "Refrigeration"));
        dict.insert("field::lowrise", QApplication::translate("FieldsOfApplication", "Air conditioning"));
        dict.insert("field::highrise", QApplication::translate("FieldsOfApplication", "Air conditioning"));
        dict.insert("field::institutional", QApplication::translate("FieldsOfApplication", "Air conditioning"));
        // --------
    }

    MTDictionary dict;
};

const MTDictionary & Global::attributeValues()
{
    static AttributeValues dict;
    return dict.dict;
}

class Permissions
{
public:
    Permissions() {
        dict.insert("edit_service_company", QApplication::translate("Permissions", "Modify service company information"));
        dict.insert("add_refrigerant_management", QApplication::translate("Permissions", "Add record of refrigerant management"));
        dict.insert("edit_refrigerant_management", QApplication::translate("Permissions", "Modify record of refrigerant management"));
        dict.insert("add_customer", QApplication::translate("Permissions", "Add customer"));
        dict.insert("edit_customer", QApplication::translate("Permissions", "Modify customer"));
        dict.insert("remove_customer", QApplication::translate("Permissions", "Remove customer"));
        dict.insert("add_circuit", QApplication::translate("Permissions", "Add circuit"));
        dict.insert("edit_circuit", QApplication::translate("Permissions", "Modify circuit"));
        dict.insert("remove_circuit", QApplication::translate("Permissions", "Remove circuit"));
        dict.insert("add_inspection", QApplication::translate("Permissions", "Add inspection"));
        dict.insert("edit_inspection", QApplication::translate("Permissions", "Modify inspection"));
        dict.insert("remove_inspection", QApplication::translate("Permissions", "Remove inspection"));
        dict.insert("add_repair", QApplication::translate("Permissions", "Add repair"));
        dict.insert("edit_repair", QApplication::translate("Permissions", "Modify repair"));
        dict.insert("remove_repair", QApplication::translate("Permissions", "Remove repair"));
        dict.insert("add_inspector", QApplication::translate("Permissions", "Add inspector"));
        dict.insert("edit_inspector", QApplication::translate("Permissions", "Modify inspector"));
        dict.insert("remove_inspector", QApplication::translate("Permissions", "Remove inspector"));
        dict.insert("add_table", QApplication::translate("Permissions", "Add table"));
        dict.insert("edit_table", QApplication::translate("Permissions", "Modify table"));
        dict.insert("remove_table", QApplication::translate("Permissions", "Remove table"));
        dict.insert("add_variable", QApplication::translate("Permissions", "Add variable"));
        dict.insert("edit_variable", QApplication::translate("Permissions", "Modify variable"));
        dict.insert("remove_variable", QApplication::translate("Permissions", "Remove variable"));
        dict.insert("add_warning", QApplication::translate("Permissions", "Add warning"));
        dict.insert("edit_warning", QApplication::translate("Permissions", "Modify warning"));
        dict.insert("remove_warning", QApplication::translate("Permissions", "Remove warning"));
        dict.insert("import_data", QApplication::translate("Permissions", "Import data"));
        dict.insert("access_assembly_record_acquisition_price", QApplication::translate("Permissions", "Access assembly record acquisition prices"));
        dict.insert("access_assembly_record_list_price", QApplication::translate("Permissions", "Access assembly record list prices"));
    }

    MTDictionary dict;
};

const MTDictionary & Global::permissions()
{
    static Permissions dict;
    return dict.dict;
}

QString Global::listRefrigerantsToString()
{
    return "R11;R12;R22;R23;R32;R123;R124;R125;R134a;R143a;R227ea;R365mfc;R401A;R401B;R401C;R402A;R402B;R403A;R403B;R404A;R405A;R406;R407A;R407B;R407C;R407D;R407E;R408A;R409A;R409B;R410A;R410B;R414A;R414B;R416A;R417A;R420A;R421A;R421B;R422A;R422B;R422C;R422D;R423A;R424A;R425A;R426A;R427A;R428A;R500;R501;R502;R503;R507;R508A;R508B";
}

MTDictionary Global::listInspectors()
{
    MTDictionary inspectors(true); QSqlQuery query;
    query.setForwardOnly(true);
    if (query.exec("SELECT id, person FROM inspectors")) {
        while (query.next()) {
            inspectors.insert(query.value(1).toString().isEmpty() ? query.value(0).toString() : query.value(1).toString(), query.value(0).toString());
        }
    }
    return inspectors;
}

MTDictionary Global::listOperators(const QString & customer)
{
    MTDictionary operators(true); QSqlQuery query;
    query.setForwardOnly(true);
    if (query.exec(QString("SELECT id, name FROM persons WHERE company_id = %1").arg(customer))) {
        while (query.next()) {
            operators.insert(query.value(1).toString().isEmpty() ? query.value(0).toString() : query.value(1).toString(), query.value(0).toString());
        }
    }
    return operators;
}

MTDictionary Global::listAssemblyRecordItemCategories(bool hide_default)
{
    MTDictionary categories(QObject::tr("No category"), "-1"); QSqlQuery query;
    categories.allowDuplicateKeys();
    query.setForwardOnly(true);
    if (query.exec(QString("SELECT id, name FROM assembly_record_item_categories%1")
        .arg(hide_default ? " WHERE id < 1000" : ""))) {
        while (query.next()) {
            categories.insert(query.value(1).toString().isEmpty() ? query.value(0).toString() : query.value(1).toString(), query.value(0).toString());
        }
    }
    return categories;
}

MTDictionary Global::listAssemblyRecordTypes()
{
    MTDictionary dict(QObject::tr("No type"), "-1"); QSqlQuery query;
    dict.allowDuplicateKeys();
    query.setForwardOnly(true);
    if (query.exec("SELECT id, name FROM assembly_record_types")) {
        while (query.next()) {
            QString name = QString("%1 - %2").arg(query.value(0).toString()).arg(query.value(1).toString().left(50));
            dict.insert(query.value(1).toString().isEmpty() ? query.value(0).toString() : name, query.value(0).toString());
        }
    }
    return dict;
}

MTDictionary Global::listAllVariables()
{
    MTDictionary dict("", "");
    dict.allowDuplicateKeys();
    Variables query;
    bool is_subvar;
    while (query.next()) {
        is_subvar = !query.value("SUBVAR_ID").toString().isEmpty();

        QString name = is_subvar ? QString("%1: %2").arg(query.value("VAR_NAME").toString()).arg(query.value("SUBVAR_NAME").toString())
            : query.value("VAR_NAME").toString();
        QString id = is_subvar ? query.value("SUBVAR_ID").toString() : query.value("VAR_ID").toString();

        dict.insert(name, id);
    }
    return dict;
}

MTDictionary Global::listDataTypes()
{
    MTDictionary dict;
    dict.insert(QObject::tr("String"), QString::number(Global::String));
    dict.insert(QObject::tr("Integer"), QString::number(Global::Integer));
    dict.insert(QObject::tr("Real number"), QString::number(Global::Numeric));
    dict.insert(QObject::tr("Text"), QString::number(Global::Text));
    dict.insert(QObject::tr("Boolean"), QString::number(Global::Boolean));
    return dict;
}

MTDictionary Global::listStyles()
{
    MTDictionary styles(QObject::tr("No style"), "-1"); QSqlQuery query;
    styles.allowDuplicateKeys();
    query.setForwardOnly(true);
    if (query.exec(QString("SELECT id, name FROM styles"))) {
        while (query.next()) {
            styles.insert(query.value(1).toString().isEmpty() ? query.value(0).toString() : query.value(1).toString(), query.value(0).toString());
        }
    }
    return styles;
}

QStringList Global::listVariableIds(bool all)
{
    QStringList ids; bool sub_empty = false;
    ids << "customer" << "circuit" << "nominal" << "repair";
    if (all) ids << "date";
    Variables query;
    while (query.next()) {
        sub_empty = query.value("SUBVAR_ID").toString().isEmpty();
        if (all || sub_empty) { ids << query.value("VAR_ID").toString(); }
        if (!sub_empty) { ids << query.value("SUBVAR_ID").toString(); }
    }
    return ids;
}

QStringList Global::listSupportedFunctions()
{
    QStringList functions;
    functions << "abs" // (A)       Absolute value of A. If A is negative, returns -A otherwise returns A.
              << "acos" // (A)      Arc-cosine of A. Returns the angle, measured in radians, whose cosine is A.
              << "acosh" // (A)     Same as acos() but for hyperbolic cosine.
              << "asin" // (A)      Arc-sine of A. Returns the angle, measured in radians, whose sine is A.
              << "asinh" // (A)     Same as asin() but for hyperbolic sine.
              << "atan" // (A)      Arc-tangent of (A). Returns the angle, measured in radians, whose tangent is (A).
              << "atan2" // (A,B)   Arc-tangent of A/B. The two main differences to atan() is that it will return the right angle depending on the signs of A and B (atan() can only return values betwen -pi/2 and pi/2), and that the return value of pi/2 and -pi/2 are possible.
              << "atanh" // (A)     Same as atan() but for hyperbolic tangent.
              << "ceil" // (A)      Ceiling of A. Returns the smallest integer greater than A. Rounds up to the next higher integer.
              << "cos" // (A)       Cosine of A. Returns the cosine of the angle A, where A is measured in radians.
              << "cosh" // (A)      Same as cos() but for hyperbolic cosine.
              << "cot" // (A)       Cotangent of A (equivalent to 1/tan(A)).
              << "csc" // (A)       Cosecant of A (equivalent to 1/sin(A)).
              << "exp" // (A)       Exponential of A. Returns the value of e raised to the power A where e is the base of the natural logarithm, i.e. the non-repeating value approximately equal to 2.71828182846.
              << "floor" // (A)     Floor of A. Returns the largest integer less than A. Rounds down to the next lower integer.
              << "if" // (A,B,C)    If int(A) differs from 0, the return value of this function is B, else C. Only the parameter which needs to be evaluated is evaluated, the other parameter is skipped.
              << "int" // (A)       Rounds A to the closest integer. 0.5 is rounded to 1.
              << "log" // (A)       Natural (base e) logarithm of A.
              << "log10" // (A)     Base 10 logarithm of A.
              << "max" // (A,B)     If A>B, the result is A, else B.
              << "min" // (A,B)     If A<B, the result is A, else B.
              << "pow" // (A,B)     Exponentiation (A raised to the power B).
              << "sec" // (A)       Secant of A (equivalent to 1/cos(A)).
              << "sin" // (A)       Sine of A. Returns the sine of the angle A, where A is measured in radians.
              << "sinh" // (A)      Same as sin() but for hyperbolic sine.
              << "sqrt" // (A)      Square root of A. Returns the value whose square is A.
              << "tan" // (A)       Tangent of A. Returns the tangent of the angle A, where A is measured in radians.
              << "tanh"; // (A)     Same as tan() but for hyperbolic tangent.
    return functions;
}
