/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2009 Matus & Michal Tomlein

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

    int i = views_list.indexOf(view);
    btn_view_level_up->setEnabled(i > 0);
    btn_view_level_down->setEnabled(i < views_list.count() - 1);
    bool service_company_view = view == tr("Service company");
    bool table_view = false;
    if (actgrp_view->checkedAction()) {
        lbl_view->setText(actgrp_view->checkedAction()->text());
        table_view = actgrp_view->checkedAction()->text() == tr("Table of inspections");
    } else { table_view = view == tr("Table of inspections"); }
    if (table_view) { cb_table_edit->setCurrentIndex(cb_table->currentIndex()); }
    bool repairs_view = view == tr("List of repairs");
    lbl_table->setEnabled(table_view);
    cb_table->setEnabled(table_view);
    lbl_since->setEnabled(service_company_view || table_view || repairs_view);
    spb_since->setEnabled(service_company_view || table_view || repairs_view);

    wv_main->setHtml(tr("Loading..."));
    qApp->processEvents();
    if (service_company_view) {
        viewServiceCompany(spb_since->value() == 1999 ? 0 : spb_since->value());
    } else if (view == tr("List of customers")) {
        viewAllCustomers();
    } else if (view == tr("Customer information") && selectedCustomer() >= 0) {
        viewCustomer(toString(selectedCustomer()));
    } else if (view == tr("Circuit information") && selectedCustomer() >= 0 && selectedCircuit() >= 0) {
        viewCircuit(toString(selectedCustomer()), toString(selectedCircuit()));
    } else if (view == tr("Inspection information") && selectedCustomer() >= 0 && selectedCircuit() >= 0 && !selectedInspection().isEmpty()) {
        viewInspection(toString(selectedCustomer()), toString(selectedCircuit()), selectedInspection());
    } else if (table_view && selectedCustomer() >= 0 && selectedCircuit() >= 0 && cb_table->currentIndex() >= 0) {
        viewTable(toString(selectedCustomer()), toString(selectedCircuit()), cb_table->currentText(), spb_since->value() == 1999 ? 0 : spb_since->value());
    } else if (repairs_view) {
        viewAllRepairs(selectedRepair(), spb_since->value() == 1999 ? 0 : spb_since->value());
    } else if (view == tr("List of inspectors")) {
        viewAllInspectors(toString(selectedInspector()));
    } else if (view == tr("Refrigerant consumption")) {
        viewRefrigerantConsumption(toString(selectedCustomer()));
    } else if (view == tr("Agenda")) {
        viewAgenda();
    } else if (view == tr("Customer information") || view == tr("Circuit information") || view == tr("Inspection information") || table_view) {
        viewLevelUp();
    } else if (actgrp_view->checkedAction()) {
        setView(actgrp_view->checkedAction()->text());
    } else {
        wv_main->setHtml(QString());
    }
}

