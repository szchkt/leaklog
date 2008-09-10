/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008 Matus & Michal Tomlein

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

#include <cmath>

QString Global::toString(const QVariant & v) { return v.toString(); }

/*QString Global::escapeDoubleQuotes(const QString & s)
{
    QString r = s;
    r.replace("\\", "\\\\");
    r.replace("\"", "\\\"");
    return r;
}*/

QString Global::upArrow() { return QApplication::translate("Global", "\342\206\221", 0, QApplication::UnicodeUTF8); }

QString Global::downArrow() { return QApplication::translate("Global", "\342\206\223", 0, QApplication::UnicodeUTF8); }

QString Global::degreeSign() { return QApplication::translate("Global", "\302\260", 0, QApplication::UnicodeUTF8); }

QString Global::delta() { return QApplication::translate("Global", "\316\224", 0, QApplication::UnicodeUTF8); }

void Global::copyTable(const QString & table, QSqlDatabase * from, QSqlDatabase * to, const QString & filter)
{
    QSqlQuery select("SELECT * FROM " + table + QString(filter.isEmpty() ? "" : (" WHERE " + filter)), *from);
    if (select.next() && select.record().count()) {
        QString copy("INSERT INTO " + table + " (");
        QStringList field_names = getTableFieldNames(table, to);
        QString field_name;
        for (int i = 0; i < select.record().count(); ++i) {
            field_name = select.record().fieldName(i);
            copy.append(i == 0 ? "" : ", ");
            copy.append(field_name);
            if (!field_names.contains(field_name)) {
                addColumn(field_name, table, to);
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

QStringList Global::getTableFieldNames(const QString & table, QSqlDatabase * database)
{
    QStringList field_names;
    QSqlQuery query("SELECT * FROM " + table, *database);
    for (int i = 0; i < query.record().count(); ++i) {
        field_names << query.record().fieldName(i);
    }
    return field_names;
}

void Global::addColumn(const QString & column, const QString & table, QSqlDatabase * database)
{
    QSqlQuery add_column("ALTER TABLE " + table + " ADD COLUMN " + column + " TEXT", *database);
}

void Global::renameColumn(const QString & column, const QString & new_name, const QString & table, QSqlDatabase * database)
{
    QStringList all_field_names = getTableFieldNames(table, database);
    all_field_names.removeAll(column);
    QString field_names = all_field_names.join(", ");
    QSqlQuery create_temp(QString("CREATE TEMPORARY TABLE _tmp (%1, _tmpcol)").arg(field_names));
    QSqlQuery copy_to_temp(QString("INSERT INTO _tmp SELECT %1, %2 FROM %3").arg(field_names).arg(column).arg(table));
    QSqlQuery drop_table(QString("DROP TABLE %1").arg(table));
    QSqlQuery recreate_table(QString("CREATE TABLE %1 (%2, %3)").arg(table).arg(field_names).arg(new_name));
    QSqlQuery copy_from_temp(QString("INSERT INTO %1 SELECT %2, _tmpcol FROM _tmp").arg(table).arg(field_names));
    QSqlQuery drop_temp(QString("DROP TABLE _tmp"));
}

void Global::dropColumn(const QString & column, const QString & table, QSqlDatabase * database)
{
    QStringList all_field_names = getTableFieldNames(table, database);
    all_field_names.removeAll(column);
    QString field_names = all_field_names.join(", ");
    QSqlQuery create_temp(QString("CREATE TEMPORARY TABLE _tmp (%1)").arg(field_names));
    QSqlQuery copy_to_temp(QString("INSERT INTO _tmp SELECT %1 FROM %2").arg(field_names).arg(table));
    QSqlQuery drop_table(QString("DROP TABLE %1").arg(table));
    QSqlQuery recreate_table(QString("CREATE TABLE %1 (%2)").arg(table).arg(field_names));
    QSqlQuery copy_from_temp(QString("INSERT INTO %1 SELECT %2 FROM _tmp").arg(table).arg(field_names));
    QSqlQuery drop_temp(QString("DROP TABLE _tmp"));
}

MTDictionary Global::parseExpression(const QString & exp, QStringList * used_ids)
{
    MTDictionary dict_exp(true);
    if (!exp.isEmpty()) {
        if (!used_ids->contains("refrigerant_amount")) { *used_ids << "refrigerant_amount"; }
        if (!used_ids->contains("oil_amount")) { *used_ids << "oil_amount"; }
        if (!used_ids->contains("sum")) { *used_ids << "sum"; }
        QSet<int> matched;
        for (int i = 0; i < used_ids->count(); ++i) {
            QRegExp expression(QString("\\b%1\\b").arg(used_ids->at(i)));
            int index = exp.indexOf(expression);
            while (index >= 0) {
                int length = expression.matchedLength();
                if (!matched.contains(index)) {
                    for (int j = index; j < index + length; j++) { matched << j; }
                }
                index = exp.indexOf(expression, index + length);
            }
        }
        QString id_, f_; bool last_id = false; bool last_sum = false;
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
                    if (id_ == "sum") {
                        last_sum = true;
                    } else {
                        if (id_ == "refrigerant_amount" || id_ == "oil_amount") {
                            dict_exp.insert(id_, "circuit_attribute");
                        } else {
                            dict_exp.insert(id_, last_sum ? "sum" : "id");
                        }
                        last_sum = false;
                    }
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
            if (id_ == "refrigerant_amount" || id_ == "oil_amount") {
                dict_exp.insert(id_, "circuit_attribute");
            } else {
                dict_exp.insert(id_, last_sum ? "sum" : "id");
            }
        }
    }
    return dict_exp;
}

double Global::evaluateExpression(QMap<QString, QVariant> & inspection, const MTDictionary & expression, const QString & customer_id, const QString & circuit_id, bool * ok)
{
    QString inspection_date = inspection.value("date").toString();
    FunctionParser fparser;
    const QString sum_query("SELECT SUM(%1) FROM inspections WHERE date LIKE :year AND customer = :customer_id AND circuit = :circuit_id AND nominal = 0");
    MTRecord circuit("circuit", circuit_id, MTDictionary("parent", customer_id));
    QMap<QString, QVariant> circuit_attributes = circuit.list();
    QString value;
    for (int i = 0; i < expression.count(); ++i) {
        if (expression.value(i) == "id") {
            value.append(inspection.value(expression.key(i)).toString());
        } else if (expression.value(i) == "sum") {
            if (inspection.value("nominal").toInt()) {
                value.append(inspection.value(expression.key(i)).toString());
                continue;
            }
            QSqlQuery sum_ins;
            sum_ins.prepare(sum_query.arg(expression.key(i)));
            sum_ins.bindValue(":customer_id", customer_id);
            sum_ins.bindValue(":circuit_id", circuit_id);
            sum_ins.bindValue(":year", QString("%1%").arg(inspection_date.left(4)));
            if (sum_ins.exec() && sum_ins.next()) {
                value.append(sum_ins.value(0).toString());
            }
        } else if (expression.value(i) == "circuit_attribute") {
            value.append(circuit_attributes.value(expression.key(i)).toString());
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
    return (double)(round(result * 100.0)/100.0);
}

QString Global::compareValues(double value1, double value2)
{
    if (value1 < value2) {
		return "<table class=\"no_border\" cellpadding=\"0\" cellspacing=\"0\"><tr><td class=\"no_border\" width=\"1%\" align=\"right\" valign=\"center\" style=\"font-size: large\">" + upArrow() + "</td><td class=\"no_border\" valign=\"center\">%1</td></tr></table>";
	} else if (value1 > value2) {
		return "<table class=\"no_border\" cellpadding=\"0\" cellspacing=\"0\"><tr><td class=\"no_border\" width=\"1%\" align=\"right\" valign=\"center\" style=\"font-size: large\">" + downArrow() + "</td><td class=\"no_border\" valign=\"center\">%1</td></tr></table>";
	} else {
		return "%1";
	}
}

MTDictionary Global::get_dict_vartypes()
{
    MTDictionary dict_vartypes;
    dict_vartypes.insert("int", QApplication::translate("VariableTypes", "Integer"));
    dict_vartypes.insert("float", QApplication::translate("VariableTypes", "Real number"));
    dict_vartypes.insert("string", QApplication::translate("VariableTypes", "String"));
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
    dict_varnames.insert("delta_t_evap", QApplication::translate("VariableNames", "\316\224T (evaporating)", 0, QApplication::UnicodeUTF8));
    dict_varnames.insert("delta_t_c", QApplication::translate("VariableNames", "\316\224T (condensing)", 0, QApplication::UnicodeUTF8));
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
    dict_varnames.insert("oil_leak", QApplication::translate("VariableNames", "Oil leak"));
    dict_varnames.insert("oil_leak_am", QApplication::translate("VariableNames", "Leaked"));
    dict_varnames.insert("dir_leak_chk", QApplication::translate("VariableNames", "Direct leak check (location)"));
    dict_varnames.insert("el_detect", QApplication::translate("VariableNames", "Electronic detection"));
    dict_varnames.insert("uv_detect", QApplication::translate("VariableNames", "UV detection"));
    dict_varnames.insert("bbl_detect", QApplication::translate("VariableNames", "Bubble detection"));
    dict_varnames.insert("refr_add", QApplication::translate("VariableNames", "Refrigerant addition"));
    dict_varnames.insert("refr_add_am", QApplication::translate("VariableNames", "kg"));
    dict_varnames.insert("refr_add_per", QApplication::translate("VariableNames", "%"));
    dict_varnames.insert("refr_reco", QApplication::translate("VariableNames", "Refrigerant recovery"));
    dict_varnames.insert("refr_recy", QApplication::translate("VariableNames", "Refrigerant recycling"));
    dict_varnames.insert("refr_disp", QApplication::translate("VariableNames", "Refrigerant disposal"));
    dict_varnames.insert("inspector", QApplication::translate("VariableNames", "Inspector"));
    dict_varnames.insert("operator", QApplication::translate("VariableNames", "Operator"));
    return dict_varnames;
}

MTDictionary Global::get_dict_attrvalues()
{
    MTDictionary dict_attrvalues;
    dict_attrvalues.insert("field::car", QApplication::translate("AttributeValues", "Car air conditioning"));
    dict_attrvalues.insert("field::lowrise", QApplication::translate("AttributeValues", "Low-rise residential buildings"));
    dict_attrvalues.insert("field::highrise", QApplication::translate("AttributeValues", "High-rise residential buildings"));
    dict_attrvalues.insert("field::commercial", QApplication::translate("AttributeValues", "Commercial buildings"));
    dict_attrvalues.insert("field::institutional", QApplication::translate("AttributeValues", "Institutional buildings"));
    dict_attrvalues.insert("field::industrial", QApplication::translate("AttributeValues", "Industrial spaces"));
    dict_attrvalues.insert("field::transportation", QApplication::translate("AttributeValues", "Transportation"));
    dict_attrvalues.insert("field::airconditioning", QApplication::translate("AttributeValues", "Air conditioning"));
    dict_attrvalues.insert("field::heatpumps", QApplication::translate("AttributeValues", "Heat pumps"));
    dict_attrvalues.insert("oil::mo", QApplication::translate("AttributeValues", "MO (Mineral oil)"));
    dict_attrvalues.insert("oil::ab", QApplication::translate("AttributeValues", "AB (Alkylbenzene oil)"));
    dict_attrvalues.insert("oil::poe", QApplication::translate("AttributeValues", "POE (Polyolester oil)"));
    dict_attrvalues.insert("oil::pao", QApplication::translate("AttributeValues", "PAO (Polyalphaolefin oil)"));
    dict_attrvalues.insert("oil::pve", QApplication::translate("AttributeValues", "PVE (Polyvinylether oil)"));
    dict_attrvalues.insert("oil::pag", QApplication::translate("AttributeValues", "PAG (Polyglycol oil)"));
    return dict_attrvalues;
}

using namespace Global;

MTRecord::MTRecord(const QString & type, const QString & id, const MTDictionary & parents)
{
    r_type = type;
    r_id = id;
    r_parents = parents;
}

MTRecord::MTRecord(const MTRecord & other):
QObject()
{
    r_type = other.r_type;
    r_id = other.r_id;
    r_parents = other.r_parents;
}

MTRecord & MTRecord::operator=(const MTRecord & other)
{
    r_type = other.r_type;
    r_id = other.r_id;
    r_parents = other.r_parents;
    return *this;
}

QString MTRecord::tableForRecordType(const QString & type)
{
    /*if (type == "customer") {
        return "customers";
    } else if (type == "circuit") {
        return "circuits";
    } else if (type == "inspection") {
        return "inspections";
    } else if (type == "variable") {
        return "variables";
    } else if (type == "subvariable") {
        return "subvariables";
    } else if (type == "table") {
        return "tables";
    } else if (type == "warning") {
        return "warnings";
    } else if (type == "inspector") {
        return "inspectors";
    } else {*/
        return type + "s";
    //}
}

bool MTRecord::exists()
{
    if (r_id.isEmpty()) { return false; }
    QString id_field = r_type == "inspection" ? "date" : "id";
    QSqlQuery find_record = select(id_field);
    find_record.exec();
    return find_record.next();
}

QSqlQuery MTRecord::select(const QString & fields)
{
    bool has_id = !r_id.isEmpty();
    QString id_field = r_type == "inspection" ? "date" : "id";
    QString select = "SELECT " + fields + " FROM " + tableForRecordType(r_type);
    if (has_id || r_parents.count()) { select.append(" WHERE "); }
    if (has_id) { select.append(id_field + " = :_id"); }
    for (int i = 0; i < r_parents.count(); ++i) {
        if (has_id || i != 0) { select.append(" AND "); }
        select.append(r_parents.key(i) + " = :" + r_parents.key(i));
    }
    select.append(" ORDER BY " + id_field);
    QSqlQuery query;
    query.prepare(select);
    if (has_id) { query.bindValue(":_id", r_id); }
    for (int i = 0; i < r_parents.count(); ++i) {
        query.bindValue(":" + r_parents.key(i), r_parents.value(i));
    }
    return query;
}

QMap<QString, QVariant> MTRecord::list(const QString & fields)
{
    QMap<QString, QVariant> list;
    QSqlQuery query = select(fields);
    query.setForwardOnly(true);
    query.exec();
    if (!query.next()) { return list; }
    for (int i = 0; i < query.record().count(); ++i) {
        list.insert(query.record().fieldName(i), query.value(i));
    }
    return list;
}

QList<QMap<QString, QVariant> > MTRecord::listAll(const QString & fields)
{
    QList<QMap<QString, QVariant> > list;
    QSqlQuery query = select(fields);
    query.setForwardOnly(true);
    query.exec();
    while (query.next()) {
        QMap<QString, QVariant> map;
        for (int i = 0; i < query.record().count(); ++i) {
            map.insert(query.record().fieldName(i), query.value(i));
        }
        list << map;
    }
    return list;
}

bool MTRecord::update(const QMap<QString, QVariant> & set, bool add_columns)
{
    bool has_id = !r_id.isEmpty();
    QString id_field = r_type == "inspection" ? "date" : "id";
    QString update;
    QMapIterator<QString, QVariant> i(set);
    if (add_columns) {
        QSqlDatabase db = QSqlDatabase::database();
        QStringList field_names = getTableFieldNames(tableForRecordType(r_type), &db);
        while (i.hasNext()) { i.next();
            if (!field_names.contains(i.key())) {
                addColumn(i.key(), tableForRecordType(r_type), &db);
            }
        }
        i.toFront();
    }
    if (has_id && !exists()) { has_id = false; }
    if (has_id) {
        update = "UPDATE " + tableForRecordType(r_type) + " SET ";
        while (i.hasNext()) { i.next();
            update.append(i.key() + " = :" + i.key());
            if (i.hasNext()) { update.append(", "); }
        }
        update.append(" WHERE " + id_field + " = :_id");
        for (int p = 0; p < r_parents.count(); ++p) {
            update.append(" AND " + r_parents.key(p) + " = :_" + r_parents.key(p));
        }
    } else {
        update = "INSERT INTO " + tableForRecordType(r_type) + " (";
        while (i.hasNext()) { i.next();
            update.append(i.key());
            if (i.hasNext() || r_parents.count()) { update.append(", "); }
        }
        for (int p = 0; p < r_parents.count(); ++p) {
            update.append(r_parents.key(p));
            if (p < r_parents.count() - 1) { update.append(", "); }
        }
        update.append(") VALUES (");
        i.toFront();
        while (i.hasNext()) { i.next();
            update.append(":" + i.key());
            if (i.hasNext() || r_parents.count()) { update.append(", "); }
        }
        for (int p = 0; p < r_parents.count(); ++p) {
            update.append(":_" + r_parents.key(p));
            if (p < r_parents.count() - 1) { update.append(", "); }
        }
        update.append(")");
    }
    QSqlQuery query;
    query.prepare(update);
    if (has_id) { query.bindValue(":_id", r_id); }
    for (int p = 0; p < r_parents.count(); ++p) {
        query.bindValue(":_" + r_parents.key(p), r_parents.value(p));
    }
    i.toFront();
    while (i.hasNext()) { i.next();
        query.bindValue(":" + i.key(), i.value());
        if (r_parents.contains(i.key())) { r_parents.setValue(i.key(), i.value().toString()); }
    }
    bool result = query.exec();
    if (has_id) {
        r_id = set.value(id_field, r_id).toString();
    } else {
        r_id = set.value(id_field, query.lastInsertId()).toString();
    }
    return result;
}

bool MTRecord::remove()
{
    if (r_id.isEmpty() && r_parents.isEmpty()) { return false; }
    bool has_id = !r_id.isEmpty();
    QString id_field = r_type == "inspection" ? "date" : "id";
    QString remove = "DELETE FROM " + tableForRecordType(r_type) + " WHERE ";
    if (has_id) { remove.append(id_field + " = :_id"); }
    for (int i = 0; i < r_parents.count(); ++i) {
        if (has_id || i != 0) { remove.append(" AND "); }
        remove.append(r_parents.key(i) + " = :" + r_parents.key(i));
    }
    QSqlQuery query;
    query.prepare(remove);
    if (has_id) { query.bindValue(":_id", r_id); }
    for (int i = 0; i < r_parents.count(); ++i) {
        query.bindValue(":" + r_parents.key(i), r_parents.value(i));
    }
    bool result = query.exec();
    if (result) { r_id.clear(); }
    return result;
}
