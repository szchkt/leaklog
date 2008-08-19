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

    bool table_view = cb_view->currentText() == tr("Table of inspections");
    lbl_table->setEnabled(table_view);
    cb_table->setEnabled(table_view);
    lbl_since->setEnabled(table_view);
    spb_since->setEnabled(table_view);

    if (view == tr("All customers")) {
        viewAllCustomers();
    } else if (view == tr("Customer information") && lw_customers->highlightedRow() >= 0) {
        viewCustomer(lw_customers->highlightedItem()->data(Qt::UserRole).toString());
    } else if (view == tr("Circuit information") && lw_customers->highlightedRow() >= 0 && lw_circuits->highlightedRow() >= 0) {
        viewCircuit(lw_customers->highlightedItem()->data(Qt::UserRole).toString(), lw_circuits->highlightedItem()->data(Qt::UserRole).toString());
    } else if (view == tr("Inspection information") && lw_customers->highlightedRow() >= 0 && lw_circuits->highlightedRow() >= 0 && lw_inspections->highlightedRow() >= 0) {
        viewInspection(lw_customers->highlightedItem()->data(Qt::UserRole).toString(), lw_circuits->highlightedItem()->data(Qt::UserRole).toString(), lw_inspections->highlightedItem()->data(Qt::UserRole).toString());
    } else if (table_view && lw_customers->highlightedRow() >= 0 && lw_circuits->highlightedRow() >= 0 && cb_table->currentIndex() >= 0) {
        viewTable(lw_customers->highlightedItem()->data(Qt::UserRole).toString(), lw_circuits->highlightedItem()->data(Qt::UserRole).toString(), cb_table->currentText(), spb_since->value() == 1999 ? 0 : spb_since->value());
    } else {
        wv_main->setHtml(QString());
    }
}

void MainWindow::viewAllCustomers()
{
    QString html; QTextStream out(&html);
    QSqlQuery query;
    query.setForwardOnly(true);
    query.prepare("SELECT id, company, contact_person, address, mail, phone FROM customers ORDER BY id");
    query.exec();
    //query.setForwardOnly();
    //query.prepare("");
    //query.bindVariable();
    //query.exec();
    while (query.next()) {
        out << "<tr style=\"background-color: #eee;\"><td colspan=\"2\" style=\"font-size: large; text-align: center;\"><b>" << tr("Company:") << "&nbsp;";
        out << "<a href=\"customer:" << query.value(0).toString() << "\">" << query.value(1).toString() << "</a></b></td></tr>";
        out << "<tr><td><table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\"><tr><td style=\"text-align: right; width:50%;\">" << tr("ID:") << "&nbsp;</td>";
        out << "<td>" << query.value(0).toString() << "</td></tr>";
        out << "<tr><td style=\"text-align: right; width:50%;\"><b>" << tr("Contact person:") << "&nbsp;</b></td>";
        out << "<td><b>" << query.value(2).toString() << "</b></td></tr>";
        out << "<tr><td style=\"text-align: right; width:50%;\">" << tr("Address:") << "&nbsp;</td>";
        out << "<td>" << query.value(3).toString() << "</td></tr>";
        out << "<tr><td style=\"text-align: right; width:50%;\">" << tr("E-mail:") << "&nbsp;</td>";
        out << "<td>" << query.value(4).toString() << "</td></tr>";
        out << "</table></td><td><table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">";
        out << "<tr><td style=\"text-align: right; width:50%;\">" << tr("Phone:") << "&nbsp;</td>";
        out << "<td>" << query.value(5).toString() << "</td></tr>";
        out << "<tr><td style=\"text-align: right; width:50%;\">" << tr("Number of circuits:") << "&nbsp;</td>";
        out << "<td>";
        QSqlQuery circuits;
        circuits.setForwardOnly(true);
        circuits.prepare("SELECT id FROM circuits WHERE parent = :parent");
        circuits.bindValue(":parent", query.value(0).toInt());
        circuits.exec();
        int num_circuits = 0, num_inspections = 0;
        while (circuits.next()) {
            num_circuits++;
            QSqlQuery inspections;
            inspections.setForwardOnly(true);
            inspections.prepare("SELECT COUNT(date) FROM inspections WHERE parent = :parent");
            inspections.bindValue(":parent", circuits.value(0).toInt());
            inspections.exec();
            if (inspections.next()) { num_inspections += inspections.value(0).toInt(); }
        }
        out << num_circuits;
        out << "</td></tr>";
        out << "<tr><td style=\"text-align: right; width:50%;\">" << tr("Total number of inspections:") << "&nbsp;</td>";
        out << "<td>" << num_inspections << "</td></tr>";
        out << "</table></td></tr>";
    }
    wv_main->setHtml(dict_html.value(tr("All customers")).arg(html));
}

