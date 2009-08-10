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

#include "report_data_controller.h"
#include "records.h"

using namespace Global;

void ReportData::addToStore(QMap<int, QMap<QString, double> > & store, QList<int> & years, int year, const QString & refrigerant, double value)
{
    if (!years.contains(year)) {
        years << year;
        qSort(years);
        QList<int>::const_iterator pos = qBinaryFind(years, year);
        if (pos != years.constBegin()) {
            pos--;
            store.insert(year, store.value(*pos));
        }
    }
    store[year][refrigerant] += value;
    for (int i = years.count() - 1; i >= 0 && years.at(i) > year; --i) {
        store[years.at(i)][refrigerant] += value;
    }
}

ReportData::ReportData(int since)
{
    QVector<double> * sum_list; int year = 0; QString date, refrigerant;
    QVariant purchased, purchased_reco, sold, sold_reco, refr_add_am, refr_add_am_recy, refr_reco, refr_rege, refr_disp, leaked, leaked_reco;
    RecordOfRefrigerantManagement refr_man_record("");
    ListOfStringVariantMaps refr_man(refr_man_record.listAll());
    for (int i = 0; i < refr_man.count(); ++i) {
        refrigerant = refr_man.at(i).value("refrigerant").toString();
        purchased = refr_man.at(i).value("purchased");
        purchased_reco = refr_man.at(i).value("purchased_reco");
        sold = refr_man.at(i).value("sold");
        sold_reco = refr_man.at(i).value("sold_reco");
        refr_rege = refr_man.at(i).value("refr_rege");
        refr_disp = refr_man.at(i).value("refr_disp");
        leaked = refr_man.at(i).value("leaked");
        leaked_reco = refr_man.at(i).value("leaked_reco");
        date = refr_man.at(i).value("date").toString();
        year = date.left(4).toInt();

        addToStore(store, store_years, year, refrigerant, purchased.toDouble() - sold.toDouble() - leaked.toDouble());
        addToStore(store_recovered, store_recovered_years, year, refrigerant, purchased_reco.toDouble() - sold_reco.toDouble() - refr_rege.toDouble() - refr_disp.toDouble() - leaked_reco.toDouble());
        addToStore(store_leaked, store_leaked_years, year, refrigerant, leaked.toDouble() + leaked_reco.toDouble());

        if (year < since) { continue; }

        QVector<QString> entries_list(14);
        entries_list[0] = QString("recordofrefrigerantmanagement:%1/modify").arg(date);
        entries_list[1] = refrigerant;
        entries_list[2] = purchased.toString();
        entries_list[3] = purchased_reco.toString();
        entries_list[4] = sold.toString();
        entries_list[5] = sold_reco.toString();
        entries_list[10] = refr_rege.toString();
        entries_list[11] = refr_disp.toString();
        entries_list[12] = leaked.toString();
        entries_list[13] = leaked_reco.toString();
        entries_map.insert(date, entries_list);
        // ----------------------------------------------------
        if (!sums_map.contains(toString(year))) { sums_map.insert(toString(year), NULL); }
        if (!sums_map.contains(QString("%1::%2").arg(year).arg(refrigerant))) {
            sum_list = new QVector<double>(12);
            sums_map.insert(QString("%1::%2").arg(year).arg(refrigerant), sum_list);
        } else {
            sum_list = sums_map.value(QString("%1::%2").arg(year).arg(refrigerant));
        }
        // ----------------------------------------------------
        (*sum_list)[0] += purchased.toDouble();
        (*sum_list)[1] += purchased_reco.toDouble();
        (*sum_list)[2] += sold.toDouble();
        (*sum_list)[3] += sold_reco.toDouble();
        (*sum_list)[8] += refr_rege.toDouble();
        (*sum_list)[9] += refr_disp.toDouble();
        (*sum_list)[10] += leaked.toDouble();
        (*sum_list)[11] += leaked_reco.toDouble();
    }
    MTRecord circuits_record("circuits", "", MTDictionary());
    MultiMapOfStringVariantMaps circuits(circuits_record.mapAll("parent::id", "refrigerant"));
    MTRecord inspections_record("inspections", "", MTDictionary());
    ListOfStringVariantMaps inspections(inspections_record.listAll("customer, circuit, date, nominal, refr_add_am, refr_add_am_recy, refr_reco"));
    Repair repairs_rec("");
    inspections << repairs_rec.listAll("date, refrigerant, refr_add_am, refr_add_am_recy, refr_reco");
    for (int i = 0; i < inspections.count(); ++i) {
        refr_add_am = inspections.at(i).value("refr_add_am");
        refr_add_am_recy = inspections.at(i).value("refr_add_am_recy");
        refr_reco = inspections.at(i).value("refr_reco");
        if (refr_add_am.toDouble() == 0.0 && refr_add_am_recy.toDouble() == 0.0 && refr_reco.toDouble() == 0.0) continue;

        date = inspections.at(i).value("date").toString();
        year = date.left(4).toInt();
        QVector<QString> entries_list(14);
        if (inspections.at(i).contains("customer")) {
            entries_list[0] = QString("customer:%1/circuit:%2/%3:%4")
                            .arg(inspections.at(i).value("customer").toString())
                            .arg(inspections.at(i).value("circuit").toString())
                            .arg(inspections.at(i).value("nominal").toInt() ? "nominalinspection" : "inspection")
                            .arg(date);
            refrigerant = circuits.value(QString("%1::%2")
                            .arg(inspections.at(i).value("customer").toString())
                            .arg(inspections.at(i).value("circuit").toString()))
                            .value("refrigerant").toString();
            entries_list[1] = refrigerant;
        } else {
            entries_list[0] = QString("repair:%1").arg(date);
            refrigerant = inspections.at(i).value("refrigerant").toString();
            entries_list[1] = refrigerant;
        }

        addToStore(store, store_years, year, refrigerant, - refr_add_am.toDouble());
        addToStore(store_recovered, store_recovered_years, year, refrigerant, refr_reco.toDouble() - refr_add_am_recy.toDouble());

        if (year < since) { continue; }

        entries_list[6] = refr_add_am.toString();
        entries_list[7] = refr_add_am_recy.toString();
        entries_list[8] = toString(refr_add_am.toDouble() + refr_add_am_recy.toDouble());
        entries_list[9] = refr_reco.toString();
        entries_map.insert(date, entries_list);
        // ----------------------------------------------------
        if (!sums_map.contains(toString(year))) { sums_map.insert(toString(year), NULL); }
        if (!sums_map.contains(QString("%1::%2").arg(year).arg(refrigerant))) {
            sum_list = new QVector<double>(12);
            sums_map.insert(QString("%1::%2").arg(year).arg(refrigerant), sum_list);
        } else {
            sum_list = sums_map.value(QString("%1::%2").arg(year).arg(refrigerant));
        }
        // ----------------------------------------------------
        (*sum_list)[4] += refr_add_am.toDouble();
        (*sum_list)[5] += refr_add_am_recy.toDouble();
        (*sum_list)[6] += refr_add_am.toDouble() + refr_add_am_recy.toDouble();
        (*sum_list)[7] += refr_reco.toDouble();
    }
}

