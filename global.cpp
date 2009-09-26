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

#include "global.h"

QString Global::toString(const QVariant & v) { return v.toString(); }

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

QString Global::variantTypeToSqlType(QVariant::Type type)
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
    MTRecord circuit("circuits", circuit_id, MTDictionary("parent", customer_id));
    StringVariantMap circuit_attributes = circuit.list();
    QString value;
    for (int i = 0; i < expression.count(); ++i) {
        if (expression.value(i) == "id") {
            value.append(toString(inspection.value(expression.key(i)).toDouble()));
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
            value.append(toString(v));
        } else if (expression.value(i) == "circuit_attribute") {
            value.append(toString(circuit_attributes.value(expression.key(i)).toDouble()));
        } else if (expression.value(i) == "p_to_t") {
            MTRecord circuit("circuits", circuit_id, MTDictionary("parent", customer_id));
            QString refrigerant = circuit.list("refrigerant").value("refrigerant").toString();
            value.append(toString(refrigerants.pressureToTemperature(refrigerant, round(inspection.value(expression.key(i)).toDouble() * 10.0) / 10.0)));
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
    dict_dbtables.insert("repairs", "date TEXT, customer TEXT, field TEXT, refrigerant TEXT, refrigerant_amount NUMERIC, refr_add_am NUMERIC, refr_add_am_recy NUMERIC, refr_reco NUMERIC, refr_reco_cust NUMERIC, repairman TEXT, arno TEXT");
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
    dict_varnames.insert("t", QApplication::translate("VariableNames", "Temperature"));
    dict_varnames.insert("t_out", QApplication::translate("VariableNames", "out"));
    dict_varnames.insert("t_in", QApplication::translate("VariableNames", "in"));
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
    dict_fields.insert(QApplication::translate("AttributeValues", "Car air conditioning"), "car");
    dict_fields.insert(QApplication::translate("AttributeValues", "Home air conditioning"), "home");
    dict_fields.insert(QApplication::translate("AttributeValues", "Commercial buildings"), "commercial");
    dict_fields.insert(QApplication::translate("AttributeValues", "Industrial spaces"), "industrial");
    dict_fields.insert(QApplication::translate("AttributeValues", "Agricultural air conditioning"), "agricultural");
    dict_fields.insert(QApplication::translate("AttributeValues", "Transportation"), "transportation");
    dict_fields.insert(QApplication::translate("AttributeValues", "Air conditioning"), "airconditioning");
    dict_fields.insert(QApplication::translate("AttributeValues", "Heat pumps"), "heatpumps");
    dict_fields.insert(QApplication::translate("AttributeValues", "Other"), "other");
    // OBSOLETE
    dict_fields.insert(QApplication::translate("AttributeValues", "Home air conditioning"), "lowrise");
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
    dict_attrvalues.insert("field::car", QApplication::translate("AttributeValues", "Car air conditioning"));
    dict_attrvalues.insert("field::home", QApplication::translate("AttributeValues", "Home air conditioning"));
    dict_attrvalues.insert("field::commercial", QApplication::translate("AttributeValues", "Commercial buildings"));
    dict_attrvalues.insert("field::industrial", QApplication::translate("AttributeValues", "Industrial spaces"));
    dict_attrvalues.insert("field::agricultural", QApplication::translate("AttributeValues", "Agricultural air conditioning"));
    dict_attrvalues.insert("field::transportation", QApplication::translate("AttributeValues", "Transportation"));
    dict_attrvalues.insert("field::airconditioning", QApplication::translate("AttributeValues", "Air conditioning"));
    dict_attrvalues.insert("field::heatpumps", QApplication::translate("AttributeValues", "Heat pumps"));
    dict_attrvalues.insert("field::other", QApplication::translate("AttributeValues", "Other"));
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
    dict_attrvalues.insert("field::lowrise", QApplication::translate("AttributeValues", "Home air conditioning"));
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
    return "R11;R12;R22;R32;R123;R124;R125;R134a;R143a;R227ea;R365mfc;R404A;R407C;R410A;R502;R507";
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
              << "atan2" // (A,B)	Arc-tangent of A/B. The two main differences to atan() is that it will return the right angle depending on the signs of A and B (atan() can only return values betwen -pi/2 and pi/2), and that the return value of pi/2 and -pi/2 are possible.
              << "atanh" // (A)     Same as atan() but for hyperbolic tangent.
              << "ceil" // (A)      Ceiling of A. Returns the smallest integer greater than A. Rounds up to the next higher integer.
              << "cos" // (A)       Cosine of A. Returns the cosine of the angle A, where A is measured in radians.
              << "cosh" // (A)      Same as cos() but for hyperbolic cosine.
              << "cot" // (A)       Cotangent of A (equivalent to 1/tan(A)).
              << "csc" // (A)       Cosecant of A (equivalent to 1/sin(A)).
              << "exp" // (A)       Exponential of A. Returns the value of e raised to the power A where e is the base of the natural logarithm, i.e. the non-repeating value approximately equal to 2.71828182846.
              << "floor" // (A)     Floor of A. Returns the largest integer less than A. Rounds down to the next lower integer.
              << "if" // (A,B,C)	If int(A) differs from 0, the return value of this function is B, else C. Only the parameter which needs to be evaluated is evaluated, the other parameter is skipped.
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

using namespace Global;

Variables::Variables(QSqlDatabase db, bool exec_query):
MTSqlQueryResult(db)
{
    dict_varnames = get_dict_varnames();
    if (exec_query)
        exec("SELECT variables.id, variables.name, variables.type, variables.unit, variables.value, variables.compare_nom, variables.tolerance, variables.col_bg, subvariables.id, subvariables.name, subvariables.type, subvariables.unit, subvariables.value, subvariables.compare_nom, subvariables.tolerance FROM variables LEFT JOIN subvariables ON variables.id = subvariables.parent");
}

const int VAR_ID = 0; const int VAR_NAME = 1; const int VAR_TYPE = 2; const int VAR_UNIT = 3; const int VAR_VALUE = 4; const int VAR_COMPARE_NOM = 5; const int VAR_TOLERANCE = 6; const int VAR_COL_BG = 7;
const int SUBVAR_ID = 8; const int SUBVAR_NAME = 9; const int SUBVAR_TYPE = 10; const int SUBVAR_UNIT = 11; const int SUBVAR_VALUE = 12; const int SUBVAR_COMPARE_NOM = 13; const int SUBVAR_TOLERANCE = 14;

void Variables::saveResult()
{
    bool insert = true;
    *pos() = -1;
    result()->clear();
    initVariables();
    StringVariantMap row;
    QSet<int> updated_indices;
    while (query()->next()) {
        row.clear();
        insert = true;
        if (dict_varnames.contains(query()->value(VAR_ID).toString())) {
            QSet<int> indices = var_indices.values(query()->value(VAR_ID).toString()).toSet();
            indices.unite(var_indices.values(query()->value(SUBVAR_ID).toString()).toSet());
            foreach (int index, indices) {
                insert = false;
                StringVariantMap _row = result()->at(index);
                if (!updated_indices.contains(index)) {
                    updated_indices << index;
                    if (!query()->value(VAR_COMPARE_NOM).toString().isEmpty()) { insert = true; _row.insert("VAR_COMPARE_NOM", query()->value(VAR_COMPARE_NOM)); }
                    if (!query()->value(VAR_TOLERANCE).toString().isEmpty()) { insert = true; _row.insert("VAR_TOLERANCE", query()->value(VAR_TOLERANCE)); }
                    if (!query()->value(VAR_COL_BG).toString().isEmpty()) { insert = true; _row.insert("VAR_COL_BG", query()->value(VAR_COL_BG)); }
                }
                if (query()->value(SUBVAR_ID).toString() == _row.value("SUBVAR_ID").toString()) {
                    if (!query()->value(SUBVAR_COMPARE_NOM).toString().isEmpty()) { insert = true; _row.insert("SUBVAR_COMPARE_NOM", query()->value(SUBVAR_COMPARE_NOM)); }
                    if (!query()->value(SUBVAR_TOLERANCE).toString().isEmpty()) { insert = true; _row.insert("SUBVAR_TOLERANCE", query()->value(SUBVAR_TOLERANCE)); }
                }
                if (insert) { result()->replace(index, _row); insert = false; }
            }
        }
        if (insert) {
            row.insert("VAR_ID", query()->value(VAR_ID));
            row.insert("VAR_NAME", query()->value(VAR_NAME));
            row.insert("VAR_TYPE", query()->value(VAR_TYPE));
            row.insert("VAR_UNIT", query()->value(VAR_UNIT));
            row.insert("VAR_VALUE", query()->value(VAR_VALUE));
            row.insert("VAR_COMPARE_NOM", query()->value(VAR_COMPARE_NOM));
            row.insert("VAR_TOLERANCE", query()->value(VAR_TOLERANCE));
            row.insert("VAR_COL_BG", query()->value(VAR_COL_BG));
            row.insert("SUBVAR_ID", query()->value(SUBVAR_ID));
            row.insert("SUBVAR_NAME", query()->value(SUBVAR_NAME));
            row.insert("SUBVAR_TYPE", query()->value(SUBVAR_TYPE));
            row.insert("SUBVAR_UNIT", query()->value(SUBVAR_UNIT));
            row.insert("SUBVAR_VALUE", query()->value(SUBVAR_VALUE));
            row.insert("SUBVAR_COMPARE_NOM", query()->value(SUBVAR_COMPARE_NOM));
            row.insert("SUBVAR_TOLERANCE", query()->value(SUBVAR_TOLERANCE));
            *result() << row;
        }
    }
}

void Variables::initVariables(const QString & filter)
{
    initVariable(filter, "t", "");
    initSubvariable(filter, "t", "", "t_out", "float", tr("%1C").arg(degreeSign()), "", true, 0.0);
    initSubvariable(filter, "t", "", "t_in", "float", tr("%1C").arg(degreeSign()), "", true, 0.0);

    initVariable(filter, "p_0", "float", tr("Bar"), "", true, 0.0, "aliceblue");
    initVariable(filter, "t_0", "float", tr("%1C").arg(degreeSign()), "p_to_t(p_0)", true, 0.0, "aliceblue");
    initVariable(filter, "delta_t_evap", "float", tr("%1C").arg(degreeSign()), "abs(t_in-p_to_t(p_0))", true, 0.0, "aliceblue");
    initVariable(filter, "t_evap_out", "float", tr("%1C").arg(degreeSign()), "", true, 0.0, "aliceblue");
    initVariable(filter, "t_comp_in", "float", tr("%1C").arg(degreeSign()), "", true, 0.0, "aliceblue");

    initVariable(filter, "t_sh", "aliceblue");
    initSubvariable(filter, "t_sh", "aliceblue", "t_sh_evap", "float", tr("%1C").arg(degreeSign()), "t_evap_out-p_to_t(p_0)", true, 0.0);
    initSubvariable(filter, "t_sh", "aliceblue", "t_sh_comp", "float", tr("%1C").arg(degreeSign()), "t_comp_in-p_to_t(p_0)", true, 0.0);

    initVariable(filter, "p_c", "float", tr("Bar"), "", true, 0.0, "floralwhite");
    initVariable(filter, "t_c", "float", tr("%1C").arg(degreeSign()), "p_to_t(p_c)", true, 0.0, "floralwhite");
    initVariable(filter, "delta_t_c", "float", tr("%1C").arg(degreeSign()), "abs(t_out-p_to_t(p_c))", true, 0.0, "floralwhite");
    initVariable(filter, "t_ev", "float", tr("%1C").arg(degreeSign()), "", true, 0.0, "floralwhite");
    initVariable(filter, "t_sc", "float", tr("%1C").arg(degreeSign()), "p_to_t(p_c)-t_ev", true, 0.0, "floralwhite");
    initVariable(filter, "t_comp_out", "float", tr("%1C").arg(degreeSign()), "", true, 0.0, "floralwhite");
    initVariable(filter, "ep_comp", "float", tr("kW"), "", true, 0.0, "");

    initVariable(filter, "ec", "");
    initSubvariable(filter, "ec", "", "ec_l1", "float", tr("A"), "", true, 0.0);
    initSubvariable(filter, "ec", "", "ec_l2", "float", tr("A"), "", true, 0.0);
    initSubvariable(filter, "ec", "", "ec_l3", "float", tr("A"), "", true, 0.0);

    initVariable(filter, "ev", "");
    initSubvariable(filter, "ev", "", "ev_l1", "float", tr("V"), "", true, 0.0);
    initSubvariable(filter, "ev", "", "ev_l2", "float", tr("V"), "", true, 0.0);
    initSubvariable(filter, "ev", "", "ev_l3", "float", tr("V"), "", true, 0.0);

    initVariable(filter, "ppsw", "");
    initSubvariable(filter, "ppsw", "", "ppsw_hip", "float", tr("Bar"), "", true, 0.0);
    initSubvariable(filter, "ppsw", "", "ppsw_lop", "float", tr("Bar"), "", true, 0.0);
    initSubvariable(filter, "ppsw", "", "ppsw_diff", "float", tr("Bar"), "", true, 0.0);

    initVariable(filter, "sftsw", "float", tr("Bar"), "", true, 0.0, "");
    initVariable(filter, "rmds", "text", "", "", false, 0.0, "");
    initVariable(filter, "arno", "string", "", "", false, 0.0, "");

    initVariable(filter, "vis_aur_chk", "");
    initSubvariable(filter, "vis_aur_chk", "", "corr_def", "bool", "", "", false, 0.0);
    initSubvariable(filter, "vis_aur_chk", "", "noise_vibr", "bool", "", "", false, 0.0);
    initSubvariable(filter, "vis_aur_chk", "", "bbl_lvl", "bool", "", "", false, 0.0);
    initSubvariable(filter, "vis_aur_chk", "", "oil_leak_am", "float", tr("kg"), "", false, 0.0);

    initVariable(filter, "dir_leak_chk", "green");
    initSubvariable(filter, "dir_leak_chk", "green", "el_detect", "bool", "", "", false, 0.0);
    initSubvariable(filter, "dir_leak_chk", "green", "uv_detect", "bool", "", "", false, 0.0);
    initSubvariable(filter, "dir_leak_chk", "green", "bbl_detect", "bool", "", "", false, 0.0);

    initVariable(filter, "refr_add", "yellow");
    initSubvariable(filter, "refr_add", "yellow", "refr_add_am", "float", tr("kg"), "", false, 0.0);
    initSubvariable(filter, "refr_add", "yellow", "refr_add_am_recy", "float", tr("kg"), "", false, 0.0);
    initSubvariable(filter, "refr_add", "yellow", "refr_add_am_total", "float", tr("kg"), "refr_add_am+refr_add_am_recy", false, 0.0);
    initSubvariable(filter, "refr_add", "yellow", "refr_add_per", "float", tr("%"), "100*(sum(refr_add_am_total)-sum(refr_reco)-sum(refr_reco_cust))/refrigerant_amount", false, 0.0);

    initVariable(filter, "refr_recovery", "yellow");
    initSubvariable(filter, "refr_recovery", "yellow", "refr_reco", "float", tr("kg"), "", false, 0.0);
    initSubvariable(filter, "refr_recovery", "yellow", "refr_reco_cust", "float", tr("kg"), "", false, 0.0);
    initVariable(filter, "inspector", "string", "", "", false, 0.0, "");
    initVariable(filter, "operator", "string", "", "", false, 0.0, "");
}

void Variables::initVariable(const QString & filter, const QString & id, const QString & type, const QString & unit, const QString & value, bool compare_nom, double tolerance, const QString & col_bg)
{
    if (!filter.isEmpty() && filter != id) { return; }
    StringVariantMap row;
    row.insert("VAR_ID", id);
    row.insert("VAR_NAME", dict_varnames.value(id));
    row.insert("VAR_TYPE", type);
    row.insert("VAR_UNIT", unit);
    row.insert("VAR_VALUE", value);
    row.insert("VAR_COMPARE_NOM", compare_nom ? 1 : 0);
    row.insert("VAR_TOLERANCE", tolerance);
    row.insert("VAR_COL_BG", col_bg);
    *result() << row;
    var_indices.insert(id, result()->count() - 1);
}

void Variables::initVariable(const QString & filter, const QString & id, const QString & col_bg)
{
    if (filter.isEmpty() || filter != id) { return; }
    StringVariantMap row;
    row.insert("VAR_ID", id);
    row.insert("VAR_NAME", dict_varnames.value(id));
    row.insert("VAR_COL_BG", col_bg);
    *result() << row;
    var_indices.insert(id, result()->count() - 1);
}

void Variables::initSubvariable(const QString & filter, const QString & parent, const QString & col_bg, const QString & id, const QString & type, const QString & unit, const QString & value, bool compare_nom, double tolerance)
{
    if (!filter.isEmpty() && filter != id) { return; }
    StringVariantMap row;
    row.insert("VAR_ID", parent);
    row.insert("VAR_NAME", dict_varnames.value(parent));
    row.insert("VAR_COL_BG", col_bg);
    row.insert("SUBVAR_ID", id);
    row.insert("SUBVAR_NAME", dict_varnames.value(id));
    row.insert("SUBVAR_TYPE", type);
    row.insert("SUBVAR_UNIT", unit);
    row.insert("SUBVAR_VALUE", value);
    row.insert("SUBVAR_COMPARE_NOM", compare_nom ? 1 : 0);
    row.insert("SUBVAR_TOLERANCE", tolerance);
    *result() << row;
    var_indices.insert(parent, result()->count() - 1);
    var_indices.insert(id, result()->count() - 1);
}

Variable::Variable(const QString & id, QSqlDatabase db):
Variables(db, false)
{
    var_id = id;
    prepare("SELECT id, name, type, unit, value, compare_nom, tolerance, col_bg FROM variables" + QString(id.isEmpty() ? "" : " WHERE id = :id"));
    if (!id.isEmpty()) { bindValue(":id", var_id); }
    exec();
}

void Variable::saveResult()
{
    const int VAR_ID = 0; const int VAR_NAME = 1; const int VAR_TYPE = 2; const int VAR_UNIT = 3;
    const int VAR_VALUE = 4; const int VAR_COMPARE_NOM = 5; const int VAR_TOLERANCE = 6; const int VAR_COL_BG = 7;
    bool insert = true;
    *pos() = -1;
    result()->clear();
    initVariables(var_id);
    StringVariantMap row;
    while (query()->next()) {
        row.clear();
        insert = true;
        if (dict_varnames.contains(query()->value(VAR_ID).toString())) {
            int index = var_indices.value(query()->value(VAR_ID).toString(), -1);
            if (index < 0) { insert = true; }
            else {
                insert = false;
                StringVariantMap _row = result()->at(index);
                if (!query()->value(VAR_COMPARE_NOM).toString().isEmpty()) { insert = true; _row.insert("VAR_COMPARE_NOM", query()->value(VAR_COMPARE_NOM)); }
                if (!query()->value(VAR_TOLERANCE).toString().isEmpty()) { insert = true; _row.insert("VAR_TOLERANCE", query()->value(VAR_TOLERANCE)); }
                if (!query()->value(VAR_COL_BG).toString().isEmpty()) { insert = true; _row.insert("VAR_COL_BG", query()->value(VAR_COL_BG)); }
                if (insert) { result()->replace(index, _row); insert = false; }
            }
        }
        if (insert) {
            row.insert("VAR_ID", query()->value(VAR_ID));
            row.insert("VAR_NAME", query()->value(VAR_NAME));
            row.insert("VAR_TYPE", query()->value(VAR_TYPE));
            row.insert("VAR_UNIT", query()->value(VAR_UNIT));
            row.insert("VAR_VALUE", query()->value(VAR_VALUE));
            row.insert("VAR_COMPARE_NOM", query()->value(VAR_COMPARE_NOM));
            row.insert("VAR_TOLERANCE", query()->value(VAR_TOLERANCE));
            row.insert("VAR_COL_BG", query()->value(VAR_COL_BG));
            *result() << row;
        }
    }
}

Subvariable::Subvariable(const QString & parent, const QString & id, QSqlDatabase db):
Variables(db, false)
{
    var_id = id;
    QString query = "SELECT parent, id, name, type, unit, value, compare_nom, tolerance FROM subvariables";
    if (!parent.isEmpty() || !id.isEmpty()) { query.append(" WHERE "); }
    if (!parent.isEmpty()) {
        query.append("parent = :parent");
        if (!id.isEmpty()) { query.append(" AND "); }
    }
    if (!id.isEmpty()) { query.append("id = :id"); }
    prepare(query);
    if (!parent.isEmpty()) { bindValue(":parent", parent); }
    if (!id.isEmpty()) { bindValue(":id", var_id); }
    exec();
}

void Subvariable::saveResult()
{
    const int VAR_ID = 0; const int SUBVAR_ID = 1; const int SUBVAR_NAME = 2; const int SUBVAR_TYPE = 3;
    const int SUBVAR_UNIT = 4; const int SUBVAR_VALUE = 5; const int SUBVAR_COMPARE_NOM = 6; const int SUBVAR_TOLERANCE = 7;
    bool insert = true;
    *pos() = -1;
    result()->clear();
    initVariables(var_id);
    StringVariantMap row;
    while (query()->next()) {
        row.clear();
        insert = true;
        if (dict_varnames.contains(query()->value(SUBVAR_ID).toString())) {
            int index = var_indices.value(query()->value(SUBVAR_ID).toString(), -1);
            if (index < 0) { insert = true; }
            else {
                insert = false;
                StringVariantMap _row = result()->at(index);
                if (!query()->value(SUBVAR_COMPARE_NOM).toString().isEmpty()) { insert = true; _row.insert("SUBVAR_COMPARE_NOM", query()->value(SUBVAR_COMPARE_NOM)); }
                if (!query()->value(SUBVAR_TOLERANCE).toString().isEmpty()) { insert = true; _row.insert("SUBVAR_TOLERANCE", query()->value(SUBVAR_TOLERANCE)); }
                if (insert) { result()->replace(index, _row); insert = false; }
            }
        }
        if (insert) {
            row.insert("VAR_ID", query()->value(VAR_ID));
            row.insert("SUBVAR_ID", query()->value(SUBVAR_ID));
            row.insert("SUBVAR_NAME", query()->value(SUBVAR_NAME));
            row.insert("SUBVAR_TYPE", query()->value(SUBVAR_TYPE));
            row.insert("SUBVAR_UNIT", query()->value(SUBVAR_UNIT));
            row.insert("SUBVAR_VALUE", query()->value(SUBVAR_VALUE));
            row.insert("SUBVAR_COMPARE_NOM", query()->value(SUBVAR_COMPARE_NOM));
            row.insert("SUBVAR_TOLERANCE", query()->value(SUBVAR_TOLERANCE));
            *result() << row;
        }
    }
}

Warnings::Warnings(QSqlDatabase db, bool enabled_only, const QString & customer_id, const QString & circuit_id):
MTSqlQueryResult(db)
{
    database = db.isValid() ? db : QSqlDatabase::database();
    this->enabled_only = enabled_only;
    if (exec("SELECT id, enabled, name, description, delay FROM warnings" + QString(enabled_only ? " WHERE enabled = 1" : ""))
        && !customer_id.isEmpty() && !circuit_id.isEmpty()) {
        MTRecord circuit("circuits", circuit_id, MTDictionary("parent", customer_id));
        StringVariantMap circuit_attributes = circuit.list();
        QString circuit_attribute, function, value;
        bool ok1 = true, ok2 = true; double f_circuit_attribute = 0.0, f_value = 0.0;
        bool skip = false; int id; QStringList used_ids = listVariableIds();
        for (int i = 0; i < result()->count(); ++i) {
            id = result()->at(i).value("id").toInt();
            skip = false;
            WarningFilters warning_filters(id);
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
                    else if (function == "<" && circuit_attribute < value) {}
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

int Warnings::warningConditionValueInsCount(int id) { return conditions_value_ins.value(id).count(); }

MTDictionary Warnings::warningConditionValueIns(int id, int i) { return conditions_value_ins.value(id).at(i); }

int Warnings::warningConditionValueNomCount(int id) { return conditions_value_nom.value(id).count(); }

MTDictionary Warnings::warningConditionValueNom(int id, int i) { return conditions_value_nom.value(id).at(i); }

int Warnings::warningConditionFunctionCount(int id) { return conditions_functions.value(id).count(); }

QString Warnings::warningConditionFunction(int id, int i) { return conditions_functions.value(id).at(i); }

void Warnings::saveResult()
{
    int n = query()->record().count();
    *pos() = -1;
    result()->clear();
    initWarnings(database, result(), 0, -1, enabled_only);
    StringVariantMap row;
    while (query()->next()) {
        if (query()->value(0).toInt() >= 1000) { continue; }
        row.clear();
        for (int i = 0; i < n; ++i) {
            row.insert(query()->record().fieldName(i), query()->value(i));
        }
        *result() << row;
    }
}

void Warnings::initWarnings(QSqlDatabase _database, ListOfStringVariantMaps * map, int type, int id, bool enabled_only)
{
    QSqlDatabase database = _database.isValid() ? _database : QSqlDatabase::database(); QString w;
    w = "1000";
    if (id < 0 || id == w.toInt()) {
        if (type == 0) {
            initWarning(database, map, w, tr("Refrigerant leakage above limit"), tr("3 - 10 kg, before 2011"), 0, enabled_only);
        } else if (type == 1) {
            initFilter(map, w, "commissioning", "<", "2011.07.04");
            initFilter(map, w, "refrigerant_amount", ">=", "3");
            initFilter(map, w, "refrigerant_amount", "<", "10");
        } else if (type == 2) {
            initCondition(map, w, "100*(refr_add_am+refr_add_am_recy)/refrigerant_amount", ">", "6");
        }
    }
    w = "1001";
    if (id < 0 || id == w.toInt()) {
        if (type == 0) {
            initWarning(database, map, w, tr("Refrigerant leakage above limit"), tr("3 - 10 kg, after 2011"), 0, enabled_only);
        } else if (type == 1) {
            initFilter(map, w, "commissioning", ">=", "2011.07.04");
            initFilter(map, w, "refrigerant_amount", ">=", "3");
            initFilter(map, w, "refrigerant_amount", "<", "10");
        } else if (type == 2) {
            initCondition(map, w, "100*(refr_add_am+refr_add_am_recy)/refrigerant_amount", ">", "8");
        }
    }
    w = "1002";
    if (id < 0 || id == w.toInt()) {
        if (type == 0) {
            initWarning(database, map, w, tr("Refrigerant leakage above limit"), tr("10 - 100 kg, before 2011"), 0, enabled_only);
        } else if (type == 1) {
            initFilter(map, w, "commissioning", "<", "2011.07.04");
            initFilter(map, w, "refrigerant_amount", ">=", "10");
            initFilter(map, w, "refrigerant_amount", "<", "100");
        } else if (type == 2) {
            initCondition(map, w, "100*(refr_add_am+refr_add_am_recy)/refrigerant_amount", ">", "4");
        }
    }
    w = "1003";
    if (id < 0 || id == w.toInt()) {
        if (type == 0) {
            initWarning(database, map, w, tr("Refrigerant leakage above limit"), tr("10 - 100 kg, after 2011"), 0, enabled_only);
        } else if (type == 1) {
            initFilter(map, w, "commissioning", ">=", "2011.07.04");
            initFilter(map, w, "refrigerant_amount", ">=", "10");
            initFilter(map, w, "refrigerant_amount", "<", "100");
        } else if (type == 2) {
            initCondition(map, w, "100*(refr_add_am+refr_add_am_recy)/refrigerant_amount", ">", "6");
        }
    }
    w = "1004";
    if (id < 0 || id == w.toInt()) {
        if (type == 0) {
            initWarning(database, map, w, tr("Refrigerant leakage above limit"), tr("above 100 kg, before 2011"), 0, enabled_only);
        } else if (type == 1) {
            initFilter(map, w, "commissioning", "<", "2011.07.04");
            initFilter(map, w, "refrigerant_amount", ">=", "100");
        } else if (type == 2) {
            initCondition(map, w, "100*(refr_add_am+refr_add_am_recy)/refrigerant_amount", ">", "2");
        }
    }
    w = "1005";
    if (id < 0 || id == w.toInt()) {
        if (type == 0) {
            initWarning(database, map, w, tr("Refrigerant leakage above limit"), tr("above 100 kg, after 2011"), 0, enabled_only);
        } else if (type == 1) {
            initFilter(map, w, "commissioning", ">=", "2011.07.04");
            initFilter(map, w, "refrigerant_amount", ">=", "100");
        } else if (type == 2) {
            initCondition(map, w, "100*(refr_add_am+refr_add_am_recy)/refrigerant_amount", ">", "4");
        }
    }
    w = "1100";
    if (id < 0 || id == w.toInt()) {
        if (type == 0) {
            initWarning(database, map, w, tr("Refrigerant leakage"), "", 0, enabled_only);
        } else if (type == 2) {
            initCondition(map, w, "p_to_t(p_0)", "<", "p_to_t(p_0)");
            initCondition(map, w, "t_evap_out-p_to_t(p_0)", ">", "t_evap_out-p_to_t(p_0)");
            initCondition(map, w, "p_to_t(p_c)", "<", "p_to_t(p_c)");
            initCondition(map, w, "p_to_t(p_c)-t_ev", "<", "p_to_t(p_c)-t_ev");
            initCondition(map, w, "t_comp_out", ">", "t_comp_out");
            initCondition(map, w, "abs(t_out-p_to_t(p_c))", "<", "abs(t_out-p_to_t(p_c))");
            initCondition(map, w, "abs(t_in-p_to_t(p_0))", "<", "abs(t_in-p_to_t(p_0))");
        }
    }
    w = "1101";
    if (id < 0 || id == w.toInt()) {
        if (type == 0) {
            initWarning(database, map, w, tr("Compressor valve leakage"), "", 0, enabled_only);
        } else if (type == 2) {
            initCondition(map, w, "p_to_t(p_0)", ">", "p_to_t(p_0)");
            initCondition(map, w, "t_evap_out-p_to_t(p_0)", "<", "t_evap_out-p_to_t(p_0)");
            initCondition(map, w, "p_to_t(p_c)", "<", "p_to_t(p_c)");
            initCondition(map, w, "p_to_t(p_c)-t_ev", "<", "p_to_t(p_c)-t_ev");
            initCondition(map, w, "t_comp_out", ">", "t_comp_out");
            initCondition(map, w, "abs(t_out-p_to_t(p_c))", "<", "abs(t_out-p_to_t(p_c))");
            initCondition(map, w, "abs(t_in-p_to_t(p_0))", "<", "abs(t_in-p_to_t(p_0))");
        }
    }
    w = "1102";
    if (id < 0 || id == w.toInt()) {
        if (type == 0) {
            initWarning(database, map, w, tr("Liquid-line restriction"), "", 0, enabled_only);
        } else if (type == 2) {
            initCondition(map, w, "p_to_t(p_0)", "<", "p_to_t(p_0)");
            initCondition(map, w, "t_evap_out-p_to_t(p_0)", ">", "t_evap_out-p_to_t(p_0)");
            initCondition(map, w, "p_to_t(p_c)", "<", "p_to_t(p_c)");
            initCondition(map, w, "p_to_t(p_c)-t_ev", ">", "p_to_t(p_c)-t_ev");
            initCondition(map, w, "t_comp_out", ">", "t_comp_out");
            initCondition(map, w, "abs(t_out-p_to_t(p_c))", "<", "abs(t_out-p_to_t(p_c))");
            initCondition(map, w, "abs(t_in-p_to_t(p_0))", "<", "abs(t_in-p_to_t(p_0))");
        }
    }
    w = "1103";
    if (id < 0 || id == w.toInt()) {
        if (type == 0) {
            initWarning(database, map, w, tr("Condenser fouling"), "", 0, enabled_only);
        } else if (type == 2) {
            initCondition(map, w, "p_to_t(p_0)", ">", "p_to_t(p_0)");
            initCondition(map, w, "t_evap_out-p_to_t(p_0)", "<", "t_evap_out-p_to_t(p_0)");
            initCondition(map, w, "p_to_t(p_c)", ">", "p_to_t(p_c)");
            initCondition(map, w, "p_to_t(p_c)-t_ev", "<", "p_to_t(p_c)-t_ev");
            initCondition(map, w, "t_comp_out", ">", "t_comp_out");
            initCondition(map, w, "abs(t_out-p_to_t(p_c))", ">", "abs(t_out-p_to_t(p_c))");
            initCondition(map, w, "abs(t_in-p_to_t(p_0))", "<", "abs(t_in-p_to_t(p_0))");
        }
    }
    w = "1104";
    if (id < 0 || id == w.toInt()) {
        if (type == 0) {
            initWarning(database, map, w, tr("Evaporator fouling"), "", 0, enabled_only);
        } else if (type == 2) {
            initCondition(map, w, "p_to_t(p_0)", "<", "p_to_t(p_0)");
            initCondition(map, w, "t_evap_out-p_to_t(p_0)", "<", "t_evap_out-p_to_t(p_0)");
            initCondition(map, w, "p_to_t(p_c)", "<", "p_to_t(p_c)");
            initCondition(map, w, "p_to_t(p_c)-t_ev", "<", "p_to_t(p_c)-t_ev");
            initCondition(map, w, "t_comp_out", "<", "t_comp_out");
            initCondition(map, w, "abs(t_out-p_to_t(p_c))", "<", "abs(t_out-p_to_t(p_c))");
            initCondition(map, w, "abs(t_in-p_to_t(p_0))", ">", "abs(t_in-p_to_t(p_0))");
        }
    }
    w = "1200";
    if (id < 0 || id == w.toInt()) {
        if (type == 0) {
            initWarning(database, map, w, tr("Needs inspection"), tr("3 - 30 kg"), 365, enabled_only);
        } else if (type == 1) {
            initFilter(map, w, "hermetic", "=", "0");
            initFilter(map, w, "refrigerant_amount", ">=", "3");
            initFilter(map, w, "refrigerant_amount", "<", "30");
        }
    }
    w = "1202";
    if (id < 0 || id == w.toInt()) {
        if (type == 0) {
            initWarning(database, map, w, tr("Needs inspection"), tr("6 - 30 kg, hermetically sealed"), 365, enabled_only);
        } else if (type == 1) {
            initFilter(map, w, "hermetic", "=", "1");
            initFilter(map, w, "refrigerant_amount", ">=", "6");
            initFilter(map, w, "refrigerant_amount", "<", "30");
        }
    }
    w = "1204";
    if (id < 0 || id == w.toInt()) {
        if (type == 0) {
            initWarning(database, map, w, tr("Needs inspection"), tr("30 - 300 kg"), 182, enabled_only);
        } else if (type == 1) {
            initFilter(map, w, "leak_detector", "=", "0");
            initFilter(map, w, "refrigerant_amount", ">=", "30");
            initFilter(map, w, "refrigerant_amount", "<", "300");
        }
    }
    w = "1205";
    if (id < 0 || id == w.toInt()) {
        if (type == 0) {
            initWarning(database, map, w, tr("Needs inspection"), tr("30 - 300 kg, leakage detector installed"), 365, enabled_only);
        } else if (type == 1) {
            initFilter(map, w, "leak_detector", "=", "1");
            initFilter(map, w, "refrigerant_amount", ">=", "30");
            initFilter(map, w, "refrigerant_amount", "<", "300");
        }
    }
    w = "1206";
    if (id < 0 || id == w.toInt()) {
        if (type == 0) {
            initWarning(database, map, w, tr("Needs inspection"), tr("above 300 kg"), 91, enabled_only);
        } else if (type == 1) {
            initFilter(map, w, "leak_detector", "=", "0");
            initFilter(map, w, "refrigerant_amount", ">=", "300");
        }
    }
    w = "1207";
    if (id < 0 || id == w.toInt()) {
        if (type == 0) {
            initWarning(database, map, w, tr("Needs inspection"), tr("above 300 kg, leakage detector installed"), 182, enabled_only);
        } else if (type == 1) {
            initFilter(map, w, "leak_detector", "=", "1");
            initFilter(map, w, "refrigerant_amount", ">=", "300");
        }
    }
}

void Warnings::initWarning(QSqlDatabase database, ListOfStringVariantMaps * map, const QString & id, const QString & name, const QString & description, int delay, bool enabled_only)
{
    QSqlQuery query(database);
    query.prepare("SELECT enabled FROM warnings WHERE id = :id");
    query.bindValue(":id", id);
    query.exec();
    StringVariantMap set;
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

void Warnings::initFilter(ListOfStringVariantMaps * map, const QString & parent, const QString & circuit_attribute, const QString & function, const QString & value)
{
    StringVariantMap set;
    set.insert("parent", parent);
    set.insert("circuit_attribute", circuit_attribute);
    set.insert("function", function);
    set.insert("value", value);
    *map << set;
}

void Warnings::initCondition(ListOfStringVariantMaps * map, const QString & parent, const QString & value_ins, const QString & function, const QString & value_nom)
{
    StringVariantMap set;
    set.insert("parent", parent);
    set.insert("value_ins", value_ins);
    set.insert("function", function);
    set.insert("value_nom", value_nom);
    *map << set;
}

Warning::Warning(int id, QSqlDatabase db):
MTSqlQueryResult(db)
{
    database = db.isValid() ? db : QSqlDatabase::database();
    this->id = id;
    prepare("SELECT id, enabled, name, description, delay FROM warnings WHERE id = :id");
    bindValue(":id", id);
    exec();
}

void Warning::saveResult()
{
    int n = query()->record().count();
    *pos() = -1;
    result()->clear();
    Warnings::initWarnings(database, result(), 0, id);
    if (id >= 1000) { return; }
    StringVariantMap row;
    while (query()->next()) {
        row.clear();
        for (int i = 0; i < n; ++i) {
            row.insert(query()->record().fieldName(i), query()->value(i));
        }
        *result() << row;
    }
}

WarningFilters::WarningFilters(int id, QSqlDatabase db):
MTSqlQueryResult(db)
{
    database = db.isValid() ? db : QSqlDatabase::database();
    this->id = id;
    prepare("SELECT parent, circuit_attribute, function, value FROM warnings_filters WHERE parent = :parent");
    bindValue(":parent", id);
    exec();
}

void WarningFilters::saveResult()
{
    int n = query()->record().count();
    *pos() = -1;
    result()->clear();
    Warnings::initWarnings(database, result(), 1, id);
    if (id >= 1000) { return; }
    StringVariantMap row;
    while (query()->next()) {
        row.clear();
        for (int i = 0; i < n; ++i) {
            row.insert(query()->record().fieldName(i), query()->value(i));
        }
        *result() << row;
    }
}

WarningConditions::WarningConditions(int id, QSqlDatabase db):
MTSqlQueryResult(db)
{
    database = db.isValid() ? db : QSqlDatabase::database();
    this->id = id;
    prepare("SELECT parent, value_ins, function, value_nom FROM warnings_conditions WHERE parent = :parent");
    bindValue(":parent", id);
    exec();
}

void WarningConditions::saveResult()
{
    int n = query()->record().count();
    *pos() = -1;
    result()->clear();
    Warnings::initWarnings(database, result(), 2, id);
    if (id >= 1000) { return; }
    StringVariantMap row;
    while (query()->next()) {
        row.clear();
        for (int i = 0; i < n; ++i) {
            row.insert(query()->record().fieldName(i), query()->value(i));
        }
        *result() << row;
    }
}

bool MTWebPage::acceptNavigationRequest(QWebFrame *, const QNetworkRequest & request, QWebPage::NavigationType type)
{
    if (type == NavigationTypeLinkClicked || type == NavigationTypeOther) {
        switch (linkDelegationPolicy()) {
            case DontDelegateLinks:
                return true;

            case DelegateExternalLinks:
            case DelegateAllLinks:
                emit linkClicked(request.url());
                return false;
        }
    }
    return true;
}

Highlighter::Highlighter(QStringList used_ids, QTextDocument * parent):
QSyntaxHighlighter(parent)
{
    HighlightingRule rule;
    keywordFormat.setFontWeight(QFont::Bold);
    QStringList keywordPatterns;
    for (int i = 0; i < used_ids.count(); ++i) {
        keywordPatterns << QString("\\b%1\\b").arg(used_ids.at(i));
    }
    foreach (QString pattern, keywordPatterns) {
        rule.pattern = QRegExp(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }
}

void Highlighter::highlightBlock(const QString & text)
{
    foreach (HighlightingRule rule, highlightingRules) {
        QRegExp expression(rule.pattern);
        int index = text.indexOf(expression);
        while (index >= 0) {
            int length = expression.matchedLength();
            setFormat(index, length, rule.format);
            index = text.indexOf(expression, index + length);
        }
    }
}
