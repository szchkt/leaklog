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

#ifndef LEAKAGES_BY_APPLICATION_H
#define LEAKAGES_BY_APPLICATION_H

#include "mtdictionary.h"

#include <QObject>
#include <QMap>
#include <QVector>
#include <QStringList>

class LeakagesByApplication : QObject
{
    Q_OBJECT

public:
    class Key {
    public:
        static const QString All;

        explicit Key(const QString & refrigerant = All, const QString & field = All):
            year(0), refrigerant(refrigerant), field(field)
        {}
        explicit Key(int year, const QString & refrigerant, const QString & field):
            year(year), refrigerant(refrigerant), field(field)
        {}

        int year;
        QString refrigerant;
        QString field;

        inline bool operator<(const Key & other) const {
            return (year < other.year || (year == other.year && (refrigerant < other.refrigerant || (refrigerant == other.refrigerant && field < other.field))));
        }
    };

    enum Table {
        RefrigerantAddition,
        RefrigerantAmount,
        PercentageOfLeakage,
        TableCount
    };

    LeakagesByApplication(bool total);

    inline int startYear() const { return min_year; }
    inline int endYear() const { return max_year; }
    inline QStringList tableNames() const { return tables; }
    inline MTDictionary usedRefrigerants() const { return used_refrigerants; }

    inline QVector<double> value(const QString & refrigerant = Key::All, const QString & field = Key::All) const {
        return values.value(Key(refrigerant, field), QVector<double>(TableCount));
    }
    inline QVector<double> value(int year, const QString & refrigerant, const QString & field) const {
        return values.value(Key(year, refrigerant, field), QVector<double>(TableCount));
    }

protected:
    void addToValues(const Key & key, Table table, double value);

    int min_year;
    int max_year;
    QStringList tables;
    QMap<Key, QVector<double> > values;
    MTDictionary used_refrigerants;
};

#endif // LEAKAGES_BY_APPLICATION_H
