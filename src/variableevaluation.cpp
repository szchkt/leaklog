/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2019 Matus & Michal Tomlein

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
#include "expression.h"
#include "records.h"

using namespace Global;

VariableEvaluation::EvaluationContext::EvaluationContext(int vars_scope):
    ns(DBInfo::databaseUUID()),
    vars_scope(vars_scope)
{
    init();
}

VariableEvaluation::EvaluationContext::EvaluationContext(const QString &customer_uuid, const QString &circuit_uuid, int vars_scope):
    ns(DBInfo::databaseUUID()),
    customer_id(customer_uuid),
    circuit_id(circuit_uuid),
    vars_scope(vars_scope)
{
    circuit = Circuit(circuit_uuid).list("*, " + circuitRefrigerantAmountQuery());
    persons = Customer(customer_uuid).persons().mapAll("uuid", "name");
    inspectors = Inspector::query().mapAll("uuid", "certificate_number, person");
    ar_types = AssemblyRecordType::query().mapAll("uuid", "name");

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
        var = vars_map.value(vars.uuid(), NULL);
        if (!var) {
            var = new VariableEvaluation::Variable;
            vars_map.insert(vars.uuid(), var);
            vars_list.append(var);
        }

        var->setParentUUID(vars.parentUUID());
        var->setUUID(vars.uuid());
        var->setID(vars.id());
        var->setName(vars.name());
        var->setType(vars.type());
        var->setUnit(vars.unit());
        var->setValue(vars.valueExpression());
        var->setCompareNom(vars.compareNom());
        var->setColBg(vars.colBg());
        var->setTolerance(vars.tolerance());

        if (!vars.parentUUID().isEmpty()) {
            parent_var = vars_map.value(vars.parentUUID(), NULL);
            if (!parent_var) {
                parent_var = new VariableEvaluation::Variable;
                vars_map.insert(vars.parentUUID(), parent_var);
            }

            parent_var->addSubvariable(var);
        }
    }
}

QString VariableEvaluation::EvaluationContext::variableName(Variable *var, bool is_nominal) const
{
    if (is_nominal && var->id() == "refr_add_am")
        return QObject::tr("New charge");

    return var->name();
}

VariableEvaluation::Variable *VariableEvaluation::EvaluationContext::variable(const QString &name) const
{
    return vars_map.value(createUUIDv5(ns, name));
}

QString VariableEvaluation::EvaluationContext::evaluate(const QString &var_name, const QVariantMap &inspection, QString &nom_value)
{
    VariableEvaluation::Variable *var = vars_map.value(createUUIDv5(ns, var_name));
    if (!var) return QString();
    return var->evaluate(*this, inspection, nom_value);
}

QString VariableEvaluation::EvaluationContext::evaluate(VariableEvaluation::Variable *var, const QVariantMap &inspection, QString &nom_value)
{
    return var->evaluate(*this, inspection, nom_value);
}

QString VariableEvaluation::Variable::evaluate(EvaluationContext &context, const QVariantMap &inspection, QString &nom_value)
{
    QString ins_value = inspection.value(id()).toString();

    if (value().isEmpty()) {
        if (!ins_value.isEmpty()) {
            if (id() == "inspector_uuid") {
                QVariantMap inspector = context.inspectors.value(ins_value);
                if (inspector.isEmpty()) {
                    ins_value.clear();
                } else {
                    QString certificate_number = inspector.value("certificate_number").toString();
                    if (certificate_number.isEmpty()) {
                        ins_value = inspector.value("person").toString();
                    } else {
                        ins_value = QString("%1 (%2)").arg(inspector.value("person").toString()).arg(certificate_number);
                    }
                }
            } else if (id() == "person_uuid") {
                ins_value = context.persons.value(ins_value).value("name").toString();
            } else if (id() == "ar_type_uuid") {
                ins_value = context.ar_types.value(ins_value).value("name").toString();
            }
        }

        if (compareNom()) {
            nom_value = context.nominal_ins.value(id()).toString();
        }
    } else {
        Expression expression(value());
        bool ok_eval, is_null;
        ins_value = QString::number(expression.evaluate(inspection, context.circuit, &ok_eval, &is_null));
        if (!ok_eval || is_null) ins_value.clear();

        if (context.nominal_ins.isEmpty()) nom_value.clear();
        else if (compareNom()) {
            nom_value = QString::number(expression.evaluate(context.nominal_ins, context.circuit, &ok_eval));
            if (!ok_eval) nom_value.clear();
        }
    }

    return ins_value;
}
