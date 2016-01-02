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

#include "warnings.h"
#include "global.h"
#include "variables.h"

#include <QSqlRecord>
#include <QApplication>

using namespace Global;

Warnings::Warnings(QSqlDatabase db, bool enabled_only, const QVariantMap &circuit_attributes, int scope):
    MTSqlQueryResultBase<QString>(db),
    enabled_only(enabled_only),
    m_scope(scope)
{
    database = db.isValid() ? db : QSqlDatabase::database();
    QStringList query_where;
    if (scope > 0) query_where << QString("scope = %1").arg(scope);
    if (enabled_only) query_where << "enabled = 1";
    if (exec(QString("SELECT uuid, enabled, name, description, delay FROM warnings%1").arg(query_where.count() ? (" WHERE " + query_where.join(" AND ")) : ""))
        && !circuit_attributes.isEmpty()) {
        QStringList used_ids = listVariableIds();
        for (int i = 0; i < result()->count(); ++i) {
            QString id = result()->at(i).value("uuid").toString();
            bool skip = false;
            WarningFilters warning_filters(id);
            while (warning_filters.next()) {
                QString circuit_attribute = circuit_attributes.value(warning_filters.value("circuit_attribute").toString()).toString();
                QString function = warning_filters.value("function").toString();
                QString value = warning_filters.value("value").toString();
                bool ok1 = true, ok2 = true;
                double f_circuit_attribute = circuit_attribute.toDouble(&ok1);
                double f_value = value.toDouble(&ok2);
                if (ok1 && ok2) {
                    if (function == "=" && f_circuit_attribute == f_value) {}
                    else if (function == "!=" && f_circuit_attribute != f_value) {}
                    else if (function == ">" && f_circuit_attribute > f_value) {}
                    else if (function == ">=" && f_circuit_attribute >= f_value) {}
                    else if (function == "<" && f_circuit_attribute < f_value) {}
                    else if (function == "<=" && f_circuit_attribute <= f_value) {}
                    else { skip = true; break; }
                } else {
                    if (function == "=" && circuit_attribute == value) {}
                    else if (function == "!=" && circuit_attribute != value) {}
                    else if (function == ">" && circuit_attribute > value) {}
                    else if (function == ">=" && circuit_attribute >= value) {}
                    else if (function == "<" && circuit_attribute < value) {}
                    else if (function == "<=" && circuit_attribute <= value) {}
                    else { skip = true; break; }
                }
            }
            if (skip) { result()->removeAt(i); i--; continue; }
            WarningConditions warning_conditions(id);
            while (warning_conditions.next()) {
                conditions_value_ins[id] << parseExpression(warning_conditions.value("value_ins").toString(), used_ids);
                conditions_value_nom[id] << parseExpression(warning_conditions.value("value_nom").toString(), used_ids);
                conditions_functions[id] << warning_conditions.value("function").toString();
            }
        }
    }
}

QString Warnings::tr(const char *s) { return QApplication::translate("Warnings", s); }

int Warnings::warningConditionValueInsCount(const QString &uuid) { return conditions_value_ins.value(uuid).count(); }

MTDictionary Warnings::warningConditionValueIns(const QString &uuid, int i) { return conditions_value_ins.value(uuid).at(i); }

int Warnings::warningConditionValueNomCount(const QString &uuid) { return conditions_value_nom.value(uuid).count(); }

MTDictionary Warnings::warningConditionValueNom(const QString &uuid, int i) { return conditions_value_nom.value(uuid).at(i); }

int Warnings::warningConditionFunctionCount(const QString &uuid) { return conditions_functions.value(uuid).count(); }

QString Warnings::warningConditionFunction(const QString &uuid, int i) { return conditions_functions.value(uuid).at(i); }

void Warnings::saveResult()
{
    int n = query()->record().count();
    *pos() = -1;
    result()->clear();
    initWarnings(database, result(), 0, QString(), enabled_only, m_scope);
    QVariantMap row;
    while (query()->next()) {
        if (query()->value(0).toInt() >= 1000) { continue; }
        row.clear();
        for (int i = 0; i < n; ++i) {
            row.insert(query()->record().fieldName(i), query()->value(i));
        }
        *result() << row;
    }
}

