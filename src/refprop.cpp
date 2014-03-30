/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2014 Matus & Michal Tomlein

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

#include "refprop.h"

#ifdef REFPROP

#include <refprop_lib.h>

#include <cstring>

#include <QDir>
#include <QCoreApplication>
#include <QDebug>

static char *setString(char *destination, const char *source, size_t size)
{
    destination[0] = '\0';
    return strncat(destination, source, size);
}

static char hpth[filepathlength + 1];
static char herr[errormessagelength + 1];
static char hfiles[filepathlength * ncmax * 2 + 1];
static char hfmix[filepathlength + 1];
static char hmxnme[filepathlength + 1];
static DOUBLE_PRECISION x[ncmax];
static INTEGER ncc = 0;
static DOUBLE_PRECISION rhol = 0;
static DOUBLE_PRECISION rhov = 0;
static DOUBLE_PRECISION xliq[ncmax];
static DOUBLE_PRECISION xvap[ncmax];

static INTEGER setupForRefrigerant(const QString &refrigerant)
{
    static INTEGER ierr = 0;
    static QString current_refrigerant;

    if (current_refrigerant == refrigerant)
        return ierr;

    QDir resources(QCoreApplication::applicationDirPath());
#ifdef Q_OS_MAC
    resources.cdUp();
    resources.cd("Resources");
#endif

    if (current_refrigerant.isEmpty()) {
        setString(hpth, resources.absolutePath().toUtf8().constData(), filepathlength);
        SETPATHdll(hpth);

        setString(hfmix, "fluids/HMX.BNC", filepathlength);
    }

    ierr = 0;
    setString(herr, "", errormessagelength);

    char hrf[] = { 'D', 'E', 'F', '\0' };

    QString fluid_path = QString("fluids/%1.FLD").arg(refrigerant);
    QString mixture_path = QString("mixtures/%1.MIX").arg(refrigerant);

    if (resources.exists(fluid_path)) {
        setString(hfiles, fluid_path.toUtf8().constData(), filepathlength);

        INTEGER i = 1;

        SETUPdll(i, hfiles, hfmix, hrf, ierr, herr);
    } else if (resources.exists(mixture_path)) {
        setString(hmxnme, mixture_path.toUtf8().constData(), filepathlength);

        SETMIXdll(hmxnme, hfmix, hrf, ncc, hfiles, x, ierr, herr, filepathlength, filepathlength, 3, filepathlength * ncmax, errormessagelength);
    } else {
        ierr = -101;
    }

    if (ierr)
        qDebug() << herr;

    current_refrigerant = refrigerant;

    return ierr;
}

double RefProp::pressureToTemperature(const QString &refrigerant, double pressure, RefProp::Phase phase)
{
    if (setupForRefrigerant(refrigerant))
        return -273.15;

    INTEGER ierr = 0;
    setString(herr, "", errormessagelength);

    DOUBLE_PRECISION p = pressure * 100 + 100;
    DOUBLE_PRECISION t = 0;
    INTEGER kph = phase;

    SATPdll(p, x, kph, t, rhol, rhov, xliq, xvap, ierr, herr, errormessagelength);

    if (ierr)
        qDebug() << herr;

    return t - 273.15;
}

#else // !defined(REFPROP)

#include <QApplication>
#include <QDir>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>

#include <cmath>

class RefPropDatabase
{
public:
    double pressureToTemperature(const QString &refrigerant, double pressure, RefProp::Phase phase);

protected:
    QSqlDatabase _database;
    bool _open;
    QMap<QString, QPair<double, double> > _limits;
};

double RefPropDatabase::pressureToTemperature(const QString &refrigerant, double pressure, RefProp::Phase phase)
{
    if (!_open) {
        QDir resources(QApplication::applicationDirPath());
#ifdef Q_OS_MAC
        resources.cdUp();
        resources.cd("Resources");
#endif

        _database = QSqlDatabase::addDatabase("QSQLITE", "RefPropDatabase");
        _database.setDatabaseName(resources.absoluteFilePath("RefPropDatabase.sqlite"));
        _open = _database.open();

        if (!_open)
            return -273.15;

        QSqlQuery limits("SELECT refrigerant, MIN(pressure), MAX(pressure) FROM satp GROUP BY refrigerant", _database);
        while (limits.next()) {
            _limits.insert(limits.value(0).toString(), QPair<double, double>(limits.value(1).toDouble(), limits.value(2).toDouble()));
        }
    }

    if (_limits.isEmpty() || !_limits.contains(refrigerant))
        return -273.15;

    QPair<double, double> limit = _limits.value(refrigerant);
    if (pressure < limit.first)
        pressure = limit.first;
    else if (pressure > limit.second)
        pressure = limit.second;

    int column_index = phase - 1;

    QSqlQuery query(_database);
    query.prepare("SELECT temp_liq, temp_vap FROM satp WHERE refrigerant = :r AND pressure >= :p_min AND pressure <= :p_max");
    query.bindValue(":r", refrigerant);

    double pressure_rounded = round(pressure * 10.0) / 10.0;
    double pressure_min = pressure_rounded;
    double pressure_max = pressure_rounded;
    if (pressure_rounded != pressure) {
        if (pressure_rounded < pressure) {
            pressure_max += 0.1;
        } else {
            pressure_min -= 0.1;
        }
    }

    query.bindValue(":p_min", pressure_min - 0.001);
    query.bindValue(":p_max", pressure_max + 0.001);

    query.exec();

    double temp_min = 273.15;
    double temp_max = -273.15;

    int count = 0;
    while (query.next()) {
        count++;

        double temp = query.value(column_index).toDouble();

        if (temp_min > temp)
            temp_min = temp;
        if (temp_max < temp)
            temp_max = temp;
    }

    if (!count) {
        QSqlQuery query(_database);
        query.prepare("SELECT temp_liq, temp_vap, pressure FROM satp WHERE refrigerant = :r ORDER BY ABS(pressure - :p) LIMIT 2");
        query.bindValue(":r", refrigerant);
        query.bindValue(":p", pressure);

        query.exec();

        pressure_min = 1000.0;
        pressure_max = -1.0;

        while (query.next()) {
            count++;

            double p = query.value(2).toDouble();

            if (pressure_min > p)
                pressure_min = p;
            if (pressure_max < p)
                pressure_max = p;

            double temp = query.value(column_index).toDouble();

            if (temp_min > temp)
                temp_min = temp;
            if (temp_max < temp)
                temp_max = temp;
        }

        if (!count)
            return -273.15;
    }

    if (pressure_min == pressure_max)
        return temp_min;

    return (pressure - pressure_min) / (pressure_max - pressure_min) * (temp_max - temp_min) + temp_min;
}

static RefPropDatabase _refPropDatabase;

double RefProp::pressureToTemperature(const QString &refrigerant, double pressure, RefProp::Phase phase)
{
    return _refPropDatabase.pressureToTemperature(refrigerant, pressure, phase);
}

#endif
