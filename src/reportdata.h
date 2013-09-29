/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2013 Matus & Michal Tomlein

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

#ifndef REPORT_DATA_H
#define REPORT_DATA_H

#include <QObject>
#include <QMultiMap>
#include <QSet>

namespace ENTRIES {
    enum ENTRIES {
        LINK = 0, REFRIGERANT,
        PURCHASED, PURCHASED_RECO, SOLD, SOLD_RECO, NEW_CHARGE,
        REFR_ADD_AM, REFR_RECO, REFR_REGE, REFR_DISP,
        LEAKED, LEAKED_RECO, COUNT
    };
}

namespace SUMS {
    enum SUMS {
        PURCHASED = 0, PURCHASED_RECO, SOLD, SOLD_RECO, NEW_CHARGE,
        REFR_ADD_AM, REFR_RECO, REFR_REGE, REFR_DISP,
        LEAKED, LEAKED_RECO, COUNT
    };
}

namespace FIELD_IDS {
    enum FIELD_IDS {
        COMMERCIAL = 2,
        AC = 8,
        TRANSPORT = 32,
        HP = 64,
        INDUSTRIAL = 512,
        DOMESTIC = 2048
    };
}

class ReportData : public QObject
{
    Q_OBJECT

protected:
    void addToStore(QMap<int, QMap<QString, double> > &, QList<int> &, int, const QString &, double);

public:
    ReportData(int year = 0, bool by_field = false, const QSet<QString> &refrigerants_by_field = QSet<QString>());
    virtual ~ReportData();

    QMap<int, QMap<QString, double> > store; QList<int> store_years;
    QMap<int, QMap<QString, double> > store_recovered; QList<int> store_recovered_years;
    QMap<int, QMap<QString, double> > store_leaked; QList<int> store_leaked_years;
    QMultiMap<QString, QVector<QString> > entries_map;
    QMap<QString, QVector<double> *> sums_map;
};

#endif // REPORT_DATA_H
