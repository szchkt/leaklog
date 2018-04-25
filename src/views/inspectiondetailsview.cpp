/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2018 Matus & Michal Tomlein

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

#include "inspectiondetailsview.h"

#include "global.h"
#include "mttextstream.h"
#include "records.h"
#include "viewtabsettings.h"
#include "mainwindowsettings.h"
#include "toolbarstack.h"
#include "htmlbuilder.h"
#include "variableevaluation.h"
#include "variables.h"
#include "warnings.h"

using namespace Global;

InspectionDetailsView::InspectionDetailsView(ViewTabSettings *settings):
    CircuitsView(settings)
{
}

QString InspectionDetailsView::renderHTML()
{
    QString customer_id = settings->selectedCustomer();
    QString circuit_id = settings->selectedCircuit();
    QString inspection_date = settings->selectedInspection();

    QString html; MTTextStream out(&html);

    writeServiceCompany(out);

    writeCustomersTable(out, customer_id);
    out << "<br>";
    writeCircuitsTable(out, customer_id, circuit_id, 8);

    QVariantMap circuit = Circuit(customer_id, circuit_id).list("*, " + circuitRefrigerantAmountQuery());

    Inspection inspection_record(customer_id, circuit_id, inspection_date);
    QVariantMap inspection = inspection_record.list();
    bool nominal = inspection.value("nominal").toInt();
    Inspection::Repair repair = (Inspection::Repair)inspection.value("repair").toInt();
    Inspection nom_inspection_record(customer_id, circuit_id, "");
    nom_inspection_record.parents().insert("nominal", "1");
    nom_inspection_record.addFilter("date <= ?", inspection_date);
    QVariantMap nominal_ins = nom_inspection_record.list("*", "date DESC");

    HTMLTable *table = new HTMLTable("cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\" class=\"no_border\""),
        *_table;
    HTMLTableRow *header_row = table->addRow();
    HTMLTableRow *table_row = table->addRow();
    HTMLTableCell *cell;
    HTMLParentElement *el;
    HTMLDiv div;

    div << html;
    div.newLine();

    el = div.table("cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\" class=\"no_border\"")
            ->addRow()->addHeaderCell("colspan=\"2\" style=\"font-size: medium; background-color: lightgoldenrodyellow;\"")
            ->link("customer:" + customer_id + "/circuit:" + circuit_id + (repair == Inspection::IsRepair ? "/repair:" : "/inspection:") + inspection_date + "/edit");
    *el << Inspection::titleForInspection(nominal, repair);
    *el << "&nbsp;" << settings->mainWindowSettings().formatDateTime(inspection_date);
    div.newLine();

    VariableEvaluation::EvaluationContext var_evaluation(customer_id, circuit_id);
    VariableEvaluation::Variable *variable = NULL;

    var_evaluation.setNominalInspection(nominal_ins);

    Table tables_record("", QString(), MTDictionary("scope", "1"));
    MTSqlQuery tables = tables_record.select("id, variables", Qt::DescendingOrder);
    tables.setForwardOnly(true);
    tables.exec();

    QSet<QString> all_variables;

    Variables vars;
    while (vars.next()) {
        if (vars.parentID().isEmpty())
            all_variables << vars.id();
    }

    if (!settings->isShowNotesChecked())
        all_variables.remove("notes");

    while (tables.next() || all_variables.count()) {
        QStringList table_vars;
        cell = header_row->addHeaderCell("width=\"50%\"");
        if (tables.isValid()) {
            table_vars = tables.stringValue("variables").split(";");
            all_variables.subtract(table_vars.toSet());
            *cell << tables.stringValue("id");
        }
        else {
            table_vars = all_variables.toList();
            all_variables.clear();
            *cell << tr("Other");
        }

        _table = table_row->addCell("style=\"vertical-align: top;\"")->table();

        for (int n = 0; n < table_vars.count(); ++n) {
            variable = var_evaluation.variable(table_vars.at(n));
            if (!variable) continue;
            showVariableInInspectionTable(variable, var_evaluation, inspection, _table);
        }
    }
    div << table->customHtml(2);

    InspectionsCompressor inspections_compressor_rec(QString(), MTDictionary(QStringList() << "customer_id" << "circuit_id" << "date",
                                                                             QStringList() << customer_id << circuit_id << inspection_date));
    ListOfVariantMaps inspections_compressors = inspections_compressor_rec.listAll();
    if (inspections_compressors.count()) {
        VariableEvaluation::EvaluationContext compressor_var_evaluation = VariableEvaluation::EvaluationContext(customer_id, circuit_id, Variable::Compressor);
        QList<VariableEvaluation::Variable *> compressor_vars = compressor_var_evaluation.listVariables();

        table = new HTMLTable("cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\" class=\"no_border\"");
        header_row = table->addRow();
        table_row = table->addRow();

        for (int i = 0; i < inspections_compressors.count(); ++i) {
            QVariantMap compressor = Compressor(inspections_compressors.at(i).value("compressor_id").toString()).list();

            *(header_row->addHeaderCell("width=\"50%\"")) << compressor.value("name").toString();
            _table = table_row->addCell("style=\"vertical-align: top;\"")->table();
            for (int n = 0; n < compressor_vars.count(); ++n) {
                if (compressor_vars[n]->parentID().isEmpty())
                    showVariableInInspectionTable(compressor_vars[n], compressor_var_evaluation, inspections_compressors[i], _table);
            }
        }
        div.newLine();
        *(div.table("cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\" class=\"no_border\"")->addRow()->addHeaderCell("style=\"font-size: medium;\""))
                << tr("Compressors");
        div.newLine();
        div << table->customHtml(2);
    }

//*** Warnings ***
    Warnings warnings(settings->toolBarStack()->isCO2EquivalentChecked(), true, circuit);
    QStringList warnings_list = listWarnings(warnings, circuit, nominal_ins, inspection);
    if (warnings_list.count()) {
        div.newLine();
        _table = div.table("cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\"");
        *(_table->addRow()->addHeaderCell("style=\"font-size: medium;\"")) << tr("Warnings");
        *(_table->addRow()->addCell()) << warnings_list.join(", ");
    }
    return viewTemplate("inspection").arg(div.html());
}

