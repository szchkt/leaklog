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

#include "refrigerants.h"

static Refrigerants refrigerants;

double RefProp::pressureToTemperature(const QString &refrigerant, double pressure, RefProp::Phase)
{
    return refrigerants.pressureToTemperature(refrigerant, round(pressure * 10.0) / 10.0 + 1.0);
}

#endif