void MainWindow::viewServiceCompany(int since)
{
    QString html; QTextStream out(&html);
    MTRecord serv_company_rec("service_company", DBInfoValueForKey("default_service_company"), MTDictionary());
    StringVariantMap serv_company = serv_company_rec.list();
    out << "<table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">";
    out << "<tr style=\"background-color: #DFDFDF;\"><td colspan=\"2\" style=\"font-size: larger; width:100%; text-align: center;\"><b>";
    out << "<a href=\"servicecompany:" << serv_company.value("id").toString() << "/modify\">";
    out << tr("Service company") << "</a></b></td></tr>";
    out << "<tr><td width=\"50%\"><table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">";
    int sc_length = QString("service_companies::").length();
    int num_valid = 0; QString attr_value;
    for (int n = dict_attrnames.indexOfKey("service_companies::certification_num"); n < dict_attrnames.count() && dict_attrnames.key(n).startsWith("service_companies::"); ++n) {
        attr_value = dict_attrnames.key(n).mid(sc_length);
        if (serv_company.value(attr_value).toString().isEmpty()) continue;
        out << "<num_attr>" << num_valid << "</num_attr>";
        out << "<tr><td style=\"text-align: right; width:50%;\">" << dict_attrnames.value(n) << "&nbsp;</td>";
        attr_value = serv_company.value(attr_value).toString();
        out << "<td>" << attr_value << "</td></tr>";
        num_valid++;
    }
    //out << "<num_attr>" << num_valid << "</num_attr>"; num_valid++;
    //out << "<tr><td style=\"text-align: right; width:50%;\"><b>" << tr("Amount of refrigerant in store:") << "&nbsp;</b></td>";
    //out << "<td><b><num_stored /></b></td></tr>";
    if (num_valid != 0) {
        html.replace(QString("<num_attr>%1</num_attr>").arg(int(num_valid / 2 + num_valid % 2)), "</table></td><td width=\"50%\"><table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">");
    }
    for (int k = 0; k < num_valid; ++k) {
        html.remove(QString("<num_attr>%1</num_attr>").arg(k));
    }
    out << "</td></tr></table>";
    out << "</table>";
    out << "<table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">";
    out << "<tr><td rowspan=\"4\" style=\"width:5%;\"/><td colspan=\"2\" style=\"background-color: #eee; font-size: medium; text-align: center; width:90%;\"><b>";
    out << tr("Store") << "</b></td><td rowspan=\"4\" style=\"width:5%;\"/></tr>";
    out << "<tr><td align=\"center\"><table cellspacing=\"0\" cellpadding=\"4\" style=\"width:60%;\" class=\"centred_with_borders\">";
    out << "<tr><th>" << tr("Refrigerant") << "</th>";
    out << "<th>" << tr("New in store") << "</th>";
    out << "<th>" << tr("Recovered in store") << "</th>";
    out << "<th>" << tr("Recycled in store") << "</th></tr>";
    out << "<store />";
    out << "</table></td></tr>";
    out << "<tr><td colspan=\"2\" style=\"background-color: #eee; font-size: medium; text-align: center; width:90%;\"><b>";
    out << tr("Refrigerant management") << "</b></td></tr>";
    out << "<tr><td><table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\" class=\"centred_with_borders\">";
    out << "<tr><th rowspan=\"2\">" << tr("Date") << "</th>";
    out << "<th rowspan=\"2\">" << tr("Refrigerant") << "</th>";
    out << "<th rowspan=\"2\">" << tr("Purchased") << "</th>";
    out << "<th rowspan=\"2\">" << tr("Sold") << "</th>";
    out << "<th colspan=\"3\">" << tr("Added") << "</th>";
    out << "<th rowspan=\"2\">" << tr("Recovered") << "</th>";
    out << "<th rowspan=\"2\">" << tr("Recycled") << "</th>";
    out << "<th rowspan=\"2\">" << tr("Regenerated") << "</th>";
    out << "<th rowspan=\"2\">" << tr("Disposed of") << "</th></tr>";
    out << "<tr><td>" << QApplication::translate("VariableNames", "New") << "</td>";
    out << "<td>" << QApplication::translate("VariableNames", "Recycled") << "</td>";
    out << "<td>" << QApplication::translate("VariableNames", "Total") << "</td></tr>";
    QMap<QString, double> store;
    QMap<QString, double> store_recovered;
    QMap<QString, double> store_recycled;
    QMultiMap<QString, QStringList> entries_map;
    QMap<QString, QList<double> *> sums_map;
    QList<double> * sum_list; int year = 0; QString date, refrigerant;
    QVariant purchased, sold, refr_add_am, refr_add_am_recy, refr_reco, refr_recy, refr_rege, refr_disp;
    MTRecord refr_man_rec("refrigerant_management", QString(), MTDictionary());
    ListOfStringVariantMapsPtr refr_man(refr_man_rec.listAll());
    for (int i = 0; i < refr_man->count(); ++i) {
        refrigerant = refr_man->at(i).value("refrigerant").toString();
        purchased = refr_man->at(i).value("purchased");
        sold = refr_man->at(i).value("sold");
        refr_recy = refr_man->at(i).value("refr_recy");
        refr_rege = refr_man->at(i).value("refr_rege");
        refr_disp = refr_man->at(i).value("refr_disp");
        if (!store.contains(refrigerant)) { store.insert(refrigerant, 0.0); }
        store[refrigerant] += purchased.toDouble() - sold.toDouble();
        if (!store_recovered.contains(refrigerant)) { store_recovered.insert(refrigerant, 0.0); }
        store_recovered[refrigerant] -= refr_recy.toDouble() + refr_rege.toDouble() + refr_disp.toDouble();
        if (!store_recycled.contains(refrigerant)) { store_recycled.insert(refrigerant, 0.0); }
        store_recycled[refrigerant] += refr_recy.toDouble();

        date = refr_man->at(i).value("date").toString();
        year = date.left(4).toInt();
        if (year < since) { continue; }

        QStringList entries_list;
        entries_list << QString("recordofrefrigerantmanagement:%1/modify").arg(date);
        entries_list << refrigerant;
        entries_list << purchased.toString();
        entries_list << sold.toString();
        entries_list << QString() << QString() << QString() << QString();
        entries_list << refr_recy.toString();
        entries_list << refr_rege.toString();
        entries_list << refr_disp.toString();
        entries_map.insert(date, entries_list);
        // ----------------------------------------------------
        if (!sums_map.contains(toString(year))) { sums_map.insert(toString(year), NULL); }
        if (!sums_map.contains(QString("%1::%2").arg(year).arg(refrigerant))) {
            sum_list = new QList<double>;
            *sum_list << 0.0 << 0.0 << 0.0 << 0.0 << 0.0 << 0.0 << 0.0 << 0.0 << 0.0;
            sums_map.insert(QString("%1::%2").arg(year).arg(refrigerant), sum_list);
        } else {
            sum_list = sums_map.value(QString("%1::%2").arg(year).arg(refrigerant));
        }
        // ----------------------------------------------------
        (*sum_list)[0] += purchased.toDouble();
        (*sum_list)[1] += sold.toDouble();
        (*sum_list)[6] += refr_recy.toDouble();
        (*sum_list)[7] += refr_rege.toDouble();
        (*sum_list)[8] += refr_disp.toDouble();
    }
    MTRecord circuits_record("circuit", "", MTDictionary());
    MultiMapOfStringVariantMapsPtr circuits(circuits_record.mapAll("parent::id", "refrigerant"));
    MTRecord inspections_record("inspection", "", MTDictionary());
    ListOfStringVariantMapsPtr inspections(inspections_record.listAll("customer, circuit, date, nominal, refr_add_am, refr_add_am_recy, refr_reco"));
    MTRecord repairs_rec("repair", "", MTDictionary());
    *inspections << *(repairs_rec.listAll("date, refrigerant, refr_add_am, refr_reco"));
    for (int i = 0; i < inspections->count(); ++i) {
        date = inspections->at(i).value("date").toString();
        QStringList entries_list;
        if (inspections->at(i).contains("customer")) {
            entries_list << QString("customer:%1/circuit:%2/%3:%4")
                            .arg(inspections->at(i).value("customer").toString())
                            .arg(inspections->at(i).value("circuit").toString())
                            .arg(inspections->at(i).value("nominal").toInt() ? "nominalinspection" : "inspection")
                            .arg(date);
            refrigerant = circuits->value(QString("%1::%2")
                            .arg(inspections->at(i).value("customer").toString())
                            .arg(inspections->at(i).value("circuit").toString()))
                            .value("refrigerant").toString();
            entries_list << refrigerant;
        } else {
            entries_list << QString("repair:%1").arg(date);
            refrigerant = inspections->at(i).value("refrigerant").toString();
            entries_list << refrigerant;
        }
        refr_add_am = inspections->at(i).value("refr_add_am");
        refr_add_am_recy = inspections->at(i).value("refr_add_am_recy");
        refr_reco = inspections->at(i).value("refr_reco");
        if (!store.contains(refrigerant)) { store.insert(refrigerant, 0.0); }
        store[refrigerant] -= refr_add_am.toDouble();
        if (!store_recovered.contains(refrigerant)) { store_recovered.insert(refrigerant, 0.0); }
        store_recovered[refrigerant] += refr_reco.toDouble();
        if (!store_recycled.contains(refrigerant)) { store_recycled.insert(refrigerant, 0.0); }
        store_recycled[refrigerant] -= refr_add_am_recy.toDouble();

        year = date.left(4).toInt();
        if (year < since) { continue; }

        if (!refr_add_am.toDouble() && !refr_reco.toDouble()) continue;
        entries_list << QString() << QString();
        entries_list << refr_add_am.toString();
        entries_list << refr_add_am_recy.toString();
        entries_list << toString(refr_add_am.toDouble() + refr_add_am_recy.toDouble());
        entries_list << refr_reco.toString();
        entries_list << QString() << QString() << QString();
        entries_map.insert(date, entries_list);
        // ----------------------------------------------------
        if (!sums_map.contains(toString(year))) { sums_map.insert(toString(year), NULL); }
        if (!sums_map.contains(QString("%1::%2").arg(year).arg(refrigerant))) {
            sum_list = new QList<double>;
            *sum_list << 0.0 << 0.0 << 0.0 << 0.0 << 0.0 << 0.0 << 0.0 << 0.0 << 0.0;
            sums_map.insert(QString("%1::%2").arg(year).arg(refrigerant), sum_list);
        } else {
            sum_list = sums_map.value(QString("%1::%2").arg(year).arg(refrigerant));
        }
        // ----------------------------------------------------
        (*sum_list)[2] += refr_add_am.toDouble();
        (*sum_list)[3] += refr_add_am_recy.toDouble();
        (*sum_list)[4] += refr_add_am.toDouble() + refr_add_am_recy.toDouble();
        (*sum_list)[5] += refr_reco.toDouble();
    }
    QString store_html;
    QStringList list_refrigerants = listRefrigerantsToString().split(";");
    list_refrigerants.insert(0, "");
    for (int i = 0; i < list_refrigerants.count(); ++i) {
        if (store.contains(list_refrigerants.at(i)) || store_recovered.contains(list_refrigerants.at(i)) || store_recycled.contains(list_refrigerants.at(i))) {
            store_html.append("<tr><td>" + list_refrigerants.at(i) + "</td><td>");
            store_html.append(toString(store.value(list_refrigerants.at(i))));
            store_html.append("</td><td>");
            store_html.append(toString(store_recovered.value(list_refrigerants.at(i))));
            store_html.append("</td><td>");
            store_html.append(toString(store_recycled.value(list_refrigerants.at(i))));
            store_html.append("</td></tr>");
        }
    }
    html.replace("<store />", store_html);
    int last_year = 0; bool it = false, bf = false; QString link;
    QMap<QString, QList<double> *>::const_iterator sums_iterator;
    QMapIterator<QString, QStringList> i(entries_map);
    i.toBack();
    while (i.hasPrevious()) { i.previous();
        year = i.key().left(4).toInt();
        if (year < last_year) { last_year = 0; }
        if (!last_year) {
            last_year = year;
            out << "<tr><th rowspan=\"<rowspan />\"><a href=\"toggledetailedview:\">" << year << "</a></th>";
            int row_count = 0;
            sums_iterator = sums_map.constFind(toString(year));
            if (++sums_iterator != sums_map.constEnd()) {
                while (sums_iterator != sums_map.constEnd() && (sum_list = sums_iterator.value())) {
                    if (row_count) { out << "</tr><tr>"; }
                    out << "<th>" << sums_iterator.key().split("::").last() << "</th>";
                    for (int n = 0; n < sum_list->count(); ++n) {
                        out << "<th>";
                        if (sum_list->at(n)) out << sum_list->at(n);
                        out << "</th>";
                    }
                    row_count++;
                    ++sums_iterator;
                }
            }
            out << "</tr>";
            html.replace("<rowspan />", toString(row_count));
        }
        if (show_details_in_service_company_view) {
            link = i.value().at(0);
            bf = link.contains("nominal");
            it = link.startsWith("repair:");
            if (bf) link.remove("nominal");
            out << "<tr><td";
            if (bf) out << " style=\"font-weight: bold;\"";
            else if (it) out << " style=\"font-style: italic;\"";
            out << "><a href=\"" << link << "\">" << i.key() << "</a></td>";
            out << "<td";
            if (bf) out << " style=\"font-weight: bold;\"";
            else if (it) out << " style=\"font-style: italic;\"";
            out << ">" << i.value().at(1) << "</td>";
            for (int n = 2; n < i.value().count(); ++n) {
                out << "<td";
                if (bf) out << " style=\"font-weight: bold;\"";
                else if (it) out << " style=\"font-style: italic;\"";
                out << ">";
                if (i.value().at(n).toDouble()) out << i.value().at(n).toDouble();
                out << "</td>";
            }
            out << "</tr>";
        }
    }
    sums_iterator = sums_map.constBegin();
    while (sums_iterator != sums_map.constEnd()) {
        delete sums_iterator.value();
        ++sums_iterator;
    }
    out << "</table></td></tr>";
    out << "</table>";
    wv_main->setHtml(dict_html.value(tr("Service company")).arg(html));
}

void MainWindow::writeCustomersTable(QTextStream & out, const QString & customer_id)
{
    MTRecord all_customers("customer", customer_id, MTDictionary());
    ListOfStringVariantMapsPtr list(all_customers.listAll());
    out << "<table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">";
    int cu_length = QString("customer::").length();
    QString thead = "<tr>"; int thead_colspan = 2;
    for (int n = dict_attrnames.indexOfKey("customer::id"); n < dict_attrnames.count() && dict_attrnames.key(n).startsWith("customer::"); ++n) {
        thead.append("<th>" + dict_attrnames.value(n) + "</th>");
        thead_colspan++;
    }
    thead.append("<th>" + tr("Number of circuits") + "</th>");
    thead.append("<th>" + tr("Total number of inspections") + "</th>");
    thead.append("</tr>");
    out << "<tr><th colspan=\"" << thead_colspan << "\" style=\"font-size: large;\">";
    if (customer_id.isEmpty()) { out << tr("List of customers"); }
    else { out << "<a href=\"customer:" << customer_id << "/modify\">" << tr("Customer") << "</a>"; }
    out << "</th></tr>";
    out << thead;
    QString id; QString highlighted_id = toString(selectedCustomer());
    for (int i = 0; i < list->count(); ++i) {
        id = list->at(i).value("id").toString();
        if (id == highlighted_id) {
            out << "<tr style=\"background-color: #F3F3F3;\">";
        } else { out << "<tr>"; }
        out << "<td>" << toolTipLink("customer", id.rightJustified(8, '0'), id) << "</td>";
        for (int n = dict_attrnames.indexOfKey("customer::id") + 1; n < dict_attrnames.count() && dict_attrnames.key(n).startsWith("customer::"); ++n) {
            out << "<td>" << list->at(i).value(dict_attrnames.key(n).mid(cu_length)).toString() << "</td>";
        }
        MTRecord circuits_record("circuit", "", MTDictionary("parent", id));
        out << "<td>" << circuits_record.listAll("id")->count() << "</td>";
        MTRecord inspection_record("inspection", "", MTDictionary("customer", id));
        out << "<td>" << inspection_record.listAll("date")->count() << "</td>";
        out << "</tr>";
    }
    out << "</table>";
}

void MainWindow::writeCircuitsTable(QTextStream & out, const QString & customer_id, const QString & circuit_id)
{
    MTRecord circuits_record("circuit", circuit_id, MTDictionary("parent", customer_id));
    ListOfStringVariantMapsPtr circuits(circuits_record.listAll());
    out << "<table cellspacing=\"0\" style=\"width:100%;\">";
    int cc_length = QString("circuit::").length();
    QString thead = "<tr>"; int thead_colspan = 2;
    for (int n = dict_attrnames.indexOfKey("circuit::id"); n < dict_attrnames.count() && dict_attrnames.key(n).startsWith("circuit::"); ++n) {
        thead.append("<th>" + dict_attrnames.value(n).split("||").first() + "</th>");
        thead_colspan++;
    }
    thead.append("<th>" + dict_attrnames.value("circuit::refrigerant") + "</th>");
    thead.append("<th>" + dict_attrnames.value("circuit::oil") + "</th>");
    thead.append("</tr>");
    out << "<tr><th colspan=\"" << thead_colspan << "\" style=\"font-size: large;\">";
    if (circuit_id.isEmpty()) { out << tr("Circuits"); }
    else { out << "<a href=\"customer:" << customer_id << "/circuit:" << circuit_id << "/modify\">" << tr("Circuit") << "</a>"; }
    out << "</th></tr>";
    out << thead;
    QString attr_value; QStringList dict_value; QString id;
    QString highlighted_id = toString(selectedCircuit());
    bool show_disused = false;
    for (int i = 0; i < circuits->count(); ++i) {
        if (circuit_id.isEmpty() && circuits->at(i).value("disused").toInt()) { show_disused = true; continue; }
        id = circuits->at(i).value("id").toString();
        if (id == highlighted_id) {
            out << "<tr style=\"background-color: #F3F3F3;\">";
        } else { out << "<tr>"; }
        out << "<td>" << toolTipLink("customer/circuit", id.rightJustified(4, '0'), customer_id, id) << "</td>";
        for (int n = dict_attrnames.indexOfKey("circuit::id") + 1; n < dict_attrnames.count() && dict_attrnames.key(n).startsWith("circuit::"); ++n) {
            dict_value = dict_attrnames.value(n).split("||");
            attr_value = circuits->at(i).value(dict_attrnames.key(n).mid(cc_length)).toString();
            if (dict_attrnames.key(n) == "circuit::field") {
                if (dict_attrvalues.contains("field::" + attr_value)) {
                    attr_value = dict_attrvalues.value("field::" + attr_value);
                }
            } else if (dict_attrnames.key(n) == "circuit::hermetic") {
                attr_value = attr_value.toInt() ? tr("Yes") : tr("No");
            }
            out << "<td>" << attr_value;
            if (dict_value.count() > 1) { out << "&nbsp;" << dict_value.last(); }
            out << "</td>";
        }
        out << "<td>" << circuits->at(i).value("refrigerant_amount").toDouble() << "&nbsp;" << tr("kg");
        out << " " << circuits->at(i).value("refrigerant").toString() << "</td>";
        out << "<td>" << circuits->at(i).value("oil_amount").toDouble() << "&nbsp;" << tr("kg");
        out << " " << circuits->at(i).value("oil").toString().toUpper() << "</td>";
        out << "</tr>";
    }
    out << "</table>";
    if (show_disused) {
        out << "<br><table cellspacing=\"0\" style=\"width:100%;\"><tr>";
        out << "<th colspan=\"5\" style=\"font-size: large;\">" << tr("Disused circuits") << "</th></tr><tr>";
        out << "<th>" << dict_attrnames.value("circuit::id") << "</th>";
        out << "<th>" << dict_attrnames.value("circuit::manufacturer") << "</th>";
        out << "<th>" << dict_attrnames.value("circuit::type") << "</th>";
        out << "<th>" << dict_attrnames.value("circuit::sn") << "</th>";
        out << "<th>" << dict_attrnames.value("circuit::commissioning") << "</th></tr>";
        for (int i = 0; i < circuits->count(); ++i) {
            if (!circuits->at(i).value("disused").toInt()) continue;
            id = circuits->at(i).value("id").toString();
            if (id == highlighted_id) {
                out << "<tr style=\"background-color: #F3F3F3;\">";
            } else { out << "<tr>"; }
            out << "<td>" << toolTipLink("customer/circuit", id.rightJustified(4, '0'), customer_id, id) << "</td>";
            out << "<td>" << circuits->at(i).value("manufacturer").toString() << "</td>";
            out << "<td>" << circuits->at(i).value("type").toString() << "</td>";
            out << "<td>" << circuits->at(i).value("sn").toString() << "</td>";
            out << "<td>" << circuits->at(i).value("commissioning").toString() << "</td></tr>";
        }
        out << "</table>";
    }
}

void MainWindow::viewAllCustomers()
{
    QString html; QTextStream out(&html);
    writeCustomersTable(out);
    wv_main->setHtml(dict_html.value(tr("List of customers")).arg(html), QUrl("qrc:/html/"));
}

void MainWindow::viewCustomer(const QString & customer_id)
{
    QString html; QTextStream out(&html);
    writeCustomersTable(out, customer_id);
    out << "<br>";
    writeCircuitsTable(out, customer_id);
    wv_main->setHtml(dict_html.value(tr("Customer information")).arg(html), QUrl("qrc:/html/"));
}

void MainWindow::viewCircuit(const QString & customer_id, const QString & circuit_id)
{
    QString html; QTextStream out(&html);
    writeCustomersTable(out, customer_id);
    out << "<br>";
    writeCircuitsTable(out, customer_id, circuit_id);
    MTDictionary inspection_parents("circuit", circuit_id);
    inspection_parents.insert("customer", customer_id);
    MTRecord inspection_record("inspection", "", inspection_parents);
    ListOfStringVariantMapsPtr inspections(inspection_record.listAll("date, nominal, repair, rmds, arno, inspector, operator, refr_add_am, refr_add_am_recy, refr_reco"));
    if (inspections->count()) {
        MTRecord inspectors_record("inspector", "", MTDictionary());
        MultiMapOfStringVariantMapsPtr inspectors(inspectors_record.mapAll("id", "person"));
        out << "<br><table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">";
        out << "<tr><th colspan=\"8\" style=\"font-size: large;\"><a href=\"customer:" << customer_id << "/circuit:" << circuit_id << "/table\">";
        out << tr("Inspections and repairs") << "</a></th></tr>";
        out << "<tr><th rowspan=\"2\">" << tr("Date") << "</th>";
        out << "<th rowspan=\"2\">" << dict_varnames.value("arno") << "</th>";
        out << "<th rowspan=\"2\">" << dict_varnames.value("inspector") << "</th>";
        out << "<th rowspan=\"2\">" << dict_varnames.value("operator") << "</th>";
        out << "<th colspan=\"2\">" << dict_varnames.value("refr_add") << "</th>";
        out << "<th rowspan=\"2\">" << dict_varnames.value("refr_reco") << "</th>";
        out << "<th rowspan=\"2\">" << dict_varnames.value("rmds") << "</th></tr>";
        out << "<tr><th>" << dict_varnames.value("refr_add_am") << "</th>";
        out << "<th>" << dict_varnames.value("refr_add_am_recy") << "</th></tr>";
        bool is_nominal, is_repair;
        QString id; QString highlighted_id = selectedInspection();
        for (int i = 0; i < inspections->count(); ++i) {
            id = inspections->at(i).value("date").toString();
            if (id == highlighted_id) {
                out << "<tr style=\"background-color: #F3F3F3;\">";
            } else { out << "<tr>"; }
            out << "<td>";
            is_nominal = inspections->at(i).value("nominal").toInt();
            is_repair = inspections->at(i).value("repair").toInt();
            if (is_nominal) { out << "<b>"; }
            else if (is_repair) { out << "<i>"; }
            out << toolTipLink("customer/circuit/inspection", id, customer_id, circuit_id, id);
            if (is_nominal) { out << "<b>"; }
            else if (is_repair) { out << "<i>"; }
            out << "</td>";
            out << "<td>" << inspections->at(i).value("arno").toString() << "</td>";
            out << "<td>" << inspectors->value(inspections->at(i).value("inspector").toString()).value("person").toString() << "</td>";
            out << "<td>" << inspections->at(i).value("operator").toString() << "</td>";
            out << "<td>" << inspections->at(i).value("refr_add_am").toDouble() << "&nbsp;" << tr("kg") << "</td>";
            out << "<td>" << inspections->at(i).value("refr_add_am_recy").toDouble() << "&nbsp;" << tr("kg") << "</td>";
            out << "<td>" << inspections->at(i).value("refr_reco").toDouble() << "&nbsp;" << tr("kg") << "</td>";
            if (!inspections->at(i).value("rmds").toString().isEmpty()) {
                out << "<td onmouseover=\"Tip('" << escapeString(inspections->at(i).value("rmds").toString()) << "')\" onmouseout=\"UnTip()\">...</td>";
            } else {
                out << "<td></td>";
            }
            out << "</tr>";
        }
        out << "</table>";
    }
    wv_main->setHtml(dict_html.value(tr("Circuit information")).arg(html), QUrl("qrc:/html/"));
}

void MainWindow::viewInspection(const QString & customer_id, const QString & circuit_id, const QString & inspection_date)
{
    QString html; QTextStream out(&html);
    writeCustomersTable(out, customer_id);
    out << "<br>";
    writeCircuitsTable(out, customer_id, circuit_id);
    MTDictionary inspection_parents("circuit", circuit_id);
    inspection_parents.insert("customer", customer_id);
    MTRecord inspection_record("inspection", inspection_date, inspection_parents);
    StringVariantMap inspection = inspection_record.list();
    bool nominal = inspection.value("nominal").toInt();
    bool repair = inspection.value("repair").toInt();
    inspection_parents.insert("nominal", "1");
    MTRecord nom_inspection_record("inspection", "", inspection_parents);
    StringVariantMap nominal_ins = nom_inspection_record.list();
    MTRecord circuit_rec("circuit", circuit_id, MTDictionary("parent", customer_id));
    QString circuit_name = circuit_rec.list("name").value("name").toString();

    out << "<br><table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\" class=\"no_border\">";
    out << "<tr><th colspan=\"4\" style=\"font-size: large;\">";
    out << "<a href=\"customer:" << customer_id << "/circuit:" << circuit_id << "/inspection:" << inspection_date << "/modify\">";
    if (nominal) out << tr("Nominal inspection:"); else if (repair) out << tr("Repair:"); else out << tr("Inspection:");
	out << "&nbsp;" << inspection_date << "</a></th></tr>";

    QStringList used_ids = listVariableIds(); // all = false
    Variables vars;
    int half = vars.count() % 2 ? (vars.count() / 2 + 1) : (vars.count() / 2);
    QStringList rows; int row = 0, iterator = 0;
    while (vars.next()) {
        QString var_id; bool compare_nom = false; bool ok_eval = true;
        MTDictionary expression; double tolerance = 0; bool is_subvar = false;
        if (vars.value("SUBVAR_ID").toString().isEmpty()) {
            var_id = vars.value("VAR_ID").toString();
            tolerance = vars.value("VAR_TOLERANCE").toDouble();
        } else {
            var_id = vars.value("SUBVAR_ID").toString();
            tolerance = vars.value("SUBVAR_TOLERANCE").toDouble();
            is_subvar = true;
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
        //if (ins_value.isEmpty()) continue;
        if (var_id == "inspector") {
            MTRecord inspector("inspector", ins_value, MTDictionary());
            ins_value = inspector.list("person").value("person").toString();
        }
        if (iterator == half) {
            row = 0;
        } else if (iterator < half) {
            rows << QString();
        }
        rows[row].append("<td style=\"text-align: right; width: 25%; border-style: none;\">");
        if (!is_subvar) {
            rows[row].append(vars.value("VAR_NAME").toString() + ":");
        } else {
            rows[row].append(vars.value("VAR_NAME").toString() + ": " + vars.value("SUBVAR_NAME").toString() + ":");
        }
        rows[row].append("</td><td style=\"width: 25%; border-style: none;\"><table cellpadding=\"0\" cellspacing=\"0\" class=\"no_border\" style=\"width: 0%\"><tr><td ");
        if ((is_subvar && vars.value("SUBVAR_TYPE").toString() != "text") || (!is_subvar && vars.value("VAR_TYPE").toString() != "text")) rows[row].append("align=\"right\"");
        rows[row].append(" valign=\"center\" style=\"border-style: none;\">");
        if (compare_nom) {
            rows[row].append(compareValues(nom_value.toDouble(), ins_value.toDouble(), tolerance).arg(ins_value));
        } else {
            rows[row].append(ins_value);
        }
        rows[row].append("</td><td valign=\"center\" style=\"border-style: none;\">&nbsp;");
        if (!vars.value("VAR_UNIT").toString().isEmpty()) {
            rows[row].append(vars.value("VAR_UNIT").toString());
        } else if (!vars.value("SUBVAR_UNIT").toString().isEmpty()) {
            rows[row].append(vars.value("SUBVAR_UNIT").toString());
        }
        rows[row].append("</td></tr></table></td>");
        row++; iterator++;
    }
    for (int i = 0; i < rows.count(); ++i) {
        out << "<tr>" << rows.at(i) << "</tr>";
    }
    out << "</table>";
//*** Warnings ***
    QStringList global_warnings;
    QStringList warnings_list = listWarnings(inspection, nominal_ins, customer_id, circuit_id, used_ids, global_warnings);
    if (warnings_list.count()) {
        out << "<br><table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">";
        out << "<tr><th style=\"font-size: larger;\">" << tr("Warnings") << "</th></tr>";
        out << "<tr><td>" << warnings_list.join(", ") << "</td></tr></table>";
    }
    wv_main->setHtml(dict_html.value(tr("Inspection information")).arg(html), QUrl("qrc:/html/"));
}

void MainWindow::viewTable(const QString & customer_id, const QString & circuit_id, const QString & table_id, int year)
{
    QString html; QTextStream out(&html);

    Variables vars;

    QStringList used_ids = listVariableIds();

//*** Mapping variables ***
    MapOfStringVariantMaps variables;
    QString last_id; StringVariantMap map; QList<QVariant> subvariables;
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
            StringVariantMap subvariable;
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
    StringVariantMap table = table_record.list();
    QStringList table_vars = table.value("variables").toString().split(";", QString::SkipEmptyParts);

    MTDictionary inspection_parents("circuit", circuit_id);
    inspection_parents.insert("customer", customer_id);
    MTRecord inspection_record("inspection", "", inspection_parents);
    ListOfStringVariantMapsPtr inspections(inspection_record.listAll());
    for (int i = 0; i < inspections->count(); ++i) {
        if (table.value("highlight_nominal").toInt() && inspections->at(i).value("nominal").toInt()) {}
        else if (inspections->at(i).value("date").toString().split(".").first().toInt() < year) {
            inspections->removeAt(i);
            i--;
        }
    }

    StringVariantMap nominal_ins;
    for (int i = 0; i < inspections->count(); ++i) {
        if (inspections->at(i).value("nominal").toInt()) {
            nominal_ins = (*inspections)[i];
            break;
        }
    }

//*** Top tables ***
    MTRecord customer("customer", customer_id, MTDictionary());
    StringVariantMap customer_info = customer.list("company, contact_person, address, mail, phone");
    MTRecord circuit("circuit", circuit_id, MTDictionary("parent", customer_id));
    StringVariantMap circuit_info = circuit.list("name, manufacturer, type, sn, year, commissioning, field, refrigerant, refrigerant_amount, oil, oil_amount, life, runtime, utilisation");
    out << "<table><tr><th>" << tr("ID");
    out << "</th><th>" << tr("Company");
    out << "</th><th>" << tr("Contact person");
    out << "</th><th>" << tr("Address");
    out << "</th><th>" << tr("E-mail");
    out << "</th><th>" << tr("Phone");
    out << "</th></tr><tr>";
    out << "<td>" << toolTipLink("customer", customer_id.rightJustified(8, '0'), customer_id) << "</td>";
    out << "<td>" << customer_info.value("company").toString() << "</td>";
    out << "<td>" << customer_info.value("contact_person").toString() << "</td>";
    out << "<td>" << customer_info.value("address").toString() << "</td>";
    out << "<td>" << customer_info.value("mail").toString() << "</td>";
    out << "<td>" << customer_info.value("phone").toString() << "</td>";
    out << "</tr></table><br />";
    out << "<table><tr><th>" << tr("ID");
    out << "</th><th>" << tr("Name");
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
    out << "<td>" << toolTipLink("customer/circuit", circuit_id.rightJustified(4, '0'), customer_id, circuit_id) << "</td>";
    out << "<td>" << circuit_info.value("name").toString() << "</td>";
    out << "<td>" << circuit_info.value("manufacturer").toString() << "</td>";
    out << "<td>" << circuit_info.value("type").toString() << "</td>";
    out << "<td>" << circuit_info.value("year").toString() << "</td>";
    out << "<td>" << circuit_info.value("commissioning").toString() << "</td>";
    out << "<td>" << circuit_info.value("refrigerant").toString() << "</td>";
    out << "<td>" << circuit_info.value("refrigerant_amount").toString() << "&nbsp;" << tr("kg") << "</td>";
    out << "<td>";
    if (dict_attrvalues.contains("oil::" + circuit_info.value("oil").toString())) {
        out << dict_attrvalues.value("oil::" + circuit_info.value("oil").toString());
    }
    out << "</td>";
    out << "<td>" << circuit_info.value("oil_amount").toString() << "&nbsp;" << tr("kg") << "</td>";
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
    bool is_nominal, is_repair;
    out << "<tbody>";
    for (int i = 0; i < inspections->count(); ++i) {
        is_nominal = inspections->at(i).value("nominal").toInt();
        is_repair = inspections->at(i).value("repair").toInt();
        out << "<tr class=\"";
        if (is_nominal && table.value("highlight_nominal").toInt()) {
            out << "nominal";
        }
        out << "\"><td>";
        if (is_nominal) { out << "<b>"; }
        else if (is_repair) { out << "<i>"; }
        out << toolTipLink("customer/circuit/inspection", inspections->at(i).value("date").toString(), customer_id, circuit_id, inspections->at(i).value("date").toString());
        if (is_nominal) { out << "</b>"; }
        else if (is_repair) { out << "</i>"; }
        out << "</td>";
        for (int n = 0; n < table_vars.count(); ++n) {
            bool compare_nom = false; int rowspan = 1; QString ins_value = ""; QString nom_value = ""; bool ok_eval;
            subvariables = variables.value(table_vars.at(n)).value("subvariables").toList();
            if (subvariables.count() > 0) {
                for (int s = 0; s < subvariables.count(); ++s) {
                    compare_nom = subvariables.at(s).toMap().value("compare_nom").toInt() > 0;
                    if (subvariables.at(s).toMap().value("value").toString().contains("sum")) {
                        QString i_year = inspections->at(i).value("date").toString().split(".").first();
                        if (is_nominal) rowspan = 1;
                        else if (i > 0 && !inspections->at(i-1).value("nominal").toInt() && inspections->at(i-1).value("date").toString().split(".").first() == i_year) continue;
                        else {
                            int in = i;
                            for (; in < inspections->count(); ++in) {
                                if (inspections->at(in).value("nominal").toInt()) in--;
                                if (inspections->at(in).value("date").toString().split(".").first() != i_year) {
                                    break;
                                }
                            }
                            rowspan = in - i;
                        }
                    } else rowspan = 1;
                    if (subvariables.at(s).toMap().value("value").toString().isEmpty()) {
                        ins_value = inspections->at(i).value(subvariables.at(s).toMap().value("id").toString()).toString();
                        if (compare_nom) {
                            nom_value = nominal_ins.value(subvariables.at(s).toMap().value("id").toString()).toString();
                            if (nom_value.isEmpty()) compare_nom = false;
                        }
                    } else {
                        MTDictionary expression = parseExpression(subvariables.at(s).toMap().value("value").toString(), &used_ids);
                        ins_value = toString(evaluateExpression((*inspections)[i], expression, customer_id, circuit_id, &ok_eval));
                        if (!ok_eval) ins_value = "";
                        if (compare_nom) {
                            nom_value = toString(evaluateExpression(nominal_ins, expression, customer_id, circuit_id, &ok_eval));
                            if (!ok_eval) compare_nom = false;
                        }
                    }
                    if (subvariables.at(s).toMap().value("type").toString() == "bool") {
                        ins_value = ins_value.toInt() ? tr("Yes") : tr("No");
                    }
                    writeTableVarCell(out, subvariables.at(s).toMap().value("type").toString(), ins_value, nom_value, variables.value(table_vars.at(n)).value("col_bg").toString(), compare_nom, rowspan, subvariables.at(s).toMap().value("tolerance").toDouble());
                }
            } else {
                compare_nom = variables.value(table_vars.at(n)).value("compare_nom").toInt() > 0;
                if (variables.value(table_vars.at(n)).value("value").toString().isEmpty()) {
                    ins_value = inspections->at(i).value(table_vars.at(n)).toString();
                    if (compare_nom) {
                        nom_value = nominal_ins.value(table_vars.at(n)).toString();
                        if (nom_value.isEmpty()) compare_nom = false;
                    }
                } else {
                    MTDictionary expression = parseExpression(variables.value(table_vars.at(n)).value("value").toString(), &used_ids);
                    ins_value = toString(evaluateExpression((*inspections)[i], expression, customer_id, circuit_id, &ok_eval));
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
                writeTableVarCell(out, variables.value(table_vars.at(n)).value("type").toString(), ins_value, nom_value, variables.value(table_vars.at(n)).value("col_bg").toString(), compare_nom, rowspan, variables.value(table_vars.at(n)).value("tolerance").toDouble());
            }
        }
        out << "</tr>";
    }
    out << "</tbody>";
    out << "<tfoot>";

//*** Foot ***
    MTDictionary foot_functions;
    foot_functions.insert("sum", tr("Sum"));
    foot_functions.insert("avg", tr("Average"));
    for (int f = 0; f < foot_functions.count(); ++f) {
        if (!table.value(foot_functions.key(f)).toString().isEmpty()) {
            out << "<tr class=\"border_top border_bottom\">";
            out << "<th>" << foot_functions.value(f) << "</th>";
            QStringList f_vars = table.value(foot_functions.key(f)).toString().split(";", QString::SkipEmptyParts);
            for (int i = 0; i < table_vars.count(); ++i) {
                bool is_in_foot = f_vars.contains(table_vars.at(i));
                if (variables.value(table_vars.at(i)).value("subvariables").toList().count() > 0) {
                    subvariables = variables.value(table_vars.at(i)).value("subvariables").toList();
                    for (int s = 0; s < subvariables.count(); ++s) {
                        is_in_foot = f_vars.contains(table_vars.at(i));
                        if (subvariables.at(s).toMap().value("type").toString() != "float" && subvariables.at(s).toMap().value("type").toString() != "int") is_in_foot = false;
                        out << "<td class=\"" << variables.value(table_vars.at(i)).value("col_bg").toString() << "\">";
                        if (is_in_foot) {
                            double value = 0.0; int num_ins = 0;
                            if (subvariables.at(s).toMap().value("value").toString().isEmpty()) {
                                num_ins = inspections->count();
                                for (int ins = 0; ins < inspections->count(); ++ins) {
                                    value += inspections->at(ins).value(subvariables.at(s).toMap().value("id").toString()).toDouble();
                                }
                            } else {
                                MTDictionary expression = parseExpression(subvariables.at(s).toMap().value("value").toString(), &used_ids);
                                for (int ins = 0; ins < inspections->count(); ++ins) {
                                    if (subvariables.at(s).toMap().value("value").toString().contains("sum") &&
                                        ins > 0 && !inspections->at(ins-1).value("nominal").toInt() &&
                                        inspections->at(ins-1).value("date").toString().split(".").first() == inspections->at(ins).value("date").toString().split(".").first())
                                            continue;
                                    num_ins++;
                                    value += evaluateExpression((*inspections)[ins], expression, customer_id, circuit_id);
                                }
                            }
                            if (num_ins && foot_functions.key(f) == "avg") { value /= (double)num_ins; }
                            out << value;
                        }
                        out << "</td>";
                    }
                } else {
                    if (variables.value(table_vars.at(i)).value("type").toString() != "float" && variables.value(table_vars.at(i)).value("type").toString() != "int") is_in_foot = false;
                    out << "<td class=\"" << variables.value(table_vars.at(i)).value("col_bg").toString() << "\">";
                    if (is_in_foot) {
                        double value = 0.0; int num_ins = inspections->count();
                        if (variables.value(table_vars.at(i)).value("value").toString().isEmpty()) {
                            for (int ins = 0; ins < inspections->count(); ++ins) {
                                value += inspections->at(ins).value(table_vars.at(i)).toDouble();
                            }
                        } else {
                            MTDictionary expression = parseExpression(variables.value(table_vars.at(i)).value("value").toString(), &used_ids);
                            for (int ins = 0; ins < inspections->count(); ++ins) {
                                value += evaluateExpression((*inspections)[ins], expression, customer_id, circuit_id);
                            }
                        }
                        if (num_ins && foot_functions.key(f) == "avg") { value /= (double)num_ins; }
                        out << value;
                    }
                    out << "</td>";
                }
            }
            out << "</tr>";
        }
    }
    out << "</tfoot>";
    out << "</table>";

//*** Warnings ***
    QString warnings_html;
    QStringList global_warnings;
    QStringList last_warnings_list;
    for (int i = 0; i < inspections->count(); ++i) {
        QStringList warnings_list = listWarnings((*inspections)[i], nominal_ins, customer_id, circuit_id, used_ids, global_warnings, i == inspections->count()-1);
        QStringList backup_warnings = warnings_list;
        for (int n = 0; n < warnings_list.count(); ++n) {
            if (last_warnings_list.contains(warnings_list.at(n))) {
                warnings_list[n].prepend("<span style=\"color: red;\"><b>");
                warnings_list[n].append("</b></span>");
            }
        }
        if (warnings_list.count()) {
            warnings_html.append("<tr><td><a href=\"customer:" + customer_id + "/circuit:" + circuit_id + "/inspection:" + inspections->at(i).value("date").toString() + "\">");
            warnings_html.append(inspections->at(i).value("date").toString() + "</a>");
            warnings_html.append("</td><td>");
            warnings_html.append(warnings_list.join(", "));
            warnings_html.append("</td></tr>");
        }
        last_warnings_list = backup_warnings;
    }
    if (global_warnings.count()) {
        warnings_html.append("<tr><td colspan=\"2\"><b>");
        warnings_html.append(global_warnings.join(", "));
        warnings_html.append("</b></td></tr>");
    }
    if (!warnings_html.isEmpty()) {
        out << "<br /><table>";
        out << "<tr><th width=\"20%\">" << tr("Date") << "</th><th>" << tr("Warnings") << "</th></tr>";
        out << warnings_html;
        out << "</table>";
    }
    wv_main->setHtml(dict_html.value(tr("Table of inspections")).arg(html), QUrl("qrc:/html/"));
}

void MainWindow::writeTableVarCell(QTextStream & out, const QString & var_type, const QString & ins_value, const QString & nom_value, const QString & bg_class, bool compare_nom, int rowspan, double tolerance)
{
    out << "<td class=\"" << bg_class << "\" rowspan=\"" << rowspan << "\"";
    if (var_type == "text" && !ins_value.isEmpty()) {
        out << "onmouseover=\"Tip('" << escapeString(ins_value) << "')\" onmouseout=\"UnTip()\"";
    }
    out << ">";
    if (compare_nom) {
        out << compareValues(nom_value.toDouble(), ins_value.toDouble(), tolerance, bg_class).arg(ins_value);
    } else if (var_type == "text" && !ins_value.isEmpty()) {
        out << "...";
    } else {
        out << ins_value;
    }
    out << "</td>";
}

QStringList MainWindow::listWarnings(StringVariantMap & inspection, StringVariantMap & nominal_ins, const QString & customer_id, const QString & circuit_id, QStringList & used_ids, QStringList & global_warnings, bool is_last)
{
    Warnings warnings(db, true);
    QStringList warnings_list;
    MTRecord circuit("circuit", circuit_id, MTDictionary("parent", customer_id));
    StringVariantMap circuit_attributes = circuit.list();
    while (warnings.next()) {
        bool show_warning = true;
        int delay = warnings.value("delay").toInt();
        if (delay) {
            int interval = circuit_attributes.value("inspection_interval").toInt();
            if (interval) { delay = interval; }
            if (!is_last || QDate::fromString(inspection.value("date").toString().split("-").first(), "yyyy.MM.dd").daysTo(QDate::currentDate()) < delay) {
                show_warning = false; continue;
            }
        }

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
            if (delay && is_last) {
                global_warnings << warnings.value("name").toString();
            } else {
                warnings_list << warnings.value("name").toString();
            }
        }
    }
    return warnings_list;
}

void MainWindow::viewAllRepairs(const QString & highlighted_id, int year)
{
    QString html; QTextStream out(&html);
    MTRecord repairs_rec("repair", "", MTDictionary());
    ListOfStringVariantMapsPtr repairs(repairs_rec.listAll());
    for (int i = 0; i < repairs->count();) {
        if (repairs->at(i).value("date").toString().split(".").first().toInt() < year) {
            repairs->removeAt(i);
        } else { ++i; }
    }
    out << "<table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">";
    int re_length = QString("repairs::").length();
    QString thead = "<tr>"; int thead_colspan = 0;
    for (int n = dict_attrnames.indexOfKey("repairs::date"); n < dict_attrnames.count() && dict_attrnames.key(n).startsWith("repairs::"); ++n) {
        thead.append("<th>" + dict_attrnames.value(n) + "</th>");
        thead_colspan++;
    }
    thead.append("</tr>");
    out << "<tr><th colspan=\"" << thead_colspan << "\" style=\"font-size: large;\">" << tr("List of repairs") << "</th></tr>";
    out << thead;
    if (repairs->count()) {
        QString attr_value;
        MTRecord inspectors_record("inspector", "", MTDictionary());
        MultiMapOfStringVariantMapsPtr inspectors(inspectors_record.mapAll("id", "person"));
        for (int i = 0; i < repairs->count(); ++i) {
            if (highlighted_id == repairs->at(i).value("date").toString()) {
                out << "<tr style=\"background-color: #F3F3F3; font-weight: bold;\">";
                out << "<td><a href=\"repair:" << repairs->at(i).value("date").toString() << "/modify\">";
            } else {
                out << "<tr><td><a href=\"repair:" << repairs->at(i).value("date").toString() << "\">";
            }
            out << repairs->at(i).value("date").toString() << "</a>";
            for (int n = dict_attrnames.indexOfKey("repairs::date") + 1; n < dict_attrnames.count() && dict_attrnames.key(n).startsWith("repairs::"); ++n) {
                attr_value = repairs->at(i).value(dict_attrnames.key(n).mid(re_length)).toString();
                out << "<td>";
                if (dict_attrnames.key(n) == "repairs::field") {
                    if (dict_attrvalues.contains("field::" + attr_value)) {
                        attr_value = dict_attrvalues.value("field::" + attr_value);
                    }
                } else if (dict_attrnames.key(n) == "repairs::repairman") {
                    attr_value = inspectors->value(attr_value).value("person").toString();
                }
                out << attr_value << "</td>";
            }
            out << "</tr>";
        }
    }
    out << "</table>";
    wv_main->setHtml(dict_html.value(tr("List of repairs")).arg(html), QUrl("qrc:/html/"));
}

void MainWindow::viewAllInspectors(const QString & highlighted_id)
{
    QString html; QTextStream out(&html);
    MTRecord inspectors_rec("inspector", "", MTDictionary());
    ListOfStringVariantMapsPtr inspectors(inspectors_rec.listAll());
    int in_length = QString("inspectors::").length();
    QString thead = "<tr>"; int thead_colspan = 2;
    for (int n = dict_attrnames.indexOfKey("inspectors::id"); n < dict_attrnames.count() && dict_attrnames.key(n).startsWith("inspectors::"); ++n) {
        thead.append("<th>" + dict_attrnames.value(n) + "</th>");
        thead_colspan++;
    }
    thead.append("<th>" + tr("Number of inspections") + "</th>");
    thead.append("<th>" + tr("Number of repairs") + "</th>");
    thead.append("</tr>");
    out << "<tr><th colspan=\"" << thead_colspan << "\" style=\"font-size: large;\">" << tr("List of inspectors") << "</th></tr>";
    out << thead;
    QString id;
    for (int i = 0; i < inspectors->count(); ++i) {
        id = inspectors->at(i).value("id").toString();
        if (highlighted_id == id) {
            out << "<tr style=\"background-color: #F3F3F3; font-weight: bold;\">";
            out << "<td><a href=\"inspector:" << id << "/modify\">";
        } else {
            out << "<tr><td><a href=\"inspector:" << id << "\">";
        }
        out << id.rightJustified(4, '0') << "</a>";
        for (int n = dict_attrnames.indexOfKey("inspectors::id") + 1; n < dict_attrnames.count() && dict_attrnames.key(n).startsWith("inspectors::"); ++n) {
            out << "<td>" << inspectors->at(i).value(dict_attrnames.key(n).mid(in_length)).toString() << "</td>";
        }
        MTRecord inspections("inspection", "", MTDictionary("inspector", id));
        out << "<td>" << inspections.listAll("date")->count() << "</td>";
        MTRecord repairs("repair", "", MTDictionary("repairman", id));
        out << "<td>" << repairs.listAll("date")->count() << "</td>";
        out << "</tr>";
    }
    wv_main->setHtml(dict_html.value(tr("List of inspectors")).arg(html), QUrl("qrc:/html/"));
}

void MainWindow::viewRefrigerantConsumption(const QString & customer_id)
{
    QString html; QTextStream out(&html);
    QSqlQuery query;
    query.prepare("SELECT circuits.field, circuits.refrigerant, inspections.refr_add_am, inspections.refr_reco FROM inspections LEFT JOIN circuits ON inspections.circuit = circuits.id AND inspections.customer = circuits.parent" + QString(customer_id.toInt() < 0 ? "" : " WHERE inspections.customer = :customer_id"));
    if (customer_id.toInt() >= 0) { query.bindValue(":customer_id", customer_id); }
    query.exec();
    QMap<QString, QStringList> sums_map; QString current_name;
    while (query.next()) {
        current_name = query.value(1).toString() + "::" + dict_attrvalues.value(dict_attrvalues.indexOfKey("field::" + query.value(0).toString()));
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

    QStringList used_fields;
    MTDictionary used_refrs;
    while (iter.hasNext()) {
        iter.next();
        QString refr = iter.key().split("::").first();
        QString field = iter.key().split("::").last();
        if (refr == field) continue;
        if (refr != "") {
            used_refrs.insert(refr, dict_attrvalues.value(dict_attrvalues.indexOfKey("refrigerant::" + refr)));
        }
        if (field != "" && !used_fields.contains(field)) {
            used_fields << field;
        }
    }
    used_fields.sort();

    QString ver_refr_str; QString refr_str = tr("Refrigerants");
    for (int i = 0; i < refr_str.count(); ++i) {
        ver_refr_str.append(QString(refr_str.at(i)) + "<br />");
    }

    out << "<table class=\"default_table\" cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\"><thead><tr class=\"normal_table\" style=\"background-color:#eee\">";
    out << "<td class=\"normal_table\" style=\"font-size: large; text-align: center;\"><b>" << tr("Refrigerant consumption:") << "&nbsp;";
    if (customer_id.toInt() < 0) {
        out << "<a href=\"allcustomers:\">" << tr("List of customers") << "</a>";
    } else {
        out << tr("Customer:") << "&nbsp;" << "<a href=\"customer:" << customer_id << "\">" << customer_id.rightJustified(8, '0') << "</a>";
    }
    out << "</b></td></tr></thead></table>";

    QStringList default_return_list;
    default_return_list << "0"; default_return_list << "0"; default_return_list << "0"; default_return_list << "0";
    QStringList tables;
    tables << dict_varnames.value("refr_add"); tables << dict_varnames.value("refr_reco");
    //tables << dict_varnames.value("refr_recy"); tables << dict_varnames.value("refr_disp");
    for (int t = 0; t < tables.count(); ++t) {
        out << "<br /><table><thead><tr><th colspan=\"2\" rowspan=\"2\">" << tables.at(t) << "</th>";
        out << "<th colspan=\"" << used_fields.count()+1 << "\">" << tr("Fields") << "</th></tr>";
        out << "<tr><th>" << tr("All") << "</th>";
        for (int i = 0; i < used_fields.count(); ++i) {
            out << "<th>" << used_fields.at(i) << "</th>";
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
                if (iter.key().endsWith(used_fields.at(n))) {
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
                current_name = used_refrs.key(i) + "::" + used_fields.at(n);
                out << "<td>" << sums_map.value(current_name, default_return_list).at(t).toDouble() << "</td>";
            }
            out << "</tr>";
        }
        out << "<tr></tr></table>";
    }
    wv_main->setHtml(dict_html.value(tr("Refrigerant consumption")).arg(html), QUrl("qrc:/html/"));
}

void MainWindow::viewAgenda()
{
    QString html; QTextStream out(&html);

    MTRecord inspections_rec("inspection", "", MTDictionary());
    ListOfStringVariantMapsPtr inspections(inspections_rec.listAll("date, customer, circuit"));
    MTRecord circuits_rec("circuit", "", MTDictionary());
    ListOfStringVariantMapsPtr circuits(circuits_rec.listAll());
    MTRecord customers_rec("customer", "", MTDictionary());
    MultiMapOfStringVariantMapsPtr customers(customers_rec.mapAll("id", "company"));

    out << "<table class=\"default_table\" cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\"><thead><tr class=\"normal_table\" style=\"background-color:#eee\">";
    out << "<td class=\"normal_table\" style=\"font-size: large; text-align: center;\"><b>" << tr("Agenda");
    out << "</b></td></tr></thead></table>";
    out << "<br /><table><tr><th>" << tr("Next inspection") << "</th><th>" << tr("Customer") << "</th>";
    out << "<th>" << tr("Circuit") << "</th><th>" << tr("Last inspection") << "</th></tr>";
    QMap<QString, QString> inspections_map;
    QMap<QString, QString> next_inspections_map;
    Warnings warnings(db, true);
    for (int i = 0; i < circuits->count(); ++i) {
        QString newest_ins = "0000.00.00-00:00";
        QString circuit = circuits->at(i).value("id").toString();
        QString customer = circuits->at(i).value("parent").toString();
        for (int j = 0; j < inspections->count(); ++j) {
            if (inspections->at(j).value("circuit").toString() != circuit ||
                inspections->at(j).value("customer").toString() != customer) { continue; }
            if (newest_ins < inspections->at(j).value("date").toString()) {
                newest_ins = inspections->at(j).value("date").toString();
            }
        }
        if (newest_ins == "0000.00.00-00:00") {
            newest_ins = circuits->at(i).value("commissioning").toString();
            if (newest_ins == "") continue;
        }
        inspections_map.insert(customer + "::" + circuit, newest_ins);
        int interval = circuits->at(i).value("inspection_interval").toInt();
        QDate ins_date = QDate::fromString(newest_ins.split("-").first(), "yyyy.MM.dd");
        while (warnings.next()) {
            int delay = warnings.value("delay").toInt();
            if (!delay) continue;
            bool show_warning = true;

            WarningFilters warnings_filters(warnings.value("id").toInt());
            while (warnings_filters.next()) {
                QString circuit_attribute = circuits->at(i).value(warnings_filters.value("circuit_attribute").toString()).toString();
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
            if (!show_warning) continue;

            if (interval) delay = interval;
            next_inspections_map.insert(ins_date.addDays(delay).toString("yyyy.MM.dd"), customer + "::" + circuit + ";" + circuits->at(i).value("name").toString());
        }
    }
    QMapIterator<QString, QString> i(next_inspections_map);
    QString next_ins; QString colour;
    while (i.hasNext()) { i.next();
        QString customer = i.value().left(i.value().indexOf("::"));
        QString circuit = i.value();
        circuit.remove(0, circuit.indexOf("::") + 2);
        QString circuit_name = circuit.right(circuit.length() - circuit.indexOf(";") - 1);
        circuit.truncate(circuit.indexOf(";"));
        int days_to = QDate::currentDate().daysTo(QDate::fromString(i.key(), "yyyy.MM.dd"));
        if (days_to == 0) {
            next_ins = tr("Today");
        } else if (days_to == 1) {
            next_ins = tr("Tomorrow");
        } else if (days_to == -1) {
            next_ins = tr("Yesterday");
        } else {
            next_ins = i.key();
        }
        if (days_to < 0) colour = "tomato";
        else if (days_to == 0) colour = "yellow";
        else colour = "";
        out << "<tr><td class=\"" << colour << "\">" << next_ins << "</td>";
        out << "<td class=\"" << colour << "\"><a href=\"customer:" << customer << "\">";
        out << customer.rightJustified(8, '0') << " (" << customers->value(customer).value("company").toString() << ")</a></td>";
        out << "<td class=\"" << colour << "\"><a href=\"customer:" << customer << "/circuit:" << circuit << "\">";
        out << circuit.rightJustified(4, '0');
        if (!circuit_name.isEmpty()) { out << " (" << circuit_name << ")"; }
        out << "</a></td>";
        out << "<td class=\"" << colour << "\"><a ";
        if (inspections_map.value(customer + "::" + circuit).split("-").count() > 1) {
            out << "href=\"customer:" << customer << "/circuit:" << circuit << "/inspection:" << inspections_map.value(customer + "::" + circuit) << "\"";
        }
        out << ">" << inspections_map.value(customer + "::" + circuit) << "</a></td></tr>";
    }
    out << "</table>";

    wv_main->setHtml(dict_html.value(tr("Agenda")).arg(html), QUrl("qrc:/html/"));
}
