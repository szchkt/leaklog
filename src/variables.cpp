/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2020 Matus & Michal Tomlein

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
#include "editdialogue.h"
#include "inputwidgets.h"

#include <QSet>
#include <QApplication>

using namespace Global;

Variables::Variables(QSqlDatabase db, int scope):
    MTSqlQueryResultBase<int>(db),
    m_scope(scope)
{
    initVariables();
    exec(QString("SELECT v.parent_uuid, v.uuid, v.id, v.name, v.type, v.unit, v.scope, v.value, v.compare_nom, v.tolerance, v.col_bg,"
                 " (SELECT COUNT(c.uuid) FROM variables AS c WHERE v.uuid = c.parent_uuid) AS count_children"
                 " FROM variables AS v LEFT JOIN variables AS p ON v.parent_uuid = p.uuid"
                 " WHERE v.scope & %1 > 0 ORDER BY COALESCE(p.id, v.id), v.id").arg(scope > 0 ? scope : 0xFFFF));
}

Variables *Variables::defaultVariables(int scope)
{
    return new Variables(scope);
}

Variables::Variables(int scope):
    MTSqlQueryResultBase<int>(QSqlDatabase()),
    m_scope(scope)
{
    initVariables();
}

Variables::Variables(QSqlDatabase db, const QString &filter, int scope):
    MTSqlQueryResultBase<int>(db),
    m_scope(scope),
    m_filter(filter)
{
    initVariables();
}

VariableContract Variables::variableForID(const QString &id)
{
    return variableForUUID(createUUIDv5(ns, id));
}

VariableContract Variables::variableForUUID(const QString &uuid)
{
    int index = var_indices.value(uuid, -1);
    if (index >= 0)
        return VariableContract(result()->at(index));
    return VariableContract();
}

void Variables::saveResult()
{
    *pos() = -1;
    while (query()->next()) {
        int index = var_indices.value(query()->value(Variable::UUID).toString(), -1);
        if (index >= 0) {
            bool insert = false;
            QMap<int, QVariant> row = result()->at(index);
            if (!query()->value(Variable::CompareNom).toString().isEmpty()) {
                insert = true; row.insert(Variable::CompareNom, query()->value(Variable::CompareNom));
            }
            if (!query()->value(Variable::Tolerance).toString().isEmpty()) {
                insert = true; row.insert(Variable::Tolerance, query()->value(Variable::Tolerance));
            }
            if (!query()->value(Variable::ColBg).toString().isEmpty()) {
                insert = true; row.insert(Variable::ColBg, query()->value(Variable::ColBg));
            }
            if (insert) { result()->replace(index, row); }
        } else {
            QMap<int, QVariant> row;
            for (int i = 0; i < Variable::FieldCount; ++i)
                row.insert(i, query()->value(i));
            *result() << row;
            var_indices.insert(query()->value(Variable::UUID).toString(), result()->count() - 1);
        }
    }
}

