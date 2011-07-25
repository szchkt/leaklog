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

#include "variables.h"
#include "global.h"
#include "modify_dialogue.h"
#include "input_widgets.h"

#include <QSet>
#include <QApplication>

using namespace Global;

Variables::Variables(QSqlDatabase db, bool exec_query, int scope):
    MTSqlQueryResult(db),
    m_scope(scope)
{
    if (exec_query)
        exec(QString("SELECT variables.id, variables.name, variables.type, variables.unit, variables.value, variables.compare_nom, variables.tolerance, variables.col_bg, subvariables.id, subvariables.name, subvariables.type, subvariables.unit, subvariables.value, subvariables.compare_nom, subvariables.tolerance"
             " FROM variables LEFT JOIN subvariables ON variables.id = subvariables.parent WHERE variables.scope & %1 > 0").arg(scope));
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
    initVariable(filter, "t_sec", Variable::Inspection, "mintcream");
    initSubvariable(filter, "t_sec", Variable::Inspection, "mintcream", "t_sec_evap_in", QApplication::translate("Units", "%1C").arg(degreeSign()), "", true, 0.0);
    initSubvariable(filter, "t_sec", Variable::Inspection, "mintcream", "t_sec_cond_in", QApplication::translate("Units", "%1C").arg(degreeSign()), "", true, 0.0);

    initVariable(filter, "p_0", Variable::Inspection | Variable::Compressor, QApplication::translate("Units", "Bar"), "", true, 0.0, "aliceblue");
    initVariable(filter, "t_0", Variable::Inspection, QApplication::translate("Units", "%1C").arg(degreeSign()), "p_to_t(p_0)", true, 0.0, "aliceblue");
    initVariable(filter, "delta_t_evap", Variable::Inspection, QApplication::translate("Units", "%1C").arg(degreeSign()), "abs(t_sec_evap_in-p_to_t(p_0))", true, 0.0, "aliceblue");
    initVariable(filter, "t_evap_out", Variable::Inspection, QApplication::translate("Units", "%1C").arg(degreeSign()), "", true, 0.0, "aliceblue");
    initVariable(filter, "t_comp_in", Variable::Inspection, QApplication::translate("Units", "%1C").arg(degreeSign()), "", true, 0.0, "aliceblue");

    initVariable(filter, "t_sh", Variable::Inspection, "aliceblue");
    initSubvariable(filter, "t_sh", Variable::Inspection, "aliceblue", "t_sh_evap", QApplication::translate("Units", "%1C").arg(degreeSign()), "t_evap_out-p_to_t(p_0)", true, 0.0);
    initSubvariable(filter, "t_sh", Variable::Inspection, "aliceblue", "t_sh_comp", QApplication::translate("Units", "%1C").arg(degreeSign()), "t_comp_in-p_to_t(p_0)", true, 0.0);

    initVariable(filter, "p_c", Variable::Inspection | Variable::Compressor, QApplication::translate("Units", "Bar"), "", true, 0.0, "floralwhite");
    initVariable(filter, "t_c", Variable::Inspection, QApplication::translate("Units", "%1C").arg(degreeSign()), "p_to_t(p_c)", true, 0.0, "floralwhite");
    initVariable(filter, "delta_t_c", Variable::Inspection, QApplication::translate("Units", "%1C").arg(degreeSign()), "abs(t_sec_cond_in-p_to_t(p_c))", true, 0.0, "floralwhite");
    initVariable(filter, "t_ev", Variable::Inspection, QApplication::translate("Units", "%1C").arg(degreeSign()), "", true, 0.0, "floralwhite");
    initVariable(filter, "t_sc", Variable::Inspection, QApplication::translate("Units", "%1C").arg(degreeSign()), "p_to_t(p_c)-t_ev", true, 0.0, "floralwhite");
    initVariable(filter, "t_comp_out", Variable::Inspection | Variable::Compressor, QApplication::translate("Units", "%1C").arg(degreeSign()), "", true, 0.0, "floralwhite");
    initVariable(filter, "ep_comp", Variable::Compressor, QApplication::translate("Units", "kW"), "", true, 0.0, "");

    initVariable(filter, "ec", Variable::Compressor, "");
    initSubvariable(filter, "ec", Variable::Compressor, "", "ec_l1", QApplication::translate("Units", "A"), "", true, 0.0);
    initSubvariable(filter, "ec", Variable::Compressor, "", "ec_l2", QApplication::translate("Units", "A"), "", true, 0.0);
    initSubvariable(filter, "ec", Variable::Compressor, "", "ec_l3", QApplication::translate("Units", "A"), "", true, 0.0);

    initVariable(filter, "ev", Variable::Compressor, "");
    initSubvariable(filter, "ev", Variable::Compressor, "", "ev_l1", QApplication::translate("Units", "V"), "", true, 0.0);
    initSubvariable(filter, "ev", Variable::Compressor, "", "ev_l2", QApplication::translate("Units", "V"), "", true, 0.0);
    initSubvariable(filter, "ev", Variable::Compressor, "", "ev_l3", QApplication::translate("Units", "V"), "", true, 0.0);

    initVariable(filter, "ppsw", Variable::Inspection, "");
    initSubvariable(filter, "ppsw", Variable::Inspection, "", "ppsw_hip", QApplication::translate("Units", "Bar"), "", true, 0.0);
    initSubvariable(filter, "ppsw", Variable::Inspection, "", "ppsw_lop", QApplication::translate("Units", "Bar"), "", true, 0.0);
    initSubvariable(filter, "ppsw", Variable::Inspection, "", "ppsw_diff", QApplication::translate("Units", "Bar"), "", true, 0.0);

    initVariable(filter, "sftsw", Variable::Inspection, QApplication::translate("Units", "Bar"), "", true, 0.0, "");
    initVariable(filter, "rmds", Variable::Inspection, "", "", false, 0.0, "");
    initVariable(filter, "arno", Variable::Inspection, "", "", false, 0.0, "");
    initVariable(filter, "ar_type", Variable::Inspection, "", "", false, 0.0, "");

    initVariable(filter, "vis_aur_chk", Variable::Inspection, "");
    initSubvariable(filter, "vis_aur_chk", Variable::Inspection, "", "corr_def", "", "", false, 0.0);
    initSubvariable(filter, "vis_aur_chk", Variable::Inspection, "", "noise_vibr", "", "", false, 0.0);
    initSubvariable(filter, "vis_aur_chk", Variable::Inspection, "", "bbl_lvl", "", "", false, 0.0);
    initSubvariable(filter, "vis_aur_chk", Variable::Inspection, "", "oil_leak_am", QApplication::translate("Units", "kg"), "", false, 0.0);

    initVariable(filter, "dir_leak_chk", Variable::Inspection, "green");
    initSubvariable(filter, "dir_leak_chk", Variable::Inspection, "green", "el_detect", "", "", false, 0.0);
    initSubvariable(filter, "dir_leak_chk", Variable::Inspection, "green", "uv_detect", "", "", false, 0.0);
    initSubvariable(filter, "dir_leak_chk", Variable::Inspection, "green", "bbl_detect", "", "", false, 0.0);

    initVariable(filter, "refr_add_am", Variable::Inspection, QApplication::translate("Units", "kg"), "", false, 0.0, "yellow");
    initVariable(filter, "refr_reco", Variable::Inspection, QApplication::translate("Units", "kg"), "", false, 0.0, "yellow");
    initVariable(filter, "refr_add_per", Variable::Inspection, QApplication::translate("Units", "%"), "(1-nominal)*100*(sum(refr_add_am)-sum(refr_reco))/refrigerant_amount", false, 0.0, "yellow");

    initVariable(filter, "inspector", Variable::Inspection, "", "", false, 0.0, "");
    initVariable(filter, "operator", Variable::Inspection, "", "", false, 0.0, "");
}

