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

#include "variableevaluation.h"

#include "variables.h"
#include "global.h"
#include "records.h"

using namespace Global;

VariableEvaluation::EvaluationContext::EvaluationContext(int vars_scope):
    vars_scope(vars_scope)
{
    init();
}

VariableEvaluation::EvaluationContext::EvaluationContext(const QString &customer_uuid, const QString &circuit_uuid, int vars_scope):
    customer_id(customer_uuid),
    circuit_id(circuit_uuid),
    vars_scope(vars_scope)
{
    circuit = MTRecord("circuits", "uuid", circuit_uuid, {"customer_uuid", customer_uuid}).list("*, " + circuitRefrigerantAmountQuery());
    persons = Customer(customer_uuid).persons().mapAll("uuid", "name");
    inspectors = Inspector().mapAll("uuid", "person");
    ar_types = AssemblyRecordType().mapAll("uuid", "name");

    init();
}

VariableEvaluation::EvaluationContext::~EvaluationContext()
{
    QMapIterator<QString, VariableEvaluation::Variable *> i(vars_map);
    while (i.hasNext()) {
        i.next();
        delete i.value();
    }
}

void VariableEvaluation::EvaluationContext::init()
{
    Variables vars(QSqlDatabase(), vars_scope);
    VariableEvaluation::Variable *parent_var, *var;

    while (vars.next()) {
        var = vars_map.value(vars.id(), NULL);
        if (!var) {
            var = new VariableEvaluation::Variable;
            vars_map.insert(vars.id(), var);
            vars_list.append(var);
        }

        var->setParentID(vars.parentID());
        var->setID(vars.id());
        var->setName(vars.name());
        var->setType(vars.type());
        var->setUnit(vars.unit());
        var->setValue(vars.valueExpression());
        var->setCompareNom(vars.compareNom());
        var->setColBg(vars.colBg());
        var->setTolerance(vars.tolerance());

        if (!vars.parentID().isEmpty()) {
            parent_var = vars_map.value(vars.parentID(), NULL);
            if (!parent_var) {
                parent_var = new VariableEvaluation::Variable;
                vars_map.insert(vars.parentID(), parent_var);
            }

            parent_var->addSubvariable(var);
        }
    }

    used_ids = listVariableIds();
}

QString VariableEvaluation::EvaluationContext::variableName(Variable *var, bool is_nominal) const
{
    if (is_nominal && var->id() == "refr_add_am")
        return QObject::tr("New charge");

    return var->name();
}

QString VariableEvaluation::EvaluationContext::evaluate(const QString &var_name, QVariantMap &inspection, QString &nom_value)
{
    VariableEvaluation::Variable *var = vars_map.value(var_name);
    if (!var) return QString();
    return var->evaluate(*this, inspection, nom_value);
}

QString VariableEvaluation::EvaluationContext::evaluate(VariableEvaluation::Variable *var, QVariantMap &inspection, QString &nom_value)
{
    return var->evaluate(*this, inspection, nom_value);
}

QString VariableEvaluation::Variable::evaluate(EvaluationContext &context, QVariantMap &inspection, QString &nom_value)
{
    QString ins_value = inspection.value(id()).toString();

    if (value().isEmpty()) {
        if (!ins_value.isEmpty()) {
            if (id() == "inspector_uuid")
                ins_value = context.inspectors.value(ins_value).value("person").toString();
            else if (id() == "person_uuid")
                ins_value = context.persons.value(ins_value).value("name").toString();
            else if (id() == "ar_type_uuid")
                ins_value = context.ar_types.value(ins_value).value("name").toString();
        }

        if (compareNom()) {
            nom_value = context.nominal_ins.value(id()).toString();
        }
    } else {
        MTDictionary expression = parseExpression(value(), context.used_ids);
        bool ok_eval, is_null;
        ins_value = QString::number(evaluateExpression(inspection, expression, context.circuit, &ok_eval, &is_null));
        if (!ok_eval || is_null) ins_value.clear();

        if (context.nominal_ins.isEmpty()) nom_value.clear();
        else if (compareNom()) {
            nom_value = QString::number(evaluateExpression(context.nominal_ins, expression, context.circuit, &ok_eval));
            if (!ok_eval) nom_value.clear();
        }
    }

    return ins_value;
}