void Warnings::initWarnings(QSqlDatabase _database, ListOfVariantMaps *map, int type, const QString &id, bool enabled_only, int scope)
{
    QSqlDatabase database = _database.isValid() ? _database : QSqlDatabase::database(); QString w;
    if (scope == 0 || (scope & Variable::Inspection)) {
        w = "1000";
        if (id.isEmpty() || id == w) {
            if (type == 0) {
                initWarning(database, map, w, tr("Refrigerant leakage above limit"), tr("3 - 10 kg, before 2011"), 0, enabled_only);
            } else if (type == 1) {
                initFilter(map, w, "commissioning", "<", "2011.07.04");
                initFilter(map, w, "refrigerant_amount", ">=", "3");
                initFilter(map, w, "refrigerant_amount", "<", "10");
            } else if (type == 2) {
                initCondition(map, w, "nominal", "=", "0");
                initCondition(map, w, "100*(refr_add_am-refr_reco)/refrigerant_amount", ">", "8");
            }
        }
        w = "1001";
        if (id.isEmpty() || id == w) {
            if (type == 0) {
                initWarning(database, map, w, tr("Refrigerant leakage above limit"), tr("3 - 10 kg, after 2011"), 0, enabled_only);
            } else if (type == 1) {
                initFilter(map, w, "commissioning", ">=", "2011.07.04");
                initFilter(map, w, "refrigerant_amount", ">=", "3");
                initFilter(map, w, "refrigerant_amount", "<", "10");
            } else if (type == 2) {
                initCondition(map, w, "nominal", "=", "0");
                initCondition(map, w, "100*(refr_add_am-refr_reco)/refrigerant_amount", ">", "6");
            }
        }
        w = "1002";
        if (id.isEmpty() || id == w) {
            if (type == 0) {
                initWarning(database, map, w, tr("Refrigerant leakage above limit"), tr("10 - 100 kg, before 2011"), 0, enabled_only);
            } else if (type == 1) {
                initFilter(map, w, "commissioning", "<", "2011.07.04");
                initFilter(map, w, "refrigerant_amount", ">=", "10");
                initFilter(map, w, "refrigerant_amount", "<", "100");
            } else if (type == 2) {
                initCondition(map, w, "nominal", "=", "0");
                initCondition(map, w, "100*(refr_add_am-refr_reco)/refrigerant_amount", ">", "6");
            }
        }
        w = "1003";
        if (id.isEmpty() || id == w) {
            if (type == 0) {
                initWarning(database, map, w, tr("Refrigerant leakage above limit"), tr("10 - 100 kg, after 2011"), 0, enabled_only);
            } else if (type == 1) {
                initFilter(map, w, "commissioning", ">=", "2011.07.04");
                initFilter(map, w, "refrigerant_amount", ">=", "10");
                initFilter(map, w, "refrigerant_amount", "<", "100");
            } else if (type == 2) {
                initCondition(map, w, "nominal", "=", "0");
                initCondition(map, w, "100*(refr_add_am-refr_reco)/refrigerant_amount", ">", "4");
            }
        }
        w = "1004";
        if (id.isEmpty() || id == w) {
            if (type == 0) {
                initWarning(database, map, w, tr("Refrigerant leakage above limit"), tr("above 100 kg, before 2011"), 0, enabled_only);
            } else if (type == 1) {
                initFilter(map, w, "commissioning", "<", "2011.07.04");
                initFilter(map, w, "refrigerant_amount", ">=", "100");
            } else if (type == 2) {
                initCondition(map, w, "nominal", "=", "0");
                initCondition(map, w, "100*(refr_add_am-refr_reco)/refrigerant_amount", ">", "4");
            }
        }
        w = "1005";
        if (id.isEmpty() || id == w) {
            if (type == 0) {
                initWarning(database, map, w, tr("Refrigerant leakage above limit"), tr("above 100 kg, after 2011"), 0, enabled_only);
            } else if (type == 1) {
                initFilter(map, w, "commissioning", ">=", "2011.07.04");
                initFilter(map, w, "refrigerant_amount", ">=", "100");
            } else if (type == 2) {
                initCondition(map, w, "nominal", "=", "0");
                initCondition(map, w, "100*(refr_add_am-refr_reco)/refrigerant_amount", ">", "2");
            }
        }
        w = "1100";
        if (id.isEmpty() || id == w) {
            if (type == 0) {
                initWarning(database, map, w, tr("Refrigerant leakage"), "", 0, enabled_only);
            } else if (type == 2) {
                initCondition(map, w, "p_to_t_vap(p_0)", "<", "p_to_t_vap(p_0)");
                initCondition(map, w, "t_evap_out-p_to_t_vap(p_0)", ">", "t_evap_out-p_to_t_vap(p_0)");
                initCondition(map, w, "p_to_t(p_c)", "<", "p_to_t(p_c)");
                initCondition(map, w, "p_to_t(p_c)-t_ev", "<", "p_to_t(p_c)-t_ev");
                initCondition(map, w, "t_comp_out", ">", "t_comp_out");
                initCondition(map, w, "abs(t_sec_cond_in-p_to_t(p_c))", "<", "abs(t_sec_cond_in-p_to_t(p_c))");
                initCondition(map, w, "abs(t_sec_evap_in-p_to_t_vap(p_0))", "<", "abs(t_sec_evap_in-p_to_t_vap(p_0))");
            }
        }
        w = "1101";
        if (id.isEmpty() || id == w) {
            if (type == 0) {
                initWarning(database, map, w, tr("Compressor valve leakage"), "", 0, enabled_only);
            } else if (type == 2) {
                initCondition(map, w, "p_to_t_vap(p_0)", ">", "p_to_t_vap(p_0)");
                initCondition(map, w, "t_evap_out-p_to_t_vap(p_0)", "<", "t_evap_out-p_to_t_vap(p_0)");
                initCondition(map, w, "p_to_t(p_c)", "<", "p_to_t(p_c)");
                initCondition(map, w, "p_to_t(p_c)-t_ev", "<", "p_to_t(p_c)-t_ev");
                initCondition(map, w, "t_comp_out", ">", "t_comp_out");
                initCondition(map, w, "abs(t_sec_cond_in-p_to_t(p_c))", "<", "abs(t_sec_cond_in-p_to_t(p_c))");
                initCondition(map, w, "abs(t_sec_evap_in-p_to_t_vap(p_0))", "<", "abs(t_sec_evap_in-p_to_t_vap(p_0))");
            }
        }
        w = "1102";
        if (id.isEmpty() || id == w) {
            if (type == 0) {
                initWarning(database, map, w, tr("Liquid-line restriction"), "", 0, enabled_only);
            } else if (type == 2) {
                initCondition(map, w, "p_to_t_vap(p_0)", "<", "p_to_t_vap(p_0)");
                initCondition(map, w, "t_evap_out-p_to_t_vap(p_0)", ">", "t_evap_out-p_to_t_vap(p_0)");
                initCondition(map, w, "p_to_t(p_c)", "<", "p_to_t(p_c)");
                initCondition(map, w, "p_to_t(p_c)-t_ev", ">", "p_to_t(p_c)-t_ev");
                initCondition(map, w, "t_comp_out", ">", "t_comp_out");
                initCondition(map, w, "abs(t_sec_cond_in-p_to_t(p_c))", "<", "abs(t_sec_cond_in-p_to_t(p_c))");
                initCondition(map, w, "abs(t_sec_evap_in-p_to_t_vap(p_0))", "<", "abs(t_sec_evap_in-p_to_t_vap(p_0))");
            }
        }
        w = "1103";
        if (id.isEmpty() || id == w) {
            if (type == 0) {
                initWarning(database, map, w, tr("Condenser fouling"), "", 0, enabled_only);
            } else if (type == 2) {
                initCondition(map, w, "p_to_t_vap(p_0)", ">", "p_to_t_vap(p_0)");
                initCondition(map, w, "t_evap_out-p_to_t_vap(p_0)", "<", "t_evap_out-p_to_t_vap(p_0)");
                initCondition(map, w, "p_to_t(p_c)", ">", "p_to_t(p_c)");
                initCondition(map, w, "p_to_t(p_c)-t_ev", "<", "p_to_t(p_c)-t_ev");
                initCondition(map, w, "t_comp_out", ">", "t_comp_out");
                initCondition(map, w, "abs(t_sec_cond_in-p_to_t(p_c))", ">", "abs(t_sec_cond_in-p_to_t(p_c))");
                initCondition(map, w, "abs(t_sec_evap_in-p_to_t_vap(p_0))", "<", "abs(t_sec_evap_in-p_to_t_vap(p_0))");
            }
        }
        w = "1104";
        if (id.isEmpty() || id == w) {
            if (type == 0) {
                initWarning(database, map, w, tr("Evaporator fouling"), "", 0, enabled_only);
            } else if (type == 2) {
                initCondition(map, w, "p_to_t_vap(p_0)", "<", "p_to_t_vap(p_0)");
                initCondition(map, w, "t_evap_out-p_to_t_vap(p_0)", "<", "t_evap_out-p_to_t_vap(p_0)");
                initCondition(map, w, "p_to_t(p_c)", "<", "p_to_t(p_c)");
                initCondition(map, w, "p_to_t(p_c)-t_ev", "<", "p_to_t(p_c)-t_ev");
                initCondition(map, w, "t_comp_out", "<", "t_comp_out");
                initCondition(map, w, "abs(t_sec_cond_in-p_to_t(p_c))", "<", "abs(t_sec_cond_in-p_to_t(p_c))");
                initCondition(map, w, "abs(t_sec_evap_in-p_to_t_vap(p_0))", ">", "abs(t_sec_evap_in-p_to_t_vap(p_0))");
            }
        }
        w = "1200";
        if (id.isEmpty() || id == w) {
            if (type == 0) {
                initWarning(database, map, w, tr("Needs inspection"), tr("3 - 30 kg"), 365, enabled_only);
            } else if (type == 1) {
                initFilter(map, w, "hermetic", "=", "0");
                initFilter(map, w, "refrigerant_amount", ">=", "3");
                initFilter(map, w, "refrigerant_amount", "<", "30");
            }
        }
        w = "1202";
        if (id.isEmpty() || id == w) {
            if (type == 0) {
                initWarning(database, map, w, tr("Needs inspection"), tr("6 - 30 kg, hermetically sealed"), 365, enabled_only);
            } else if (type == 1) {
                initFilter(map, w, "hermetic", "=", "1");
                initFilter(map, w, "refrigerant_amount", ">=", "6");
                initFilter(map, w, "refrigerant_amount", "<", "30");
            }
        }
        w = "1204";
        if (id.isEmpty() || id == w) {
            if (type == 0) {
                initWarning(database, map, w, tr("Needs inspection"), tr("30 - 300 kg"), 182, enabled_only);
            } else if (type == 1) {
                initFilter(map, w, "leak_detector", "=", "0");
                initFilter(map, w, "refrigerant_amount", ">=", "30");
                initFilter(map, w, "refrigerant_amount", "<", "300");
            }
        }
        w = "1205";
        if (id.isEmpty() || id == w) {
            if (type == 0) {
                initWarning(database, map, w, tr("Needs inspection"), tr("30 - 300 kg, leakage detector installed"), 365, enabled_only);
            } else if (type == 1) {
                initFilter(map, w, "leak_detector", "=", "1");
                initFilter(map, w, "refrigerant_amount", ">=", "30");
                initFilter(map, w, "refrigerant_amount", "<", "300");
            }
        }
        w = "1206";
        if (id.isEmpty() || id == w) {
            if (type == 0) {
                initWarning(database, map, w, tr("Needs inspection"), tr("above 300 kg"), 91, enabled_only);
            } else if (type == 1) {
                initFilter(map, w, "leak_detector", "=", "0");
                initFilter(map, w, "refrigerant_amount", ">=", "300");
            }
        }
        w = "1207";
        if (id.isEmpty() || id == w) {
            if (type == 0) {
                initWarning(database, map, w, tr("Needs inspection"), tr("above 300 kg, leakage detector installed"), 182, enabled_only);
            } else if (type == 1) {
                initFilter(map, w, "leak_detector", "=", "1");
                initFilter(map, w, "refrigerant_amount", ">=", "300");
            }
        }
    }
}

