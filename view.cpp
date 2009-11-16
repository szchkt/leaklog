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

QString MainWindow::viewChanged(int view)
{
    QString html;
    if (!db.isOpen()) { wv_main->setHtml(html); return html; }

    wv_main->setHtml(tr("Loading..."));
    qApp->processEvents();
    bool ok = true;
    do {
        navigation->setView(view, false);
        ok = true;
        switch (view) {
            case Navigation::ServiceCompany:
                html = viewServiceCompany(navigation->filterSinceValue() == 1999 ? 0 : navigation->filterSinceValue());
                break;
            case Navigation::ListOfCustomers:
                html = viewAllCustomers();
                break;
            case Navigation::ListOfCircuits:
                if (selectedCustomer() >= 0) {
                    html = viewCustomer(toString(selectedCustomer()));
                } else {
                    view = Navigation::ListOfCustomers; ok = false;
                }
                break;
            case Navigation::ListOfInspections:
                if (selectedCustomer() >= 0 && selectedCircuit() >= 0) {
                    html = viewCircuit(toString(selectedCustomer()), toString(selectedCircuit()), navigation->filterSinceValue() == 1999 ? 0 : navigation->filterSinceValue());
                } else {
                    view = Navigation::ListOfCircuits; ok = false;
                }
                break;
            case Navigation::Inspection:
                if (selectedCustomer() >= 0 && selectedCircuit() >= 0 && !selectedInspection().isEmpty()) {
                    html = viewInspection(toString(selectedCustomer()), toString(selectedCircuit()), selectedInspection());
                } else {
                    view = Navigation::ListOfInspections; ok = false;
                }
                break;
            case Navigation::TableOfInspections:
                cb_table_edit->setCurrentIndex(navigation->tableComboBox()->currentIndex());
                if (selectedCustomer() >= 0 && selectedCircuit() >= 0 && navigation->tableComboBox()->currentIndex() >= 0) {
                    html = viewTable(toString(selectedCustomer()), toString(selectedCircuit()), navigation->tableComboBox()->currentText(), navigation->filterSinceValue() == 1999 ? 0 : navigation->filterSinceValue());
                } else {
                    view = Navigation::ListOfCircuits; ok = false;
                }
                break;
            case Navigation::ListOfRepairs:
                html = viewRepairs(selectedRepair(), navigation->filterSinceValue() == 1999 ? 0 : navigation->filterSinceValue());
                break;
            case Navigation::ListOfInspectors:
                html = viewAllInspectors(toString(selectedInspector()));
                break;
            case Navigation::LeakagesByApplication:
                html = viewLeakagesByApplication();
                break;
            case Navigation::Agenda:
                html = viewAgenda();
                break;
            default:
                view = Navigation::ServiceCompany;
                break;
        }
    } while (!ok);
    wv_main->setHtml(html, QUrl("qrc:/html/")); return html;
}