ReportData::~ReportData()
{
    QMap<QString, QVector<double> *>::const_iterator sums_iterator = sums_map.constBegin();
    while (sums_iterator != sums_map.constEnd()) {
        delete sums_iterator.value();
        ++sums_iterator;
    }
}

ReportDataController::ReportDataController(QWebView * wv, QWidget * parent):
QObject(parent) {
    wv_main = wv;
    btn_autofill = new QPushButton(tr("Autofill"), parent);
    parent->layout()->addWidget(btn_autofill);
    QObject::connect(btn_autofill, SIGNAL(clicked()), this, SLOT(autofill()));
    btn_done = new QPushButton(tr("Done"), parent);
    parent->layout()->addWidget(btn_done);
    QObject::connect(btn_done, SIGNAL(clicked()), this, SLOT(done()));
    wp_default = wv_main->page();
    wv_main->setPage(new QWebPage(wv_main));
    //: URL to the data report system of the notified body
    wv_main->load(QUrl::QUrl(tr("http://szchkt.org/cert/")));
}

void ReportDataController::autofill()
{
    emit processing(true);
    qApp->processEvents();

    QDate current_date = QDate::currentDate();
    int year = current_date.year();
    //: Report deadline (month.day with leading zeros)
    if (current_date < QDate::fromString(QString("%1.%2").arg(current_date.year()).arg(tr("02.01")), "yyyy.MM.dd"))
        year--;

    QString js; QTextStream out(&js);
    out << "clearAll();" << endl;

    ListOfStringVariantMaps inspectors(Inspector("").listAll("person_reg_num"));
    for (ListOfStringVariantMaps::const_iterator i = inspectors.constBegin(); i != inspectors.constEnd(); ++i) {
        out << "addEmployee({ \"certification_num\": \"" << i->value("person_reg_num").toString().replace("\"", "\\\"") << "\" });" << endl;
    }

    ReportData data(year);
    QMap<QString, QVector<double> *>::const_iterator sums_iterator = data.sums_map.constFind(toString(year));
    QVector<double> * sum_list = NULL;
    QString refrigerant;
    QStringList sums_fieldnames;
    sums_fieldnames << "purchased"
                    << "purchased_reco"
                    << "sold"
                    << "sold_reco"
                    << "refr_add_am"
                    << "refr_add_am_recy"
                    << "refr_add_am_total" // [6]
                    << "refr_reco"
                    << "refr_rege"
                    << "refr_disp"
                    << "leaked"
                    << "leaked_reco";
    if (++sums_iterator != data.sums_map.constEnd()) {
        while (sums_iterator != data.sums_map.constEnd() && (sum_list = sums_iterator.value())) {
            refrigerant = sums_iterator.key().split("::").last();
            if (!refrigerant.isEmpty()) {
                out << "addRefrigerantManagementEntry({" << endl;
                out << "\t\"refrigerant\": \"" << refrigerant << "\"," << endl;
                for (int j = 0; j < sums_fieldnames.count(); ++j) {
                    if (j == 6) continue; // "refr_add_am_total"
                    out << "\t\"" << sums_fieldnames.at(j) << "\": " << sum_list->at(j);
                    if (j == sums_fieldnames.count() - 1) out << endl; else out << "," << endl;
                }
                out << "});" << endl;
            }
            ++sums_iterator;
        }
    }
    if (data.store_years.contains(year)) {
        QStringList list_refrigerants = listRefrigerantsToString().split(";");
        QMap<QString, double> store_map = data.store.value(year);
        QMap<QString, double> store_recovered_map = data.store_recovered.value(year);
        QMap<QString, double> store_leaked_map = data.store_leaked.value(year);
        for (int i = 0; i < list_refrigerants.count(); ++i) {
            refrigerant = list_refrigerants.at(i);
            if (refrigerant.isEmpty()) continue;
            if (store_map.contains(refrigerant) || store_recovered_map.contains(refrigerant) || store_leaked_map.contains(refrigerant)) {
                out << "addStoreEntry({" << endl;
                out << "\t\"refrigerant\": \"" << refrigerant << "\"," << endl;
                out << "\t\"store_new\": " << store_map.value(refrigerant) << "," << endl;
                out << "\t\"recovered\": " << store_recovered_map.value(refrigerant) << "," << endl;
                out << "\t\"leaked\": " << store_leaked_map.value(refrigerant) << endl << "});" << endl;
            }
        }
    }

    wv_main->page()->mainFrame()->evaluateJavaScript(js);
    emit processing(false);
}

void ReportDataController::done()
{
    wv_main->setPage(wp_default);
    btn_autofill->deleteLater();
    btn_done->deleteLater();
    deleteLater();
}