void Variables::initVariables()
{
    ns = DBInfo::databaseUUID();

    initVariable("t_sec", Variable::Inspection, "mintcream");
    initSubvariable("t_sec", Variable::Inspection, "mintcream", "t_sec_evap_in", QApplication::translate("Units", "%1C").arg(degreeSign()), "", true, 0.0);
    initSubvariable("t_sec", Variable::Inspection, "mintcream", "t_sec_cond_in", QApplication::translate("Units", "%1C").arg(degreeSign()), "", true, 0.0);

    initVariable("p_0", Variable::Inspection, QApplication::translate("Units", "Bar"), "", true, 0.0, "aliceblue");
    initVariable("t_0", Variable::Inspection, QApplication::translate("Units", "%1C").arg(degreeSign()), "p_to_t_vap(p_0)", true, 0.0, "aliceblue");
    initVariable("delta_t_evap", Variable::Inspection, QApplication::translate("Units", "%1C").arg(degreeSign()), "abs(t_sec_evap_in-p_to_t_vap(p_0))", true, 0.0, "aliceblue");
    initVariable("t_evap_out", Variable::Inspection, QApplication::translate("Units", "%1C").arg(degreeSign()), "", true, 0.0, "aliceblue");
    initVariable("t_comp_in", Variable::Compressor, QApplication::translate("Units", "%1C").arg(degreeSign()), "", true, 0.0, "aliceblue");

    initVariable("t_sh", Variable::Inspection, "aliceblue");
    initSubvariable("t_sh", Variable::Inspection, "aliceblue", "t_sh_evap", QApplication::translate("Units", "%1C").arg(degreeSign()), "t_evap_out-p_to_t_vap(p_0)", true, 0.0);
    initSubvariable("t_sh", Variable::Inspection, "aliceblue", "t_sh_comp", QApplication::translate("Units", "%1C").arg(degreeSign()), "t_comp_in-p_to_t_vap(p_0)", true, 0.0);

    initVariable("p_c", Variable::Inspection, QApplication::translate("Units", "Bar"), "", true, 0.0, "floralwhite");
    initVariable("t_c", Variable::Inspection, QApplication::translate("Units", "%1C").arg(degreeSign()), "p_to_t(p_c)", true, 0.0, "floralwhite");
    initVariable("delta_t_c", Variable::Inspection, QApplication::translate("Units", "%1C").arg(degreeSign()), "abs(t_sec_cond_in-p_to_t(p_c))", true, 0.0, "floralwhite");
    initVariable("t_ev", Variable::Inspection, QApplication::translate("Units", "%1C").arg(degreeSign()), "", true, 0.0, "floralwhite");
    initVariable("t_sc", Variable::Inspection, QApplication::translate("Units", "%1C").arg(degreeSign()), "p_to_t(p_c)-t_ev", true, 0.0, "floralwhite");
    initVariable("t_comp_out", Variable::Compressor, QApplication::translate("Units", "%1C").arg(degreeSign()), "", true, 0.0, "floralwhite");
    initVariable("ep_comp", Variable::Compressor, QApplication::translate("Units", "kW"), "", true, 0.0, "");

    initVariable("ec", Variable::Compressor, "");
    initSubvariable("ec", Variable::Compressor, "", "ec_l1", QApplication::translate("Units", "A"), "", true, 0.0);
    initSubvariable("ec", Variable::Compressor, "", "ec_l2", QApplication::translate("Units", "A"), "", true, 0.0);
    initSubvariable("ec", Variable::Compressor, "", "ec_l3", QApplication::translate("Units", "A"), "", true, 0.0);

    initVariable("ev", Variable::Compressor, "");
    initSubvariable("ev", Variable::Compressor, "", "ev_l1", QApplication::translate("Units", "V"), "", true, 0.0);
    initSubvariable("ev", Variable::Compressor, "", "ev_l2", QApplication::translate("Units", "V"), "", true, 0.0);
    initSubvariable("ev", Variable::Compressor, "", "ev_l3", QApplication::translate("Units", "V"), "", true, 0.0);

    initVariable("ppsw", Variable::Inspection, "");
    initSubvariable("ppsw", Variable::Inspection, "", "ppsw_hip", QApplication::translate("Units", "Bar"), "", true, 0.0);
    initSubvariable("ppsw", Variable::Inspection, "", "ppsw_lop", QApplication::translate("Units", "Bar"), "", true, 0.0);
    initSubvariable("ppsw", Variable::Inspection, "", "ppsw_diff", QApplication::translate("Units", "Bar"), "", true, 0.0);

    initVariable("sftsw", Variable::Inspection, QApplication::translate("Units", "Bar"), "", true, 0.0, "");
    initVariable("risks", Variable::Inspection, "", "", false, 0.0, "");
    initVariable("rmds", Variable::Inspection, "", "", false, 0.0, "");
    initVariable("notes", Variable::Inspection, "", "", false, 0.0, "");
    initVariable("arno", Variable::Inspection, "", "", false, 0.0, "");
    initVariable("ar_type_uuid", Variable::Inspection, "", "", false, 0.0, "");

    initVariable("vis_aur_chk", Variable::Inspection, "");
    initSubvariable("vis_aur_chk", Variable::Inspection, "", "corr_def", "", "", false, 0.0);
    initSubvariable("vis_aur_chk", Variable::Inspection, "", "noise_vibr", "", "", false, 0.0);
    initSubvariable("vis_aur_chk", Variable::Inspection, "", "bbl_lvl", "", "", false, 0.0);
    initSubvariable("vis_aur_chk", Variable::Inspection, "", "oil_leak", "", "", false, 0.0);

    initVariable("oil_shortage", Variable::Compressor, "", "", false, 0.0, "");
    initVariable("noise_vibr_comp", Variable::Compressor, "", "", false, 0.0, "");
    initVariable("comp_runtime", Variable::Compressor, QApplication::translate("Units", "hours"), "", false, 0.0, "");

    initVariable("dir_leak_chk", Variable::Inspection, "green");
    initSubvariable("dir_leak_chk", Variable::Inspection, "green", "el_detect", "", "", false, 0.0);
    initSubvariable("dir_leak_chk", Variable::Inspection, "green", "uv_detect", "", "", false, 0.0);
    initSubvariable("dir_leak_chk", Variable::Inspection, "green", "bbl_detect", "", "", false, 0.0);

    initVariable("refr_add_am", Variable::Inspection, QApplication::translate("Units", "kg"), "", false, 0.0, "yellow");
    initVariable("refr_reco", Variable::Inspection, QApplication::translate("Units", "kg"), "", false, 0.0, "yellow");
    initVariable("refr_add_per", Variable::Inspection, QApplication::translate("Units", "%"), "100*(sum(refr_add_am)-sum(refr_reco))/refrigerant_amount", false, 0.0, "yellow");

    initVariable("oil_leak_am", Variable::Inspection, QApplication::translate("Units", "kg"), "", false, 0.0, "");

    initVariable("inspector_uuid", Variable::Inspection, "", "", false, 0.0, "");
    initVariable("person_uuid", Variable::Inspection, "", "", false, 0.0, "");
}