int Warnings::circuitInspectionInterval(const QString &refrigerant, double refrigerant_amount, bool CO2_equivalent, bool hermetic, bool leak_detector, int interval)
{
    int result = 0;
    if (CO2_equivalent) {
        refrigerant_amount *= refrigerantGWP(refrigerant);
        if (refrigerant_amount >= 500000.0) {
            result = leak_detector ? 182 : 91;
        } else if (refrigerant_amount >= 50000.0) {
            result = leak_detector ? 365 : 182;
        } else if (refrigerant_amount >= (hermetic ? 10000.0 : 5000.0)) {
            result = leak_detector ? 730 : 365;
        }
    } else {
        if (refrigerant_amount >= 300.0) {
            result = leak_detector ? 182 : 91;
        } else if (refrigerant_amount >= 30.0) {
            result = leak_detector ? 365 : 182;
        } else if (refrigerant_amount >= (hermetic ? 6.0 : 3.0)) {
            result = 365;
        }
    }
    if (result == 0 || (interval && interval < result))
        return interval;
    return result;
}

void Warnings::initWarning(QSqlDatabase database, ListOfVariantMaps *map, const QString &uuid, const QString &name, const QString &description, int delay, bool enabled_only)
{
    MTSqlQuery query(database);
    query.prepare("SELECT enabled FROM warnings WHERE uuid = :uuid");
    query.bindValue(":uuid", uuid);
    query.exec();
    QVariantMap set;
    if (query.next()) {
        if (enabled_only && !query.value(0).toInt()) { return; }
        set.insert("enabled", query.value(0).toInt());
    } else {
        set.insert("enabled", 1);
    }
    set.insert("uuid", uuid);
    set.insert("name", name);
    set.insert("description", description);
    set.insert("delay", delay);
    *map << set;
}

