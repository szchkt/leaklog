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

#ifndef WARNINGS_H
#define WARNINGS_H

#include "expression.h"
#include "mtsqlqueryresult.h"

class Warnings : public MTSqlQueryResultBase<QString>
{
public:
    Warnings(bool CO2_equivalent = true, bool = false, QVariantMap = QVariantMap(), int = 0);

    int warningConditionValueInsCount(const QString &uuid);
    Expression warningConditionValueIns(const QString &uuid, int i);
    int warningConditionValueNomCount(const QString &uuid);
    Expression warningConditionValueNom(const QString &uuid, int i);
    int warningConditionFunctionCount(const QString &uuid);
    QString warningConditionFunction(const QString &uuid, int i);

    static void initWarnings(ListOfVariantMaps *, int, const QString &id = QString(), bool = false, bool = false, int = 0);

    static int circuitInspectionInterval(const QString &refrigerant, double refrigerant_amount, bool CO2_equivalent, bool hermetic, bool leak_detector, int interval = 0);

protected:
    void saveResult();

    static void initWarning(ListOfVariantMaps *, const QString &, const QString &, const QString &, int, bool);
    static void initFilter(ListOfVariantMaps *, const QString &, const QString &, const QString &, const QString &);
    static void initCondition(ListOfVariantMaps *, const QString &, const QString &, const QString &, const QString &);

    static QString tr(const char *);

    bool CO2_equivalent;
    bool enabled_only;
    int m_scope;
    QMap<QString, QList<Expression> > conditions_value_ins;
    QMap<QString, QList<Expression> > conditions_value_nom;
    QMap<QString, QStringList> conditions_functions;
};

class Warning : public MTSqlQueryResultBase<QString>
{
public:
    Warning(const QString &uuid, bool CO2_equivalent = true);

protected:
    void saveResult();

    QString uuid;
    bool CO2_equivalent;
};

class WarningFilters : public MTSqlQueryResultBase<QString>
{
public:
    WarningFilters(const QString &warning_uuid, bool CO2_equivalent = true);

protected:
    void saveResult();

    QString warning_uuid;
    bool CO2_equivalent;
};

class WarningConditions : public MTSqlQueryResultBase<QString>
{
public:
    WarningConditions(const QString &warning_uuid, bool CO2_equivalent = true);

protected:
    void saveResult();

    QString warning_uuid;
    bool CO2_equivalent;
};

#endif // WARNINGS_H
