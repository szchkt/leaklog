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

#include "warnings.h"
#include "global.h"
#include "variables.h"

#include <QSqlRecord>
#include <QApplication>

using namespace Global;

Warnings::Warnings(bool CO2_equivalent, bool enabled_only, QVariantMap circuit_attributes, int scope):
    MTSqlQueryResultBase<QString>(),
    CO2_equivalent(CO2_equivalent),
    enabled_only(enabled_only),
    m_scope(scope)
{
    QStringList query_where;
    if (scope > 0) query_where << QString("scope = %1").arg(scope);
    if (enabled_only) query_where << "enabled = 1";
    if (exec(QString("SELECT id, enabled, name, description, delay FROM warnings%1").arg(query_where.count() ? (" WHERE " + query_where.join(" AND ")) : ""))
        && !circuit_attributes.isEmpty()) {
        double GWP = refrigerantGWP(circuit_attributes.value("refrigerant").toString());
        circuit_attributes.insert("gwp", GWP);
        circuit_attributes.insert("co2_equivalent", GWP * circuit_attributes.value("refrigerant_amount").toDouble() / 1000.0);

        QString circuit_attribute, function, value;
        bool ok1 = true, ok2 = true; double f_circuit_attribute = 0.0, f_value = 0.0;
        bool skip = false; int id; QStringList used_ids = listVariableIds();
        for (int i = 0; i < result()->count(); ++i) {
            id = result()->at(i).value("id").toInt();
            skip = false;
            WarningFilters warning_filters(id, CO2_equivalent);
            while (warning_filters.next()) {
                circuit_attribute = circuit_attributes.value(warning_filters.value("circuit_attribute").toString()).toString();
                function = warning_filters.value("function").toString();
                value = warning_filters.value("value").toString();
                f_circuit_attribute = circuit_attribute.toDouble(&ok1);
                f_value = value.toDouble(&ok2);
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
            WarningConditions warning_conditions(id, CO2_equivalent);
            while (warning_conditions.next()) {
                conditions_value_ins[id] << Expression(warning_conditions.value("value_ins").toString());
                conditions_value_nom[id] << Expression(warning_conditions.value("value_nom").toString());
                conditions_functions[id] << warning_conditions.value("function").toString();
            }
        }
    }
}

QString Warnings::tr(const char *s) { return QApplication::translate("Warnings", s); }

int Warnings::warningConditionValueInsCount(int id) { return conditions_value_ins.value(id).count(); }

Expression Warnings::warningConditionValueIns(int id, int i) { return conditions_value_ins.value(id).at(i); }

int Warnings::warningConditionValueNomCount(int id) { return conditions_value_nom.value(id).count(); }

Expression Warnings::warningConditionValueNom(int id, int i) { return conditions_value_nom.value(id).at(i); }

int Warnings::warningConditionFunctionCount(int id) { return conditions_functions.value(id).count(); }

QString Warnings::warningConditionFunction(int id, int i) { return conditions_functions.value(id).at(i); }

void Warnings::saveResult()
{
    int n = query()->record().count();
    *pos() = -1;
    result()->clear();
    initWarnings(result(), 0, -1, CO2_equivalent, enabled_only, m_scope);
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

void Warnings::initWarnings(ListOfVariantMaps *map, int type, int id, bool CO2_equivalent, bool enabled_only, int scope)
{
    QString w;
    if (scope == 0 || (scope & Variable::Inspection)) {
        w = "1000";
        if (id < 0 || id == w.toInt()) {
            if (type == 0) {
                initWarning(map, w, tr("Refrigerant leakage above limit"),
                            CO2_equivalent ? replaceUnsupportedCharacters(tr("5 - 50 t of CO\342\202\202 equivalent, before 2011"))
                                           : tr("3 - 10 kg, before 2011"),
                            0, enabled_only);
            } else if (type == 1) {
                initFilter(map, w, "commissioning", "<", "2011.07.04");
                if (CO2_equivalent) {
                    initFilter(map, w, "co2_equivalent", ">=", "5");
                    initFilter(map, w, "co2_equivalent", "<", "50");
                } else {
                    initFilter(map, w, "refrigerant_amount", ">=", "3");
                    initFilter(map, w, "refrigerant_amount", "<", "10");
                }
            } else if (type == 2) {
                initCondition(map, w, "nominal", "=", "0");
                initCondition(map, w, "100*(refr_add_am-refr_reco)/refrigerant_amount", ">", "8");
            }
        }
        w = "1001";
        if (id < 0 || id == w.toInt()) {
            if (type == 0) {
                initWarning(map, w, tr("Refrigerant leakage above limit"),
                            CO2_equivalent ? replaceUnsupportedCharacters(tr("5 - 50 t of CO\342\202\202 equivalent, after 2011"))
                                           : tr("3 - 10 kg, after 2011"),
                            0, enabled_only);
            } else if (type == 1) {
                initFilter(map, w, "commissioning", ">=", "2011.07.04");
                if (CO2_equivalent) {
                    initFilter(map, w, "co2_equivalent", ">=", "5");
                    initFilter(map, w, "co2_equivalent", "<", "50");
                } else {
                    initFilter(map, w, "refrigerant_amount", ">=", "3");
                    initFilter(map, w, "refrigerant_amount", "<", "10");
                }
            } else if (type == 2) {
                initCondition(map, w, "nominal", "=", "0");
                initCondition(map, w, "100*(refr_add_am-refr_reco)/refrigerant_amount", ">", "6");
            }
        }
        w = "1002";
        if (id < 0 || id == w.toInt()) {
            if (type == 0) {
                initWarning(map, w, tr("Refrigerant leakage above limit"),
                            CO2_equivalent ? replaceUnsupportedCharacters(tr("50 - 500 t of CO\342\202\202 equivalent, before 2011"))
                                           : tr("10 - 100 kg, before 2011"),
                            0, enabled_only);
            } else if (type == 1) {
                initFilter(map, w, "commissioning", "<", "2011.07.04");
                if (CO2_equivalent) {
                    initFilter(map, w, "co2_equivalent", ">=", "50");
                    initFilter(map, w, "co2_equivalent", "<", "500");
                } else {
                    initFilter(map, w, "refrigerant_amount", ">=", "10");
                    initFilter(map, w, "refrigerant_amount", "<", "100");
                }
            } else if (type == 2) {
                initCondition(map, w, "nominal", "=", "0");
                initCondition(map, w, "100*(refr_add_am-refr_reco)/refrigerant_amount", ">", "6");
            }
        }
        w = "1003";
        if (id < 0 || id == w.toInt()) {
            if (type == 0) {
                initWarning(map, w, tr("Refrigerant leakage above limit"),
                            CO2_equivalent ? replaceUnsupportedCharacters(tr("50 - 500 t of CO\342\202\202 equivalent, after 2011"))
                                           : tr("10 - 100 kg, after 2011"),
                            0, enabled_only);
            } else if (type == 1) {
                initFilter(map, w, "commissioning", ">=", "2011.07.04");
                if (CO2_equivalent) {
                    initFilter(map, w, "co2_equivalent", ">=", "50");
                    initFilter(map, w, "co2_equivalent", "<", "500");
                } else {
                    initFilter(map, w, "refrigerant_amount", ">=", "10");
                    initFilter(map, w, "refrigerant_amount", "<", "100");
                }
            } else if (type == 2) {
                initCondition(map, w, "nominal", "=", "0");
                initCondition(map, w, "100*(refr_add_am-refr_reco)/refrigerant_amount", ">", "4");
            }
        }
        w = "1004";
        if (id < 0 || id == w.toInt()) {
            if (type == 0) {
                initWarning(map, w, tr("Refrigerant leakage above limit"),
                            CO2_equivalent ? replaceUnsupportedCharacters(tr("above 500 t of CO\342\202\202 equivalent, before 2011"))
                                           : tr("above 100 kg, before 2011"),
                            0, enabled_only);
            } else if (type == 1) {
                initFilter(map, w, "commissioning", "<", "2011.07.04");
                if (CO2_equivalent) {
                    initFilter(map, w, "co2_equivalent", ">=", "500");
                } else {
                    initFilter(map, w, "refrigerant_amount", ">=", "100");
                }
            } else if (type == 2) {
                initCondition(map, w, "nominal", "=", "0");
                initCondition(map, w, "100*(refr_add_am-refr_reco)/refrigerant_amount", ">", "4");
            }
        }
        w = "1005";
        if (id < 0 || id == w.toInt()) {
            if (type == 0) {
                initWarning(map, w, tr("Refrigerant leakage above limit"),
                            CO2_equivalent ? replaceUnsupportedCharacters(tr("above 500 t of CO\342\202\202 equivalent, after 2011"))
                                           : tr("above 100 kg, after 2011"),
                            0, enabled_only);
            } else if (type == 1) {
                initFilter(map, w, "commissioning", ">=", "2011.07.04");
                if (CO2_equivalent) {
                    initFilter(map, w, "co2_equivalent", ">=", "500");
                } else {
                    initFilter(map, w, "refrigerant_amount", ">=", "100");
                }
            } else if (type == 2) {
                initCondition(map, w, "nominal", "=", "0");
                initCondition(map, w, "100*(refr_add_am-refr_reco)/refrigerant_amount", ">", "2");
            }
        }
        w = "1100";
        if (id < 0 || id == w.toInt()) {
            if (type == 0) {
                initWarning(map, w, tr("Refrigerant leakage"), "", 0, enabled_only);
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
        if (id < 0 || id == w.toInt()) {
            if (type == 0) {
                initWarning(map, w, tr("Compressor valve leakage"), "", 0, enabled_only);
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
        if (id < 0 || id == w.toInt()) {
            if (type == 0) {
                initWarning(map, w, tr("Liquid-line restriction"), "", 0, enabled_only);
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
        if (id < 0 || id == w.toInt()) {
            if (type == 0) {
                initWarning(map, w, tr("Condenser fouling"), "", 0, enabled_only);
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
        if (id < 0 || id == w.toInt()) {
            if (type == 0) {
                initWarning(map, w, tr("Evaporator fouling"), "", 0, enabled_only);
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

        if (CO2_equivalent) {
            w = "1210";
            if (id < 0 || id == w.toInt()) {
                if (type == 0) {
                    initWarning(map, w, tr("Needs inspection"), tr("once in 2 years"), 730, enabled_only);
                } else if (type == 1) {
                    initFilter(map, w, "leak_detector", "=", "1");
                } else if (type == 2) {
                    initCondition(map, w, "co2_equivalent", ">=", "if(hermetic,10,5)");
                    initCondition(map, w, "co2_equivalent", "<", "50");
                }
            }
            w = "1211";
            if (id < 0 || id == w.toInt()) {
                if (type == 0) {
                    initWarning(map, w, tr("Needs inspection"), tr("once a year"), 365, enabled_only);
                } else if (type == 2) {
                    initCondition(map, w, "co2_equivalent", ">=", "if(leak_detector,50,if(hermetic,10,5))");
                    initCondition(map, w, "co2_equivalent", "<", "if(leak_detector,500,50)");
                }
            }
            w = "1212";
            if (id < 0 || id == w.toInt()) {
                if (type == 0) {
                    initWarning(map, w, tr("Needs inspection"), tr("once in 6 months"), 182, enabled_only);
                } else if (type == 2) {
                    initCondition(map, w, "co2_equivalent", ">=", "if(leak_detector,500,50)");
                    initCondition(map, w, "if(leak_detector,0,co2_equivalent)", "<", "if(leak_detector,1,500)");
                }
            }
            w = "1213";
            if (id < 0 || id == w.toInt()) {
                if (type == 0) {
                    initWarning(map, w, tr("Needs inspection"), tr("once in 3 months"), 91, enabled_only);
                } else if (type == 1) {
                    initFilter(map, w, "leak_detector", "=", "0");
                } else if (type == 2) {
                    initCondition(map, w, "co2_equivalent", ">=", "500");
                }
            }
            return;
        }

        // Pre-CO2 equivalent

        w = "1200";
        if (id < 0 || id == w.toInt()) {
            if (type == 0) {
                initWarning(map, w, tr("Needs inspection"), tr("3 - 30 kg"), 365, enabled_only);
            } else if (type == 1) {
                initFilter(map, w, "hermetic", "=", "0");
                initFilter(map, w, "refrigerant_amount", ">=", "3");
                initFilter(map, w, "refrigerant_amount", "<", "30");
            }
        }
        w = "1202";
        if (id < 0 || id == w.toInt()) {
            if (type == 0) {
                initWarning(map, w, tr("Needs inspection"), tr("6 - 30 kg, hermetically sealed"), 365, enabled_only);
            } else if (type == 1) {
                initFilter(map, w, "hermetic", "=", "1");
                initFilter(map, w, "refrigerant_amount", ">=", "6");
                initFilter(map, w, "refrigerant_amount", "<", "30");
            }
        }
        w = "1204";
        if (id < 0 || id == w.toInt()) {
            if (type == 0) {
                initWarning(map, w, tr("Needs inspection"), tr("30 - 300 kg"), 182, enabled_only);
            } else if (type == 1) {
                initFilter(map, w, "leak_detector", "=", "0");
                initFilter(map, w, "refrigerant_amount", ">=", "30");
                initFilter(map, w, "refrigerant_amount", "<", "300");
            }
        }
        w = "1205";
        if (id < 0 || id == w.toInt()) {
            if (type == 0) {
                initWarning(map, w, tr("Needs inspection"), tr("30 - 300 kg, leakage detector installed"), 365, enabled_only);
            } else if (type == 1) {
                initFilter(map, w, "leak_detector", "=", "1");
                initFilter(map, w, "refrigerant_amount", ">=", "30");
                initFilter(map, w, "refrigerant_amount", "<", "300");
            }
        }
        w = "1206";
        if (id < 0 || id == w.toInt()) {
            if (type == 0) {
                initWarning(map, w, tr("Needs inspection"), tr("above 300 kg"), 91, enabled_only);
            } else if (type == 1) {
                initFilter(map, w, "leak_detector", "=", "0");
                initFilter(map, w, "refrigerant_amount", ">=", "300");
            }
        }
        w = "1207";
        if (id < 0 || id == w.toInt()) {
            if (type == 0) {
                initWarning(map, w, tr("Needs inspection"), tr("above 300 kg, leakage detector installed"), 182, enabled_only);
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

void Warnings::initWarning(ListOfVariantMaps *map, const QString &id, const QString &name, const QString &description, int delay, bool enabled_only)
{
    MTSqlQuery query;
    query.prepare("SELECT enabled FROM warnings WHERE id = :id");
    query.bindValue(":id", id);
    query.exec();
    QVariantMap set;
    if (query.next()) {
        if (enabled_only && !query.value(0).toInt()) { return; }
        set.insert("enabled", query.value(0).toInt());
    } else {
        set.insert("enabled", 1);
    }
    set.insert("id", id);
    set.insert("name", name);
    set.insert("description", description);
    set.insert("delay", delay);
    *map << set;
}

void Warnings::initFilter(ListOfVariantMaps *map, const QString &parent, const QString &circuit_attribute, const QString &function, const QString &value)
{
    QVariantMap set;
    set.insert("parent", parent);
    set.insert("circuit_attribute", circuit_attribute);
    set.insert("function", function);
    set.insert("value", value);
    *map << set;
}

void Warnings::initCondition(ListOfVariantMaps *map, const QString &parent, const QString &value_ins, const QString &function, const QString &value_nom)
{
    QVariantMap set;
    set.insert("parent", parent);
    set.insert("value_ins", value_ins);
    set.insert("function", function);
    set.insert("value_nom", value_nom);
    *map << set;
}

Warning::Warning(int id, bool CO2_equivalent):
    MTSqlQueryResultBase<QString>(),
    id(id),
    CO2_equivalent(CO2_equivalent)
{
    prepare("SELECT id, enabled, name, description, delay, scope FROM warnings WHERE id = :id");
    bindValue(":id", id);
    exec();
}

void Warning::saveResult()
{
    int n = query()->record().count();
    *pos() = -1;
    result()->clear();
    Warnings::initWarnings(result(), 0, id, CO2_equivalent);
    if (id >= 1000) { return; }
    QVariantMap row;
    while (query()->next()) {
        row.clear();
        for (int i = 0; i < n; ++i) {
            row.insert(query()->record().fieldName(i), query()->value(i));
        }
        *result() << row;
    }
}

WarningFilters::WarningFilters(int id, bool CO2_equivalent):
    MTSqlQueryResultBase<QString>(),
    id(id),
    CO2_equivalent(CO2_equivalent)
{
    prepare("SELECT parent, circuit_attribute, function, value FROM warnings_filters WHERE parent = :parent");
    bindValue(":parent", id);
    exec();
}

void WarningFilters::saveResult()
{
    int n = query()->record().count();
    *pos() = -1;
    result()->clear();
    Warnings::initWarnings(result(), 1, id, CO2_equivalent);
    if (id >= 1000) { return; }
    QVariantMap row;
    while (query()->next()) {
        row.clear();
        for (int i = 0; i < n; ++i) {
            row.insert(query()->record().fieldName(i), query()->value(i));
        }
        *result() << row;
    }
}

WarningConditions::WarningConditions(int id, bool CO2_equivalent):
    MTSqlQueryResultBase<QString>(),
    id(id),
    CO2_equivalent(CO2_equivalent)
{
    prepare("SELECT parent, value_ins, function, value_nom FROM warnings_conditions WHERE parent = :parent");
    bindValue(":parent", id);
    exec();
}

void WarningConditions::saveResult()
{
    int n = query()->record().count();
    *pos() = -1;
    result()->clear();
    Warnings::initWarnings(result(), 2, id, CO2_equivalent);
    if (id >= 1000) { return; }
    QVariantMap row;
    while (query()->next()) {
        row.clear();
        for (int i = 0; i < n; ++i) {
            row.insert(query()->record().fieldName(i), query()->value(i));
        }
        *result() << row;
    }
}
