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

#include "main_window.h"

void MainWindow::viewChanged(const QString & view)
{
    if (!db.isOpen()) { wv_main->setHtml(QString()); return; }

    tbtn_view_level_up->setEnabled(cb_view->currentIndex() > 0);
    tbtn_view_level_down->setEnabled(cb_view->currentIndex() < cb_view->count() - 1);
    bool table_view = cb_view->currentText() == tr("Table of inspections");
    lbl_table->setEnabled(table_view);
    cb_table->setEnabled(table_view);
    lbl_since->setEnabled(table_view);
    spb_since->setEnabled(table_view);

    wv_main->setHtml(tr("Loading..."));
    qApp->processEvents();
    if (view == tr("All customers")) {
        viewAllCustomers();
    } else if (view == tr("Customer information") && selectedCustomer() >= 0) {
        viewCustomer(toString(selectedCustomer()));
    } else if (view == tr("Circuit information") && selectedCustomer() >= 0 && selectedCircuit() >= 0) {
        viewCircuit(toString(selectedCustomer()), toString(selectedCircuit()));
    } else if (view == tr("Inspection information") && selectedCustomer() >= 0 && selectedCircuit() >= 0 && !selectedInspection().isNull()) {
        viewInspection(toString(selectedCustomer()), toString(selectedCircuit()), selectedInspection());
    } else if (table_view && selectedCustomer() >= 0 && selectedCircuit() >= 0 && cb_table->currentIndex() >= 0) {
        viewTable(toString(selectedCustomer()), toString(selectedCircuit()), cb_table->currentText(), spb_since->value() == 1999 ? 0 : spb_since->value());
    } else if (view == tr("Inspectors")) {
        viewAllInspectors(toString(selectedInspector()));
    } else if (view == tr("Refrigerant consumption")) {
        viewRefrigerantConsumption(toString(selectedCustomer()));
    } else if (view == tr("Customer information") || view == tr("Circuit information") || view == tr("Inspection information") || table_view) {
        viewLevelUp();
    } else {
        wv_main->setHtml(QString());
    }
}

