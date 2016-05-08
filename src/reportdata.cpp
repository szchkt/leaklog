/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2016 Matus & Michal Tomlein

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

ReportData::ReportData(int since, bool by_field, const QSet<QString> &refrigerants_by_field)
{
    QVector<double> *sum_list;

    foreach (const QVariantMap &refr_man, RefrigerantRecord::query().listAll()) {
        QString refrigerant = refr_man.value("refrigerant").toString();
        QVariant purchased = refr_man.value("purchased");
        QVariant purchased_reco = refr_man.value("purchased_reco");
        QVariant sold = refr_man.value("sold");
        QVariant sold_reco = refr_man.value("sold_reco");
        QVariant refr_rege = refr_man.value("refr_rege");
        QVariant refr_disp = refr_man.value("refr_disp");
        QVariant leaked = refr_man.value("leaked");
        QVariant leaked_reco = refr_man.value("leaked_reco");
        QString date = refr_man.value("date").toString();
        QString uuid = refr_man.value("uuid").toString();
        int year = date.left(4).toInt();

        addToStore(store, store_years, year, refrigerant, purchased.toDouble() - sold.toDouble() - leaked.toDouble());
        addToStore(store_recovered, store_recovered_years, year, refrigerant, purchased_reco.toDouble() - sold_reco.toDouble() - refr_rege.toDouble() - refr_disp.toDouble() - leaked_reco.toDouble());
        addToStore(store_leaked, store_leaked_years, year, refrigerant, leaked.toDouble() + leaked_reco.toDouble());

        if (year < since) { continue; }

        QVector<QString> entries_list(ENTRIES::COUNT);
        entries_list[ENTRIES::LINK] = QString("refrigerantrecord:%1/edit").arg(uuid);
        entries_list[ENTRIES::COMPANY] = refr_man.value("partner").toString();
        entries_list[ENTRIES::COMPANY_ID] = refr_man.value("partner_id").toString();
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

        if (!sums_map.contains(QString::number(year))) {
            sums_map.insert(QString::number(year), NULL);
        }

        QString year_refrigerant = QString("%1::%2").arg(year).arg(refrigerant);
        if (!sums_map.contains(year_refrigerant)) {
            sum_list = new QVector<double>(SUMS::COUNT);
            sums_map.insert(year_refrigerant, sum_list);
        } else {
            sum_list = sums_map.value(year_refrigerant);
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

    MultiMapOfVariantMaps circuits(Circuit::query().mapAll("uuid", "refrigerant"));

    MTQuery inspections_query = Inspection::query();
    inspections_query.addJoin("LEFT JOIN customers ON inspections.customer_uuid = customers.uuid");
    if (by_field)
        inspections_query.addJoin("LEFT JOIN circuits ON inspections.circuit_uuid = circuits.uuid");
    QString fields = "inspections.customer_uuid, inspections.circuit_uuid, inspections.uuid, inspections.date, "
                     "inspections.nominal, inspections.refr_add_am, inspections.refr_reco, "
                     "customers.company, customers.id AS company_id";
    if (by_field)
        fields += ", circuits.field";
    ListOfVariantMaps inspections(inspections_query.listAll(fields));

    MTQuery repairs_query = Repair::query();
    repairs_query.addJoin("LEFT JOIN customers ON customer_uuid = customers.uuid");
    fields = "COALESCE(customers.company, repairs.customer) AS company, customers.id AS company_id, "
             "repairs.customer_uuid, repairs.uuid, repairs.date, "
             "repairs.refrigerant, repairs.refr_add_am, repairs.refr_reco";
    if (by_field)
        fields += ", repairs.field";
    inspections << repairs_query.listAll(fields);

    foreach (const QVariantMap &inspection, inspections) {
        QVariant refr_add_am = inspection.value("refr_add_am");
        QVariant refr_reco = inspection.value("refr_reco");
        if (refr_add_am.toDouble() == 0.0 &&
            refr_reco.toDouble() == 0.0) continue;

        QString uuid = inspection.value("uuid").toString();
        QString date = inspection.value("date").toString();
        int year = date.left(4).toInt();
        QString refrigerant;
        QVector<QString> entries_list(ENTRIES::COUNT);
        if (inspection.contains("circuit_uuid")) {
            entries_list[ENTRIES::LINK] = QString("customer:%1/circuit:%2/%3:%4")
                                        .arg(inspection.value("customer_uuid").toString())
                                        .arg(inspection.value("circuit_uuid").toString())
                                        .arg(inspection.value("nominal").toInt() ? "nominalinspection" : "inspection")
                                        .arg(uuid);
            refrigerant = circuits.value(inspection.value("circuit_uuid").toString()).value("refrigerant").toString();
        } else {
            entries_list[ENTRIES::LINK] = QString("repair:%1/edit").arg(uuid);
            refrigerant = inspection.value("refrigerant").toString();
        }

        addToStore(store, store_years, year, refrigerant, - refr_add_am.toDouble());
        addToStore(store_recovered, store_recovered_years, year, refrigerant, refr_reco.toDouble());

        if (year < since) { continue; }

        entries_list[ENTRIES::COMPANY] = inspection.value("company").toString();
        entries_list[ENTRIES::COMPANY_ID] = inspection.value("company_id").toString();

        QVariant new_charge = 0.0;
        if (inspection.contains("nominal") && inspection.value("nominal").toInt()) {
            new_charge = refr_add_am;
            refr_add_am = 0.0;
        }

        entries_list[ENTRIES::NEW_CHARGE] = new_charge.toString();
        entries_list[ENTRIES::REFR_ADD_AM] = refr_add_am.toString();
        entries_list[ENTRIES::REFR_RECO] = refr_reco.toString();

        if (by_field) {
            if (refrigerants_by_field.isEmpty() || refrigerants_by_field.contains(refrigerant))
                refrigerant += QString(":%1").arg(Global::fieldOfApplicationToId(inspection.value("field").toString()));
            else
                refrigerant += ":0";
        }

        entries_list[ENTRIES::REFRIGERANT] = refrigerant;
        entries_map.insert(date, entries_list);

        if (!sums_map.contains(QString::number(year))) {
            sums_map.insert(QString::number(year), NULL);
        }

        QString year_refrigerant = QString("%1::%2").arg(year).arg(refrigerant);
        if (!sums_map.contains(year_refrigerant)) {
            sum_list = new QVector<double>(SUMS::COUNT);
            sums_map.insert(year_refrigerant, sum_list);
        } else {
            sum_list = sums_map.value(year_refrigerant);
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