void MainWindow::viewCustomer(const QString & customer_id)
{
    QString html; QTextStream out(&html);
    QSqlQuery query;
    query.setForwardOnly(true);
    query.prepare("SELECT company, contact_person, address, mail, phone FROM customers WHERE id = :customer_id");
    query.bindValue(":customer_id", customer_id);
    query.exec();
    //query.setForwardOnly();
    //query.prepare("");
    //query.bindVariable();
    //query.exec();
    while (query.next()) {
        out << "<table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">";
        out << "<tr style=\"background-color: #DFDFDF;\"><td colspan=\"2\" style=\"font-size: larger; width:100%; text-align: center;\"><b>" << tr("Company:") << "&nbsp;";
        out << "<a href=\"customer:" << customer_id << "/modify\">" << query.value(0).toString() << "</a></b></td></tr>";
        out << "<tr><td><table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">";
        out << "<tr><td style=\"text-align: right; width:50%;\">" << tr("ID:") << "&nbsp;</td>";
        out << "<td style=\"width:50%;\">" << customer_id << "</td></tr>";
        out << "<tr><td style=\"text-align: right;\"><b>" << tr("Contact person:") << "&nbsp;</b></td>";
        out << "<td><b>" << query.value(1).toString() << "</b></td></tr>";
        out << "<tr><td style=\"text-align: right;\">" << tr("Address:") << "&nbsp;</td>";
        out << "<td>" << query.value(2).toString() << "</td></tr>";
        out << "<tr><td style=\"text-align: right;\">" << tr("E-mail:") << "&nbsp;</td>";
        out << "<td>" << query.value(3).toString() << "</td></tr>";
        out << "</table></td>";
        out << "<td><table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">";
        out << "<tr><td style=\"text-align: right; width:50%;\">" << tr("Phone:") << "&nbsp;</td>";
        out << "<td style=\"width:50%;\">" << query.value(4).toString() << "</td></tr>";
        out << "<tr><td style=\"text-align: right; width:50%;\">" << tr("Number of circuits:") << "&nbsp;</td>";
        out << "<td>";
        QSqlQuery circuits;
        //circuits.setForwardOnly(true);
        circuits.prepare("SELECT id, manufacturer, type, sn, year, commissioning, field, refrigerant, refrigerant_amount, oil, oil_amount, life, runtime, utilisation FROM circuits WHERE parent = :parent ORDER BY id");
        circuits.bindValue(":parent", customer_id.toInt());
        circuits.exec();
        int num_circuits = 0, num_inspections = 0;
        while (circuits.next()) {
            num_circuits++;
            QSqlQuery inspections;
            inspections.setForwardOnly(true);
            inspections.prepare("SELECT COUNT(date) FROM inspections WHERE parent = :parent");
            inspections.bindValue(":parent", circuits.value(0).toInt());
            inspections.exec();
            if (inspections.next()) { num_inspections += inspections.value(0).toInt(); }
        }
        out << num_circuits;
        out << "</td></tr>";
        out << "<tr><td style=\"text-align: right; width:50%;\">" << tr("Total number of inspections:") << "&nbsp;</td>";
        out << "<td>" << num_inspections << "</td></tr>";
        out << "</table></td></tr>";
        out << "</table>";
        if (circuits.first()) {
            do {
                out << "<table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\"><tr><td rowspan=\"8\" style=\"width:10%;\"/>";
                out << "<td colspan=\"2\" style=\"background-color: #eee; font-size: medium; text-align: center; width:80%;\"><b>" << tr("Circuit:") << "&nbsp;";
                out << "<a href=\"customer:" << customer_id << "/circuit:" << circuits.value(0).toString() << "\">" << circuits.value(0).toString() << "</a></b></td>";
                out << "<td rowspan=\"8\" style=\"width:10%;\"/></tr>";
                out << "<tr><td><table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">";
                out << "<tr><td style=\"text-align: right; width:50%;\"><b>" << tr("Manufacturer:") << "&nbsp;</b></td>";
                out << "<td style=\"width:50%;\"><b>" << circuits.value(1).toString() << "</b></td></tr>";
                out << "<tr><td style=\"text-align: right;\">" << tr("Type:") << "&nbsp;</td>";
                out << "<td>" << circuits.value(2).toString() << "</td></tr>";
                out << "<tr><td style=\"text-align: right;\">" << tr("Serial number:") << "&nbsp;</td>";
                out << "<td>" << circuits.value(3).toString() << "</td></tr>";
                out << "<tr><td style=\"text-align: right; width:50%;\">" << tr("Year of purchase:") << "&nbsp;</td>";
                out << "<td>" << circuits.value(4).toString() << "</td></tr>";
                out << "<tr><td style=\"text-align: right; width:50%;\">" << tr("Date of commissioning:") << "&nbsp;</td>";
                out << "<td style=\"width:50%;\">" << circuits.value(5).toString() << "</td></tr>";
                out << "<tr><td style=\"text-align: right; width:50%;\">" << tr("Field of application:") << "&nbsp;</td>";
                out << "<td style=\"width:50%;\">" << circuits.value(6).toString() << "</td></tr>";
                out << "</table></td>";
                out << "<td><table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">";
                out << "<tr><td style=\"text-align: right; width:50%;\">" << tr("Refrigerant:") << "&nbsp;</td>";
                out << "<td style=\"width:50%;\">" << circuits.value(7).toString() << "</td></tr>";
                out << "<tr><td style=\"text-align: right;\">" << tr("Amount of refrigerant:") << "&nbsp;</td>";
                out << "<td>" << circuits.value(8).toString() << "&nbsp;kg</td></tr>";
                out << "<tr><td style=\"text-align: right;\">" << tr("Oil:") << "&nbsp;</td>";
                out << "<td>" << circuits.value(9).toString() << "</td></tr>";
                out << "<tr><td style=\"text-align: right;\">" << tr("Amount of oil:") << "&nbsp;</td>";
                out << "<td>" << circuits.value(10).toString() << "&nbsp;kg</td></tr>";
                out << "<tr><td style=\"text-align: right;\">" << tr("Service life:") << "&nbsp;</td>";
                out << "<td>" << circuits.value(11).toString() << "&nbsp;" << tr("years") << "</td></tr>";
                out << "<tr><td style=\"text-align: right;\">" << tr("Run-time per day:") << "&nbsp;</td>";
                out << "<td>" << circuits.value(12).toString() << "</td></tr>";
                out << "<tr><td style=\"text-align: right;\">" << tr("Rate of utilisation:") << "&nbsp;</td>";
                out << "<td>" << circuits.value(13).toString() << "&nbsp;%</td></tr>";
                out << "</table></td></tr>";
                out << "</table>";
            } while (circuits.next());
        }
    }
    wv_main->setHtml(dict_html.value(tr("Customer information")).arg(html));
}

