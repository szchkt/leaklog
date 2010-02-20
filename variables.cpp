/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2010 Matus & Michal Tomlein

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

#include "variables.h"
#include "global.h"

#include <QSet>
#include <QApplication>

using namespace Global;

Variables::Variables(QSqlDatabase db, bool exec_query):
MTSqlQueryResult(db)
{
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
    QVariantMap row;
    QSet<int> updated_indices;
    while (query()->next()) {
        row.clear();
        insert = true;
        if (variableNames().contains(query()->value(VAR_ID).toString())) {
            QSet<int> indices = var_indices.values(query()->value(VAR_ID).toString()).toSet();
            indices.unite(var_indices.values(query()->value(SUBVAR_ID).toString()).toSet());
            foreach (int index, indices) {
                insert = false;
                QVariantMap _row = result()->at(index);
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
    initVariable(filter, "t_sec", "mintcream");
    initSubvariable(filter, "t_sec", "mintcream", "t_sec_evap_in", "float", QApplication::translate("Units", "%1C").arg(degreeSign()), "", true, 0.0);
    initSubvariable(filter, "t_sec", "mintcream", "t_sec_cond_in", "float", QApplication::translate("Units", "%1C").arg(degreeSign()), "", true, 0.0);

    initVariable(filter, "p_0", "float", QApplication::translate("Units", "Bar"), "", true, 0.0, "aliceblue");
    initVariable(filter, "t_0", "float", QApplication::translate("Units", "%1C").arg(degreeSign()), "p_to_t(p_0)", true, 0.0, "aliceblue");
    initVariable(filter, "delta_t_evap", "float", QApplication::translate("Units", "%1C").arg(degreeSign()), "abs(t_sec_evap_in-p_to_t(p_0))", true, 0.0, "aliceblue");
    initVariable(filter, "t_evap_out", "float", QApplication::translate("Units", "%1C").arg(degreeSign()), "", true, 0.0, "aliceblue");
    initVariable(filter, "t_comp_in", "float", QApplication::translate("Units", "%1C").arg(degreeSign()), "", true, 0.0, "aliceblue");

    initVariable(filter, "t_sh", "aliceblue");
    initSubvariable(filter, "t_sh", "aliceblue", "t_sh_evap", "float", QApplication::translate("Units", "%1C").arg(degreeSign()), "t_evap_out-p_to_t(p_0)", true, 0.0);
    initSubvariable(filter, "t_sh", "aliceblue", "t_sh_comp", "float", QApplication::translate("Units", "%1C").arg(degreeSign()), "t_comp_in-p_to_t(p_0)", true, 0.0);

    initVariable(filter, "p_c", "float", QApplication::translate("Units", "Bar"), "", true, 0.0, "floralwhite");
    initVariable(filter, "t_c", "float", QApplication::translate("Units", "%1C").arg(degreeSign()), "p_to_t(p_c)", true, 0.0, "floralwhite");
    initVariable(filter, "delta_t_c", "float", QApplication::translate("Units", "%1C").arg(degreeSign()), "abs(t_sec_cond_in-p_to_t(p_c))", true, 0.0, "floralwhite");
    initVariable(filter, "t_ev", "float", QApplication::translate("Units", "%1C").arg(degreeSign()), "", true, 0.0, "floralwhite");
    initVariable(filter, "t_sc", "float", QApplication::translate("Units", "%1C").arg(degreeSign()), "p_to_t(p_c)-t_ev", true, 0.0, "floralwhite");
    initVariable(filter, "t_comp_out", "float", QApplication::translate("Units", "%1C").arg(degreeSign()), "", true, 0.0, "floralwhite");
    initVariable(filter, "ep_comp", "float", QApplication::translate("Units", "kW"), "", true, 0.0, "");

    initVariable(filter, "ec", "");
    initSubvariable(filter, "ec", "", "ec_l1", "float", QApplication::translate("Units", "A"), "", true, 0.0);
    initSubvariable(filter, "ec", "", "ec_l2", "float", QApplication::translate("Units", "A"), "", true, 0.0);
    initSubvariable(filter, "ec", "", "ec_l3", "float", QApplication::translate("Units", "A"), "", true, 0.0);

    initVariable(filter, "ev", "");
    initSubvariable(filter, "ev", "", "ev_l1", "float", QApplication::translate("Units", "V"), "", true, 0.0);
    initSubvariable(filter, "ev", "", "ev_l2", "float", QApplication::translate("Units", "V"), "", true, 0.0);
    initSubvariable(filter, "ev", "", "ev_l3", "float", QApplication::translate("Units", "V"), "", true, 0.0);

    initVariable(filter, "ppsw", "");
    initSubvariable(filter, "ppsw", "", "ppsw_hip", "float", QApplication::translate("Units", "Bar"), "", true, 0.0);
    initSubvariable(filter, "ppsw", "", "ppsw_lop", "float", QApplication::translate("Units", "Bar"), "", true, 0.0);
    initSubvariable(filter, "ppsw", "", "ppsw_diff", "float", QApplication::translate("Units", "Bar"), "", true, 0.0);

    initVariable(filter, "sftsw", "float", QApplication::translate("Units", "Bar"), "", true, 0.0, "");
    initVariable(filter, "rmds", "text", "", "", false, 0.0, "");
    initVariable(filter, "arno", "string", "", "", false, 0.0, "");

    initVariable(filter, "vis_aur_chk", "");
    initSubvariable(filter, "vis_aur_chk", "", "corr_def", "bool", "", "", false, 0.0);
    initSubvariable(filter, "vis_aur_chk", "", "noise_vibr", "bool", "", "", false, 0.0);
    initSubvariable(filter, "vis_aur_chk", "", "bbl_lvl", "bool", "", "", false, 0.0);
    initSubvariable(filter, "vis_aur_chk", "", "oil_leak_am", "float", QApplication::translate("Units", "kg"), "", false, 0.0);

    initVariable(filter, "dir_leak_chk", "green");
    initSubvariable(filter, "dir_leak_chk", "green", "el_detect", "bool", "", "", false, 0.0);
    initSubvariable(filter, "dir_leak_chk", "green", "uv_detect", "bool", "", "", false, 0.0);
    initSubvariable(filter, "dir_leak_chk", "green", "bbl_detect", "bool", "", "", false, 0.0);

    initVariable(filter, "refr_add_am", "float", QApplication::translate("Units", "kg"), "", false, 0.0, "yellow");
    initVariable(filter, "refr_reco", "float", QApplication::translate("Units", "kg"), "", false, 0.0, "yellow");
    initVariable(filter, "refr_add_per", "float", QApplication::translate("Units", "%"), "(1-nominal)*100*(sum(refr_add_am)-sum(refr_reco))/refrigerant_amount", false, 0.0, "yellow");

    initVariable(filter, "inspector", "string", "", "", false, 0.0, "");
    initVariable(filter, "operator", "string", "", "", false, 0.0, "");
}

void Variables::initVariable(const QString & filter, const QString & id, const QString & type, const QString & unit, const QString & value, bool compare_nom, double tolerance, const QString & col_bg)
{
    if (!filter.isEmpty() && filter != id) { return; }
    QVariantMap row;
    row.insert("VAR_ID", id);
    row.insert("VAR_NAME", variableNames().value(id));
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
    QVariantMap row;
    row.insert("VAR_ID", id);
    row.insert("VAR_NAME", variableNames().value(id));
    row.insert("VAR_COL_BG", col_bg);
    *result() << row;
    var_indices.insert(id, result()->count() - 1);
}

void Variables::initSubvariable(const QString & filter, const QString & parent, const QString & col_bg, const QString & id, const QString & type, const QString & unit, const QString & value, bool compare_nom, double tolerance)
{
    if (!filter.isEmpty() && filter != id) { return; }
    QVariantMap row;
    row.insert("VAR_ID", parent);
    row.insert("VAR_NAME", variableNames().value(parent));
    row.insert("VAR_COL_BG", col_bg);
    row.insert("SUBVAR_ID", id);
    row.insert("SUBVAR_NAME", variableNames().value(id));
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
    QVariantMap row;
    while (query()->next()) {
        row.clear();
        insert = true;
        if (variableNames().contains(query()->value(VAR_ID).toString())) {
            int index = var_indices.value(query()->value(VAR_ID).toString(), -1);
            if (index < 0) { insert = true; }
            else {
                insert = false;
                QVariantMap _row = result()->at(index);
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
    QVariantMap row;
    while (query()->next()) {
        row.clear();
        insert = true;
        if (variableNames().contains(query()->value(SUBVAR_ID).toString())) {
            int index = var_indices.value(query()->value(SUBVAR_ID).toString(), -1);
            if (index < 0) { insert = true; }
            else {
                insert = false;
                QVariantMap _row = result()->at(index);
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