QString MainWindow::viewServiceCompany(int since)
{
    QString html; MTTextStream out(&html);
    ServiceCompany serv_company_record(DBInfoValueForKey("default_service_company"));
    StringVariantMap serv_company = serv_company_record.list();
    out << "<table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">";
    out << "<tr style=\"background-color: #DFDFDF;\"><td colspan=\"2\" style=\"font-size: larger; width:100%; text-align: center;\"><b>";
    out << "<a href=\"servicecompany:" << serv_company.value("id").toString() << "/modify\">";
    out << tr("Service company") << "</a></b></td></tr>";
    out << "<tr><td width=\"50%\"><table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">";
    int sc_length = QString("service_companies::").length();
    int num_valid = 0; QString attr_value;
    for (int n = dict_attrnames.indexOfKey("service_companies::name"); n < dict_attrnames.count() && dict_attrnames.key(n).startsWith("service_companies::"); ++n) {
        attr_value = dict_attrnames.key(n).mid(sc_length);
        if (serv_company.value(attr_value).toString().isEmpty()) continue;
        out << "<num_attr>" << num_valid << "</num_attr>";
        out << "<tr><td style=\"text-align: right; width:50%;\">" << dict_attrnames.value(n) << "&nbsp;</td>";
        out << "<td>" << MTVariant(serv_company.value(attr_value), dict_fieldtypes.value(attr_value)) << "</td></tr>";
        num_valid++;
    }
    if (num_valid != 0) {
        html.replace(QString("<num_attr>%1</num_attr>").arg(int(num_valid / 2 + num_valid % 2)), "</table></td><td width=\"50%\"><table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">");
    }
    for (int k = 0; k < num_valid; ++k) {
        html.remove(QString("<num_attr>%1</num_attr>").arg(k));
    }
    out << "</td></tr></table>";
    out << "</table>";
    out << "<table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">";
    out << "<tr><td style=\"background-color: #eee; font-size: medium; text-align: center;\"><b>";
    out << tr("Store") << "</b></td></tr>";
    out << "<tr><td align=\"center\"><table cellspacing=\"0\" cellpadding=\"4\" style=\"width:60%;\" class=\"centred_with_borders\">";
    out << "<tr><th>" << tr("Year") << "</th>";
    out << "<th>" << tr("Refrigerant") << "</th>";
    out << "<th>" << tr("New in store") << "</th>";
    out << "<th>" << tr("Recovered in store") << "</th>";
    out << "<th><a href=\"toggledetailedview:leakedinstore\">" << tr("Leaked in store") << "</a></th></tr>";
    out << "<store />";
    out << "</table></td></tr>";
    out << "<tr><td style=\"background-color: #eee; font-size: medium; text-align: center;\"><b>";
    out << tr("Refrigerant management") << "</b></td></tr>";
    out << "<tr><td><table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\" class=\"centred_with_borders\">";
    out << "<tr><th rowspan=\"2\">" << tr("Date") << "</th>";
    out << "<th rowspan=\"2\">" << tr("Refrigerant") << "</th>";
    out << "<th colspan=\"2\">" << tr("Purchased") << "</th>";
    out << "<th colspan=\"2\">" << tr("Sold") << "</th>";
    out << "<th colspan=\"4\">" << tr("Added") << "</th>";
    out << "<th colspan=\"2\">" << tr("Recovered") << "</th>";
    out << "<th rowspan=\"2\">" << tr("Reclaimed") << "</th>";
    out << "<th rowspan=\"2\">" << tr("Disposed of") << "</th>";
    if (show_leaked_in_store_in_service_company_view)
        out << "<th colspan=\"2\">" << tr("Leaked in store") << "</th>";
    out << "</tr><tr style=\"background-color: #FBFBFB;\">";
    out << "<td>" << QApplication::translate("VariableNames", "New") << "</td>";
    out << "<td>" << QApplication::translate("VariableNames", "Recovered") << "</td>";
    out << "<td>" << QApplication::translate("VariableNames", "New") << "</td>";
    out << "<td>" << QApplication::translate("VariableNames", "Recovered") << "</td>";
    out << "<td>" << QApplication::translate("VariableNames", "New charge") << "</td>";
    out << "<td>" << QApplication::translate("VariableNames", "New") << "</td>";
    out << "<td>" << QApplication::translate("VariableNames", "Recovered") << "</td>";
    out << "<td>" << QApplication::translate("VariableNames", "Total") << "</td>";
    out << "<td>" << QApplication::translate("VariableNames", "Store") << "</td>";
    out << "<td>" << QApplication::translate("VariableNames", "Customer") << "</td>";
    if (show_leaked_in_store_in_service_company_view) {
        out << "<td>" << QApplication::translate("VariableNames", "New") << "</td>";
        out << "<td>" << QApplication::translate("VariableNames", "Recovered") << "</td>";
    }
    out << "</tr>";
    ReportData data(since);
    QString store_html; MTTextStream store_out(&store_html);
    QStringList list_refrigerants = listRefrigerantsToString().split(";");
    list_refrigerants.insert(0, "");
    QString refrigerant;
    QMap<QString, double> store_map;
    QMap<QString, double> store_recovered_map;
    QMap<QString, double> store_leaked_map;
    QList<int>::const_iterator y = data.store_years.constEnd();
    while (y != data.store_years.constBegin()) {
        y--;
        if (*y < since) break;
        store_map = data.store.value(*y);
        store_recovered_map = data.store_recovered.value(*y);
        store_leaked_map = data.store_leaked.value(*y);
        store_out << "<tr><th rowspan=\"" << store_map.size() << "\">" << *y << "</th>";
        for (int i = 0, n = 0; i < list_refrigerants.count(); ++i) {
            refrigerant = list_refrigerants.at(i);
            if (store_map.contains(refrigerant) || store_recovered_map.contains(refrigerant) || store_leaked_map.contains(refrigerant)) {
                if (n) { store_out << "<tr>"; }
                store_out << "<td>" << refrigerant << "</td>";
                store_out << "<td>" << store_map.value(refrigerant) << "</td>";
                store_out << "<td>" << store_recovered_map.value(refrigerant) << "</td>";
                store_out << "<td>" << store_leaked_map.value(refrigerant) << "</td></tr>";
                n++;
            }
        }
    }
    html.replace("<store />", store_html);
    int x = show_leaked_in_store_in_service_company_view ? 0 : 2;
    int year, last_year = 0; bool it = false, bf = false; QString link;
    QMap<QString, QVector<double> *>::const_iterator sums_iterator;
    QVector<double> * sum_list = NULL;
    QMapIterator<QString, QVector<QString> > i(data.entries_map);
    i.toBack();
    while (i.hasPrevious()) { i.previous();
        year = i.key().left(4).toInt();
        if (year < last_year) { last_year = 0; }
        if (!last_year) {
            last_year = year;
            out << "<tr><th rowspan=\"<rowspan />\"><a href=\"toggledetailedview:" << year << "\">" << year << "</a></th>";
            int row_count = 0;
            sums_iterator = data.sums_map.constFind(toString(year));
            if (++sums_iterator != data.sums_map.constEnd()) {
                while (sums_iterator != data.sums_map.constEnd() && (sum_list = sums_iterator.value())) {
                    if (row_count) { out << "</tr><tr>"; }
                    out << "<th>" << sums_iterator.key().split("::").last() << "</th>";
                    for (int n = 0; n < sum_list->count() - x; ++n) {
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
        if (years_expanded_in_service_company_view.contains(year)) {
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
            for (int n = 2; n < i.value().count() - x; ++n) {
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
    out << "</table></td></tr>";
    out << "</table>";
    return dict_html.value(Navigation::ServiceCompany).arg(html);
}

void MainWindow::writeCustomersTable(MTTextStream & out, const QString & customer_id)
{
    Customer all_customers(customer_id);
    if (customer_id.isEmpty() && !navigation->filterKeyword().isEmpty()) {
        all_customers.addFilter(navigation->filterColumn(), "%" + navigation->filterKeyword() + "%");
    }
    ListOfStringVariantMaps list(all_customers.listAll());
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
    out << "<tr><th colspan=\"" << thead_colspan << "\" style=\"font-size: large; background-color: floralwhite;\">";
    if (customer_id.isEmpty()) { out << tr("List of customers"); }
    else { out << "<a href=\"customer:" << customer_id << "/modify\">" << tr("Customer") << "</a>"; }
    out << "</th></tr>";
    out << thead;
    QString id; QString highlighted_id = toString(selectedCustomer());
    for (int i = 0; i < list.count(); ++i) {
        id = list.at(i).value("id").toString();
        out << "<tr onclick=\"window.location = 'customer:" << id << "'\" style=\"cursor: pointer;";
        if (id == highlighted_id) {
            out << " background-color: rgb(242, 248, 255);\">";
        } else { out << "\">"; }
        out << "<td>" << toolTipLink("customer", id.rightJustified(8, '0'), id) << "</td>";
        for (int n = dict_attrnames.indexOfKey("customer::id") + 1; n < dict_attrnames.count() && dict_attrnames.key(n).startsWith("customer::"); ++n) {
            out << "<td>" << MTVariant(list.at(i).value(dict_attrnames.key(n).mid(cu_length)),
                                       dict_fieldtypes.value(dict_attrnames.key(n).mid(cu_length))) << "</td>";
        }
        out << "<td>" << Circuit(id, "").listAll("id").count() << "</td>";
        out << "<td>" << MTRecord("inspections", "date", "", MTDictionary("customer", id)).listAll("date").count() << "</td>";
        out << "</tr>";
    }
    out << "</table>";
}

void MainWindow::writeCircuitsTable(MTTextStream & out, const QString & customer_id, const QString & circuit_id)
{
    Circuit circuits_record(customer_id, circuit_id);
    if (circuit_id.isEmpty() && !navigation->filterKeyword().isEmpty()) {
        circuits_record.addFilter(navigation->filterColumn(), "%" + navigation->filterKeyword() + "%");
    }
    ListOfStringVariantMaps circuits(circuits_record.listAll());
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
    out << "<tr><th colspan=\"" << thead_colspan << "\" style=\"font-size: large; background-color: aliceblue;\">";
    if (circuit_id.isEmpty()) { out << tr("List of circuits"); }
    else { out << "<a href=\"customer:" << customer_id << "/circuit:" << circuit_id << "/modify\">" << tr("Circuit") << "</a>"; }
    out << "</th></tr>";
    out << thead;
    QString attr_value; QStringList dict_value; QString id;
    QString highlighted_id = toString(selectedCircuit());
    bool show_disused = false;
    for (int i = 0; i < circuits.count(); ++i) {
        if (circuit_id.isEmpty() && circuits.at(i).value("disused").toInt()) { show_disused = true; continue; }
        id = circuits.at(i).value("id").toString();
        out << "<tr onclick=\"window.location = 'customer:" << customer_id << "/circuit:" << id << "'\" style=\"cursor: pointer;";
        if (id == highlighted_id) {
            out << " background-color: rgb(242, 248, 255);\">";
        } else { out << "\">"; }
        out << "<td>" << toolTipLink("customer/circuit", id.rightJustified(4, '0'), customer_id, id) << "</td>";
        for (int n = dict_attrnames.indexOfKey("circuit::id") + 1; n < dict_attrnames.count() && dict_attrnames.key(n).startsWith("circuit::"); ++n) {
            dict_value = dict_attrnames.value(n).split("||");
            attr_value = circuits.at(i).value(dict_attrnames.key(n).mid(cc_length)).toString();
            if (dict_attrnames.key(n) == "circuit::field") {
                if (dict_attrvalues.contains("field::" + attr_value)) {
                    attr_value = dict_attrvalues.value("field::" + attr_value);
                }
            } else if (dict_attrnames.key(n) == "circuit::hermetic") {
                attr_value = attr_value.toInt() ? tr("Yes") : tr("No");
            }
            out << "<td>" << escapeString(attr_value);
            if (dict_value.count() > 1) { out << "&nbsp;" << dict_value.last(); }
            out << "</td>";
        }
        out << "<td>" << circuits.at(i).value("refrigerant_amount").toDouble() << "&nbsp;" << tr("kg");
        out << " " << circuits.at(i).value("refrigerant").toString() << "</td>";
        out << "<td>" << circuits.at(i).value("oil_amount").toDouble() << "&nbsp;" << tr("kg");
        out << " " << circuits.at(i).value("oil").toString().toUpper() << "</td>";
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
        for (int i = 0; i < circuits.count(); ++i) {
            if (!circuits.at(i).value("disused").toInt()) continue;
            id = circuits.at(i).value("id").toString();
            out << "<tr onclick=\"window.location = 'customer:" << customer_id << "/circuit:" << id << "'\" style=\"cursor: pointer;";
            if (id == highlighted_id) {
                out << " background-color: rgb(242, 248, 255);\">";
            } else { out << "\">"; }
            out << "<td>" << toolTipLink("customer/circuit", id.rightJustified(4, '0'), customer_id, id) << "</td>";
            out << "<td>" << circuits.at(i).value("manufacturer").toString() << "</td>";
            out << "<td>" << circuits.at(i).value("type").toString() << "</td>";
            out << "<td>" << circuits.at(i).value("sn").toString() << "</td>";
            out << "<td>" << circuits.at(i).value("commissioning").toString() << "</td></tr>";
        }
        out << "</table>";
    }
}

QString MainWindow::viewAllCustomers()
{
    QString html; MTTextStream out(&html);
    writeCustomersTable(out);
    return dict_html.value(Navigation::ListOfCustomers).arg(html);
}

QString MainWindow::viewCustomer(const QString & customer_id)
{
    if (actionBasic_logbook->isChecked()) {
        return viewRepairs(selectedRepair(),
            navigation->filterSinceValue() == 1999 ? 0 : navigation->filterSinceValue(),
            customer_id);
    }
    QString html; MTTextStream out(&html);
    writeCustomersTable(out, customer_id);
    out << "<br>";
    writeCircuitsTable(out, customer_id);
    return dict_html.value(Navigation::ListOfCircuits).arg(html);
}

QString MainWindow::viewCircuit(const QString & customer_id, const QString & circuit_id, int year)
{
    QString html; MTTextStream out(&html);
    writeCustomersTable(out, customer_id);
    out << "<br>";
    writeCircuitsTable(out, customer_id, circuit_id);
    Inspection inspection_record(customer_id, circuit_id, "");
    if (!navigation->filterKeyword().isEmpty()) {
        inspection_record.addFilter(navigation->filterColumn(), "%" + navigation->filterKeyword() + "%");
    }
    ListOfStringVariantMaps inspections(inspection_record.listAll("date, nominal, repair, rmds, arno, inspector, operator, refr_add_am, refr_add_am_recy, refr_reco, refr_reco_cust"));
    if (year) {
        for (int i = 0; i < inspections.count();) {
            if (inspections.at(i).value("date").toString().split(".").first().toInt() < year) {
                inspections.removeAt(i);
            } else { ++i; }
        }
    }
    Inspector inspectors_record("");
    MultiMapOfStringVariantMaps inspectors(inspectors_record.mapAll("id", "person"));
    out << "<br><table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">";
    out << "<tr><th colspan=\"9\" style=\"font-size: large; background-color: lightgoldenrodyellow;\">";
    out << "<a href=\"customer:" << customer_id << "/circuit:" << circuit_id << "/table\">";
    out << tr("Inspections and repairs") << "</a></th></tr>";
    out << "<tr><th rowspan=\"2\">" << tr("Date") << "</th>";
    out << "<th colspan=\"2\">" << dict_varnames.value("refr_add") << "</th>";
    out << "<th colspan=\"2\">" << dict_varnames.value("refr_recovery") << "</th>";
    out << "<th rowspan=\"2\">" << dict_varnames.value("inspector") << "</th>";
    out << "<th rowspan=\"2\">" << dict_varnames.value("operator") << "</th>";
    out << "<th rowspan=\"2\">" << dict_varnames.value("rmds") << "</th>";
    out << "<th rowspan=\"2\">" << dict_varnames.value("arno") << "</th></tr>";
    out << "<tr><th>" << dict_varnames.value("refr_add_am") << "</th>";
    out << "<th>" << dict_varnames.value("refr_add_am_recy") << "</th>";
    out << "<th>" << dict_varnames.value("refr_reco") << "</th>";
    out << "<th>" << dict_varnames.value("refr_reco_cust") << "</th></tr>";
    bool is_nominal, is_repair;
    QString id; QString highlighted_id = selectedInspection();
    for (int i = 0; i < inspections.count(); ++i) {
        id = inspections.at(i).value("date").toString();
        is_nominal = inspections.at(i).value("nominal").toInt();
        is_repair = inspections.at(i).value("repair").toInt();
        out << "<tr onclick=\"window.location = 'customer:" << customer_id << "/circuit:" << circuit_id;
        out << (is_repair ? "/repair:" : "/inspection:") << id << "'\" style=\"cursor: pointer;";
        if (id == highlighted_id) {
            out << " background-color: rgb(242, 248, 255);\">";
        } else { out << "\">"; }
        out << "<td>";
        if (is_nominal) { out << "<b>"; }
        else if (is_repair) { out << "<i>"; }
        out << toolTipLink(is_repair ? "customer/circuit/repair" : "customer/circuit/inspection", id, customer_id, circuit_id, id, !isRecordLocked(id));
        if (is_nominal) { out << "<b>"; }
        else if (is_repair) { out << "<i>"; }
        out << "</td>";
        out << "<td>" << inspections.at(i).value("refr_add_am").toDouble() << "&nbsp;" << tr("kg") << "</td>";
        out << "<td>" << inspections.at(i).value("refr_add_am_recy").toDouble() << "&nbsp;" << tr("kg") << "</td>";
        out << "<td>" << inspections.at(i).value("refr_reco").toDouble() << "&nbsp;" << tr("kg") << "</td>";
        out << "<td>" << inspections.at(i).value("refr_reco_cust").toDouble() << "&nbsp;" << tr("kg") << "</td>";
        out << "<td>" << escapeString(inspectors.value(inspections.at(i).value("inspector").toString()).value("person", inspections.at(i).value("inspector")).toString()) << "</td>";
        out << "<td>" << escapeString(inspections.at(i).value("operator").toString()) << "</td>";
        if (!inspections.at(i).value("rmds").toString().isEmpty()) {
            out << "<td onmouseover=\"Tip('" << escapeString(inspections.at(i).value("rmds").toString(), true, true) << "')\" onmouseout=\"UnTip()\">...</td>";
        } else {
            out << "<td></td>";
        }
        out << "<td>" << inspections.at(i).value("arno").toString() << "</td>";
        out << "</tr>";
    }
    out << "</table>";
    return dict_html.value(Navigation::ListOfInspections).arg(html);
}

QString MainWindow::viewInspection(const QString & customer_id, const QString & circuit_id, const QString & inspection_date)
{
    QString html; MTTextStream out(&html);
    writeCustomersTable(out, customer_id);
    out << "<br>";
    writeCircuitsTable(out, customer_id, circuit_id);
    Inspection inspection_record(customer_id, circuit_id, inspection_date);
    StringVariantMap inspection = inspection_record.list();
    bool nominal = inspection.value("nominal").toInt();
    bool repair = inspection.value("repair").toInt();
    bool locked = isRecordLocked(inspection_date);
    Inspection nom_inspection_record(customer_id, circuit_id, "");
    nom_inspection_record.parents()->insert("nominal", "1");
    StringVariantMap nominal_ins = nom_inspection_record.list();
    Circuit circuit_rec(customer_id, circuit_id);
    QString circuit_name = circuit_rec.list("name").value("name").toString();

    out << "<br><table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\" class=\"no_border\">";
    out << "<tr><th colspan=\"4\" style=\"font-size: large; background-color: lightgoldenrodyellow;\">";
    if (!locked) {
        out << "<a href=\"customer:" << customer_id << "/circuit:" << circuit_id;
        out << (repair ? "/repair:" : "/inspection:") << inspection_date << "/modify\">";
    }
    if (nominal) out << tr("Nominal inspection:"); else if (repair) out << tr("Repair:"); else out << tr("Inspection:");
    out << "&nbsp;" << inspection_date;
    if (!locked) out << "</a>";
    out << "</th></tr>";

    QStringList used_ids = listVariableIds(); // all = false
    Variables vars;
    int half = vars.count() % 2 ? (vars.count() / 2 + 1) : (vars.count() / 2);
    QVector<QString> rows(half); int row = 0;
    QString var_id, var_type; QString ins_value, nom_value;
    bool compare_nom = false, ok_eval = true, is_subvar = false;
    MTDictionary expression; double tolerance = 0.0;
    while (vars.next()) {
        compare_nom = false; ok_eval = true; is_subvar = false;
        if (vars.value("SUBVAR_ID").toString().isEmpty()) {
            var_id = vars.value("VAR_ID").toString();
            var_type = vars.value("VAR_TYPE").toString();
            tolerance = vars.value("VAR_TOLERANCE").toDouble();
        } else {
            var_id = vars.value("SUBVAR_ID").toString();
            var_type = vars.value("SUBVAR_TYPE").toString();
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
        expression.clear();
        if (!vars.value("VAR_VALUE").toString().isEmpty()) {
            expression = parseExpression(vars.value("VAR_VALUE").toString(), used_ids);
        } else if (!vars.value("SUBVAR_VALUE").toString().isEmpty()) {
            expression = parseExpression(vars.value("SUBVAR_VALUE").toString(), used_ids);
        }
        if (expression.count()) {
            ins_value = toString(evaluateExpression(inspection, expression, customer_id, circuit_id, &ok_eval));
            if (!ok_eval) continue;
            if (compare_nom) {
                nom_value = toString(evaluateExpression(nominal_ins, expression, customer_id, circuit_id, &ok_eval));
                if (!ok_eval) compare_nom = false;
            }
        } else {
            ins_value = inspection.value(var_id).toString();
            if (compare_nom) {
                nom_value = nominal_ins.value(var_id).toString();
                if (nom_value.isEmpty()) compare_nom = false;
            }
        }
        //if (ins_value.isEmpty()) continue;
        if (var_id == "inspector" && !ins_value.isEmpty()) {
            Inspector inspector(ins_value);
            ins_value = inspector.list("person").value("person", ins_value).toString();
        } else if (var_type == "bool") {
            ins_value = ins_value.toInt() ? tr("Yes") : tr("No");
        }
        if (row == half) { row = 0; }
        rows[row].append("<td style=\"text-align: right; width: 25%; border-style: none;\">");
        if (!is_subvar) {
            rows[row].append(vars.value("VAR_NAME").toString() + ":");
        } else {
            rows[row].append(vars.value("VAR_NAME").toString() + ": " + vars.value("SUBVAR_NAME").toString() + ":");
        }
        rows[row].append("</td><td style=\"width: 25%; border-style: none;\"><table cellpadding=\"0\" cellspacing=\"0\" class=\"no_border\" style=\"width: 0%\"><tr><td ");
        if ((is_subvar && vars.value("SUBVAR_TYPE").toString() != "text") || (!is_subvar && vars.value("VAR_TYPE").toString() != "text"))
            rows[row].append("align=\"right\"");
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
        row++;
    }
    for (int i = 0; i < rows.count(); ++i) {
        out << "<tr>" << rows.at(i) << "</tr>";
    }
    out << "</table>";
//*** Warnings ***
    Warnings warnings(db, true, customer_id, circuit_id);
    QStringList warnings_list = listWarnings(warnings, customer_id, circuit_id, nominal_ins, inspection);
    if (warnings_list.count()) {
        out << "<br><table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">";
        out << "<tr><th style=\"font-size: larger;\">" << tr("Warnings") << "</th></tr>";
        out << "<tr><td>" << warnings_list.join(", ") << "</td></tr></table>";
    }
    return dict_html.value(Navigation::Inspection).arg(html);
}

QString MainWindow::viewTable(const QString & customer_id, const QString & circuit_id, const QString & table_id, int year)
{
    QString html; MTTextStream out(&html);

    Variables vars;
    QStringList used_ids = listVariableIds();
//*** Map variables ***
    QString last_id;
    MapOfStringVariantMaps variables; StringVariantMap variable;
    QList<QVariant> subvariables; StringVariantMap subvariable;
    while (vars.next()) {
        if (vars.value("VAR_ID").toString() != last_id) {
            if (!last_id.isEmpty()) {
                variable.insert("subvariables", subvariables);
                variables.insert(last_id, variable);
            }
            variable.clear(); subvariables.clear();
            variable.insert("name", vars.value("VAR_NAME").toString());
            variable.insert("type", vars.value("VAR_TYPE").toString());
            variable.insert("unit", vars.value("VAR_UNIT").toString());
            variable.insert("value", vars.value("VAR_VALUE").toString());
            variable.insert("compare_nom", vars.value("VAR_COMPARE_NOM").toString());
            variable.insert("col_bg", vars.value("VAR_COL_BG").toString());
            variable.insert("tolerance", vars.value("VAR_TOLERANCE").toString());
            last_id = vars.value("VAR_ID").toString();
        }
        if (!vars.value("SUBVAR_ID").toString().isEmpty()) {
            subvariable.clear();
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
        variable.insert("subvariables", subvariables);
        variables.insert(last_id, variable);
    }

    Table table_record(table_id);
    StringVariantMap table = table_record.list();
    QStringList table_vars = table.value("variables").toString().split(";", QString::SkipEmptyParts);

    Inspection inspection_record(customer_id, circuit_id, "");
    ListOfStringVariantMaps inspections(inspection_record.listAll());
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
    StringVariantMap nominal_ins;
    for (int i = 0; i < inspections.count(); ++i) {
        if (inspections.at(i).value("nominal").toInt()) {
            nominal_ins = inspections[i];
            break;
        }
    }

//*** Top tables ***
    Customer customer(customer_id);
    StringVariantMap customer_info = customer.list("company, contact_person, address, mail, phone");
    Circuit circuit(customer_id, circuit_id);
    StringVariantMap circuit_info = circuit.list("name, manufacturer, type, sn, year, commissioning, field, refrigerant, refrigerant_amount, oil, oil_amount, runtime, utilisation");
    out << "<table><tr><th>" << QApplication::translate("Customer", "ID");
    out << "</th><th>" << tr("Company");
    out << "</th><th>" << tr("Contact person");
    out << "</th><th>" << tr("Address");
    out << "</th><th>" << tr("E-mail");
    out << "</th><th>" << tr("Phone");
    out << "</th></tr><tr>";
    out << "<td>" << toolTipLink("customer", customer_id.rightJustified(8, '0'), customer_id) << "</td>";
    out << "<td>" << MTVariant(customer_info.value("company")) << "</td>";
    out << "<td>" << MTVariant(customer_info.value("contact_person")) << "</td>";
    out << "<td>" << MTAddress(customer_info.value("address").toString()).toHtml() << "</td>";
    out << "<td>" << MTVariant(customer_info.value("mail")) << "</td>";
    out << "<td>" << MTVariant(customer_info.value("phone")) << "</td>";
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
    out << "</th></tr><tr>";
    out << "<td>" << toolTipLink("customer/circuit", circuit_id.rightJustified(4, '0'), customer_id, circuit_id) << "</td>";
    out << "<td>" << MTVariant(circuit_info.value("name")) << "</td>";
    out << "<td>" << MTVariant(circuit_info.value("manufacturer")) << "</td>";
    out << "<td>" << MTVariant(circuit_info.value("type")) << "</td>";
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
    out << "</td></tr></table><br />";

    out << "<table>";

//*** Head ***
    out << "<thead>";
    out << "<tr class=\"border_top\">";
    out << "<th rowspan=\"3\">" << tr("Date") << "</th>";
    for (int i = 0; i < table_vars.count(); ++i) {
        variable = variables.value(table_vars.at(i));
        int subvar_count = variable.value("subvariables").toList().count();
        out << "<th colspan=\"" << subvar_count << "\" rowspan=\"";
        if (subvar_count > 0) out << 1;
        else if (variable.value("unit").toString() != "") out << 2;
        else out << 3;
        out << "\" class=\"" << variable.value("col_bg").toString();
        out << "\">" << variable.value("name").toString() << "</th>";
    }
    out << "</tr><tr>";
    for (int i = 0; i < table_vars.count(); ++i) {
        variable = variables.value(table_vars.at(i));
        if (variable.value("subvariables").toList().count() > 0) {
            subvariables = variable.value("subvariables").toList();
            for (int n = 0; n < subvariables.count(); ++n) {
                subvariable = subvariables.at(n).toMap();
                out << "<th rowspan=\"";
                if (subvariable.value("unit").toString().isEmpty()) {
                    out << 2;
                } else out << 1;
                out << "\" class=\"" << variable.value("col_bg").toString();
                out << "\">" << subvariable.value("name").toString() << "</th>";
            }
        }
    }
    out << "</tr><tr class=\"border_bottom\">";
    for (int i = 0; i < table_vars.count(); ++i) {
        variable = variables.value(table_vars.at(i));
        if (variable.value("subvariables").toList().count() > 0) {
            subvariables = variable.value("subvariables").toList();
            QString unit;
            for (int n = 0; n < subvariables.count(); ++n) {
                unit = subvariables.at(n).toMap().value("unit").toString();
                if (!unit.isEmpty()) {
                    out << "<th class=\"" << variable.value("col_bg").toString();
                    out << "\">" << unit << "</th>";
                }
            }
        } else {
            if (!variable.value("unit").toString().isEmpty()) {
                out << "<th class=\"" << variable.value("col_bg").toString();
                out << "\">" << variable.value("unit").toString() << "</th>";
            }
        }
    }
    out << "</tr></thead>";

//*** Body ***
    bool is_nominal, is_repair; QString inspection_date;
    out << "<tbody>";
    for (int i = 0; i < inspections.count(); ++i) {
        is_nominal = inspections.at(i).value("nominal").toInt();
        is_repair = inspections.at(i).value("repair").toInt();
        inspection_date = inspections.at(i).value("date").toString();
        out << "<tr class=\"";
        if (is_nominal && table.value("highlight_nominal").toInt()) {
            out << "nominal";
        }
        out << "\"><td>";
        if (is_nominal) { out << "<b>"; }
        else if (is_repair) { out << "<i>"; }
        out << toolTipLink(is_repair ? "customer/circuit/repair" : "customer/circuit/inspection", inspection_date, customer_id, circuit_id, inspection_date, !isRecordLocked(inspection_date));
        if (is_nominal) { out << "</b>"; }
        else if (is_repair) { out << "</i>"; }
        out << "</td>";
        for (int n = 0; n < table_vars.count(); ++n) {
            variable = variables.value(table_vars.at(n));
            bool compare_nom = false; int rowspan = 1; QString ins_value = ""; QString nom_value = ""; bool ok_eval;
            subvariables = variable.value("subvariables").toList();
            if (subvariables.count() > 0) {
                for (int s = 0; s < subvariables.count(); ++s) {
                    subvariable = subvariables.at(s).toMap();
                    compare_nom = subvariable.value("compare_nom").toInt() > 0;
                    if (subvariable.value("value").toString().contains("sum")) {
                        QString i_year = inspection_date.split(".").first();
                        if (is_nominal) rowspan = 1;
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
                    if (subvariable.value("value").toString().isEmpty()) {
                        ins_value = inspections.at(i).value(subvariable.value("id").toString()).toString();
                        if (compare_nom) {
                            nom_value = nominal_ins.value(subvariable.value("id").toString()).toString();
                            if (nom_value.isEmpty()) compare_nom = false;
                        }
                    } else {
                        MTDictionary expression = parseExpression(subvariable.value("value").toString(), used_ids);
                        ins_value = toString(evaluateExpression(inspections[i], expression, customer_id, circuit_id, &ok_eval));
                        if (!ok_eval) ins_value = "";
                        if (nominal_ins.isEmpty()) compare_nom = false;
                        if (compare_nom) {
                            nom_value = toString(evaluateExpression(nominal_ins, expression, customer_id, circuit_id, &ok_eval));
                            if (!ok_eval) compare_nom = false;
                        }
                    }
                    writeTableVarCell(out, subvariable.value("type").toString(), ins_value, nom_value, variable.value("col_bg").toString(), compare_nom, rowspan, subvariable.value("tolerance").toDouble());
                }
            } else {
                compare_nom = variable.value("compare_nom").toInt() > 0;
                if (variable.value("value").toString().isEmpty()) {
                    ins_value = inspections.at(i).value(table_vars.at(n)).toString();
                    if (compare_nom) {
                        nom_value = nominal_ins.value(table_vars.at(n)).toString();
                        if (nom_value.isEmpty()) compare_nom = false;
                    }
                } else {
                    MTDictionary expression = parseExpression(variable.value("value").toString(), used_ids);
                    ins_value = toString(evaluateExpression(inspections[i], expression, customer_id, circuit_id, &ok_eval));
                    if (!ok_eval) ins_value = "";
                    if (nominal_ins.isEmpty()) compare_nom = false;
                    if (compare_nom) {
                        nom_value = toString(evaluateExpression(nominal_ins, expression, customer_id, circuit_id, &ok_eval));
                        if (!ok_eval) compare_nom = false;
                    }
                }
                if (table_vars.at(n) == "inspector" && !ins_value.isEmpty()) {
                    Inspector inspector(ins_value);
                    ins_value = inspector.list("person").value("person", ins_value).toString();
                }
                writeTableVarCell(out, variable.value("type").toString(), ins_value, nom_value, variable.value("col_bg").toString(), compare_nom, rowspan, variable.value("tolerance").toDouble());
            }
        }
        out << "</tr>";
    }
    out << "</tbody>";

//*** Foot ***
    out << "<tfoot>";
    MTDictionary foot_functions;
    foot_functions.insert("sum", tr("Sum"));
    foot_functions.insert("avg", tr("Average"));
    for (int f = 0; f < foot_functions.count(); ++f) {
        if (!table.value(foot_functions.key(f)).toString().isEmpty()) {
            out << "<tr class=\"border_top border_bottom\">";
            out << "<th>" << foot_functions.value(f) << "</th>";
            QStringList f_vars = table.value(foot_functions.key(f)).toString().split(";", QString::SkipEmptyParts);
            for (int i = 0; i < table_vars.count(); ++i) {
                variable = variables.value(table_vars.at(i));
                bool is_in_foot = f_vars.contains(table_vars.at(i));
                if (variable.value("subvariables").toList().count() > 0) {
                    subvariables = variable.value("subvariables").toList();
                    for (int s = 0; s < subvariables.count(); ++s) {
                        subvariable = subvariables.at(s).toMap();
                        is_in_foot = f_vars.contains(table_vars.at(i));
                        if (subvariable.value("type").toString() != "float" && subvariable.value("type").toString() != "int") is_in_foot = false;
                        out << "<td class=\"" << variable.value("col_bg").toString() << "\">";
                        if (is_in_foot) {
                            double value = 0.0; int num_ins = 0;
                            if (subvariable.value("value").toString().isEmpty()) {
                                num_ins = inspections.count();
                                for (int ins = 0; ins < inspections.count(); ++ins) {
                                    value += inspections.at(ins).value(subvariable.value("id").toString()).toDouble();
                                }
                            } else {
                                MTDictionary expression = parseExpression(subvariable.value("value").toString(), used_ids);
                                for (int ins = 0; ins < inspections.count(); ++ins) {
                                    if (subvariable.value("value").toString().contains("sum") &&
                                        ins > 0 && !inspections.at(ins-1).value("nominal").toInt() &&
                                        inspections.at(ins-1).value("date").toString().split(".").first() == inspections.at(ins).value("date").toString().split(".").first())
                                            continue;
                                    num_ins++;
                                    value += evaluateExpression(inspections[ins], expression, customer_id, circuit_id);
                                }
                            }
                            if (num_ins && (foot_functions.key(f) == "avg" || subvariable.value("unit").toString() == "%"))
                                { value /= (double)num_ins; }
                            out << value;
                        }
                        out << "</td>";
                    }
                } else {
                    if (variable.value("type").toString() != "float" && variable.value("type").toString() != "int") is_in_foot = false;
                    out << "<td class=\"" << variable.value("col_bg").toString() << "\">";
                    if (is_in_foot) {
                        double value = 0.0; int num_ins = inspections.count();
                        if (variable.value("value").toString().isEmpty()) {
                            for (int ins = 0; ins < inspections.count(); ++ins) {
                                value += inspections.at(ins).value(table_vars.at(i)).toDouble();
                            }
                        } else {
                            MTDictionary expression = parseExpression(variable.value("value").toString(), used_ids);
                            for (int ins = 0; ins < inspections.count(); ++ins) {
                                value += evaluateExpression(inspections[ins], expression, customer_id, circuit_id);
                            }
                        }
                        if (num_ins && (foot_functions.key(f) == "avg" || variable.value("unit").toString() == "%"))
                            { value /= (double)num_ins; }
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
    Warnings warnings(db, true, customer_id, circuit_id);
    QString warnings_html;
    QStringList last_warnings_list, warnings_list, backup_warnings;
    for (int i = 0; i < inspections.count(); ++i) {
        inspection_date = inspections.at(i).value("date").toString();
        warnings_list = listWarnings(warnings, customer_id, circuit_id, nominal_ins, inspections[i]);
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
            warnings_html.append(inspection_date + "</a>");
            warnings_html.append("</td><td>");
            warnings_html.append(warnings_list.join(", "));
            warnings_html.append("</td></tr>");
        }
        last_warnings_list = backup_warnings;
    }
    QStringList delayed_warnings = listDelayedWarnings(warnings, customer_id, circuit_id, nominal_ins, last_entry_date, last_inspection_date);
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
    QString colours = !actionPrinter_friendly_version->isChecked() ? "<link href=\"colours.css\" rel=\"stylesheet\" type=\"text/css\" />" : "";
    return dict_html.value(Navigation::TableOfInspections).arg(colours).arg(html);
}

void MainWindow::writeTableVarCell(MTTextStream & out, const QString & var_type, const QString & ins_value, const QString & nom_value, const QString & bg_class, bool compare_nom, int rowspan, double tolerance)
{
    out << "<td class=\"" << bg_class << "\" rowspan=\"" << rowspan << "\"";
    if (var_type == "text" && !ins_value.isEmpty()) {
        out << "onmouseover=\"Tip('" << escapeString(ins_value, true, true) << "')\" onmouseout=\"UnTip()\"";
    }
    out << ">";
    if (var_type == "text") {
        if (!ins_value.isEmpty()) out << "...";
    } else if (var_type == "string") {
        out << escapeString(ins_value);
    } else if (var_type == "bool") {
        out << (ins_value.toInt() ? tr("Yes") : tr("No"));
    } else if (compare_nom && !nom_value.isEmpty()) {
        out << compareValues(nom_value.toDouble(), ins_value.toDouble(), tolerance, bg_class).arg(ins_value);
    } else {
        out << ins_value;
    }
    out << "</td>";
}

#define checkWarningConditions(inspection) \
        num_conditions = warnings.warningConditionFunctionCount(id);\
        for (int i = 0; i < num_conditions; ++i) {\
            ok = true;\
            ins_value = evaluateExpression(inspection, warnings.warningConditionValueIns(id, i), customer_id, circuit_id, &ok);\
            if (!ok) { show_warning = false; break; }\
            nom_value = evaluateExpression(nominal_ins, warnings.warningConditionValueNom(id, i), customer_id, circuit_id, &ok);\
            if (!ok) { show_warning = false; break; }\
            function = warnings.warningConditionFunction(id, i);\
            if (function == "=" && ins_value == nom_value) {}\
            else if (function == "!=" && ins_value != nom_value) {}\
            else if (function == ">" && ins_value > nom_value) {}\
            else if (function == ">=" && ins_value >= nom_value) {}\
            else if (function == "<" && ins_value < nom_value) {}\
            else if (function == "<=" && ins_value <= nom_value) {}\
            else { show_warning = false; break; }\
        }

QStringList MainWindow::listWarnings(Warnings & warnings, const QString & customer_id, const QString & circuit_id, StringVariantMap & nominal_ins, StringVariantMap & inspection)
{
    QStringList warnings_list;
    Circuit circuit(customer_id, circuit_id);
    StringVariantMap circuit_attributes = circuit.list();
    bool show_warning, ok; QString function;
    int id, num_conditions; double ins_value, nom_value;
    while (warnings.next()) {
        show_warning = true;
        if (warnings.value("delay").toInt()) { continue; }
        id = warnings.value("id").toInt();
        checkWarningConditions(inspection)
        if (show_warning) {
            warnings_list << warnings.value("name").toString();
        }
    }
    return warnings_list;
}

QStringList MainWindow::listDelayedWarnings(Warnings & warnings, const QString & customer_id, const QString & circuit_id, StringVariantMap & nominal_ins, const QString & last_entry_date, const QString & last_inspection_date, int * delay_out)
{
    QStringList warnings_list;
    Circuit circuit(customer_id, circuit_id);
    StringVariantMap circuit_attributes = circuit.list();
    Inspection last_entry_record(customer_id, circuit_id, last_entry_date);
    StringVariantMap last_entry = last_entry_record.list();
    StringVariantMap last_inspection;
    if (last_inspection_date == last_entry_date) {
        last_inspection = last_entry;
    } else {
        Inspection last_inspection_record(customer_id, circuit_id, last_inspection_date);
        last_inspection = last_inspection_record.list();
    }
    StringVariantMap * entry;
    bool show_warning, ok; QString function;
    int id, delay, interval, num_conditions; double ins_value, nom_value;
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
        if (QDate::fromString(entry->value("date").toString().split("-").first(), "yyyy.MM.dd").daysTo(QDate::currentDate()) < delay) {
            continue;
        }
        if (!nominal_ins.isEmpty()) {
            checkWarningConditions(*entry)
        }
        if (show_warning) {
            warnings_list << warnings.value("name").toString();
        }
    }
    return warnings_list;
}

QString MainWindow::viewRepairs(const QString & highlighted_id, int year, const QString & customer_id)
{
    QString html; MTTextStream out(&html);
    MTDictionary parent;
    if (!customer_id.isEmpty()) {
        Customer customer(customer_id);
        parent.insert("customer", customer.list("company").value("company").toString());
        writeCustomersTable(out, customer_id);
        out << "<br>";
    }
    MTRecord repairs_record("repairs", "date", "", parent);
    if (!navigation->filterKeyword().isEmpty()) {
        repairs_record.addFilter(navigation->filterColumn(), "%" + navigation->filterKeyword() + "%");
    }
    ListOfStringVariantMaps repairs(repairs_record.listAll());
    if (year) {
        for (int i = 0; i < repairs.count();) {
            if (repairs.at(i).value("date").toString().split(".").first().toInt() < year) {
                repairs.removeAt(i);
            } else { ++i; }
        }
    }
    out << "<table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">";
    int re_length = QString("repairs::").length();
    out << "<tr><th colspan=\"12\" style=\"font-size: large;\">" << tr("List of repairs") << "</th></tr><tr>";
    out << "<th rowspan=\"2\">" << dict_attrnames.value("repairs::date") << "</th>";
    out << "<th rowspan=\"2\">" << dict_attrnames.value("repairs::customer") << "</th>";
    out << "<th rowspan=\"2\">" << dict_attrnames.value("repairs::device") << "</th>";
    out << "<th rowspan=\"2\">" << dict_attrnames.value("repairs::field") << "</th>";
    out << "<th rowspan=\"2\">" << dict_attrnames.value("repairs::refrigerant") << "</th>";
    out << "<th rowspan=\"2\">" << dict_attrnames.value("repairs::refrigerant_amount") << "</th>";
    out << "<th colspan=\"2\">" << dict_varnames.value("refr_add") << "</th>";
    out << "<th colspan=\"2\">" << dict_varnames.value("refr_recovery") << "</th>";
    out << "<th rowspan=\"2\">" << dict_attrnames.value("repairs::repairman") << "</th>";
    out << "<th rowspan=\"2\">" << dict_attrnames.value("repairs::arno") << "</th>";
    out << "</tr><tr>";
    out << "<th>" << dict_attrnames.value("repairs::refr_add_am") << "</th>";
    out << "<th>" << dict_attrnames.value("repairs::refr_add_am_recy") << "</th>";
    out << "<th>" << dict_attrnames.value("repairs::refr_reco") << "</th>";
    out << "<th>" << dict_attrnames.value("repairs::refr_reco_cust") << "</th>";
    out << "</tr>";
    if (repairs.count()) {
        QString id, attr_value;
        Inspector inspectors_record("");
        MultiMapOfStringVariantMaps inspectors(inspectors_record.mapAll("id", "person"));
        for (int i = 0; i < repairs.count(); ++i) {
            id = repairs.at(i).value("date").toString();
            out << "<tr onclick=\"window.location = 'repair:" << id << "";
            if (highlighted_id == id) {
                if (!isRecordLocked(id)) out << "/modify";
                out << "'\" style=\"background-color: rgb(242, 248, 255); font-weight: bold;";
            } else {
                out << "'\" style=\"";
            }
            out << " cursor: pointer;\"><td><a href=\"\">" << id << "</a></td>";
            for (int n = dict_attrnames.indexOfKey("repairs::date") + 1; n < dict_attrnames.count() && dict_attrnames.key(n).startsWith("repairs::"); ++n) {
                attr_value = repairs.at(i).value(dict_attrnames.key(n).mid(re_length)).toString();
                out << "<td>";
                if (dict_attrnames.key(n) == "repairs::field") {
                    if (dict_attrvalues.contains("field::" + attr_value)) {
                        attr_value = dict_attrvalues.value("field::" + attr_value);
                    }
                } else if (dict_attrnames.key(n) == "repairs::repairman") {
                    attr_value = inspectors.value(attr_value).value("person", attr_value).toString();
                }
                out << escapeString(attr_value) << "</td>";
            }
            out << "</tr>";
        }
    }
    out << "</table>";
    return dict_html.value(Navigation::ListOfRepairs).arg(html);
}

QString MainWindow::viewAllInspectors(const QString & highlighted_id)
{
    QString html; MTTextStream out(&html);
    Inspector inspectors_record("");
    if (!navigation->filterKeyword().isEmpty()) {
        inspectors_record.addFilter(navigation->filterColumn(), "%" + navigation->filterKeyword() + "%");
    }
    ListOfStringVariantMaps inspectors(inspectors_record.listAll());
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
    for (int i = 0; i < inspectors.count(); ++i) {
        id = inspectors.at(i).value("id").toString();
        out << "<tr onclick=\"window.location = 'inspector:" << id << "";
        if (highlighted_id == id) {
            out << "/modify'\" style=\"background-color: rgb(242, 248, 255); font-weight: bold;";
        } else {
            out << "'\" style=\"";
        }
        out << " cursor: pointer;\"><td><a href=\"\">" << id.rightJustified(4, '0') << "</a></td>";
        for (int n = dict_attrnames.indexOfKey("inspectors::id") + 1; n < dict_attrnames.count() && dict_attrnames.key(n).startsWith("inspectors::"); ++n) {
            out << "<td>" << escapeString(inspectors.at(i).value(dict_attrnames.key(n).mid(in_length)).toString()) << "</td>";
        }
        out << "<td>" << MTRecord("inspections", "date", "", MTDictionary("inspector", id)).listAll("date").count() << "</td>";
        out << "<td>" << MTRecord("repairs", "date", "", MTDictionary("repairman", id)).listAll("date").count() << "</td>";
        out << "</tr>";
    }
    return dict_html.value(Navigation::ListOfInspectors).arg(html);
}

QString MainWindow::viewLeakagesByApplication()
{
    QString html; MTTextStream out(&html);
    QMap<QString, QVector<double> > map;
    QStringList keys;
    const int VECTOR_SIZE = 3;
    map["All::All"].resize(VECTOR_SIZE);
    QSqlQuery inspections("SELECT circuits.refrigerant, circuits.field, inspections.refr_add_am, inspections.refr_add_am_recy FROM inspections LEFT JOIN circuits ON inspections.circuit = circuits.id AND inspections.customer = circuits.parent");
    while (inspections.next()) {
        keys.clear();
        keys << inspections.value(0).toString() + "::" + dict_attrvalues.value(dict_attrvalues.indexOfKey("field::" + inspections.value(1).toString()));
        keys << inspections.value(0).toString() + "::All";
        keys << "All::" + dict_attrvalues.value(dict_attrvalues.indexOfKey("field::" + inspections.value(1).toString()));
        for (int i = 0; i < keys.count(); ++i) {
            if (!map[keys.at(i)].size()) { map[keys.at(i)].resize(VECTOR_SIZE); }
            map[keys.at(i)][0] += inspections.value(2).toDouble() + inspections.value(3).toDouble();
        }
        map["All::All"][0] += inspections.value(2).toDouble() + inspections.value(3).toDouble();
    }
    QSqlQuery circuits("SELECT refrigerant, field, refrigerant_amount FROM circuits");
    while (circuits.next()) {
        keys.clear();
        keys << circuits.value(0).toString() + "::" + dict_attrvalues.value(dict_attrvalues.indexOfKey("field::" + circuits.value(1).toString()));
        keys << circuits.value(0).toString() + "::All";
        keys << "All::" + dict_attrvalues.value(dict_attrvalues.indexOfKey("field::" + circuits.value(1).toString()));
        for (int i = 0; i < keys.count(); ++i) {
            if (!map[keys.at(i)].size()) { map[keys.at(i)].resize(VECTOR_SIZE); }
            map[keys.at(i)][1] += circuits.value(2).toDouble();
        }
        map["All::All"][1] += circuits.value(2).toDouble();
    }
    QMutableMapIterator<QString, QVector<double> > iterator(map);
    QStringList used_fields;
    MTDictionary used_refrigerants;
    while (iterator.hasNext()) { iterator.next();
        if (iterator.value()[1] != 0.0) {
            iterator.value()[2] = 100.0 * iterator.value()[0] / iterator.value()[1];
        }
        QString refrigerant = iterator.key().split("::").first();
        QString field = iterator.key().split("::").last();
        if (refrigerant == field) continue;
        if (refrigerant != "" && refrigerant != "All") {
            used_refrigerants.insert(refrigerant, dict_attrvalues.value(dict_attrvalues.indexOfKey("refrigerant::" + refrigerant)));
        }
        if (field != "" && field != "All" && !used_fields.contains(field)) {
            used_fields << field;
        }
    }
    used_fields.sort();

    out << "<table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\"><tr>";
    out << "<th style=\"font-size: large;\">" << tr("Leakages by application") << "</th></tr></table><br>";
    QStringList tables;
    tables << dict_varnames.value("refr_add") << tr("Amount of refrigerant in circuits") << tr("Percentage of leakage by application");
    for (int t = 0; t < tables.count(); ++t) {
        out << "<table><thead><tr><th rowspan=\"2\">" << tables.at(t) << "</th>";
        out << "<th colspan=\"" << used_fields.count() + 1 << "\">" << tr("Fields") << "</th></tr>";
        out << "<tr><th>" << tr("All") << "</th>";
        for (int i = 0; i < used_fields.count(); ++i) {
            out << "<th>" << used_fields.at(i) << "</th>";
        }
        out << "</tr></thead>";
        out << "<tr><th>" << tr("All") << "</th>";
        out << "<td>" << map["All::All"].at(t) << "</td>";
        for (int n = 0; n < used_fields.count(); ++n) {
            out << "<td>" << map["All::" + used_fields.at(n)].at(t) << "</td>";
        }
        out << "</tr>";
        for (int i = 0; i < used_refrigerants.count(); ++i) {
            out << "<tr><th>" << used_refrigerants.value(i) << "</th>";
            out << "<td>" << map[used_refrigerants.key(i) + "::All"].at(t) << "</td>";
            for (int n = 0; n < used_fields.count(); ++n) {
                out << "<td>" << map.value(used_refrigerants.key(i) + "::" + used_fields.at(n), QVector<double>(VECTOR_SIZE)).at(t) << "</td>";
            }
            out << "</tr>";
        }
        out << "<tr></tr></table><br>";
    }
    return dict_html.value(Navigation::LeakagesByApplication).arg(html);
}

QString MainWindow::viewAgenda()
{
    QString html; MTTextStream out(&html);

    MTRecord inspections_record("inspections", "date", "", MTDictionary());
    ListOfStringVariantMaps inspections(inspections_record.listAll("date, customer, circuit, nominal, repair"));
    MTRecord circuits_record("circuits", "id", "", MTDictionary());
    ListOfStringVariantMaps circuits(circuits_record.listAll());
    MTRecord customers_record("customers", "id", "", MTDictionary());
    MultiMapOfStringVariantMaps customers(customers_record.mapAll("id", "company"));

    out << "<table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\"><tr>";
    out << "<th colspan=\"4\" style=\"font-size: large;\">" << tr("Agenda") << "</th></tr>";
    out << "<tr><th>" << tr("Next inspection") << "</th><th>" << tr("Customer") << "</th>";
    out << "<th>" << tr("Circuit") << "</th><th>" << tr("Last inspection") << "</th></tr>";
    QMap<QString, QString> inspections_map;
    QMultiMap<QString, QString> next_inspections_map;
    QString last_inspection_date, circuit, customer;
    int delay, warning_delay, id, interval;
    for (int i = 0; i < circuits.count(); ++i) {
        circuit = circuits.at(i).value("id").toString();
        customer = circuits.at(i).value("parent").toString();
        last_inspection_date.clear();
        for (int j = 0; j < inspections.count(); ++j) {
            if (inspections.at(j).value("customer").toString() == customer &&
                inspections.at(j).value("circuit").toString() == circuit &&
                !inspections.at(j).value("repair").toInt() &&
                last_inspection_date < inspections.at(j).value("date").toString()) {
                last_inspection_date = inspections.at(j).value("date").toString();
            }
        }
        if (last_inspection_date.isEmpty()) {
            last_inspection_date = circuits.at(i).value("commissioning").toString();
            if (last_inspection_date.isEmpty()) continue;
        }
        inspections_map.insert(customer + "::" + circuit, last_inspection_date);
        Warnings warnings(db, true, customer, circuit);
        delay = 0;
        while (warnings.next()) {
            warning_delay = warnings.value("delay").toInt();
            if (!warning_delay) { continue; }
            id = warnings.value("id").toInt();
            if (id >= 1200 && id < 1300) {
                interval = circuits.at(i).value("inspection_interval").toInt();
                if (interval) { warning_delay = interval; }
                if (delay < warning_delay) { delay = warning_delay; }
            }
        }
        if (delay) {
            next_inspections_map.insert(QDate::fromString(last_inspection_date.split("-").first(), "yyyy.MM.dd").addDays(delay).toString("yyyy.MM.dd"),
                                        customer + "::" + circuit + ";" + circuits.at(i).value("name").toString());
        }
    }
    QMapIterator<QString, QString> i(next_inspections_map);
    QString next_ins, colour, circuit_name;
    while (i.hasNext()) { i.next();
        customer = i.value().left(i.value().indexOf("::"));
        circuit = i.value();
        circuit.remove(0, circuit.indexOf("::") + 2);
        circuit_name = circuit.right(circuit.length() - circuit.indexOf(";") - 1);
        circuit.truncate(circuit.indexOf(";"));
        int days_to = QDate::currentDate().daysTo(QDate::fromString(i.key(), "yyyy.MM.dd"));
        switch (days_to) {
            case -1: next_ins = tr("Yesterday"); break;
            case 0: next_ins = tr("Today"); break;
            case 1: next_ins = tr("Tomorrow"); break;
            default: next_ins = i.key(); break;
        }
        if (days_to < 0) colour = "tomato";
        else if (days_to == 0) colour = "yellow";
        else colour = "";
        out << "<tr><td class=\"" << colour << "\">" << next_ins << "</td>";
        out << "<td class=\"" << colour << "\"><a href=\"customer:" << customer << "\">";
        out << customer.rightJustified(8, '0') << " (" << escapeString(customers.value(customer).value("company").toString()) << ")</a></td>";
        out << "<td class=\"" << colour << "\"><a href=\"customer:" << customer << "/circuit:" << circuit << "\">";
        out << circuit.rightJustified(4, '0');
        if (!circuit_name.isEmpty()) { out << " (" << escapeString(circuit_name) << ")"; }
        out << "</a></td>";
        out << "<td class=\"" << colour << "\"><a ";
        if (inspections_map.value(customer + "::" + circuit).split("-").count() > 1) {
            out << "href=\"customer:" << customer << "/circuit:" << circuit << "/inspection:" << inspections_map.value(customer + "::" + circuit) << "\"";
        }
        out << ">" << inspections_map.value(customer + "::" + circuit) << "</a></td></tr>";
    }
    out << "</table>";

    return dict_html.value(Navigation::Agenda).arg(html);
}