void MainWindow::viewCircuit(const QString & customer_id, const QString & circuit_id)
{
    QString html; QTextStream out(&html);
    QSqlQuery query;
    query.setForwardOnly(true);
    query.prepare("SELECT manufacturer, type, sn, year, commissioning, field, refrigerant, refrigerant_amount, oil, oil_amount, life, runtime, utilisation FROM circuits WHERE parent = :customer_id AND id = :circuit_id");
    query.bindValue(":customer_id", customer_id);
    query.bindValue(":circuit_id", circuit_id);
    query.exec();
    //query.setForwardOnly();
    //query.prepare("");
    //query.bindVariable();
    //query.exec();
    while (query.next()) {
        out << "<table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">";
        out << "<tr style=\"background-color: #eee;\"><td colspan=\"2\" style=\"font-size: larger; width:100%; text-align: center;\"><b>" << tr("Company:") << "&nbsp;";
        out << "<a href=\"customer:" << customer_id << "\">";
        QSqlQuery customer;
        customer.prepare("SELECT company FROM customers WHERE id = :customer_id");
        customer.bindValue(":customer_id", customer_id);
        customer.exec();
        if (customer.next()) { out << customer.value(0).toString(); }
        out << "</a></b></td></tr>";
        out << "<tr style=\"background-color: #DFDFDF;\"><td colspan=\"2\" style=\"font-size: large; width:100%; text-align: center;\"><b>" << tr("Circuit:") << "&nbsp;";
        out << "<a href=\"customer:" << customer_id << "/circuit:" << circuit_id << "/modify\">" << circuit_id << "</a></b></td></tr>";
        out << "<tr><td><table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">";
        out << "<tr><td style=\"text-align: right; width:50%;\"><b>" << tr("Manufacturer:") << "&nbsp;</b></td><td style=\"width:50%;\"><b>" << query.value(0).toString() << "</b></td></tr>";
        out << "<tr><td style=\"text-align: right;\">" << tr("Type:") << "&nbsp;</td><td>" << query.value(1).toString() << "</td></tr>";
        out << "<tr><td style=\"text-align: right;\">" << tr("Serial number:") << "&nbsp;</td><td>" << query.value(2).toString() << "</td></tr>";
        out << "<tr><td style=\"text-align: right; width:50%;\">" << tr("Year of purchase:") << "&nbsp;</td><td>" << query.value(3).toString() << "</td></tr>";
        out << "<tr><td style=\"text-align: right; width:50%;\">" << tr("Date of commissioning:") << "&nbsp;</td><td style=\"width:50%;\">" << query.value(4).toString() << "</td></tr>";
        out << "<tr><td style=\"text-align: right; width:50%;\">" << tr("Field of application:") << "&nbsp;</td><td style=\"width:50%;\">" << query.value(5).toString() << "</td></tr>";
        out << "</table></td>";
        out << "<td><table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">";
        out << "<tr><td style=\"text-align: right; width:50%;\">" << tr("Refrigerant:") << "&nbsp;</td><td style=\"width:50%;\">" << query.value(6).toString() << "</td></tr>";
        out << "<tr><td style=\"text-align: right;\">" << tr("Amount of refrigerant:") << "&nbsp;</td><td>" << query.value(7).toString() << "&nbsp;kg</td></tr>";
        out << "<tr><td style=\"text-align: right;\">" << tr("Oil:") << "&nbsp;</td><td>" << query.value(8).toString() << "</td></tr>";
        out << "<tr><td style=\"text-align: right;\">" << tr("Amount of oil:") << "&nbsp;</td><td>" << query.value(9).toString() << "&nbsp;kg</td></tr>";
        out << "<tr><td style=\"text-align: right;\">" << tr("Service life:") << "&nbsp;</td><td>" << query.value(10).toString() << "&nbsp;years</td></tr>";
        out << "<tr><td style=\"text-align: right;\">" << tr("Run-time per day:") << "&nbsp;</td><td>" << query.value(11).toString() << "&nbsp;hours</td></tr>";
        out << "<tr><td style=\"text-align: right;\">" << tr("Rate of utilisation:") << "&nbsp;</td><td>" << query.value(12).toString() << "&nbsp;%</td></tr>";
        out << "</table></td></tr>";
        out << "</table>";
        QSqlQuery inspections;
        inspections.prepare("SELECT date, nominal FROM inspections WHERE parent = :circuit_id");
        inspections.bindValue(":circuit_id", circuit_id);
        inspections.exec();
        int num_inspections = 0;
        while (inspections.next()) {
            num_inspections++;
        }
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
            if (inspections.first()) {
                if (num_inspections % 2 != 0) num_inspections++;
                do {
                    if (int(num_inspections / 2) == inspections.at()) {
                        out << "</table></td>";
                        out << "<td style=\"width:40%;\"><table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">";
                    }
                    out << "<tr><td style=\"text-align: center\">";
                    if (inspections.value(1).toInt() == 1) {
                        out << tr("Nominal:") << "&nbsp;";
                    }
                    out << "<a href=\"customer:" << customer_id << "/circuit:" << circuit_id << "/inspection:" << inspections.value(0).toString() << "\">" << inspections.value(0).toString() << "</a>";
                    out << "</td></tr>";
                } while (inspections.next());
            }
            out << "</table></td></tr>";
            out << "</table>";
        }
    }
    wv_main->setHtml(dict_html.value(tr("Customer information")).arg(html));
}

