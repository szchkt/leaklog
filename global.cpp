/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2009 Matus & Michal Tomlein

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
#include <QNetworkRequest>
#include <QSqlRecord>
#include <QSqlField>

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
    if (!database->driverName().contains("SQLITE")) {
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
    if (!database->driverName().contains("SQLITE")) {
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

double Global::evaluateExpression(StringVariantMap & inspection, const MTDictionary & expression, const QString & customer_id, const QString & circuit_id, bool * ok)
{
    QString inspection_date = inspection.value("date").toString();
    FunctionParser fparser;
    const QString sum_query("SELECT %1 FROM inspections WHERE date LIKE '%2%' AND customer = :customer_id AND circuit = :circuit_id AND (nominal <> 1 OR nominal IS NULL)");
    MTRecord circuit("circuits", "id", circuit_id, MTDictionary("parent", customer_id));
    StringVariantMap circuit_attributes = circuit.list();
    QString value;
    for (int i = 0; i < expression.count(); ++i) {
        if (expression.value(i) == "id") {
            value.append(QString::number(inspection.value(expression.key(i)).toDouble()));
        } else if (expression.value(i) == "sum") {
            QStringList fields;
            Subvariable subvariable("", expression.key(i));
            subvariable.next();
            if (subvariable.count()) {
                fields << subvariable.value("SUBVAR_VALUE").toString().split("+", QString::SkipEmptyParts);
            } else {
                Variable variable(expression.key(i));
                variable.next();
                if (variable.count()) {
                    fields << variable.value("VAR_VALUE").toString().split("+", QString::SkipEmptyParts);
                }
            }
            if (fields.isEmpty()) { fields << expression.key(i); }
            double v = 0.0;
            if (inspection.value("nominal").toInt()) {
                for (int a = 0; a < fields.count(); ++a) {
                    v += inspection.value(fields.at(a)).toDouble();
                }
            } else {
                QSqlQuery sum_ins;
                sum_ins.prepare(sum_query.arg(fields.join(", ")).arg(inspection_date.left(4)));
                sum_ins.bindValue(":customer_id", customer_id);
                sum_ins.bindValue(":circuit_id", circuit_id);
                if (sum_ins.exec()) {
                    while (sum_ins.next()) {
                        for (int a = 0; a < fields.count(); ++a) {
                            v += sum_ins.value(a).toDouble();
                        }
                    }
                }
            }
            value.append(QString::number(v));
        } else if (expression.value(i) == "circuit_attribute") {
            value.append(QString::number(circuit_attributes.value(expression.key(i)).toDouble()));
        } else if (expression.value(i) == "p_to_t") {
            MTRecord circuit("circuits", "id", circuit_id, MTDictionary("parent", customer_id));
            QString refrigerant = circuit.stringValue("refrigerant");
            value.append(QString::number(refrigerants.pressureToTemperature(refrigerant, round(inspection.value(expression.key(i)).toDouble() * 10.0) / 10.0)));
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
        return "<table class=\"no_border\" cellpadding=\"0\" cellspacing=\"0\"><tr><td class=\"no_border\" width=\"1%\" align=\"right\" valign=\"center\" style=\"font-size: large; " + QString(value2 - value1 > tolerance ? "color: #FF0000; " : "") + "font-weight: bold;\">" + upArrow() + "</td><td class=\"no_border " + bg_class + "\" valign=\"center\">%1</td></tr></table>";
    } else if (value1 > value2) {
        return "<table class=\"no_border\" cellpadding=\"0\" cellspacing=\"0\"><tr><td class=\"no_border\" width=\"1%\" align=\"right\" valign=\"center\" style=\"font-size: large; " + QString(value1 - value2 > tolerance ? "color: #FF0000; " : "") + "font-weight: bold;\">" + downArrow() + "</td><td class=\"no_border " + bg_class + "\" valign=\"center\">%1</td></tr></table>";
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

MTDictionary Global::get_dict_dbtables()
{
    MTDictionary dict_dbtables;
    dict_dbtables.insert("service_companies", "id INTEGER PRIMARY KEY, name TEXT, address TEXT, mail TEXT, phone TEXT, website TEXT");
    dict_dbtables.insert("customers", "id INTEGER PRIMARY KEY, company TEXT, contact_person TEXT, address TEXT, mail TEXT, phone TEXT");
    dict_dbtables.insert("circuits", "parent INTEGER, id INTEGER, name TEXT, disused INTEGER, operation TEXT, building TEXT, device TEXT, hermetic INTEGER, manufacturer TEXT, type TEXT, sn TEXT, year INTEGER, commissioning TEXT, field TEXT, refrigerant TEXT, refrigerant_amount NUMERIC, oil TEXT, oil_amount NUMERIC, leak_detector INTEGER, runtime NUMERIC, utilisation NUMERIC, inspection_interval INTEGER");
    dict_dbtables.insert("inspections", "customer INTEGER, circuit INTEGER, date TEXT, nominal INTEGER, repair INTEGER");
    dict_dbtables.insert("repairs", "date TEXT, customer TEXT, device TEXT, field TEXT, refrigerant TEXT, refrigerant_amount NUMERIC, refr_add_am NUMERIC, refr_add_am_recy NUMERIC, refr_reco NUMERIC, refr_reco_cust NUMERIC, repairman TEXT, arno TEXT");
    dict_dbtables.insert("inspectors", "id INTEGER PRIMARY KEY, person TEXT, company TEXT, person_reg_num TEXT, phone TEXT");
    dict_dbtables.insert("variables", "id TEXT, name TEXT, type TEXT, unit TEXT, value TEXT, compare_nom INTEGER, tolerance NUMERIC, col_bg TEXT");
    dict_dbtables.insert("subvariables", "parent TEXT, id TEXT, name TEXT, type TEXT, unit TEXT, value TEXT, compare_nom INTEGER, tolerance NUMERIC");
    dict_dbtables.insert("tables", "uid TEXT, id TEXT, highlight_nominal INTEGER, variables TEXT, sum TEXT, avg TEXT");
    dict_dbtables.insert("warnings", "id INTEGER PRIMARY KEY, enabled INTEGER, name TEXT, description TEXT, delay INTEGER");
    dict_dbtables.insert("warnings_filters", "parent INTEGER, circuit_attribute TEXT, function TEXT, value TEXT");
    dict_dbtables.insert("warnings_conditions", "parent INTEGER, value_ins TEXT, function TEXT, value_nom TEXT");
    dict_dbtables.insert("refrigerant_management", "date TEXT, refrigerant TEXT, purchased NUMERIC, purchased_reco NUMERIC, sold NUMERIC, sold_reco NUMERIC, refr_rege NUMERIC, refr_disp NUMERIC, leaked NUMERIC, leaked_reco NUMERIC");
    dict_dbtables.insert("db_info", "id TEXT, value TEXT");
    return dict_dbtables;
}

MTDictionary Global::get_dict_vartypes()
{
    MTDictionary dict_vartypes;
    dict_vartypes.insert("int", QApplication::translate("VariableTypes", "Integer"));
    dict_vartypes.insert("float", QApplication::translate("VariableTypes", "Real number"));
    dict_vartypes.insert("string", QApplication::translate("VariableTypes", "String"));
    dict_vartypes.insert("text", QApplication::translate("VariableTypes", "Text"));
    dict_vartypes.insert("bool", QApplication::translate("VariableTypes", "Boolean"));
    return dict_vartypes;
}

MTDictionary Global::get_dict_varnames()
{
    MTDictionary dict_varnames;
    dict_varnames.insert("t_sec", QApplication::translate("VariableNames", "Temperature sec. medium"));
    dict_varnames.insert("t_sec_evap_in", QApplication::translate("VariableNames", "evap. in"));
    dict_varnames.insert("t_sec_cond_in", QApplication::translate("VariableNames", "cond. in"));
    dict_varnames.insert("p_0", QApplication::translate("VariableNames", "Pressure evaporating"));
    dict_varnames.insert("p_c", QApplication::translate("VariableNames", "Pressure condensing"));
    dict_varnames.insert("t_0", QApplication::translate("VariableNames", "Temperature evaporating"));
    dict_varnames.insert("t_c", QApplication::translate("VariableNames", "Temperature condensing"));
    dict_varnames.insert("t_ev", QApplication::translate("VariableNames", "Temperature EV"));
    dict_varnames.insert("t_evap_out", QApplication::translate("VariableNames", "Temperature evap. out"));
    dict_varnames.insert("t_comp_in", QApplication::translate("VariableNames", "Temperature comp. in"));
    dict_varnames.insert("t_sc", QApplication::translate("VariableNames", "Subcooling"));
    dict_varnames.insert("t_sh", QApplication::translate("VariableNames", "Superheating"));
    dict_varnames.insert("t_sh_evap", QApplication::translate("VariableNames", "evap."));
    dict_varnames.insert("t_sh_comp", QApplication::translate("VariableNames", "comp."));
    dict_varnames.insert("t_comp_out", QApplication::translate("VariableNames", "Temperature discharge"));
    dict_varnames.insert("delta_t_evap", QApplication::translate("VariableNames", "%1T (evaporating)", 0, QApplication::UnicodeUTF8).arg(delta()));
    dict_varnames.insert("delta_t_c", QApplication::translate("VariableNames", "%1T (condensing)", 0, QApplication::UnicodeUTF8).arg(delta()));
    dict_varnames.insert("ep_comp", QApplication::translate("VariableNames", "Comp. el. power input"));
    dict_varnames.insert("ec", QApplication::translate("VariableNames", "Electric current"));
    dict_varnames.insert("ec_l1", QApplication::translate("VariableNames", "L1"));
    dict_varnames.insert("ec_l2", QApplication::translate("VariableNames", "L2"));
    dict_varnames.insert("ec_l3", QApplication::translate("VariableNames", "L3"));
    dict_varnames.insert("ev", QApplication::translate("VariableNames", "Electric voltage"));
    dict_varnames.insert("ev_l1", QApplication::translate("VariableNames", "L1"));
    dict_varnames.insert("ev_l2", QApplication::translate("VariableNames", "L2"));
    dict_varnames.insert("ev_l3", QApplication::translate("VariableNames", "L3"));
    dict_varnames.insert("ppsw", QApplication::translate("VariableNames", "Pneumatic pressure switches"));
    dict_varnames.insert("ppsw_hip", QApplication::translate("VariableNames", "HiP"));
    dict_varnames.insert("ppsw_lop", QApplication::translate("VariableNames", "LoP"));
    dict_varnames.insert("ppsw_diff", QApplication::translate("VariableNames", "Diff"));
    dict_varnames.insert("sftsw", QApplication::translate("VariableNames", "Safety switch"));
    dict_varnames.insert("rmds", QApplication::translate("VariableNames", "Remedies"));
    dict_varnames.insert("arno", QApplication::translate("VariableNames", "Assembly record No."));
    dict_varnames.insert("vis_aur_chk", QApplication::translate("VariableNames", "Visual and aural check"));
    dict_varnames.insert("corr_def", QApplication::translate("VariableNames", "Corr/Def"));
    dict_varnames.insert("noise_vibr", QApplication::translate("VariableNames", "Noise/Vibr"));
    dict_varnames.insert("bbl_lvl", QApplication::translate("VariableNames", "Bubble/Level"));
    dict_varnames.insert("oil_leak_am", QApplication::translate("VariableNames", "Oil leak"));
    dict_varnames.insert("dir_leak_chk", QApplication::translate("VariableNames", "Direct leak check (location)"));
    dict_varnames.insert("el_detect", QApplication::translate("VariableNames", "Electronic detection"));
    dict_varnames.insert("uv_detect", QApplication::translate("VariableNames", "UV detection"));
    dict_varnames.insert("bbl_detect", QApplication::translate("VariableNames", "Bubble detection"));
    dict_varnames.insert("refr_add", QApplication::translate("VariableNames", "Refrigerant addition"));
    dict_varnames.insert("refr_add_am", QApplication::translate("VariableNames", "New"));
    dict_varnames.insert("refr_add_am_recy", QApplication::translate("VariableNames", "Recovered"));
    dict_varnames.insert("refr_add_am_total", QApplication::translate("VariableNames", "Total"));
    dict_varnames.insert("refr_add_per", QApplication::translate("VariableNames", "%"));
    dict_varnames.insert("refr_recovery", QApplication::translate("VariableNames", "Refrigerant recovery"));
    dict_varnames.insert("refr_reco", QApplication::translate("VariableNames", "Store"));
    dict_varnames.insert("refr_reco_cust", QApplication::translate("VariableNames", "Customer"));
    dict_varnames.insert("inspector", QApplication::translate("VariableNames", "Inspector"));
    dict_varnames.insert("operator", QApplication::translate("VariableNames", "Operator"));
    return dict_varnames;
}

MTDictionary Global::get_dict_fields()
{
    MTDictionary dict_fields(true);
    dict_fields.insert(QApplication::translate("AttributeValues", "Refrigeration"), "refrigeration");
    dict_fields.insert(QApplication::translate("AttributeValues", "Transportation"), "transportation");
    dict_fields.insert(QApplication::translate("AttributeValues", "Air conditioning"), "airconditioning");
    dict_fields.insert(QApplication::translate("AttributeValues", "Heat pumps"), "heatpumps");
    // OBSOLETE
    dict_fields.insert(QApplication::translate("AttributeValues", "Air conditioning"), "car");
    dict_fields.insert(QApplication::translate("AttributeValues", "Air conditioning"), "home");
    dict_fields.insert(QApplication::translate("AttributeValues", "Air conditioning"), "commercial");
    dict_fields.insert(QApplication::translate("AttributeValues", "Refrigeration"), "industrial");
    dict_fields.insert(QApplication::translate("AttributeValues", "Air conditioning"), "agricultural");
    dict_fields.insert(QApplication::translate("AttributeValues", "Refrigeration"), "other");
    dict_fields.insert(QApplication::translate("AttributeValues", "Air conditioning"), "lowrise");
    dict_fields.insert(QApplication::translate("AttributeValues", "Air conditioning"), "highrise");
    dict_fields.insert(QApplication::translate("AttributeValues", "Air conditioning"), "institutional");
    return dict_fields;
}

MTDictionary Global::get_dict_oils()
{
    MTDictionary dict_oils;
    dict_oils.insert(QApplication::translate("AttributeValues", "MO (Mineral oil)"), "mo");
    dict_oils.insert(QApplication::translate("AttributeValues", "AB (Alkylbenzene oil)"), "ab");
    dict_oils.insert(QApplication::translate("AttributeValues", "POE (Polyolester oil)"), "poe");
    dict_oils.insert(QApplication::translate("AttributeValues", "PAO (Polyalphaolefin oil)"), "pao");
    dict_oils.insert(QApplication::translate("AttributeValues", "PVE (Polyvinylether oil)"), "pve");
    dict_oils.insert(QApplication::translate("AttributeValues", "PAG (Polyglycol oil)"), "pag");
    return dict_oils;
}

MTDictionary Global::get_dict_attrvalues()
{
    MTDictionary dict_attrvalues;
    dict_attrvalues.insert("field", QApplication::translate("AttributeValues", "Field of application"));
    dict_attrvalues.insert("field::refrigeration", QApplication::translate("AttributeValues", "Refrigeration"));
    dict_attrvalues.insert("field::transportation", QApplication::translate("AttributeValues", "Transportation"));
    dict_attrvalues.insert("field::airconditioning", QApplication::translate("AttributeValues", "Air conditioning"));
    dict_attrvalues.insert("field::heatpumps", QApplication::translate("AttributeValues", "Heat pumps"));
    dict_attrvalues.insert("oil", QApplication::translate("AttributeValues", "Oil"));
    dict_attrvalues.insert("oil::mo", QApplication::translate("AttributeValues", "MO (Mineral oil)"));
    dict_attrvalues.insert("oil::ab", QApplication::translate("AttributeValues", "AB (Alkylbenzene oil)"));
    dict_attrvalues.insert("oil::poe", QApplication::translate("AttributeValues", "POE (Polyolester oil)"));
    dict_attrvalues.insert("oil::pao", QApplication::translate("AttributeValues", "PAO (Polyalphaolefin oil)"));
    dict_attrvalues.insert("oil::pve", QApplication::translate("AttributeValues", "PVE (Polyvinylether oil)"));
    dict_attrvalues.insert("oil::pag", QApplication::translate("AttributeValues", "PAG (Polyglycol oil)"));
    dict_attrvalues.insert("refrigerant", QApplication::translate("AttributeValues", "Refrigerant"));
    QStringList list_refrigerants = listRefrigerantsToString().split(";");
    for (int i = 0; i < list_refrigerants.count(); ++i) {
        dict_attrvalues.insert(QString("refrigerant::%1").arg(list_refrigerants.at(i)), list_refrigerants.at(i));
    }
    // OBSOLETE
    dict_attrvalues.insert("field::car", QApplication::translate("AttributeValues", "Air conditioning"));
    dict_attrvalues.insert("field::home", QApplication::translate("AttributeValues", "Air conditioning"));
    dict_attrvalues.insert("field::commercial", QApplication::translate("AttributeValues", "Air conditioning"));
    dict_attrvalues.insert("field::industrial", QApplication::translate("AttributeValues", "Refrigeration"));
    dict_attrvalues.insert("field::agricultural", QApplication::translate("AttributeValues", "Air conditioning"));
    dict_attrvalues.insert("field::other", QApplication::translate("AttributeValues", "Refrigeration"));
    dict_attrvalues.insert("field::lowrise", QApplication::translate("AttributeValues", "Air conditioning"));
    dict_attrvalues.insert("field::highrise", QApplication::translate("AttributeValues", "Air conditioning"));
    dict_attrvalues.insert("field::institutional", QApplication::translate("AttributeValues", "Air conditioning"));
    // --------
    return dict_attrvalues;
}

MTDictionary Global::get_dict_attrnames()
{
    MTDictionary dict_attrnames;
    dict_attrnames.insert("customer::id", QApplication::translate("Customer", "ID"));
    dict_attrnames.insert("customer::company", QApplication::translate("AttributeNames", "Company"));
    dict_attrnames.insert("customer::contact_person", QApplication::translate("AttributeNames", "Contact person"));
    dict_attrnames.insert("customer::address", QApplication::translate("AttributeNames", "Address"));
    dict_attrnames.insert("customer::mail", QApplication::translate("AttributeNames", "E-mail"));
    dict_attrnames.insert("customer::phone", QApplication::translate("AttributeNames", "Phone"));
    dict_attrnames.insert("circuit::id", QApplication::translate("AttributeNames", "ID"));
    dict_attrnames.insert("circuit::name", QApplication::translate("AttributeNames", "Circuit name"));
    dict_attrnames.insert("circuit::operation", QApplication::translate("AttributeNames", "Place of operation"));
    dict_attrnames.insert("circuit::building", QApplication::translate("AttributeNames", "Building"));
    dict_attrnames.insert("circuit::device", QApplication::translate("AttributeNames", "Device"));
    dict_attrnames.insert("circuit::hermetic", QApplication::translate("AttributeNames", "Hermetically sealed"));
    dict_attrnames.insert("circuit::manufacturer", QApplication::translate("AttributeNames", "Manufacturer"));
    dict_attrnames.insert("circuit::type", QApplication::translate("AttributeNames", "Type"));
    dict_attrnames.insert("circuit::sn", QApplication::translate("AttributeNames", "Serial number"));
    dict_attrnames.insert("circuit::year", QApplication::translate("AttributeNames", "Year of purchase"));
    dict_attrnames.insert("circuit::commissioning", QApplication::translate("AttributeNames", "Date of commissioning"));
    dict_attrnames.insert("circuit::field", QApplication::translate("AttributeNames", "Field of application"));
    dict_attrnames.insert("service_companies::name", QApplication::translate("AttributeNames", "Name:"));
    dict_attrnames.insert("service_companies::id", QApplication::translate("ServiceCompany", "ID:"));
    dict_attrnames.insert("service_companies::address", QApplication::translate("AttributeNames", "Address:"));
    dict_attrnames.insert("service_companies::phone", QApplication::translate("AttributeNames", "Phone:"));
    dict_attrnames.insert("service_companies::mail", QApplication::translate("AttributeNames", "E-mail:"));
    dict_attrnames.insert("service_companies::website", QApplication::translate("AttributeNames", "Website:"));
    dict_attrnames.insert("repairs::date", QApplication::translate("AttributeNames", "Date"));
    dict_attrnames.insert("repairs::customer", QApplication::translate("AttributeNames", "Customer"));
    dict_attrnames.insert("repairs::device", QApplication::translate("AttributeNames", "Device"));
    dict_attrnames.insert("repairs::field", QApplication::translate("AttributeNames", "Field of application"));
    dict_attrnames.insert("repairs::refrigerant", QApplication::translate("AttributeNames", "Refrigerant"));
    dict_attrnames.insert("repairs::refrigerant_amount", QApplication::translate("AttributeNames", "Amount of refrigerant"));
    dict_attrnames.insert("repairs::refr_add_am", QApplication::translate("VariableNames", "New"));
    dict_attrnames.insert("repairs::refr_add_am_recy", QApplication::translate("VariableNames", "Recovered"));
    dict_attrnames.insert("repairs::refr_reco", QApplication::translate("VariableNames", "Store"));
    dict_attrnames.insert("repairs::refr_reco_cust", QApplication::translate("VariableNames", "Customer"));
    dict_attrnames.insert("repairs::repairman", QApplication::translate("AttributeNames", "Repairman"));
    dict_attrnames.insert("repairs::arno", QApplication::translate("AttributeNames", "Assembly record No."));
    dict_attrnames.insert("inspectors::id", QApplication::translate("AttributeNames", "ID"));
    dict_attrnames.insert("inspectors::person", QApplication::translate("AttributeNames", "Certified person"));
    dict_attrnames.insert("inspectors::person_reg_num", QApplication::translate("AttributeNames", "Person registry number"));
    //dict_attrnames.insert("inspectors::company", QApplication::translate("AttributeNames", "Certified company"));
    dict_attrnames.insert("inspectors::phone", QApplication::translate("AttributeNames", "Phone"));
    dict_attrnames.insert("circuit::disused", QApplication::translate("AttributeNames", "Disused"));
    dict_attrnames.insert("circuit::refrigerant", QApplication::translate("AttributeNames", "Refrigerant"));
    dict_attrnames.insert("circuit::refrigerant_amount", QApplication::translate("AttributeNames", "Amount of refrigerant") + "||" + QApplication::translate("AttributeNames", "kg"));
    dict_attrnames.insert("circuit::oil", QApplication::translate("AttributeNames", "Oil"));
    dict_attrnames.insert("circuit::oil_amount", QApplication::translate("AttributeNames", "Amount of oil") + "||" + QApplication::translate("AttributeNames", "kg"));
    dict_attrnames.insert("circuit::runtime", QApplication::translate("AttributeNames", "Run-time per day") + "||" + QApplication::translate("AttributeNames", "hours"));
    dict_attrnames.insert("circuit::utilisation", QApplication::translate("AttributeNames", "Rate of utilisation") + "||%");
    return dict_attrnames;
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

QStringList Global::listVariableIds(bool all)
{
    QStringList ids; bool sub_empty = false;
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
