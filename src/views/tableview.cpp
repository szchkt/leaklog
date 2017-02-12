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

#include "tableview.h"

#include "global.h"
#include "expression.h"
#include "mttextstream.h"
#include "records.h"
#include "viewtabsettings.h"
#include "mainwindowsettings.h"
#include "toolbarstack.h"
#include "htmlbuilder.h"
#include "variableevaluation.h"
#include "variables.h"
#include "warnings.h"
#include "mtaddress.h"

using namespace Global;

TableView::TableView(ViewTabSettings *settings):
    InspectionDetailsView(settings)
{
}

QString TableView::renderHTML()
{
    QString customer_id = settings->selectedCustomer();
    QString cc_id = settings->toolBarStack()->isTableForAllCircuitsChecked() ? QString() : settings->selectedCircuit();
    QString table_id = settings->currentTable();
    int year = settings->toolBarStack()->filterSinceValue();
    bool CO2_equivalent = settings->toolBarStack()->isCO2EquivalentChecked();
    QString compressor_id = settings->selectedCompressor();

    QString html; MTTextStream out(&html);

    if (settings->mainWindowSettings().serviceCompanyInformationVisible()) {
        HTMLTable *service_company = writeServiceCompany();
        out << service_company->html();
        delete service_company;
        out << "<br>";
    }

    QVariantMap customer = Customer(customer_id).list("company, address, mail, phone");

    Circuit circuits_record(customer_id, cc_id);
    if (settings->toolBarStack()->isTableForAllCircuitsExceptDecommissionedChecked()) {
        circuits_record.setCustomWhere(QString("(disused = %1 OR decommissioning >= '%2')").arg(Circuit::Commissioned).arg(settings->toolBarStack()->minimumDecommissioningDateForTableOfAllCircuits().toString(DATE_FORMAT)));
    }
    ListOfVariantMaps circuits = circuits_record.listAll("*, " + circuitRefrigerantAmountQuery());

    QVariantMap table = Table(table_id).list();

    int c = 0;
    foreach (const QVariantMap &circuit, circuits) {
        c++;
        QString circuit_id = circuit.value("id").toString();

        VariableEvaluation::EvaluationContext var_evaluation(customer_id, circuit_id);

        ListOfVariantMaps inspections(Inspection(customer_id, circuit_id, "").listAll("*", "date ASC"));
        QString last_inspection_date, last_entry_date, date;
        for (int i = 0; i < inspections.count(); ++i) {
            date = inspections.at(i).value("date").toString();
            if (date > last_entry_date) {
                last_entry_date = date;
                if (!inspections.at(i).value("repair").toInt()) {
                    last_inspection_date = date;
                }
            }
            if ((!table.value("highlight_nominal").toInt() || !inspections.at(i).value("nominal").toInt())
                && date.split(".").first().toInt() < year) {
                inspections.removeAt(i);
                i--;
            }
        }

        QString title = circuit.value("name").toString();
        if (title.simplified().isEmpty())
            title = circuit.value("device").toString();
        if (title.simplified().isEmpty())
            title = QString("%1 %2").arg(circuit.value("manufacturer").toString()).arg(circuit.value("type").toString());
        if (title.simplified().isEmpty())
            title = tr("Table of Inspections", "View Title");
        else
            title = tr("Table of Inspections: %1", "View Title").arg(title);
        out << "<table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\"><tr><th style=\"font-size: medium;\">";
        out << escapeString(title) << "</th></tr></table><br>";

        out << QString("<div%1><table>").arg(c > 1 ? " class=\"print_only\"" : "");
        out << "<tr><th>" << QApplication::translate("Customer", "ID");
        out << "</th><th>" << QApplication::translate("Customer", "Company");
        out << "</th><th>" << QApplication::translate("Customer", "Address");
        out << "</th><th>" << QApplication::translate("Customer", "E-mail");
        out << "</th><th>" << QApplication::translate("Customer", "Phone");
        out << "</th></tr><tr>";
        out << "<td>" << toolTipLink("customer", formatCompanyID(customer_id), customer_id) << "</td>";
        out << "<td>" << MTVariant(customer.value("company")) << "</td>";
        out << "<td>" << MTAddress(customer.value("address").toString()).toHtml() << "</td>";
        out << "<td>" << MTVariant(customer.value("mail")) << "</td>";
        out << "<td>" << MTVariant(customer.value("phone")) << "</td>";
        out << "</tr></table><br /></div>";
        out << "<table><tr><th>" << QApplication::translate("Circuit", "ID");
        out << "</th><th>" << QApplication::translate("Circuit", "Name");
        out << "</th><th>" << QApplication::translate("Circuit", "Device");
        out << "</th><th>" << QApplication::translate("Circuit", "Manufacturer");
        out << "</th><th>" << QApplication::translate("Circuit", "Type");
        out << "</th><th>" << QApplication::translate("Circuit", "Year of purchase");
        out << "</th><th>" << QApplication::translate("Circuit", "Commissioned on");
        out << "</th><th>" << QApplication::translate("Circuit", "Refrigerant");
        out << "</th><th>" << QApplication::translate("Circuit", "Oil");
        out << "</th></tr><tr>";
        out << "<td>" << toolTipLink("customer/circuit", circuit_id.rightJustified(5, '0'), customer_id, circuit_id) << "</td>";
        out << "<td>" << MTVariant(circuit.value("name")) << "</td>";
        out << "<td>" << MTVariant(circuit.value("device")) << "</td>";
        out << "<td>" << MTVariant(circuit.value("manufacturer")) << "</td>";
        out << "<td>" << MTVariant(circuit.value("type")) << "</td>";
        out << "<td>" << circuit.value("year").toString() << "</td>";
        out << "<td>" << settings->mainWindowSettings().formatDate(circuit.value("commissioning")) << "</td>";
        out << "<td>" << circuit.value("refrigerant_amount").toDouble()
            << "&nbsp;" << QApplication::translate("Units", "kg") << " "
            << circuit.value("refrigerant").toString() << "</td>";
        out << "<td>" << circuit.value("oil_amount").toDouble()
            << "&nbsp;" << QApplication::translate("Units", "kg") << " ";
        if (attributeValues().contains("oil::" + circuit.value("oil").toString())) {
            out << attributeValues().value("oil::" + circuit.value("oil").toString());
        }
        out << "</td></tr></table>";

        // *** Table ***
        if (table.value("scope").toInt() & Variable::Compressor) {
            HTMLTable *compressors_table = new HTMLTable();
            HTMLTableRow *compressors_table_row = compressors_table->addRow();

            HTMLTableCell *cell;
            if (compressor_id.isEmpty())
                cell = compressors_table_row->addHeaderCell();
            else
                cell = compressors_table_row->addCell("style=\"text-align: center;\"");
            *(cell->link("customer:" + customer_id + "/circuit:" + circuit_id + "/compressor:-1/table"))
                    << tr("All compressors");

            Compressor compressors_rec(QString(), MTDictionary(QStringList() << "customer_id" << "circuit_id",
                                       QStringList() << customer_id << circuit_id));
            ListOfVariantMaps compressors = compressors_rec.listAll();
            for (int i = 0; i < compressors.count(); ++i) {
                if (compressor_id == compressors.at(i).value("id").toString())
                    cell = compressors_table_row->addHeaderCell();
                else
                    cell = compressors_table_row->addCell("style=\"text-align: center;\"");
                *(cell->link("customer:" + customer_id + "/circuit:" + circuit_id
                             + "/compressor:" + compressors.at(i).value("id").toString() + "/table"))
                        << compressors.at(i).value("name").toString();
            }
            out << "<br>" << compressors_table->html();

            QStringList compressor_ids;
            if (compressor_id.isEmpty()) {
                Compressor compressors_rec(QString(), MTDictionary(QStringList() << "customer_id" << "circuit_id",
                                           QStringList() << customer_id << circuit_id));
                ListOfVariantMaps compressors = compressors_rec.listAll();
                for (int i = 0; i < compressors.count(); ++i) {
                    compressor_ids.append(compressors.at(i).value("id").toString());
                }
            } else {
                compressor_ids.append(compressor_id);
            }
            for (int i = 0; i < compressor_ids.count(); ++i) {
                InspectionsCompressor inspections_compressors_rec(QString(), MTDictionary(QStringList() << "customer_id" << "circuit_id" << "compressor_id",
                                                                                          QStringList() << customer_id << circuit_id << compressor_ids.at(i)));
                inspections_compressors_rec.setTable("inspections_compressors JOIN inspections"
                                                     " ON inspections.customer = inspections_compressors.customer_id"
                                                     " AND inspections.circuit = inspections_compressors.circuit_id"
                                                     " AND inspections.date = inspections_compressors.date");
                if (table.value("highlight_nominal").toInt())
                    inspections_compressors_rec.setCustomWhere("(inspections_compressors.date > '" + QString::number(year) + "' OR nominal > 0)");
                else
                    inspections_compressors_rec.setCustomWhere("inspections_compressors.date > '" + QString::number(year) + "'");

                ListOfVariantMaps inspections_compressors = inspections_compressors_rec.listAll("inspections_compressors.*, inspections.nominal", "date ASC");

                if (compressor_ids.count() > 1) {
                    for (int n = 0; n < compressors.count(); ++n) {
                        if (compressor_ids.at(i) == compressors.at(n).value("id").toString()) {
                            out << "<h4><a href=\"customer:" << customer_id << "/circuit:" << circuit_id
                                << "/compressor:" << compressors.at(i).value("id").toString() << "/table\">"
                                << compressors.at(n).value("name").toString() << "</a></h4>";
                            break;
                        }
                    }
                } else {
                    out << "<br>";
                }

                var_evaluation.setNominalInspection(QVariantMap());
                out << writeInspectionsTable(circuit, table, inspections_compressors, var_evaluation)->html();
            }
        } else {
            out << "<br>" << writeInspectionsTable(circuit, table, inspections, var_evaluation)->html();
        }

        //*** Warnings ***
        if (!(table.value("scope").toInt() & Variable::Compressor) || !compressor_id.isEmpty()) {
            Warnings warnings(CO2_equivalent, true, circuit, table.value("scope").toInt());
            QString warnings_html, inspection_date;
            QStringList last_warnings_list, warnings_list, backup_warnings;
            for (int i = 0; i < inspections.count(); ++i) {
                inspection_date = inspections.at(i).value("date").toString();
                warnings_list = listWarnings(warnings, circuit, var_evaluation.nominalInspection(), inspections[i]);
                backup_warnings = warnings_list;
                for (int n = 0; n < warnings_list.count(); ++n) {
                    if (last_warnings_list.contains(warnings_list.at(n))) {
                        warnings_list[n].prepend("<span style=\"color: red;\"><b>");
                        warnings_list[n].append("</b></span>");
                    }
                }
                if (warnings_list.count()) {
                    warnings_html.append("<tr><td><a href=\"customer:" + customer_id + "/circuit:" + circuit_id);
                    warnings_html.append(inspections.at(i).value("repair").toInt() ? "/repair:" : "/inspection:");
                    warnings_html.append(inspection_date + "\">");
                    warnings_html.append(settings->mainWindowSettings().formatDateTime(inspection_date) + "</a>");
                    warnings_html.append("</td><td>");
                    warnings_html.append(warnings_list.join(", "));
                    warnings_html.append("</td></tr>");
                }
                if (!inspections.at(i).value("outside_interval").toInt())
                    last_warnings_list.clear();
                last_warnings_list << backup_warnings;
            }
            QStringList delayed_warnings = listDelayedWarnings(warnings, circuit, var_evaluation.nominalInspection(),
                                                               last_entry_date, last_inspection_date);
            if (delayed_warnings.count()) {
                warnings_html.append("<tr><td colspan=\"2\"><b>");
                warnings_html.append(delayed_warnings.join(", "));
                warnings_html.append("</b></td></tr>");
            }
            if (!warnings_html.isEmpty()) {
                out << "<br /><table>";
                out << "<tr><th width=\"20%\">" << tr("Date") << "</th><th>" << tr("Warnings") << "</th></tr>";
                out << warnings_html;
                out << "</table>";
            }
        }
        if (c < circuits.count())
            out << "<div style=\"page-break-after: always;\"><br /></div>";
    }

    QString colours = !settings->isPrinterFriendlyVersionChecked() ? "<link href=\"colours.css\" rel=\"stylesheet\" type=\"text/css\" />" : "";
    return viewTemplate("table").arg(colours).arg(html);
}