void MainWindow::viewInspection(const QString & customer_id, const QString & circuit_id, const QString & inspection_date)
{
    QString html; QTextStream out(&html

    QSqlQuery vars("SELECT variables.id, variables.name, variables.type, variables.unit, subvariables.id, subvariables.name, subvariables.type, subvariables.unit FROM variables LEFT JOIN subvariables ON variables.id = subvariables.parent");
    const int VAR_ID = 0; const int VAR_NAME = 1; const int VAR_TYPE = 2; const int VAR_UNIT = 3;
    const int SUBVAR_ID = 4; const int SUBVAR_NAME = 5; const int SUBVAR_TYPE = 6; const int SUBVAR_UNIT = 7;

    QSqlQuery inspection;
    inspection.setForwardOnly(true);
    inspection.prepare("SELECT * FROM inspections WHERE parent = :circuit_id AND date = :inspection_date");
    inspection.bindValue(":circuit_id", circuit_id);
    inspection.bindValue(":inspection_date", inspection_date);
    inspection.exec();
    if (!inspection.next()) return;
    QSqlRecord ins_rec = inspection.record();
    int nominal = inspection.value(ins_rec.indexOf("nominal")).toInt();

    out << "<table cellspacing=\"0\" style=\"width:100%;\">";
    out << "<tr><td><table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">";
    out << "<tbody>";
    out << "<tr style=\"background-color: #eee;\"><td colspan=\"2\" style=\"font-size: larger; width:100%; text-align: center;\"><b>" << tr("Circuit:") << "&nbsp;";
    out << "<a href=\"customer:" << customer_id << "/circuit:" << circuit_id << "\">" << circuit_id << "</a></b></td></tr>";
    out << "<tr style=\"background-color: #DFDFDF;\"><td colspan=\"2\" style=\"font-size: larger; width:100%; text-align: center;\"><b>";
    if (nominal == 1) out << tr("Nominal inspection:") << "&nbsp;";
    else out << tr("Inspection:") << "&nbsp;";
	out << "<a href=\"customer:" << customer_id << "/circuit:" << circuit_id << "/inspection:" << inspection_date << "/modify\">";
	out << inspection_date << "</a></b></td></tr>";
    out << "<tr><td><table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">";

    int num_shown_vars = 0;
    while (vars.next()) {
        for (int i = 0; i < ins_rec.count(); ++i) {
            if (ins_rec.value(i).toString().isEmpty()) continue;
            if (vars.value(SUBVAR_ID).toString().isEmpty()) {
                if (ins_rec.field(i).name() == vars.value(VAR_ID).toString()) {
                    out << "<num_var>" << num_shown_vars << "</num_var>";
                    out << "<tr><td style=\"text-align: right; width:50%;\">" << vars.value(VAR_NAME).toString() << ":&nbsp;";
                    out << "</td><td><table cellpadding=\"0\" cellspacing=\"0\"><tr><td align=\"right\" valign=\"center\">";
                    out << ins_rec.value(i).toString();
                    out << "</td></tr></table></td></tr>";
                    //QMessageBox::information(this, "", vars.value(VAR_ID).toString());
                    num_shown_vars++;
                }
            } else {
                if (ins_rec.field(i).name() == vars.value(SUBVAR_ID).toString()) {
                    out << "<num_var>" << num_shown_vars << "</num_var>";
                    out << "<tr><td style=\"text-align: right; width:50%;\">" << vars.value(VAR_NAME).toString() << ":&nbsp;" << vars.value(SUBVAR_NAME).toString() << ":&nbsp;";
                    out << "</td><td><table cellpadding=\"0\" cellspacing=\"0\"><tr><td align=\"right\" valign=\"center\">";
                    out << ins_rec.value(i).toString();
                    out << "</td></tr></table></td></tr>";
                    //QMessageBox::information(this, "", vars.value(SUBVAR_ID).toString());
                    num_shown_vars++;
                }
            }
            //QMessageBox::information(this, "d", ins_rec.field(i).name());
        }
    }
    if (num_shown_vars != 0) {
        html.replace(QString("<num_var>%1</num_var>").arg(int(num_shown_vars / 2 + num_shown_vars % 2)), "</table></td><td style=\"width:50%;\"><table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">");
    }
    for (int i = 0; i < num_shown_vars; ++i) {
        html.remove(QString("<num_var>%1</num_var>").arg(i));
    }
    out << "</table></td></tbody>";
    out << "</table></td></tr>";
    out << "</table>";
    /*QPlainTextEdit * te = new QPlainTextEdit (dict_html.value(tr("Inspection information")).arg(html));
    te->show();*/
    //QMessageBox::information(this, "d", dict_html.value(tr("Inspection information")).arg(html));
    wv_main->setHtml(dict_html.value(tr("Inspection information")).arg(html));
}

void MainWindow::viewTable(const QString &, const QString &, const QString &, int)
{
    /*
    QStringList used_ids = listVariableIds(); // all = false
    for (;;) {
        MTDictionary expression(parseExpression(exp, &used_ids));
    }
    */
}