void Variables::initVariable(const QString &id, int scope, const QString &unit, const QString &value, bool compare_nom, double tolerance, const QString &col_bg)
{
    if (m_scope > 0 && !(scope & m_scope)) { return; }
    if (!m_filter.isEmpty() && m_filter != id) { return; }
    QMap<int, QVariant> row;
    QString uuid = createUUIDv5(ns, id);
    row.insert(Variable::UUID, uuid);
    row.insert(Variable::ID, id);
    row.insert(Variable::Name, variableNames().value(id));
    row.insert(Variable::Type, variableType(id));
    row.insert(Variable::Unit, unit);
    row.insert(Variable::ScopeValue, scope);
    row.insert(Variable::Value, value);
    row.insert(Variable::CompareNom, compare_nom ? 1 : 0);
    row.insert(Variable::Tolerance, tolerance);
    row.insert(Variable::ColBg, col_bg);
    *result() << row;
    var_indices.insert(uuid, result()->count() - 1);
}

void Variables::initVariable(const QString &id, int scope, const QString &col_bg)
{
    if (m_scope > 0 && !(scope & m_scope)) { return; }
    if (!m_filter.isEmpty() && m_filter != id) { return; }
    QMap<int, QVariant> row;
    QString uuid = createUUIDv5(ns, id);
    row.insert(Variable::UUID, uuid);
    row.insert(Variable::ID, id);
    row.insert(Variable::Name, variableNames().value(id));
    row.insert(Variable::Type, "group");
    row.insert(Variable::ScopeValue, scope);
    row.insert(Variable::ColBg, col_bg);
    *result() << row;
    var_indices.insert(uuid, result()->count() - 1);
}

void Variables::initSubvariable(const QString &parent, int scope, const QString &col_bg, const QString &id, const QString &unit, const QString &value, bool compare_nom, double tolerance)
{
    if (m_scope > 0 && !(scope & m_scope)) { return; }
    if (!m_filter.isEmpty() && m_filter != id) { return; }
    QMap<int, QVariant> row;
    row.insert(Variable::ParentUUID, createUUIDv5(ns, parent));
    QString uuid = createUUIDv5(ns, id);
    row.insert(Variable::UUID, uuid);
    row.insert(Variable::ID, id);
    row.insert(Variable::Name, variableNames().value(id));
    row.insert(Variable::Type, variableType(id));
    row.insert(Variable::Unit, unit);
    row.insert(Variable::ScopeValue, scope);
    row.insert(Variable::Value, value);
    row.insert(Variable::CompareNom, compare_nom ? 1 : 0);
    row.insert(Variable::Tolerance, tolerance);
    row.insert(Variable::ColBg, col_bg);
    *result() << row;
    var_indices.insert(uuid, result()->count() - 1);
}