HTMLTable *TableView::writeInspectionsTable(const QVariantMap &circuit, const QVariantMap &table_map,
                                            ListOfVariantMaps &inspections, VariableEvaluation::EvaluationContext &var_evaluation)
{
    QStringList table_vars = table_map.value("variables").toString().split(";", QString::SkipEmptyParts);
    VariableEvaluation::Variable *variable = NULL, *subvariable = NULL;
    HTMLTable *table = new HTMLTable;
    HTMLTableRow *row = table->addRow();
    row->addClass("border_top");
    HTMLTableCell *cell = NULL;
    HTMLParentElement *el = NULL;

//*** Head ***
    HTMLTableHead *thead = table->thead();
    row = thead->addRow();
    row->addClass("border_top");
    *(row->addHeaderCell("rowspan=\"3\"")) << tr("Date");
    for (int i = 0; i < table_vars.count(); ++i) {
        variable = var_evaluation.variable(table_vars.at(i));
        if (!variable) continue;
        int subvar_count = variable->countSubvariables();
        int rowspan = 1;
        if (subvar_count > 0) rowspan = 1;
        else if (variable->unit() != "") rowspan =  2;
        else rowspan = 3;
        *(row->addHeaderCell(QString("colspan=\"%1\" rowspan=\"%2\" class=\"%3\"")
                           .arg(subvar_count)
                           .arg(rowspan)
                           .arg(variable->colBg()))) << variable->name();
    }
    row = thead->addRow();
    for (int i = 0; i < table_vars.count(); ++i) {
        variable = var_evaluation.variable(table_vars.at(i));
        if (!variable) continue;
        if (variable->countSubvariables() > 0) {
            QList<VariableEvaluation::Variable *> subvariables = variable->subvariables();
            for (int n = 0; n < subvariables.count(); ++n) {
                subvariable = subvariables.at(n);
                *(row->addHeaderCell(QString("rowspan=\"%1\" class=\"%2\"")
                                   .arg(subvariable->unit().isEmpty() ? 2 : 1)
                                   .arg(variable->colBg())))
                        << subvariable->name();
            }
        }
    }
    row = thead->addRow();
    row->addClass("border_bottom");
    int column_count = 1 + table_vars.count();
    for (int i = 0; i < table_vars.count(); ++i) {
        variable = var_evaluation.variable(table_vars.at(i));
        if (!variable) continue;
        if (variable->countSubvariables() > 0) {
            QList<VariableEvaluation::Variable *> subvariables = variable->subvariables();
            column_count += subvariables.count();
            QString unit;
            for (int n = 0; n < subvariables.count(); ++n) {
                unit = subvariables.at(n)->unit();
                if (!unit.isEmpty()) {
                    *(row->addHeaderCell(QString("class=\"%1\"").arg(variable->colBg())))
                            << unit;
                }
            }
        } else {
            if (!variable->unit().isEmpty()) {
                *(row->addHeaderCell(QString("class=\"%1\"").arg(variable->colBg())))
                        << variable->unit();
            }
        }
    }

//*** Body ***
    HTMLTableBody *tbody = table->tbody();
    for (int i = 0; i < inspections.count(); ++i) {
        bool is_nominal = inspections.at(i).value("nominal").toInt();
        bool is_repair = inspections.at(i).value("repair").toInt();
        bool is_outside_interval = inspections.at(i).value("outside_interval").toInt();
        QString inspection_date = inspections.at(i).value("date").toString();
        QString customer_id = circuit.value("parent").toString();
        QString circuit_id = circuit.value("id").toString();
        Inspection::Type inspection_type = (Inspection::Type)inspections.at(i).value("inspection_type").toInt();

        if (inspection_type != Inspection::DefaultType) {
            QString description = Inspection::descriptionForInspectionType(inspection_type, inspections.at(i).value("inspection_type_data").toString());

            if (!description.isEmpty()) {
                row = tbody->addRow();
                *row->addCell() << toolTipLink("customer/circuit/inspection",
                                               settings->mainWindowSettings().formatDateTime(inspection_date),
                                               customer_id, circuit_id, inspection_date, ToolTipLinkItemRemove);
                *row->addCell(QString("colspan=\"%1\"").arg(column_count)) << escapeString(description);
                continue;
            }
        }

        if (is_nominal)
            var_evaluation.setNominalInspection(inspections.at(i));

        row = tbody->addRow();
        if (is_nominal && table_map.value("highlight_nominal").toInt()) {
            row->addClass("nominal");
        }
        el = cell = row->addCell();
        if (is_nominal) { el = cell->bold(); }
        else if (is_repair) { el = cell->italics(); }
        *el << toolTipLink(is_repair ? "customer/circuit/repair" : "customer/circuit/inspection",
                           settings->mainWindowSettings().formatDateTime(inspection_date),
                           customer_id, circuit_id, inspection_date);
        if (is_outside_interval) { *el << "*"; }

        for (int n = 0; n < table_vars.count(); ++n) {
            variable = var_evaluation.variable(table_vars.at(n));
            if (!variable) continue;
            bool compare_nom = false; int rowspan = 1; QString ins_value; QString nom_value;
            QList<VariableEvaluation::Variable *> subvariables = variable->subvariables();
            if (subvariables.count() > 0) {
                for (int s = 0; s < subvariables.count(); ++s) {
                    subvariable = subvariables.at(s);
                    compare_nom = subvariable->compareNom() > 0;
                    if (subvariable->value().contains("sum")) {
                        QString i_year = inspection_date.split(".").first();
                        if (i > 0 && inspections.at(i - 1).value("date").toString().split(".").first() == i_year) {
                            continue;
                        } else {
                            int in = i;
                            for (; in < inspections.count(); ++in) {
                                if (inspections.at(in).value("date").toString().split(".").first() != i_year)
                                    break;
                            }
                            rowspan = in - i;
                        }
                    } else rowspan = 1;
                    ins_value = var_evaluation.evaluate(subvariable, inspections[i], nom_value);
                    compare_nom = !nom_value.isEmpty();
                    *row << writeTableVarCell(subvariable->type(), ins_value, nom_value, variable->colBg(),
                                              compare_nom, rowspan, subvariable->tolerance());
                }
            } else {
                compare_nom = variable->compareNom() > 0;
                if (variable->value().contains("sum")) {
                    QString i_year = inspection_date.split(".").first();
                    if (i > 0 && inspections.at(i - 1).value("date").toString().split(".").first() == i_year) {
                        continue;
                    } else {
                        int in = i;
                        for (; in < inspections.count(); ++in) {
                            if (inspections.at(in).value("date").toString().split(".").first() != i_year)
                                break;
                        }
                        rowspan = in - i;
                    }
                } else rowspan = 1;
                ins_value = var_evaluation.evaluate(variable, inspections[i], nom_value);
                compare_nom = !nom_value.isEmpty();
                *row << writeTableVarCell(variable->type(), ins_value, nom_value, variable->colBg(),
                                          compare_nom, rowspan, variable->tolerance());
            }
        }
    }

//*** Foot ***
    HTMLTableFoot *tfoot = table->tfoot();
    MTDictionary foot_functions;
    foot_functions.insert("sum", tr("Sum"));
    foot_functions.insert("avg", tr("Average"));
    for (int f = 0; f < foot_functions.count(); ++f) {
        if (!table_map.value(foot_functions.key(f)).toString().isEmpty()) {
            row = tfoot->addRow("class=\"border_top border_bottom\"");
            *(row->addHeaderCell()) << foot_functions.value(f);
            QStringList f_vars = table_map.value(foot_functions.key(f)).toString().split(";", QString::SkipEmptyParts);
            for (int i = 0; i < table_vars.count(); ++i) {
                variable = var_evaluation.variable(table_vars.at(i));
                if (!variable) continue;
                bool is_in_foot = f_vars.contains(table_vars.at(i));
                if (variable->countSubvariables() > 0) {
                    QList<VariableEvaluation::Variable *> subvariables = variable->subvariables();
                    for (int s = 0; s < subvariables.count(); ++s) {
                        subvariable = subvariables.at(s);
                        is_in_foot = f_vars.contains(table_vars.at(i));
                        if (subvariable->type() != "float" && subvariable->type() != "int") is_in_foot = false;
                        cell = row->addCell();
                        cell->addClass(variable->colBg());
                        bool value_contains_sum = subvariable->value().contains(QRegExp("\\bsum\\b"));
                        if (is_in_foot) {
                            double value = 0.0; int num_ins = 0;
                            if (subvariable->value().isEmpty()) {
                                num_ins = inspections.count();
                                for (int ins = 0; ins < inspections.count(); ++ins) {
                                    value += inspections.at(ins).value(subvariable->id()).toDouble();
                                }
                            } else {
                                Expression expression(subvariable->value());
                                for (int ins = 0; ins < inspections.count(); ++ins) {
                                    if (value_contains_sum && ins > 0 &&
                                        inspections.at(ins - 1).value("date").toString().split(".").first()
                                            == inspections.at(ins).value("date").toString().split(".").first())
                                        continue;
                                    num_ins++;
                                    value += expression.evaluate(inspections[ins], circuit);
                                }
                            }
                            if (num_ins && (foot_functions.key(f) == "avg" || subvariable->unit() == "%"))
                                { value /= (double)num_ins; }
                            *cell << value;
                        }
                    }
                } else {
                    if (variable->type() != "float" && variable->type() != "int") is_in_foot = false;
                    cell = row->addCell();
                    cell->addClass(variable->colBg());
                    bool value_contains_sum = variable->value().contains(QRegExp("\\bsum\\b"));
                    if (is_in_foot) {
                        double value = 0.0; int num_ins = 0;
                        if (variable->value().isEmpty()) {
                            num_ins = inspections.count();
                            for (int ins = 0; ins < inspections.count(); ++ins) {
                                value += inspections.at(ins).value(table_vars.at(i)).toDouble();
                            }
                        } else {
                            Expression expression(variable->value());
                            for (int ins = 0; ins < inspections.count(); ++ins) {
                                if (value_contains_sum && ins > 0 &&
                                    inspections.at(ins - 1).value("date").toString().split(".").first()
                                        == inspections.at(ins).value("date").toString().split(".").first())
                                    continue;
                                num_ins++;
                                value += expression.evaluate(inspections[ins], circuit);
                            }
                        }
                        if (num_ins && (foot_functions.key(f) == "avg" || variable->unit() == "%"))
                            { value /= (double)num_ins; }
                        *cell << value;
                    }
                }
            }
        }
    }
    return table;
}

