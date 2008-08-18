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

void MainWindow::viewCustomer(const QString &)
{

}

void MainWindow::viewCircuit(const QString &, const QString &)
{

}

void MainWindow::viewInspection(const QString &, const QString &, const QString &)
{

}

void MainWindow::viewTable(const QString &, const QString &, const QString &, int)
{

}