void InspectionDetailsView::showVariableInInspectionTable(VariableEvaluation::Variable *variable,
                                                          VariableEvaluation::EvaluationContext &var_evaluation,
                                                          QVariantMap &inspection, HTMLTable *_table)
{
    bool compare_nom = false; QString ins_value; QString nom_value;
    VariableEvaluation::Variable *subvariable = NULL;
    QList<VariableEvaluation::Variable *> subvariables = variable->subvariables();
    if (!subvariables.count()) subvariables.append(variable);
    bool nominal = inspection.value("nominal").toInt();

    MTDictionary subvar_values;

    for (int s = 0; s < subvariables.count(); ++s) {
        subvariable = subvariables.at(s);
        compare_nom = subvariable->compareNom() > 0;

        ins_value = var_evaluation.evaluate(subvariable, inspection, nom_value);
        if (ins_value.isEmpty() && (inspection.value(subvariable->id()).isNull() || ins_value.isNull())) {
            continue;
        }

        compare_nom = !nom_value.isEmpty();

        subvar_values.insert(var_evaluation.variableName(subvariable, nominal),
                             tableVarValue(subvariable->type(), ins_value, nom_value,
                                                                variable->colBg(), compare_nom, subvariable->tolerance(), true)
                                             + "&nbsp;" + subvariable->unit());
    }

    if (!subvar_values.count()) return;

    HTMLTableRow *_tr = _table->addRow();
    *(_tr->addCell(QString("rowspan=\"%1\" colspan=\"%2\"")
                   .arg(subvar_values.count())
                   .arg(subvariables.count() == 1 ? 2 : 1))) << var_evaluation.variableName(variable, nominal);

    for (int i = 0; i < subvar_values.count(); ++i) {
        if (!_tr) _tr = _table->addRow();

        if (subvar_values.key(i) != var_evaluation.variableName(variable, nominal)) *(_tr->addCell()) << subvar_values.key(i);
        *(_tr->addCell()) << subvar_values.value(i);

        _tr = NULL;
    }
}

QStringList InspectionDetailsView::listWarnings(Warnings &warnings, const QVariantMap &circuit_attributes,
                                                const QVariantMap &nominal_ins, const QVariantMap &inspection)
{
    QStringList warnings_list;
    while (warnings.next()) {
        if (warnings.value("delay").toInt())
            continue;

        if (checkWarningConditions(warnings, circuit_attributes, nominal_ins, inspection))
            warnings_list << warnings.value("name").toString();
    }
    return warnings_list;
}

bool InspectionDetailsView::checkWarningConditions(Warnings &warnings, const QVariantMap &circuit_attributes, const QVariantMap &nominal_ins, const QVariantMap &inspection)
{
    int id = warnings.value("id").toInt();
    int num_conditions = warnings.warningConditionFunctionCount(id);
    for (int i = 0; i < num_conditions; ++i) {
        bool ok = true;

        double ins_value = warnings.warningConditionValueIns(id, i).evaluate(inspection, circuit_attributes, &ok);
        if (!ok)
            return false;

        double nom_value = warnings.warningConditionValueNom(id, i).evaluate(nominal_ins, circuit_attributes, &ok);
        if (!ok)
            return false;

        QString function = warnings.warningConditionFunction(id, i);
        if (function == "=" && ins_value == nom_value) {}
        else if (function == "!=" && ins_value != nom_value) {}
        else if (function == ">" && ins_value > nom_value) {}
        else if (function == ">=" && ins_value >= nom_value) {}
        else if (function == "<" && ins_value < nom_value) {}
        else if (function == "<=" && ins_value <= nom_value) {}
        else return false;
    }
    return true;
}

QString InspectionDetailsView::tableVarValue(const QString &var_type, const QString &ins_value, const QString &nom_value,
                                             const QString &bg_class, bool compare_nom, double tolerance, bool expand_text)
{
    if (var_type == "text") {
        if (expand_text) return escapeString(ins_value, false, true);
        return escapeString(elideRight(ins_value, 20));
    } else if (var_type == "string") {
        return escapeString(ins_value);
    } else if (var_type == "bool") {
        return (ins_value.toInt() ? tr("Yes") : tr("No"));
    } else if (compare_nom && settings->isCompareValuesChecked() && !nom_value.isEmpty() && !ins_value.isEmpty()) {
        double double_value = ins_value.toDouble();
        return compareValues(nom_value.toDouble(), double_value, tolerance, bg_class).arg(FLOAT_ARG(double_value));
    } else if (var_type == "float") {
        return QLocale().toString(FLOAT_ROUND(ins_value.toDouble()), FLOAT_FORMAT, FLOAT_PRECISION);
    }
    return ins_value;
}

QString InspectionDetailsView::title() const
{
    QString title = Circuit(settings->selectedCustomer(), settings->selectedCircuit()).stringValue("name");
    return Customer(settings->selectedCustomer()).stringValue("company")
            + " - " + QString(title.isEmpty() ? settings->selectedCircuit().rightJustified(5, '0') : title)
            + " - " + settings->selectedInspection();
}