QStringList TableView::listDelayedWarnings(Warnings &warnings, const QVariantMap &circuit_attributes,
                                           QVariantMap &nominal_ins, const QString &last_entry_date,
                                           const QString &last_inspection_date, int *delay_out)
{
    QString customer_id = circuit_attributes.value("parent").toString();
    QString circuit_id = circuit_attributes.value("id").toString();
    QStringList warnings_list;
    QVariantMap last_entry = Inspection(customer_id, circuit_id, last_entry_date).list();
    QVariantMap last_inspection;
    if (last_inspection_date == last_entry_date) {
        last_inspection = last_entry;
    } else {
        last_inspection = Inspection(customer_id, circuit_id, last_inspection_date).list();
    }
    QVariantMap *entry;
    bool show_warning;
    int id, delay, interval;
    while (warnings.next()) {
        show_warning = true;
        delay = warnings.value("delay").toInt();
        if (!delay) { continue; }
        id = warnings.value("id").toInt();
        if (id >= 1200 && id < 1300) {
            entry = &last_inspection;
            interval = circuit_attributes.value("inspection_interval").toInt();
            if (interval) { delay = interval; }
            if (delay_out) { *delay_out = delay; }
        } else { entry = &last_entry; }
        if (QDate::fromString(entry->value("date").toString().split("-").first(), DATE_FORMAT).daysTo(QDate::currentDate()) < delay) {
            continue;
        }
        if (!nominal_ins.isEmpty()) {
            show_warning = checkWarningConditions(warnings, circuit_attributes, nominal_ins, *entry);
        }
        if (show_warning) {
            warnings_list << warnings.value("name").toString();
        }
    }
    return warnings_list;
}