void Warnings::initFilter(ListOfVariantMaps *map, const QString &warning_uuid, const QString &circuit_attribute, const QString &function, const QString &value)
{
    QVariantMap set;
    set.insert("warning_uuid", warning_uuid);
    set.insert("circuit_attribute", circuit_attribute);
    set.insert("function", function);
    set.insert("value", value);
    *map << set;
}

void Warnings::initCondition(ListOfVariantMaps *map, const QString &warning_uuid, const QString &value_ins, const QString &function, const QString &value_nom)
{
    QVariantMap set;
    set.insert("warning_uuid", warning_uuid);
    set.insert("value_ins", value_ins);
    set.insert("function", function);
    set.insert("value_nom", value_nom);
    *map << set;
}

Warning::Warning(const QString &uuid, QSqlDatabase db):
    MTSqlQueryResultBase<QString>(db),
    uuid(uuid)
{
    database = db.isValid() ? db : QSqlDatabase::database();
    prepare("SELECT * FROM warnings WHERE uuid = :uuid");
    bindValue(":uuid", uuid);
    exec();
}

void Warning::saveResult()
{
    int n = query()->record().count();
    *pos() = -1;
    result()->clear();
    Warnings::initWarnings(database, result(), 0, uuid);
    if (uuid.toInt() >= 1000) { return; }
    QVariantMap row;
    while (query()->next()) {
        row.clear();
        for (int i = 0; i < n; ++i) {
            row.insert(query()->record().fieldName(i), query()->value(i));
        }
        *result() << row;
    }
}