void Variables::initVariable(const QString & filter, const QString & id, int scope, const QString & unit, const QString & value, bool compare_nom, double tolerance, const QString & col_bg)
{
    if (scope > 0 && !(scope & m_scope)) { return; }
    if (!filter.isEmpty() && filter != id) { return; }
    QVariantMap row;
    row.insert("VAR_ID", id);
    row.insert("VAR_NAME", variableNames().value(id));
    row.insert("VAR_TYPE", variableType(id));
    row.insert("VAR_UNIT", unit);
    row.insert("VAR_VALUE", value);
    row.insert("VAR_COMPARE_NOM", compare_nom ? 1 : 0);
    row.insert("VAR_TOLERANCE", tolerance);
    row.insert("VAR_COL_BG", col_bg);
    *result() << row;
    var_indices.insert(id, result()->count() - 1);
}

void Variables::initVariable(const QString & filter, const QString & id, int scope, const QString & col_bg)
{
    if (scope > 0 && !(scope & m_scope)) { return; }
    if (filter.isEmpty() || filter != id) { return; }
    QVariantMap row;
    row.insert("VAR_ID", id);
    row.insert("VAR_NAME", variableNames().value(id));
    row.insert("VAR_COL_BG", col_bg);
    *result() << row;
    var_indices.insert(id, result()->count() - 1);
}