void TableView::writeTableVarCell(MTTextStream &out, const QString &var_type, const QString &ins_value, const QString &nom_value,
                                  const QString &bg_class, bool compare_nom, int rowspan, double tolerance)
{
    out << writeTableVarCell(var_type, ins_value, nom_value, bg_class, compare_nom, rowspan, tolerance)->html();
}

HTMLTableCell *TableView::writeTableVarCell(const QString &var_type, const QString &ins_value, const QString &nom_value,
                                            const QString &bg_class, bool compare_nom, int rowspan, double tolerance)
{
    QString args = QString("class=\"%1\" rowspan=\"%2\"").arg(bg_class).arg(rowspan);
    if (var_type == "text" && !ins_value.isEmpty()) {
        args.append(QString("onmouseover=\"Tip('%1')\" onmouseout=\"UnTip()\"")
                    .arg(escapeString(escapeString(ins_value), true, true)));
    }
    HTMLTableCell *cell = new HTMLTableCell(args);
    *cell << tableVarValue(var_type, ins_value, nom_value, bg_class, compare_nom, tolerance);
    return cell;
}

QString TableView::tableVarValue(const QString &var_type, const QString &ins_value, const QString &nom_value,
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

QString TableView::title() const
{
    QString title;
    if (settings->isCircuitSelected()) {
        title = Circuit(settings->selectedCustomer(), settings->selectedCircuit()).stringValue("name");
        if (title.isEmpty())
            title = settings->selectedCircuit().rightJustified(5, '0');
        title.prepend(" - ");
    }
    return tr("Table of Inspections") + " - " + Customer(settings->selectedCustomer()).stringValue("company") + title;
}