WarningFilters::WarningFilters(const QString &warning_uuid, QSqlDatabase db):
    MTSqlQueryResultBase<QString>(db),
    warning_uuid(warning_uuid)
{
    database = db.isValid() ? db : QSqlDatabase::database();
    prepare("SELECT * FROM warnings_filters WHERE warning_uuid = :warning_uuid");
    bindValue(":warning_uuid", warning_uuid);
    exec();
}

void WarningFilters::saveResult()
{
    int n = query()->record().count();
    *pos() = -1;
    result()->clear();
    Warnings::initWarnings(database, result(), 1, warning_uuid);
    if (warning_uuid.toInt() >= 1000) { return; }
    QVariantMap row;
    while (query()->next()) {
        row.clear();
        for (int i = 0; i < n; ++i) {
            row.insert(query()->record().fieldName(i), query()->value(i));
        }
        *result() << row;
    }
}

WarningConditions::WarningConditions(const QString &warning_uuid, QSqlDatabase db):
    MTSqlQueryResultBase<QString>(db),
    warning_uuid(warning_uuid)
{
    database = db.isValid() ? db : QSqlDatabase::database();
    prepare("SELECT * FROM warnings_conditions WHERE warning_uuid = :warning_uuid");
    bindValue(":warning_uuid", warning_uuid);
    exec();
}

void WarningConditions::saveResult()
{
    int n = query()->record().count();
    *pos() = -1;
    result()->clear();
    Warnings::initWarnings(database, result(), 2, warning_uuid);
    if (warning_uuid.toInt() >= 1000) { return; }
    QVariantMap row;
    while (query()->next()) {
        row.clear();
        for (int i = 0; i < n; ++i) {
            row.insert(query()->record().fieldName(i), query()->value(i));
        }
        *result() << row;
    }
}