void Variables::initSubvariable(const QString & filter, const QString & parent, int scope, const QString & col_bg, const QString & id, const QString & unit, const QString & value, bool compare_nom, double tolerance)
{
    if (scope > 0 && !(scope & m_scope)) { return; }
    if (!filter.isEmpty() && filter != id) { return; }
    QVariantMap row;
    row.insert("VAR_ID", parent);
    row.insert("VAR_NAME", variableNames().value(parent));
    row.insert("VAR_COL_BG", col_bg);
    row.insert("SUBVAR_ID", id);
    row.insert("SUBVAR_NAME", variableNames().value(id));
    row.insert("SUBVAR_TYPE", variableType(id));
    row.insert("SUBVAR_UNIT", unit);
    row.insert("SUBVAR_VALUE", value);
    row.insert("SUBVAR_COMPARE_NOM", compare_nom ? 1 : 0);
    row.insert("SUBVAR_TOLERANCE", tolerance);
    *result() << row;
    var_indices.insert(parent, result()->count() - 1);
    var_indices.insert(id, result()->count() - 1);
}

void Variables::initModifyDialogueWidgets(ModifyDialogueWidgets * md, const QVariantMap & attributes, MTRecord * mt_record, const QDateTime & date, MDCheckBox * chb_repair, MDCheckBox * chb_nominal)
{
    QString var_id, var_name, var_type, subvar_id, subvar_name, subvar_type;
    MDAbstractInputWidget * iw = NULL;
    while (next()) {
        var_id = value("VAR_ID").toString();
        subvar_id = value("SUBVAR_ID").toString();
        if (subvar_id.isEmpty()) {
            if (!value("VAR_VALUE").toString().isEmpty()) { continue; }
            var_name = tr("%1:").arg(value("VAR_NAME").toString());
            var_type = value("VAR_TYPE").toString();
            if (var_id == "inspector") {
                iw = new MDComboBox(var_id, var_name, md->widget(),
                                    attributes.value(var_id).toString(), listInspectors(), value("VAR_COL_BG").toString());
                iw->label()->setAlternativeText(tr("Repairman:"));
                if (chb_repair) {
                    iw->label()->toggleAlternativeText(chb_repair->isChecked());
                    iw->label()->addConnection(chb_repair, SIGNAL(toggled(bool)), SLOT(toggleAlternativeText(bool)));
                }
                md->addInputWidget(iw);
            } else if (var_id == "rmds") {
                iw = new MDPlainTextEdit(var_id, var_name, md->widget(),
                                         attributes.value(var_id).toString(), value("VAR_COL_BG").toString());
                iw->setShowInForm(false);
                md->addInputWidget(iw);
            } else if (var_id == "operator") {
                if (mt_record) {
                    iw = new MDComboBox(var_id, var_name, md->widget(),
                                        attributes.value(var_id).toString(), listOperators(mt_record->parent("customer")), value("VAR_COL_BG").toString());
                    md->addInputWidget(iw);
                }
            } else if (var_id == "ar_type") {
                iw = new MDComboBox(var_id, var_name, md->widget(),
                                    attributes.value(var_id).toString(), listAssemblyRecordTypes(), value("VAR_COL_BG").toString());
                iw->setShowInForm(false);
                md->addInputWidget(iw);
            } else if (var_type == "int") {
                md->addInputWidget(new MDSpinBox(var_id, var_name, md->widget(), -999999999, 999999999,
                                                 attributes.value(var_id).toInt(), value("VAR_UNIT").toString(), value("VAR_COL_BG").toString()));
            } else if (var_type == "float") {
                iw = new MDNullableDoubleSpinBox(var_id, var_name, md->widget(), -999999999.9, 999999999.9,
                                                 attributes.value(var_id), value("VAR_UNIT").toString(), value("VAR_COL_BG").toString());
                if (var_id == "refr_add_am") {
                    iw->label()->setAlternativeText(tr("New charge:"));
                    if (chb_nominal) {
                        iw->label()->toggleAlternativeText(chb_nominal->isChecked());
                        iw->label()->addConnection(chb_nominal, SIGNAL(toggled(bool)), SLOT(toggleAlternativeText(bool)));
                    }
                }
                md->addInputWidget(iw);
            } else if (var_type == "string") {
                iw = new MDLineEdit(var_id, var_name, md->widget(),
                                    attributes.value(var_id).toString(), 0, value("VAR_COL_BG").toString());
                if (var_id == "arno") {
                    if (mt_record)
                        if (mt_record->id().isEmpty()) iw->setVariantValue(QString("%1-%2-%3").arg(mt_record->parent("customer")).arg(mt_record->parent("circuit")).arg(date.toString("yyMMdd")));
                    iw->setShowInForm(false);
                }
                md->addInputWidget(iw);
            } else if (var_type == "text") {
                md->addInputWidget(new MDPlainTextEdit(var_id, var_name, md->widget(),
                                                       attributes.value(var_id).toString(), value("VAR_COL_BG").toString()));
            } else if (var_type == "bool") {
                iw = new MDCheckBox(var_id, "", md->widget(), attributes.value(var_id).toInt());
                iw->label()->setLabelText(var_name);
                md->addInputWidget(iw);
            } else {
                md->addInputWidget(new MDLineEdit(var_id, var_name, md->widget(),
                                                  attributes.value(var_id).toString(), 0, value("VAR_COL_BG").toString()));
            }
        } else {
            if (!value("SUBVAR_VALUE").toString().isEmpty()) { continue; }
            subvar_name = tr("%1: %2:").arg(value("VAR_NAME").toString()).arg(value("SUBVAR_NAME").toString());
            subvar_type = value("SUBVAR_TYPE").toString();
            if (subvar_type == "int") {
                md->addInputWidget(new MDSpinBox(subvar_id, subvar_name, md->widget(), -999999999, 999999999,
                                                 attributes.value(subvar_id).toInt(), value("SUBVAR_UNIT").toString(), value("VAR_COL_BG").toString()));
            } else if (subvar_type == "float") {
                md->addInputWidget(new MDNullableDoubleSpinBox(subvar_id, subvar_name, md->widget(), -999999999.9, 999999999.9,
                                                               attributes.value(subvar_id), value("SUBVAR_UNIT").toString(), value("VAR_COL_BG").toString()));
            } else if (subvar_type == "string") {
                md->addInputWidget(new MDLineEdit(subvar_id, subvar_name, md->widget(),
                                                  attributes.value(subvar_id).toString(), 0, value("VAR_COL_BG").toString()));
            } else if (subvar_type == "text") {
                md->addInputWidget(new MDPlainTextEdit(subvar_id, subvar_name, md->widget(),
                                                       attributes.value(subvar_id).toString(), value("VAR_COL_BG").toString()));
            } else if (subvar_type == "bool") {
                iw = new MDCheckBox(subvar_id, value("SUBVAR_NAME").toString(), md->widget(),
                                    attributes.value(subvar_id).toInt());
                iw->label()->setLabelText(tr("%1:").arg(value("VAR_NAME").toString()));
                md->addInputWidget(iw);
            } else {
                md->addInputWidget(new MDLineEdit(subvar_id, subvar_name, md->widget(),
                                                  attributes.value(subvar_id).toString(), 0, value("VAR_COL_BG").toString()));
            }
        }
    }
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
