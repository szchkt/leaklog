/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2015 Matus & Michal Tomlein

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

#ifndef WARNINGS_H
#define WARNINGS_H

#include "mtsqlqueryresult.h"

class Warnings : public MTSqlQueryResultBase<QString>
{
public:
    Warnings(QSqlDatabase = QSqlDatabase(), bool = false, const QVariantMap & = QVariantMap(), int = 0);

    int warningConditionValueInsCount(int);
    MTDictionary warningConditionValueIns(int, int);
    int warningConditionValueNomCount(int);
    MTDictionary warningConditionValueNom(int, int);
    int warningConditionFunctionCount(int);
    QString warningConditionFunction(int, int);

    static void initWarnings(QSqlDatabase, ListOfVariantMaps *, int, int = -1, bool = false, int = 0);

    static int circuitInspectionInterval(const QString &refrigerant, double refrigerant_amount, bool CO2_equivalent, bool hermetic, bool leak_detector, int interval = 0);

protected:
    void saveResult();

    static void initWarning(QSqlDatabase, ListOfVariantMaps *, const QString &, const QString &, const QString &, int, bool);
    static void initFilter(ListOfVariantMaps *, const QString &, const QString &, const QString &, const QString &);
    static void initCondition(ListOfVariantMaps *, const QString &, const QString &, const QString &, const QString &);

    static QString tr(const char *);

    QSqlDatabase database;
    bool enabled_only;
    int m_scope;
    QMap<int, QList<MTDictionary> > conditions_value_ins;
    QMap<int, QList<MTDictionary> > conditions_value_nom;
    QMap<int, QList<QString> > conditions_functions;
};

class Warning : public MTSqlQueryResultBase<QString>
{
public:
    Warning(int, QSqlDatabase = QSqlDatabase());

protected:
    void saveResult();

    QSqlDatabase database;
    int id;
};

class WarningFilters : public MTSqlQueryResultBase<QString>
{
public:
    WarningFilters(int, QSqlDatabase = QSqlDatabase());

protected:
    void saveResult();

    QSqlDatabase database;
    int id;
};

class WarningConditions : public MTSqlQueryResultBase<QString>
{
public:
    WarningConditions(int, QSqlDatabase = QSqlDatabase());

protected:
    void saveResult();

    QSqlDatabase database;
    int id;
};

#endif // WARNINGS_H