void MainWindow::viewAllCustomers()
{
    QString html; QTextStream out(&html);
    MTRecord all_customers("customer", "", MTDictionary());
    QList<QMap<QString, QVariant> > list = all_customers.listAll();
    int cu_length = QString("customer::").length();
    for (int i = 0; i < list.count(); ++i) {
        out << "<tr style=\"background-color: #eee;\"><td colspan=\"2\" style=\"font-size: large; text-align: center;\"><b>" << tr("Company:") << "&nbsp;";
        out << "<a href=\"customer:" << list.at(i).value("id").toString() << "\">" << list.at(i).value("company").toString() << "</a></b></td></tr>";
        out << "<tr><td width=\"50%\"><table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">";
        QString attr_value;
        int num_valid = 0;
        for (int n = 0; n < dict_attrnames.count() && dict_attrnames.key(n).startsWith("customer::"); ++n) {
            attr_value = dict_attrnames.key(n).mid(cu_length);
            if (list.at(i).value(attr_value).toString().isEmpty()) continue;
            out << "<num_attr>" << num_valid << "</num_attr>";
            out << "<tr><td style=\"text-align: right; width:50%;\">" << dict_attrnames.value(n) << "&nbsp;</td>";
            attr_value = QString(attr_value == "id" ? list.at(i).value(attr_value).toString().rightJustified(8, '0') : list.at(i).value(attr_value).toString());
            out << "<td>" << attr_value << "</td></tr>";
            num_valid++;
        }
        out << "<num_attr>" << num_valid << "</num_attr>"; num_valid++;
        out << "<tr><td style=\"text-align: right; width:50%;\">" << tr("Number of circuits:") << "&nbsp;</td>";
        out << "<td>";
        MTRecord circuits_record("circuit", "", MTDictionary("parent", list.at(i).value("id").toString()));
        QList<QMap<QString, QVariant> > circuits = circuits_record.listAll("id");
        int num_circuits = 0, num_inspections = 0;
        for (int j = 0; j < circuits.count(); ++j) {
            num_circuits++;
            MTDictionary inspection_parents("circuit", circuits.at(j).value("id").toString());
            inspection_parents.insert("customer", list.at(i).value("id").toString());
            MTRecord inspection_record("inspection", "", inspection_parents);
            num_inspections += inspection_record.listAll("date").count();
        }
        out << num_circuits;
        out << "</td></tr>";
        out << "<num_attr>" << num_valid << "</num_attr>"; num_valid++;
        out << "<tr><td style=\"text-align: right; width:50%;\">" << tr("Total number of inspections:") << "&nbsp;</td>";
        out << "<td>" << num_inspections << "</td></tr>";
        out << "</table></td></tr>";
        if (num_valid != 0) {
            html.replace(QString("<num_attr>%1</num_attr>").arg(int(num_valid / 2 + num_valid % 2)), "</table></td><td width=\"50%\"><table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">");
        }
        for (int k = 0; k < num_valid; ++k) {
            html.remove(QString("<num_attr>%1</num_attr>").arg(k));
        }
    }
    wv_main->setHtml(dict_html.value(tr("All customers")).arg(html));
}

void MainWindow::viewCustomer(const QString & customer_id)
{
    QString html; QTextStream out(&html);
    MTRecord customer_rec("customer", customer_id, MTDictionary());
    QMap<QString, QVariant> customer = customer_rec.list();
    out << "<table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">";
    out << "<tr style=\"background-color: #DFDFDF;\"><td colspan=\"2\" style=\"font-size: larger; width:100%; text-align: center;\"><b>" << tr("Company:") << "&nbsp;";
    out << "<a href=\"customer:" << customer_id << "/modify\">" << customer.value("company").toString() << "</a></b></td></tr>";
    out << "<tr><td width=\"50%\"><table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">";
    int num_valid = 0;
    QString attr_value;
    int cu_length = QString("customer::").length();
    for (int n = 0; n < dict_attrnames.count() && dict_attrnames.key(n).startsWith("customer::"); ++n) {
        attr_value = dict_attrnames.key(n).mid(cu_length);
        if (customer.value(attr_value).toString().isEmpty()) continue;
        out << "<num_attr>" << num_valid << "</num_attr>";
        out << "<tr><td style=\"text-align: right; width:50%;\">" << dict_attrnames.value(n) << "&nbsp;</td>";
        attr_value = QString(attr_value == "id" ? customer.value(attr_value).toString().rightJustified(8, '0') : customer.value(attr_value).toString());
        out << "<td>" << attr_value << "</td></tr>";
        num_valid++;
    }
    out << "<num_attr>" << num_valid << "</num_attr>"; num_valid++;
    out << "<tr><td style=\"text-align: right; width:50%;\">" << tr("Number of circuits:") << "&nbsp;</td>";
    out << "<td>";
    MTRecord circuits_rec("circuit", "", MTDictionary("parent", customer_id));
    QList<QMap<QString, QVariant> > circuits = circuits_rec.listAll();
    int num_circuits = 0, num_inspections = 0;
    for (int i = 0; i < circuits.count(); ++i) {
        num_circuits++;
        MTDictionary inspection_parents("circuit", circuits.at(i).value("id").toString());
        inspection_parents.insert("customer", customer_id);
        MTRecord inspection_record("inspection", "", inspection_parents);
        num_inspections += inspection_record.listAll("date").count();
    }
    out << num_circuits;
    out << "</td></tr>";
    out << "<num_attr>" << num_valid << "</num_attr>"; num_valid++;
    out << "<tr><td style=\"text-align: right; width:50%;\">" << tr("Total number of inspections:") << "&nbsp;</td>";
    out << "<td>" << num_inspections << "</td></tr>";
    out << "</table></td></tr>";
    out << "</table>";
    if (num_valid != 0) {
        html.replace(QString("<num_attr>%1</num_attr>").arg(int(num_valid / 2 + num_valid % 2)), "</table></td><td width=\"50%\"><table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">");
    }
    for (int k = 0; k < num_valid; ++k) {
        html.remove(QString("<num_attr>%1</num_attr>").arg(k));
    }
    cu_length = QString("circuit::").length();
    for (int i = 0; i < circuits.count(); ++i) {
        num_valid = 0;
        out << "<table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\"><tr><td rowspan=\"8\" style=\"width:10%;\"/>";
        out << "<td colspan=\"2\" style=\"background-color: #eee; font-size: medium; text-align: center; width:80%;\"><b>" << tr("Circuit:") << "&nbsp;";
        out << "<a href=\"customer:" << customer_id << "/circuit:" << circuits.at(i).value("id").toString() << "\">" << circuits.at(i).value("id").toString().rightJustified(4, '0') << "</a></b></td>";
        out << "<td rowspan=\"8\" style=\"width:10%;\"/></tr>";
        out << "<tr><td width=\"40%\"><table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">";
        for (int n = dict_attrnames.indexOfKey("circuit::id"); n < dict_attrnames.count() && dict_attrnames.key(n).startsWith("circuit::"); ++n) {
            QStringList dict_value = dict_attrnames.value(n).split("||");
            attr_value = dict_attrnames.key(n).mid(cu_length);
            attr_value = circuits.at(i).value(attr_value).toString();
            if (attr_value.isEmpty()) continue;
            out << "<num_attr>" << num_valid << "</num_attr>";
            out << "<tr><td style=\"text-align: right; width:50%;\">" << dict_value.first() << "&nbsp;</td>";
            if (dict_attrnames.key(n) == "circuit::field") {
                if (dict_attrvalues.contains("field::" + attr_value)) {
                    attr_value = dict_attrvalues.value("field::" + attr_value);
                }
            } else if (dict_attrnames.key(n) == "circuit::oil") {
                if (dict_attrvalues.contains("oil::" + attr_value)) {
                    attr_value = dict_attrvalues.value("oil::" + attr_value);
                }
            } else if (dict_attrnames.key(n) == "circuit::hermetic") {
                attr_value = attr_value.toInt() ? tr("Yes") : tr("No");
            }
            out << "<td>" << attr_value << "&nbsp;";
            if (dict_value.count() > 1) {
                out << dict_value.last();
            }
            out << "</td></tr>";
            num_valid++;
        }
        if (num_valid != 0) {
            html.replace(QString("<num_attr>%1</num_attr>").arg(int(num_valid / 2 + num_valid % 2)), "</table></td><td width=\"40%\"><table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">");
        }
        for (int k = 0; k < num_valid; ++k) {
            html.remove(QString("<num_attr>%1</num_attr>").arg(k));
        }
        out << "</table></td></tr>";
        out << "</table>";
    }
    wv_main->setHtml(dict_html.value(tr("Customer information")).arg(html));
}

void MainWindow::viewCircuit(const QString & customer_id, const QString & circuit_id)
{
    QString html; QTextStream out(&html);
    MTRecord circuit_rec("circuit", circuit_id, MTDictionary("parent", customer_id));
    QMap<QString, QVariant> circuit = circuit_rec.list();
    out << "<table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">";
    out << "<tr style=\"background-color: #eee;\"><td colspan=\"2\" style=\"font-size: larger; width:100%; text-align: center;\"><b>" << tr("Company:") << "&nbsp;";
    out << "<a href=\"customer:" << customer_id << "\">";
    MTRecord customer("customer", customer_id, MTDictionary());
    out << customer.list("company").value("company").toString();
    out << "</a></b></td></tr>";
    out << "<tr style=\"background-color: #DFDFDF;\"><td colspan=\"2\" style=\"font-size: large; width:100%; text-align: center;\"><b>" << tr("Circuit:") << "&nbsp;";
    out << "<a href=\"customer:" << customer_id << "/circuit:" << circuit_id << "/modify\">" << circuit_id.rightJustified(4, '0') << "</a></b></td></tr>";
    out << "<tr><td width=\"50%\"><table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">";
    int num_valid = 0; QString attr_value;
    int cu_length = QString("circuit::").length();
    for (int n = dict_attrnames.indexOfKey("circuit::id"); n < dict_attrnames.count() && dict_attrnames.key(n).startsWith("circuit::"); ++n) {
        QStringList dict_value = dict_attrnames.value(n).split("||");
        attr_value = dict_attrnames.key(n).mid(cu_length);
        attr_value = circuit.value(attr_value).toString();
        if (attr_value.isEmpty()) continue;
        out << "<num_attr>" << num_valid << "</num_attr>";
        out << "<tr><td style=\"text-align: right; width:50%;\">" << dict_value.first() << "&nbsp;</td>";
        if (dict_attrnames.key(n) == "circuit::field") {
            if (dict_attrvalues.contains("field::" + attr_value)) {
                attr_value = dict_attrvalues.value("field::" + attr_value);
            }
        } else if (dict_attrnames.key(n) == "circuit::oil") {
            if (dict_attrvalues.contains("oil::" + attr_value)) {
                attr_value = dict_attrvalues.value("oil::" + attr_value);
            }
        } else if (dict_attrnames.key(n) == "circuit::hermetic") {
            attr_value = attr_value.toInt() ? tr("Yes") : tr("No");
        }
        out << "<td>" << attr_value << "&nbsp;";
        if (dict_value.count() > 1) {
            out << dict_value.last();
        }
        out << "</td></tr>";
        num_valid++;
    }
    if (num_valid != 0) {
        html.replace(QString("<num_attr>%1</num_attr>").arg(int(num_valid / 2 + num_valid % 2)), "</table></td><td width=\"40%\"><table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">");
    }
    for (int k = 0; k < num_valid; ++k) {
        html.remove(QString("<num_attr>%1</num_attr>").arg(k));
    }
    out << "</table></td></tr>";
    out << "</table>";
    MTDictionary inspection_parents("circuit", circuit_id);
    inspection_parents.insert("customer", customer_id);
    MTRecord inspection_record("inspection", "", inspection_parents);
    QList<QMap<QString, QVariant> > inspections = inspection_record.listAll("date, nominal");
    int num_inspections = inspections.count();
    if (num_inspections > 0) {
        out << "<table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">";
        out << "<tr><td rowspan=\"";
        int row_span;
        if (num_inspections % 2 == 0) row_span = num_inspections / 2 + 1;
        else row_span = num_inspections / 2 + 2;
        out << row_span;
        out << "\" style=\"width:10%;\"/>";
        out << "<td colspan=\"2\" style=\"background-color: #eee; font-size: medium; text-align: center; width:80%;\"><b>";
        out << "<a href=\"customer:" << customer_id << "/circuit:" << circuit_id << "/table\">" << tr("Inspections") << "</a></b></td>";
        out << "<td rowspan=\"";
        out << row_span;
        out << "\" style=\"width:10%;\"/></tr>";
        out << "<tr><td style=\"width:40%;\"><table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">";
        if (num_inspections % 2 != 0) num_inspections++;
        for (int i = 0; i < inspections.count(); ++i) {
            if (int(num_inspections / 2) == i) {
                out << "</table></td>";
                out << "<td style=\"width:40%;\"><table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">";
            }
            out << "<tr><td style=\"text-align: center\">";
            if (inspections.at(i).value("nominal").toInt() == 1) {
                out << tr("Nominal:") << "&nbsp;";
            }
            out << "<a href=\"customer:" << customer_id << "/circuit:" << circuit_id << "/inspection:" << inspections.at(i).value("date").toString() << "\">" << inspections.at(i).value("date").toString() << "</a>";
            out << "</td></tr>";
        }
        out << "</table></td></tr>";
        out << "</table>";
    }
    wv_main->setHtml(dict_html.value(tr("Customer information")).arg(html));
}

void MainWindow::viewInspection(const QString & customer_id, const QString & circuit_id, const QString & inspection_date)
{
    QString html; QTextStream out(&html);

    Variables vars;

    MTDictionary inspection_parents("circuit", circuit_id);
    inspection_parents.insert("customer", customer_id);
    MTRecord inspection_record("inspection", inspection_date, inspection_parents);
    QMap<QString, QVariant> inspection = inspection_record.list();
    bool nominal = inspection.value("nominal").toInt();
    MTDictionary nom_inspection_parents("circuit", circuit_id);
    nom_inspection_parents.insert("customer", customer_id);
    nom_inspection_parents.insert("nominal", "1");
    MTRecord nom_inspection_record("inspection", "", nom_inspection_parents);
    QMap<QString, QVariant> nominal_ins = nom_inspection_record.list();

    out << "<table cellspacing=\"0\" style=\"width:100%;\">";
    out << "<tr><td><table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">";
    out << "<tbody>";
    out << "<tr style=\"background-color: #eee;\"><td colspan=\"2\" style=\"font-size: larger; width:100%; text-align: center;\"><b>" << tr("Circuit:") << "&nbsp;";
    out << "<a href=\"customer:" << customer_id << "/circuit:" << circuit_id << "\">" << circuit_id.rightJustified(4, '0') << "</a></b></td></tr>";
    out << "<tr style=\"background-color: #DFDFDF;\"><td colspan=\"2\" style=\"font-size: larger; width:100%; text-align: center;\"><b>";
    if (nominal) out << tr("Nominal inspection:") << "&nbsp;";
    else out << tr("Inspection:") << "&nbsp;";
	out << "<a href=\"customer:" << customer_id << "/circuit:" << circuit_id << "/inspection:" << inspection_date << "/modify\">";
	out << inspection_date << "</a></b></td></tr>";
    out << "<tr><td><table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">";

    int num_shown_vars = 0;
    QStringList used_ids = listVariableIds(); // all = false
    while (vars.next()) {
        QString var_id; bool compare_nom = false; bool ok_eval = true;
        MTDictionary expression; double tolerance = 0;
        if (vars.value("SUBVAR_ID").toString().isEmpty()) {
            var_id = vars.value("VAR_ID").toString();
            tolerance = vars.value("VAR_TOLERANCE").toDouble();
        } else {
            var_id = vars.value("SUBVAR_ID").toString();
            tolerance = vars.value("SUBVAR_TOLERANCE").toDouble();
        }
        if (!nominal) {
            if (vars.value("VAR_COMPARE_NOM").toInt()) {
                compare_nom = true;
            } else if (vars.value("SUBVAR_COMPARE_NOM").toInt()) {
                compare_nom = true;
            }
        }
//*** Expressions and values ***
        if (!vars.value("VAR_VALUE").toString().isEmpty()) {
            expression = parseExpression(vars.value("VAR_VALUE").toString(), &used_ids);
        } else if (!vars.value("SUBVAR_VALUE").toString().isEmpty()) {
            expression = parseExpression(vars.value("SUBVAR_VALUE").toString(), &used_ids);
        }
        QString ins_value; QString nom_value;
        if (expression.count()) {
            ins_value = toString(evaluateExpression(inspection, expression, customer_id, circuit_id, &ok_eval));
            if (!ok_eval) continue;
            if (compare_nom) {
                nom_value = toString(evaluateExpression(nominal_ins, expression, customer_id, circuit_id, &ok_eval));
                if (!ok_eval) compare_nom = false;
            } else compare_nom = false;
        } else {
            ins_value = inspection.value(var_id).toString();
            if (compare_nom) {
                nom_value = nominal_ins.value(var_id).toString();
                if (nom_value == "") compare_nom = false;
            } else compare_nom = false;
        }
        if (ins_value.isEmpty()) continue;
        if (var_id == "inspector") {
            MTRecord inspector("inspector", ins_value, MTDictionary());
            ins_value = inspector.list().value("person").toString();
        }
        out << "<num_var>" << num_shown_vars << "</num_var>";
        out << "<tr><td style=\"text-align: right; width:50%;\">";
        if (vars.value("SUBVAR_ID").toString().isEmpty()) {
            out << vars.value("VAR_NAME").toString() << ":";
        } else {
            out << vars.value("VAR_NAME").toString() << ": " << vars.value("SUBVAR_NAME").toString() << ":";
        }
        out << "</td><td><table cellpadding=\"0\" cellspacing=\"0\"><tr><td align=\"right\" valign=\"center\">";
        if (compare_nom) {
            out << compareValues(nom_value.toDouble(), ins_value.toDouble(), tolerance).arg(ins_value);
        } else {
            out << ins_value;
        }
        out << "</td>";
        if (!vars.value("VAR_UNIT").toString().isEmpty()) {
            out << "<td valign=\"center\">&nbsp;";
            out << vars.value("VAR_UNIT").toString();
            out << "</td>";
        } else if (!vars.value("SUBVAR_UNIT").toString().isEmpty()) {
            out << "<td valign=\"center\">&nbsp;";
            out << vars.value("SUBVAR_UNIT").toString();
            out << "</td>";
        }
        out << "</tr></table></td>";
        num_shown_vars++;
    }
    if (num_shown_vars) {
        html.replace(QString("<num_var>%1</num_var>").arg(int(num_shown_vars / 2 + num_shown_vars % 2)), "</table></td><td style=\"width:50%;\"><table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">");
    }
    for (int i = 0; i < num_shown_vars; ++i) {
        html.remove(QString("<num_var>%1</num_var>").arg(i));
    }
    out << "</table></td></tbody>";
    out << "</table></td></tr>";
//*** Warnings ***
    QStringList warnings_list = listWarnings(inspection, nominal_ins, customer_id, circuit_id, used_ids);
    if (warnings_list.count()) {
        out << "<tr><td><table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">";
        out << "<tr><td colspan=\"2\" style=\"font-size: larger; width:100%;\">";
        out << "<b><i18n>" << tr("Warnings") << "</i18n></b></td></tr>";
        out << "<tr><td colspan=\"2\">";
        out << warnings_list.join(", ");
        out << "</td></tr>";
        out << "</table></td></tr>";
    }
    out << "</table>";
    wv_main->setHtml(dict_html.value(tr("Inspection information")).arg(html));
}

void MainWindow::viewTable(const QString & customer_id, const QString & circuit_id, const QString & table_id, int year)
{
    QString html; QTextStream out(&html);

    Variables vars;

    QStringList used_ids = listVariableIds();

//*** Mapping variables ***
    QMap<QString, QMap<QString, QVariant> > variables;
    QString last_id; QMap<QString, QVariant> map; QList<QVariant> subvariables;
    while (vars.next()) {
        if (vars.value("VAR_ID").toString() != last_id) {
            if (!last_id.isEmpty()) {
                map.insert("subvariables", subvariables);
                variables.insert(last_id, map);
            }
            map.clear(); subvariables.clear();
            map.insert("name", vars.value("VAR_NAME").toString());
            map.insert("type", vars.value("VAR_TYPE").toString());
            map.insert("unit", vars.value("VAR_UNIT").toString());
            map.insert("value", vars.value("VAR_VALUE").toString());
            map.insert("compare_nom", vars.value("VAR_COMPARE_NOM").toString());
            map.insert("col_bg", vars.value("VAR_COL_BG").toString());
            map.insert("tolerance", vars.value("VAR_TOLERANCE").toString());
            last_id = vars.value("VAR_ID").toString();
        }
        if (!vars.value("SUBVAR_ID").toString().isEmpty()) {
            QMap<QString, QVariant> subvariable;
            subvariable.insert("id", vars.value("SUBVAR_ID").toString());
            subvariable.insert("name", vars.value("SUBVAR_NAME").toString());
            subvariable.insert("type", vars.value("SUBVAR_TYPE").toString());
            subvariable.insert("unit", vars.value("SUBVAR_UNIT").toString());
            subvariable.insert("value", vars.value("SUBVAR_VALUE").toString());
            subvariable.insert("compare_nom", vars.value("SUBVAR_COMPARE_NOM").toString());
            subvariable.insert("tolerance", vars.value("SUBVAR_TOLERANCE").toString());
            subvariables << QVariant(subvariable);
        }
    }
    if (!last_id.isEmpty()) {
        map.insert("subvariables", subvariables);
        variables.insert(last_id, map);
    }

    MTRecord table_record("table", table_id, MTDictionary());
    QMap<QString, QVariant> table = table_record.list();
    QStringList table_vars = table.value("variables").toString().split(";", QString::SkipEmptyParts);

    MTDictionary inspection_parents("circuit", circuit_id);
    inspection_parents.insert("customer", customer_id);
    MTRecord inspection_record("inspection", "", inspection_parents);
    QList<QMap<QString, QVariant> > inspections = inspection_record.listAll();
    for (int i = 0; i < inspections.count(); ++i) {
        if (table.value("highlight_nominal").toInt() && inspections.at(i).value("nominal").toInt()) {}
        else if (inspections.at(i).value("date").toString().split(".").first().toInt() < year) {
            inspections.removeAt(i);
            i--;
        }
    }

    QMap<QString, QVariant> nominal_ins;
    for (int i = 0; i < inspections.count(); ++i) {
        if (inspections.at(i).value("nominal").toInt() == 1) {
            nominal_ins = inspections[i];
            break;
        }
    }

//*** Top tables ***
    MTRecord customer("customer", customer_id, MTDictionary());
    QMap<QString, QVariant> customer_info = customer.list("company, contact_person, address, mail, phone");
    MTRecord circuit("circuit", circuit_id, MTDictionary("parent", customer_id));
    QMap<QString, QVariant> circuit_info = circuit.list("manufacturer, type, sn, year, commissioning, field, refrigerant, refrigerant_amount, oil, oil_amount, life, runtime, utilisation");
    out << "<table><tr><th>" << tr("ID");
    out << "</th><th>" << tr("Company");
    out << "</th><th>" << tr("Contact person");
    out << "</th><th>" << tr("Address");
    out << "</th><th>" << tr("E-mail");
    out << "</th><th>" << tr("Phone");
    out << "</th></tr><tr>";
    out << "<td><a href=\"customer:" << customer_id << "\">" << customer_id.rightJustified(8, '0') << "</a></td>";
    out << "<td>" << customer_info.value("company").toString() << "</td>";
    out << "<td>" << customer_info.value("contact_person").toString() << "</td>";
    out << "<td>" << customer_info.value("address").toString() << "</td>";
    out << "<td>" << customer_info.value("mail").toString() << "</td>";
    out << "<td>" << customer_info.value("phone").toString() << "</td>";
    out << "</tr></table><br />";
    out << "<table><tr><th>" << tr("ID");
    out << "</th><th>" << tr("Manufacturer");
    out << "</th><th>" << tr("Type");
    out << "</th><th>" << tr("Year of purchase");
    out << "</th><th>" << tr("Date of commissioning");
    out << "</th><th>" << tr("Refrigerant");
    out << "</th><th>" << tr("Amount of refrigerant");
    out << "</th><th>" << tr("Oil");
    out << "</th><th>" << tr("Amount of oil");
    out << "</th><th>" << tr("Service life");
    out << "</th></tr><tr>";
    out << "<td><a href=\"customer:" << customer_id << "/circuit:" << circuit_id << "\">" << circuit_id.rightJustified(4, '0') << "</a></td>";
    out << "<td>" << circuit_info.value("manufacturer").toString() << "</td>";
    out << "<td>" << circuit_info.value("type").toString() << "</td>";
    out << "<td>" << circuit_info.value("year").toString() << "</td>";
    out << "<td>" << circuit_info.value("commissioning").toString() << "</td>";
    out << "<td>" << circuit_info.value("refrigerant").toString() << "</td>";
    out << "<td>" << circuit_info.value("refrigerant_amount").toString() << "</td>";
    out << "<td>";
    if (dict_attrvalues.contains("oil::" + circuit_info.value("oil").toString())) {
        out << dict_attrvalues.value("oil::" + circuit_info.value("oil").toString());
    }
    out << "</td>";
    out << "<td>" << circuit_info.value("oil_amount").toString() << "</td>";
    out << "<td>" << circuit_info.value("life").toString() << "</td>";
    out << "</td></tr></table><br />";

    out << "<table>";

//*** Head ***
    out << "<thead>";
    out << "<tr class=\"border_top\">";
    out << "<th rowspan=\"3\">" << tr("Date") << "</th>";
    for (int i = 0; i < table_vars.count(); ++i) {
        out << "<th colspan=\"";
        int subvar_count = variables.value(table_vars.at(i)).value("subvariables").toList().count();
        out << subvar_count;
        out << "\" rowspan=\"";
        if (subvar_count > 0) out << 1;
        else if (variables.value(table_vars.at(i)).value("unit").toString() != "") out << 2;
        else out << 3;
        out << "\" class=\"";
        out << variables.value(table_vars.at(i)).value("col_bg").toString();
        out << "\">";
        out << variables.value(table_vars.at(i)).value("name").toString();
        out << "</th>";
    }
    out << "</tr><tr>";
    for (int i = 0; i < table_vars.count(); ++i) {
        if (variables.value(table_vars.at(i)).value("subvariables").toList().count() > 0) {
            subvariables = variables.value(table_vars.at(i)).value("subvariables").toList();
            for (int n = 0; n < subvariables.count(); ++n) {
                out << "<th rowspan=\"";
                if (subvariables.at(n).toMap().value("unit").toString().isEmpty()) {
                    out << 2;
                } else out << 1;
                out << "\" class=\"" << variables.value(table_vars.at(i)).value("col_bg").toString();
                out << "\">" << subvariables.at(n).toMap().value("name").toString() << "</th>";
            }
        }
    }
    out << "</tr><tr class=\"border_bottom\">";
    for (int i = 0; i < table_vars.count(); ++i) {
        if (variables.value(table_vars.at(i)).value("subvariables").toList().count() > 0) {
            subvariables = variables.value(table_vars.at(i)).value("subvariables").toList();
            for (int n = 0; n < subvariables.count(); ++n) {
                if (!subvariables.at(n).toMap().value("unit").toString().isEmpty()) {
                    out << "<th class=\"" << variables.value(table_vars.at(i)).value("col_bg").toString();
                    out << "\">" << subvariables.at(n).toMap().value("unit").toString() << "</th>";
                }
            }
        } else {
            if (!variables.value(table_vars.at(i)).value("unit").toString().isEmpty()) {
                out << "<th class=\"" << variables.value(table_vars.at(i)).value("col_bg").toString();
                out << "\">" << variables.value(table_vars.at(i)).value("unit").toString() << "</th>";
            }
        }
    }
    out << "</tr></thead>";

//*** Body ***
    out << "<tbody>";
    for (int i = 0; i < inspections.count(); ++i) {
     out << "<tr class=\"";
        if (inspections.at(i).value("nominal").toInt() == 1 && table.value("highlight_nominal").toInt() != 0) {
            out << "nominal";
        }
        out << "\"><td><a href=\"customer:" << customer_id << "/circuit:" << circuit_id << "/inspection:" << inspections.at(i).value("date").toString() << "\">";
        out << inspections.at(i).value("date").toString() << "</a></td>";
        for (int n = 0; n < table_vars.count(); ++n) {
            bool compare_nom = false; int rowspan = 1; QString ins_value = ""; QString nom_value = ""; bool ok_eval;
            if (variables.value(table_vars.at(n)).value("subvariables").toList().count() > 0) {
                subvariables = variables.value(table_vars.at(n)).value("subvariables").toList();
                for (int s = 0; s < subvariables.count(); ++s) {
                    compare_nom = subvariables.at(s).toMap().value("compare_nom").toInt() > 0;
                    if (subvariables.at(s).toMap().value("value").toString().contains("sum")) {
                        QString i_year = inspections.at(i).value("date").toString().split(".").first();
                        if (inspections.at(i).value("nominal").toInt()) rowspan = 1;
                        else if (i > 0 && !inspections.at(i-1).value("nominal").toInt() && inspections.at(i-1).value("date").toString().split(".").first() == i_year) continue;
                        else {
                            int in = i;
                            for (; in < inspections.count(); ++in) {
                                if (inspections.at(in).value("nominal").toInt()) in--;
                                if (inspections.at(in).value("date").toString().split(".").first() != i_year) {
                                    break;
                                }
                            }
                            rowspan = in - i;
                        }
                    } else rowspan = 1;
                    if (subvariables.at(s).toMap().value("value").toString().isEmpty()) {
                        ins_value = inspections.at(i).value(subvariables.at(s).toMap().value("id").toString()).toString();
                        if (compare_nom) {
                            nom_value = nominal_ins.value(subvariables.at(s).toMap().value("id").toString()).toString();
                            if (nom_value.isEmpty()) compare_nom = false;
                        }
                    } else {
                        MTDictionary expression = parseExpression(subvariables.at(s).toMap().value("value").toString(), &used_ids);
                        ins_value = toString(evaluateExpression(inspections[i], expression, customer_id, circuit_id, &ok_eval));
                        if (!ok_eval) ins_value = "";
                        if (compare_nom) {
                            nom_value = toString(evaluateExpression(nominal_ins, expression, customer_id, circuit_id, &ok_eval));
                            if (!ok_eval) compare_nom = false;
                        }
                    }
                    if (subvariables.at(s).toMap().value("type").toString() == "bool") {
                        ins_value = ins_value.toInt() ? tr("Yes") : tr("No");
                    }
                    writeTableVarCell(out, ins_value, nom_value, variables.value(table_vars.at(n)).value("col_bg").toString(), compare_nom, rowspan, subvariables.at(s).toMap().value("tolerance").toDouble());
                }
            } else {
                compare_nom = variables.value(table_vars.at(n)).value("compare_nom").toInt() > 0;
                if (variables.value(table_vars.at(n)).value("value").toString().isEmpty()) {
                    ins_value = inspections.at(i).value(table_vars.at(n)).toString();
                    if (compare_nom) {
                        nom_value = nominal_ins.value(table_vars.at(n)).toString();
                        if (nom_value.isEmpty()) compare_nom = false;
                    }
                } else {
                    MTDictionary expression = parseExpression(variables.value(table_vars.at(n)).value("value").toString(), &used_ids);
                    ins_value = toString(evaluateExpression(inspections[i], expression, customer_id, circuit_id, &ok_eval));
                    if (!ok_eval) ins_value = "";
                    if (compare_nom) {
                        nom_value = toString(evaluateExpression(nominal_ins, expression, customer_id, circuit_id, &ok_eval));
                        if (!ok_eval) compare_nom = false;
                    }
                }
                if (variables.value(table_vars.at(n)).value("type").toString() == "bool") {
                    ins_value = ins_value.toInt() ? tr("Yes") : tr("No");
                }
                if (table_vars.at(n) == "inspector") {
                    MTRecord inspector("inspector", ins_value, MTDictionary());
                    ins_value = inspector.list().value("person").toString();
                }
                writeTableVarCell(out, ins_value, nom_value, variables.value(table_vars.at(n)).value("col_bg").toString(), compare_nom, rowspan, variables.value(table_vars.at(n)).value("tolerance").toDouble());
            }
        }
        out << "</tr>";
    }
    out << "</tbody>";
    out << "<tfoot>";

//*** Foot ***
    if (!table.value("sum").toString().isEmpty()) {
        out << "<tr class=\"border_top border_bottom\">";
        out << "<th>" << tr("Sum") << "</th>";
        QStringList sum_vars = table.value("sum").toString().split(";", QString::SkipEmptyParts);
        for (int i = 0; i < table_vars.count(); ++i) {
            bool is_in_foot = sum_vars.contains(table_vars.at(i));
            if (variables.value(table_vars.at(i)).value("subvariables").toList().count() > 0) {
                subvariables = variables.value(table_vars.at(i)).value("subvariables").toList();
                for (int s = 0; s < subvariables.count(); ++s) {
                    is_in_foot = sum_vars.contains(table_vars.at(i));
                    if (subvariables.at(s).toMap().value("type").toString() != "float" && subvariables.at(s).toMap().value("type").toString() != "int") is_in_foot = false;
                    out << "<td class=\"" << variables.value(table_vars.at(i)).value("col_bg").toString() << "\">";
                    if (is_in_foot) {
                        double value = 0;
                        if (subvariables.at(s).toMap().value("value").toString().isEmpty()) {
                            for (int ins = 0; ins < inspections.count(); ++ins) {
                                value += inspections.at(ins).value(subvariables.at(s).toMap().value("id").toString()).toDouble();
                            }
                            out << value;
                        } else {
                            MTDictionary expression = parseExpression(subvariables.at(s).toMap().value("value").toString(), &used_ids);
                            for (int ins = 0; ins < inspections.count(); ++ins) {
                                if (subvariables.at(s).toMap().value("value").toString().contains("sum")) {
                                    if (ins > 0 && !inspections.at(ins-1).value("nominal").toInt() && inspections.at(ins-1).value("date").toString().split(".").first() == inspections.at(ins).value("date").toString().split(".").first())
                                        continue;
                                }
                                value += evaluateExpression(inspections[ins], expression, customer_id, circuit_id);
                            }
                            out << value;
                        }
                    }
                    out << "</td>";
                }
            } else {
                if (variables.value(table_vars.at(i)).value("type").toString() != "float" && variables.value(table_vars.at(i)).value("type").toString() != "int") is_in_foot = false;
                out << "<td class=\"" << variables.value(table_vars.at(i)).value("col_bg").toString() << "\">";
                double value = 0;
                if (is_in_foot) {
                    if (variables.value(table_vars.at(i)).value("value").toString().isEmpty()) {
                        for (int ins = 0; ins < inspections.count(); ++ins) {
                            value += inspections.at(ins).value(table_vars.at(i)).toDouble();
                        }
                        out << value;
                    } else {
                        MTDictionary expression = parseExpression(variables.value(table_vars.at(i)).value("value").toString(), &used_ids);
                        for (int ins = 0; ins < inspections.count(); ++ins) {
                            value += evaluateExpression(inspections[ins], expression, customer_id, circuit_id);
                        }
                        out << value;
                    }
                }
                out << "</td>";
            }
        }
        out << "</tr>";
    }
    out << "</tfoot>";
    out << "</table>";

//*** Warnings ***
    QString warnings_html;
    QStringList last_warnings_list;
    for (int i = 0; i < inspections.count(); ++i) {
        QStringList warnings_list = listWarnings(inspections[i], nominal_ins, customer_id, circuit_id, used_ids);
        QStringList backup_warnings = warnings_list;
        for (int n = 0; n < warnings_list.count(); ++n) {
            if (last_warnings_list.contains(warnings_list.at(n))) {
                warnings_list[n].prepend("<span style=\"color: red;\"><b>");
                warnings_list[n].append("</b></span>");
            }
        }
        if (warnings_list.count() > 0) {
            warnings_html.append("<tr><td><a href=\"customer:" + customer_id + "/circuit:" + circuit_id + "/inspection:" + inspections.at(i).value("date").toString() + "\">");
            warnings_html.append(inspections.at(i).value("date").toString() + "</a>");
            warnings_html.append("</td><td>");
            warnings_html.append(warnings_list.join(", "));
            warnings_html.append("</td></tr>");
        }
        last_warnings_list = backup_warnings;
    }
    if (!warnings_html.isEmpty()) {
        out << "<br /><table>";
        out << "<tr><th width=\"20%\">" << tr("Date") << "</th><th>" << tr("Warnings") << "</th></tr>";
        out << warnings_html;
        out << "</table>";
    }
    wv_main->setHtml(dict_html.value(tr("Table of inspections")).arg(html), QUrl("qrc:/html/"));
}

void MainWindow::writeTableVarCell(QTextStream & out, const QString & ins_value, const QString & nom_value, const QString & bg_class, bool compare_nom, int rowspan, double tolerance)
{
    out << "<td class=\"" << bg_class;
    out << "\" rowspan=\"" << rowspan;
    out << "\">";
    if (compare_nom) {
        out << compareValues(nom_value.toDouble(), ins_value.toDouble(), tolerance).arg(ins_value);
    } else {
        out << ins_value;
    }
    out << "</td>";
}

QStringList MainWindow::listWarnings(QMap<QString, QVariant> & inspection, QMap<QString, QVariant> & nominal_ins, const QString & customer_id, const QString & circuit_id, QStringList & used_ids)
{
    Warnings warnings(db, true);
    QStringList warnings_list;
    MTRecord circuit("circuit", circuit_id, MTDictionary("parent", customer_id));
    QMap<QString, QVariant> circuit_attributes = circuit.list();
    while (warnings.next()) {
        bool show_warning = true;

        WarningFilters warnings_filters(warnings.value("id").toInt());
        while (warnings_filters.next()) {
            QString circuit_attribute = circuit_attributes.value(warnings_filters.value("circuit_attribute").toString()).toString();
            QString function = warnings_filters.value("function").toString();
            QString value = warnings_filters.value("value").toString();
            bool ok1 = true; bool ok2 = true;
            int int_circuit_attribute = circuit_attribute.toInt(&ok1);
            int int_value = value.toInt(&ok2);
            if (ok1 && ok2) {
                if (function == "=" && int_circuit_attribute == int_value) {}
                else if (function == "!=" && int_circuit_attribute != int_value) {}
                else if (function == ">" && int_circuit_attribute > int_value) {}
                else if (function == ">=" && int_circuit_attribute >= int_value) {}
                else if (function == "<" && int_circuit_attribute < int_value) {}
                else if (function == "<=" && int_circuit_attribute <= int_value) {}
                else {
                    show_warning = false;
                }
            } else {
                if (function == "=" && circuit_attribute == value) {}
                else if (function == "!=" && circuit_attribute != value) {}
                else if (function == ">" && circuit_attribute > value) {}
                else if (function == "<" && circuit_attribute < value) {}
                else {
                    show_warning = false;
                }
            }
        }

        if (!show_warning) { continue; }

        WarningConditions warnings_conditions(warnings.value("id").toInt());
        while (warnings_conditions.next()) {
            MTDictionary expression;
            bool ok_eval = true;
            double ins_value = 0;
            QString unparsed_expression = warnings_conditions.value("value_ins").toString();
            if (!unparsed_expression.isEmpty()) {
                if (!parsed_expressions.contains(unparsed_expression)) {
                    parsed_expressions.insert(unparsed_expression, parseExpression(unparsed_expression, &used_ids));
                }
                expression = parsed_expressions.value(unparsed_expression);
                ins_value = evaluateExpression(inspection, expression, customer_id, circuit_id, &ok_eval);
                if (!ok_eval) show_warning = false;
            }
            expression.clear();
            double nom_value = 0;
            unparsed_expression = warnings_conditions.value("value_nom").toString();
            if (!unparsed_expression.isEmpty()) {
                if (!parsed_expressions.contains(unparsed_expression)) {
                    parsed_expressions.insert(unparsed_expression, parseExpression(unparsed_expression, &used_ids));
                }
                expression = parsed_expressions.value(unparsed_expression);
                nom_value = evaluateExpression(nominal_ins, expression, customer_id, circuit_id, &ok_eval);
                if (!ok_eval) show_warning = false;
            }
            QString function = warnings_conditions.value("function").toString();
            if (function == "=" && ins_value == nom_value) {}
            else if (function == "!=" && ins_value != nom_value) {}
            else if (function == ">" && ins_value > nom_value) {}
            else if (function == ">=" && ins_value >= nom_value) {}
            else if (function == "<" && ins_value < nom_value) {}
            else if (function == "<=" && ins_value <= nom_value) {}
            else {
                show_warning = false;
            }
        }

        if (show_warning) {
            warnings_list << warnings.value("name").toString();
        }
    }
    return warnings_list;
}

void MainWindow::viewAllInspectors(const QString & highlighted_id)
{
    QString html; QTextStream out(&html);
    MTRecord inspectors_rec("inspector", "", MTDictionary());
    QList<QMap<QString, QVariant> > inspectors = inspectors_rec.listAll();

    for (int i = 0; i < inspectors.count(); ++i) {
        QString link;
        out << "<table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\"><tr style=\"background-color: ";
        if (highlighted_id == inspectors.at(i).value("id").toString()) {
            out << "#CCCCCC;";
            link = "inspector:" + inspectors.at(i).value("id").toString() + "/modify";
        } else {
            out << "#eee;";
            link = "inspector:" + inspectors.at(i).value("id").toString();
        }
        out << "\"><td colspan=\"2\" style=\"font-size: large; text-align: center;\"><b>" << tr("Inspector:") << "&nbsp;";
        out << "<a href=\"" << link << "\">" << inspectors.at(i).value("person").toString() << "</a></b></td></tr>";
        out << "<tr><td width=\"50%\"><table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\"><tr><td style=\"text-align: right; width:50%;\">" << tr("ID:") << "&nbsp;</td>";
        out << "<td>" << inspectors.at(i).value("id").toString().rightJustified(4, '0') << "</td></tr>";
        out << "<tr><td style=\"text-align: right; width:50%;\"><b>" << tr("Company:") << "&nbsp;</b></td>";
        out << "<td><b>" << inspectors.at(i).value("company").toString() << "</b></td></tr>";
        out << "</table></td><td><table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">";
        out << "<tr><td style=\"text-align: right; width:50%;\">" << tr("Person registry number:") << "&nbsp;</td>";
        out << "<td>" << inspectors.at(i).value("company_reg_num").toString() << "</td></tr>";
        out << "<tr><td style=\"text-align: right; width:50%;\">" << tr("Company registry number:") << "&nbsp;</td>";
        out << "<td>" << inspectors.at(i).value("person_reg_num").toString() << "</td></tr>";
        out << "</table></td></tr>";
    }
    wv_main->setHtml(dict_html.value(tr("Inspectors")).arg(html), QUrl("qrc:/html/"));
}

void MainWindow::viewRefrigerantConsumption(const QString & customer_id)
{
    QString html; QTextStream out(&html);
    QSqlQuery query;
    query.prepare("SELECT circuits.field, circuits.refrigerant, inspections.refr_add_am, inspections.refr_reco, inspections.refr_recy, inspections.refr_disp FROM inspections LEFT JOIN circuits ON inspections.circuit = circuits.id AND inspections.customer = circuits.parent" + QString(customer_id.toInt() < 0 ? "" : " WHERE inspections.customer = :customer_id"));
    if (customer_id.toInt() >= 0) { query.bindValue(":customer_id", customer_id); }
    query.exec();
    QMap<QString, QStringList> sums_map; QString current_name;
    while (query.next()) {
        current_name = query.value(1).toString() + "<:?:>" + query.value(0).toString();
        //double old_value = sums_map.value(current_name);
        QStringList old_str_list = sums_map.value(current_name);
        QStringList new_str_list;
        for (int i = 0; i < 4; ++i) {
            if (old_str_list.count() != 4) {
                new_str_list << query.value(2+i).toString();
            } else {
                new_str_list << toString(old_str_list.at(i).toDouble() + query.value(2+i).toDouble());
            }
        }
        //sums_map.insert(current_name, old_value + query.value(2).toDouble());
        sums_map.insert(current_name, new_str_list);
    }
    QMapIterator <QString, QStringList> iter(sums_map);

    MTDictionary used_fields;
    MTDictionary used_refrs;
    while (iter.hasNext()) {
        iter.next();
        QString refr = iter.key().split("<:?:>").first();
        QString field = iter.key().split("<:?:>").last();
        if (refr == field) continue;
        if (refr != "") {
            used_refrs.insert(refr, dict_attrvalues.value(dict_attrvalues.indexOfKey("refrigerant::" + refr)));
        }
        if (field != "") {
            used_fields.insert(field, dict_attrvalues.value(dict_attrvalues.indexOfKey("field::" + field)));
        }
    }

    QString ver_refr_str; QString refr_str = tr("Refrigerants");
    for (int i = 0; i < refr_str.count(); ++i) {
        ver_refr_str.append(QString(refr_str.at(i)) + "<br />");
    }

    out << "<table class=\"default_table\" cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\"><thead><tr class=\"normal_table\" style=\"background-color:#eee\">";
    out << "<td class=\"normal_table\" style=\"font-size: large; text-align: center;\"><b>" << tr("Refrigerant consumption:") << "&nbsp;";
    if (customer_id.toInt() < 0) {
        out << "<a href=\"allcustomers:\">" << tr("All customers") << "</a>";
    } else {
        out << tr("Customer:") << "&nbsp;" << "<a href=\"customer:" << customer_id << "\">" << customer_id.rightJustified(8, '0') << "</a>";
    }
    out << "</b></td></tr></thead></table>";

    QStringList default_return_list;
    default_return_list << "0"; default_return_list << "0"; default_return_list << "0"; default_return_list << "0";
    QStringList tables;
    tables << dict_varnames.value("refr_add"); tables << dict_varnames.value("refr_reco");
    tables << dict_varnames.value("refr_recy"); tables << dict_varnames.value("refr_disp");
    for (int t = 0; t < tables.count(); ++t) {
        out << "<br /><table><thead><tr><th colspan=\"2\" rowspan=\"2\">" << tables.at(t) << "</th>";
        out << "<th colspan=\"" << used_fields.count()+1 << "\">" << tr("Fields") << "</th></tr>";
        out << "<tr><th>" << tr("All") << "</th>";
        for (int i = 0; i < used_fields.count(); ++i) {
            out << "<th>" << used_fields.value(i) << "</th>";
        }
        out << "</tr></thead>";
        out << "<tr><th rowspan=\"" << used_refrs.count()+2 << "\">" << ver_refr_str << "</th>";
        out << "<th>" << tr("All") << "</th>";
        double s_value = 0;
        iter.toFront();
        while(iter.hasNext()) {
            iter.next();
            s_value += iter.value().at(t).toDouble();
        }
        out << "<td>" << s_value << "</td>";
        for (int n = 0; n < used_fields.count(); ++n) {
            s_value = 0;
            iter.toFront();
            while(iter.hasNext()) {
                iter.next();
                if (iter.key().endsWith(used_fields.key(n))) {
                    s_value += iter.value().at(t).toDouble();
                }
            }
            out << "<td>" << s_value << "</td>";
        }
        out << "</tr>";
        for (int i = 0; i < used_refrs.count(); ++i) {
            out << "<tr><th>" << used_refrs.value(i) << "</th>";
            s_value = 0;
            iter.toFront();
            while(iter.hasNext()) {
                iter.next();
                if (iter.key().startsWith(used_refrs.key(i))) {
                    s_value += iter.value().at(t).toDouble();
                }
            }
            out << "<td>" << s_value << "</td>";
            for (int n = 0; n < used_fields.count(); ++n) {
                current_name = used_refrs.key(i) + "<:?:>" + used_fields.key(n);
                out << "<td>" << sums_map.value(current_name, default_return_list).at(t).toDouble() << "</td>";
            }
            out << "</tr>";
        }
        out << "<tr></tr></table>";
    }
    wv_main->setHtml(dict_html.value(tr("Refrigerant consumption")).arg(html), QUrl("qrc:/html/"));
}
