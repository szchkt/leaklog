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

#include "storeview.h"

#include "global.h"
#include "mttextstream.h"
#include "records.h"
#include "viewtabsettings.h"
#include "mainwindowsettings.h"
#include "toolbarstack.h"
#include "reportdata.h"

using namespace Global;

StoreView::StoreView(ViewTabSettings *settings):
    View(settings)
{
}

QString StoreView::renderHTML()
{
    QString html; MTTextStream out(&html);
    ServiceCompany serv_company(DBInfo::valueForKey("default_service_company_uuid"));
    out << "<table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">";
    out << "<tr style=\"background-color: #DFDFDF;\"><td colspan=\"2\" style=\"font-size: large; width:100%; text-align: center;\"><b>";
    out << "<a href=\"servicecompany:" << serv_company.companyID() << "/edit\">";
    out << tr("Service Company") << "</a></b></td></tr>";
    out << "<tr><td width=\"50%\"><table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">";
    int num_valid = 0;
    for (int n = 0; n < ServiceCompany::attributes().count(); ++n) {
        QString key = ServiceCompany::attributes().key(n);
        if (serv_company.value(key).toString().isEmpty()) continue;
        out << "<num_attr>" << num_valid << "</num_attr>";
        out << "<tr><td style=\"text-align: right; width:50%;\">" << ServiceCompany::attributes().value(n) << "&nbsp;</td>";
        out << "<td>" << MTVariant(serv_company.value(key), key) << "</td></tr>";
        num_valid++;
    }
    if (num_valid != 0) {
        html.replace(QString("<num_attr>%1</num_attr>").arg(int(num_valid / 2 + num_valid % 2)),
                     "</table></td><td width=\"50%\"><table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">");
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
    bool show_leaked = settings->isShowLeakedChecked();
    if (show_leaked)
        out << "<th>" << tr("Leaked in store") << "</th>";
    out << "</tr>";
    out << "<store />";
    out << "</table></td></tr>";
    out << "<tr><td style=\"background-color: #eee; font-size: medium; text-align: center;\"><b>";
    out << tr("Refrigerant Management") << "</b></td></tr>";
    out << "<tr><td><table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\" class=\"centred_with_borders\">";
    out << "<tr><th rowspan=\"2\">" << tr("Date") << "</th>";
    out << "<th rowspan=\"2\">" << tr("Refrigerant") << "</th>";
    bool by_field = settings->toolBarStack()->isByFieldOfApplicationChecked();
    if (by_field)
        out << "<th rowspan=\"2\">" << QApplication::translate("Circuit", "Field of application") << "</th>";
    bool show_partner = settings->toolBarStack()->isShowBusinessPartnerChecked();
    if (show_partner)
        out << "<th colspan=\"2\">" << QApplication::translate("RefrigerantRecord", "Business partner") << "</th>";
    out << "<th colspan=\"2\">" << tr("Purchased") << "</th>";
    out << "<th colspan=\"2\">" << tr("Sold") << "</th>";
    out << "<th rowspan=\"2\">" << QApplication::translate("VariableNames", "New charge") << "</th>";
    out << "<th rowspan=\"2\">" << tr("Added") << "</th>";
    out << "<th rowspan=\"2\">" << tr("Recovered") << "</th>";
    out << "<th rowspan=\"2\">" << tr("Reclaimed") << "</th>";
    out << "<th rowspan=\"2\">" << tr("Disposed of") << "</th>";
    if (show_leaked)
        out << "<th colspan=\"2\">" << tr("Leaked in store") << "</th>";
    out << "</tr><tr style=\"background-color: #FBFBFB;\">";
    if (show_partner) {
        out << "<td>" << QApplication::translate("Customer", "Company") << "</td>";
        out << "<td>" << QApplication::translate("Customer", "ID") << "</td>";
    }
    out << "<td>" << QApplication::translate("VariableNames", "New") << "</td>";
    out << "<td>" << QApplication::translate("VariableNames", "Recovered") << "</td>";
    out << "<td>" << QApplication::translate("VariableNames", "New") << "</td>";
    out << "<td>" << QApplication::translate("VariableNames", "Recovered") << "</td>";
    if (show_leaked) {
        out << "<td>" << QApplication::translate("VariableNames", "New") << "</td>";
        out << "<td>" << QApplication::translate("VariableNames", "Recovered") << "</td>";
    }
    out << "</tr>";
    ReportData data(settings->toolBarStack()->filterSinceValue(), by_field);
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
        if (*y < settings->toolBarStack()->filterSinceValue()) break;
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
                if (show_leaked)
                    store_out << "<td>" << store_leaked_map.value(refrigerant) << "</td>";
                store_out << "</tr>";
                n++;
            }
        }
    }
    html.replace("<store />", store_html);
    int year, last_year = 0;
    bool it = false, bf = false;
    QString link;
    QMap<QString, QVector<double> *>::const_iterator sums_iterator;
    QVector<double> *sum_list = NULL;
    QMapIterator<QString, QVector<QString> > i(data.entries_map);
    i.toBack();
    while (i.hasPrevious()) { i.previous();
        year = i.key().left(4).toInt();
        if (year < last_year) { last_year = 0; }
        if (!last_year) {
            last_year = year;
            out << "<tr><th rowspan=\"<rowspan />\"><a name=\"" << year << "\" id=\"" << year
                << "\" href=\"toggledetailedview:" << year << "\">" << year << "</a></th>";
            int row_count = 0;
            sums_iterator = data.sums_map.constFind(QString::number(year));
            if (++sums_iterator != data.sums_map.constEnd()) {
                while (sums_iterator != data.sums_map.constEnd() && (sum_list = sums_iterator.value())) {
                    if (row_count) { out << "</tr><tr>"; }
                    refrigerant = sums_iterator.key().split("::").last();
                    out << "<th>" << refrigerant.split(':').first() << "</th>";
                    if (by_field)
                        out << "<th>" << fieldsOfApplication().value(idToFieldOfApplication(refrigerant.split(':').last().toInt())) << "</th>";
                    if (show_partner)
                        out << "<th>&nbsp;</th><th>&nbsp;</th>";
                    for (int n = 0; n < sum_list->count(); ++n) {
                        if (!show_leaked && (n == SUMS::LEAKED || n == SUMS::LEAKED_RECO))
                            continue;
                        out << "<th>";
                        if (sum_list->at(n)) out << sum_list->at(n);
                        out << "</th>";
                    }
                    row_count++;
                    ++sums_iterator;
                }
            }
            out << "</tr>";
            html.replace("<rowspan />", QString::number(row_count));
        }
        if (years_expanded.contains(year)) {
            link = i.value().at(0);
            bf = link.contains("nominal");
            it = link.startsWith("repair:");
            if (bf) link.remove("nominal");
            out << "<tr><td";
            if (bf) out << " style=\"font-weight: bold;\"";
            else if (it) out << " style=\"font-style: italic;\"";
            out << "><a href=\"" << link << "\">";
            out << settings->mainWindowSettings().formatDateTime(i.key());
            out << "</a></td><td";
            if (bf) out << " style=\"font-weight: bold;\"";
            else if (it) out << " style=\"font-style: italic;\"";
            refrigerant = i.value().at(1);
            out << ">" << refrigerant.split(':').first() << "</td>";
            if (by_field)
                out << "<td>" << fieldsOfApplication().value(idToFieldOfApplication(refrigerant.split(':').last().toInt())) << "</td>";
            if (show_partner) {
                out << "<td>" << escapeString(i.value().at(2)) << "</td>";
                out << "<td>" << escapeString(i.value().at(3)) << "</td>";
            }
            for (int n = 4; n < i.value().count(); ++n) {
                if (!show_leaked && (n == ENTRIES::LEAKED || n == ENTRIES::LEAKED_RECO))
                    continue;
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
    return viewTemplate("service_company").arg(html);
}

QString StoreView::title() const
{
    return tr("Store");
}

void StoreView::toggleYear(int year)
{
    if (years_expanded.contains(year)) {
        years_expanded.remove(year);
    } else {
        years_expanded << year;
    }
}
