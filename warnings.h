/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2010 Matus & Michal Tomlein

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

class Warnings : public MTSqlQueryResult
{
    Q_OBJECT

public:
    Warnings(QSqlDatabase = QSqlDatabase(), bool = false, const QString & = QString(), const QString & = QString());

    int warningConditionValueInsCount(int);
    MTDictionary warningConditionValueIns(int, int);
    int warningConditionValueNomCount(int);
    MTDictionary warningConditionValueNom(int, int);
    int warningConditionFunctionCount(int);
    QString warningConditionFunction(int, int);

    static void initWarnings(QSqlDatabase, ListOfVariantMaps *, int, int = -1, bool = false);

    static int circuitInspectionInterval(double, bool, bool, int = 0);

protected:
    void saveResult();

    static void initWarning(QSqlDatabase, ListOfVariantMaps *, const QString &, const QString &, const QString &, int, bool);
    static void initFilter(ListOfVariantMaps *, const QString &, const QString &, const QString &, const QString &);
    static void initCondition(ListOfVariantMaps *, const QString &, const QString &, const QString &, const QString &);

    QSqlDatabase database;
    bool enabled_only;
    QMap<int, QList<MTDictionary> > conditions_value_ins;
    QMap<int, QList<MTDictionary> > conditions_value_nom;
    QMap<int, QList<QString> > conditions_functions;
};

class Warning : public MTSqlQueryResult
{
    Q_OBJECT

public:
    Warning(int, QSqlDatabase = QSqlDatabase());

protected:
    void saveResult();

    QSqlDatabase database;
    int id;
};

class WarningFilters : public MTSqlQueryResult
{
    Q_OBJECT

public:
    WarningFilters(int, QSqlDatabase = QSqlDatabase());

protected:
    void saveResult();

    QSqlDatabase database;
    int id;
};

class WarningConditions : public MTSqlQueryResult
{
    Q_OBJECT

public:
    WarningConditions(int, QSqlDatabase = QSqlDatabase());

protected:
    void saveResult();

    QSqlDatabase database;
    int id;
};

#endif // WARNINGS_H