void Variables::initEditDialogueWidgets(EditDialogueWidgets *md, const QVariantMap &attributes, Inspection *inspection,
                                        const QDateTime &date, MDComboBox *cb_nominal)
{
    while (next()) {
        QString var_type = type();

        if (!valueExpression().isEmpty() || var_type == "group")
            continue;

        QString parent_uuid = parentUUID();
        VariableContract parent;
        QString var_id = id();
        QString var_name = QApplication::translate("MainWindow", "%1:").arg(name());
        QString col_bg;
        if (parent_uuid.isEmpty()) {
            col_bg = colBg();
        } else {
            parent = variableForUUID(parent_uuid);
            md->addInputWidgetGroup(parent_uuid, parent.name());
            col_bg = parent.colBg();
        }

        MDAbstractInputWidget *iw = NULL;

        if (var_id == "inspector_uuid") {
            iw = new MDComboBox(var_id, var_name, md->widget(),
                                attributes.value(var_id).toString(), listInspectors(), col_bg);
            md->addInputWidget(iw);
        } else if (var_id == "risks" || var_id == "rmds" || var_id == "notes") {
            iw = new MDPlainTextEdit(var_id, var_name, md->widget(),
                                     attributes.value(var_id).toString(), col_bg);
            iw->setRowSpan(0);
            md->addInputWidget(iw);
        } else if (var_id == "person_uuid") {
            if (inspection) {
                iw = new MDComboBox(var_id, var_name, md->widget(),
                                    attributes.value(var_id).toString(), listOperators(inspection->customerUUID()), col_bg);
                md->addInputWidget(iw);
            }
        } else if (var_id == "ar_type_uuid") {
            iw = new MDComboBox(var_id, var_name, md->widget(),
                                attributes.value(var_id).toString(), listAssemblyRecordTypes(), col_bg);
            iw->setRowSpan(0);
            md->addInputWidget(iw);
        } else if (var_type == "int") {
            iw = new MDSpinBox(var_id, var_name, md->widget(), -999999999, 999999999,
                               attributes.value(var_id).toInt(), unit(), col_bg);
            md->addInputWidget(iw);
        } else if (var_type == "float") {
            iw = new MDNullableDoubleSpinBox(var_id, var_name, md->widget(), -999999999.9, 999999999.9,
                                             attributes.value(var_id), unit(), col_bg);
            if (var_id == "refr_add_am") {
                iw->label()->setDefaultText(QApplication::translate("Variables", "New charge:"));
                if (cb_nominal) {
                    iw->label()->toggleAlternativeText(cb_nominal->currentIndex());
                    iw->label()->addConnection(cb_nominal, SIGNAL(toggled(bool)), SLOT(toggleAlternativeText(bool)));
                }
            }
            md->addInputWidget(iw);
        } else if (var_type == "string") {
            iw = new MDLineEdit(var_id, var_name, md->widget(),
                                attributes.value(var_id).toString(), 0, col_bg);
            if (var_id == "arno") {
                if (inspection && inspection->uuid().isEmpty()) {
                    MTQuery other_inspections = Inspection::query({{"circuit_uuid", inspection->circuitUUID()}});
                    other_inspections.addFilter("date", date.toString("yyyy.MM.dd%"));
                    int count = other_inspections.list("COUNT(date) AS count", QString()).value("count").toInt();
                    iw->setVariantValue(QString("%1-%2-%3%4")
                                        .arg(inspection->customer().companyID())
                                        .arg(inspection->circuit().circuitID())
                                        .arg(date.toString("yyMMdd"))
                                        .arg(count ? QString("-%1").arg(count + 1) : ""));
                }
                iw->setRowSpan(0);
            }
            md->addInputWidget(iw);
        } else if (var_type == "text") {
            md->addInputWidget(new MDPlainTextEdit(var_id, var_name, md->widget(),
                                                   attributes.value(var_id).toString(), col_bg));
        } else if (var_type == "bool") {
            iw = new MDCheckBox(var_id, name(), md->widget(), attributes.value(var_id).toInt());
            md->addInputWidget(iw);
        } else {
            iw = new MDLineEdit(var_id, var_name, md->widget(), attributes.value(var_id).toString(), 0, col_bg);
            md->addInputWidget(iw);
        }

        if (iw) {
            iw->setGroupId(parent_uuid);
            iw->setColour(col_bg);
        }
    }
}

Variable::Variable(const QString &id, QSqlDatabase db):
    Variables(db, id)
{
    prepare("SELECT parent_uuid, uuid, id, name, type, unit, scope, value, compare_nom, tolerance, col_bg FROM variables" + QString(id.isEmpty() ? "" : " WHERE id = :id"));
    if (!id.isEmpty()) {
        bindValue(":id", id);
    }
    exec();
}

void Variable::saveResult()
{
    *pos() = -1;
    while (query()->next()) {
        int index = var_indices.value(query()->value(Variable::UUID).toString(), -1);
        if (index >= 0) {
            bool insert = false;
            QMap<int, QVariant> row = result()->at(index);
            if (!query()->value(Variable::CompareNom).toString().isEmpty()) {
                insert = true; row.insert(Variable::CompareNom, query()->value(Variable::CompareNom));
            }
            if (!query()->value(Variable::Tolerance).toString().isEmpty()) {
                insert = true; row.insert(Variable::Tolerance, query()->value(Variable::Tolerance));
            }
            if (!query()->value(Variable::ColBg).toString().isEmpty()) {
                insert = true; row.insert(Variable::ColBg, query()->value(Variable::ColBg));
            }
            if (insert) { result()->replace(index, row); insert = false; }
        } else {
            QMap<int, QVariant> row;
            for (int i = 0; i < Variable::FieldCount; ++i)
                row.insert(i, query()->value(i));
            *result() << row;
            var_indices.insert(query()->value(Variable::UUID).toString(), result()->count() - 1);
        }
    }
}
