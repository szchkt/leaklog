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

#include "global.h"
#include "reportdata.h"
#include "records.h"

#include <QVariant>
#include <QVector>

void ReportData::addToStore(QMap<int, QMap<QString, double> > &store, QList<int> &years, int year, const QString &refrigerant, double value)
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

ReportData::ReportData(int since, const QString &filter_refrigerant, bool by_field, const QSet<QString> &refrigerants_by_field)
{
    QVector<double> *sum_list; int year = 0; QString date, refrigerant;
    QVariant purchased, purchased_reco, sold, sold_reco, new_charge, refr_add_am, refr_reco, refr_rege, refr_disp, leaked, leaked_reco;
    RefrigerantRecord refr_man_record("");
    if (!filter_refrigerant.isEmpty())
        refr_man_record.parents().insert("refrigerant", filter_refrigerant);
    ListOfVariantMaps refr_man(refr_man_record.listAll());
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

        QVector<QString> entries_list(ENTRIES::COUNT);
        entries_list[ENTRIES::LINK] = QString("refrigerantrecord:%1/edit").arg(date);
        entries_list[ENTRIES::COMPANY] = refr_man.at(i).value("partner").toString();
        int company_id = refr_man.at(i).value("partner_id").toInt();
        entries_list[ENTRIES::COMPANY_ID] = company_id ? Global::formatCompanyID(company_id) : QString();
        entries_list[ENTRIES::REFRIGERANT] = refrigerant;
        entries_list[ENTRIES::PURCHASED] = purchased.toString();
        entries_list[ENTRIES::PURCHASED_RECO] = purchased_reco.toString();
        entries_list[ENTRIES::SOLD] = sold.toString();
        entries_list[ENTRIES::SOLD_RECO] = sold_reco.toString();
        entries_list[ENTRIES::REFR_REGE] = refr_rege.toString();
        entries_list[ENTRIES::REFR_DISP] = refr_disp.toString();
        entries_list[ENTRIES::LEAKED] = leaked.toString();
        entries_list[ENTRIES::LEAKED_RECO] = leaked_reco.toString();
        entries_map.insert(date, entries_list);

        if (by_field)
            refrigerant += ":0";

        if (!sums_map.contains(QString::number(year))) { sums_map.insert(QString::number(year), NULL); }
        if (!sums_map.contains(QString("%1::%2").arg(year).arg(refrigerant))) {
            sum_list = new QVector<double>(SUMS::COUNT);
            sums_map.insert(QString("%1::%2").arg(year).arg(refrigerant), sum_list);
        } else {
            sum_list = sums_map.value(QString("%1::%2").arg(year).arg(refrigerant));
        }

        (*sum_list)[SUMS::PURCHASED] += purchased.toDouble();
        (*sum_list)[SUMS::PURCHASED_RECO] += purchased_reco.toDouble();
        (*sum_list)[SUMS::SOLD] += sold.toDouble();
        (*sum_list)[SUMS::SOLD_RECO] += sold_reco.toDouble();
        (*sum_list)[SUMS::REFR_REGE] += refr_rege.toDouble();
        (*sum_list)[SUMS::REFR_DISP] += refr_disp.toDouble();
        (*sum_list)[SUMS::LEAKED] += leaked.toDouble();
        (*sum_list)[SUMS::LEAKED_RECO] += leaked_reco.toDouble();
    }

    MTRecord inspections_record("inspections", "date", "", MTDictionary());
    inspections_record.addJoin("LEFT JOIN customers ON customer = customers.id");
    inspections_record.addJoin("LEFT JOIN circuits ON customer = circuits.parent AND circuit = circuits.id");
    QString fields = "inspections.customer, inspections.circuit, inspections.date, "
                     "inspections.nominal, inspections.refr_add_am, inspections.refr_reco, "
                     "customers.company, circuits.refrigerant";
    if (by_field)
        fields += ", circuits.field";
    if (!filter_refrigerant.isEmpty())
        inspections_record.addFilter("circuits.refrigerant = ?", filter_refrigerant);
    ListOfVariantMaps inspections(inspections_record.listAll(fields));

    Repair repairs_rec("");
    repairs_rec.addJoin("LEFT JOIN customers ON parent = customers.id");
    fields = "COALESCE(customers.company, repairs.customer) AS company, repairs.parent AS customer, repairs.date, "
             "repairs.refrigerant, repairs.refr_add_am, repairs.refr_reco";
    if (by_field)
        fields += ", repairs.field";
    if (!filter_refrigerant.isEmpty())
        repairs_rec.parents().insert("refrigerant", filter_refrigerant);
    inspections << repairs_rec.listAll(fields);

    for (int i = 0; i < inspections.count(); ++i) {
        refr_add_am = inspections.at(i).value("refr_add_am");
        refr_reco = inspections.at(i).value("refr_reco");
        if (refr_add_am.toDouble() == 0.0 &&
            refr_reco.toDouble() == 0.0) continue;

        refrigerant = inspections.at(i).value("refrigerant").toString();
        date = inspections.at(i).value("date").toString();
        year = date.left(4).toInt();
        QVector<QString> entries_list(ENTRIES::COUNT);
        if (inspections.at(i).contains("circuit")) {
            entries_list[ENTRIES::LINK] = QString("customer:%1/circuit:%2/%3:%4")
                                        .arg(inspections.at(i).value("customer").toString())
                                        .arg(inspections.at(i).value("circuit").toString())
                                        .arg(inspections.at(i).value("nominal").toInt() ? "nominalinspection" : "inspection")
                                        .arg(date);
        } else {
            entries_list[ENTRIES::LINK] = QString("repair:%1/edit").arg(date);
        }

        addToStore(store, store_years, year, refrigerant, - refr_add_am.toDouble());
        addToStore(store_recovered, store_recovered_years, year, refrigerant, refr_reco.toDouble());

        if (year < since) { continue; }

        entries_list[ENTRIES::COMPANY] = inspections.at(i).value("company").toString();
        int company_id = inspections.at(i).value("customer").toInt();
        entries_list[ENTRIES::COMPANY_ID] = company_id ? Global::formatCompanyID(company_id) : QString();

        new_charge = 0.0;
        if (inspections.at(i).contains("nominal") && inspections.at(i).value("nominal").toInt()) {
            new_charge = refr_add_am;
            refr_add_am = 0.0;
        }

        entries_list[ENTRIES::NEW_CHARGE] = new_charge.toString();
        entries_list[ENTRIES::REFR_ADD_AM] = refr_add_am.toString();
        entries_list[ENTRIES::REFR_RECO] = refr_reco.toString();

        if (by_field) {
            if (refrigerants_by_field.isEmpty() || refrigerants_by_field.contains(refrigerant))
                refrigerant += QString(":%1").arg(Global::fieldOfApplicationToId(inspections.at(i).value("field").toString()));
            else
                refrigerant += ":0";
        }

        entries_list[ENTRIES::REFRIGERANT] = refrigerant;
        entries_map.insert(date, entries_list);

        if (!sums_map.contains(QString::number(year))) { sums_map.insert(QString::number(year), NULL); }
        if (!sums_map.contains(QString("%1::%2").arg(year).arg(refrigerant))) {
            sum_list = new QVector<double>(SUMS::COUNT);
            sums_map.insert(QString("%1::%2").arg(year).arg(refrigerant), sum_list);
        } else {
            sum_list = sums_map.value(QString("%1::%2").arg(year).arg(refrigerant));
        }

        (*sum_list)[SUMS::NEW_CHARGE] += new_charge.toDouble();
        (*sum_list)[SUMS::REFR_ADD_AM] += refr_add_am.toDouble();
        (*sum_list)[SUMS::REFR_RECO] += refr_reco.toDouble();
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
