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

#ifndef GLOBAL_H
#define GLOBAL_H

#include "defs.h"

class MTDictionary;
class QStringList;
class QColor;
class QSqlDatabase;
class QTextStream;
class QSqlError;

namespace Global {
    QString escapeString(QString, bool = false, bool = false);
    QString upArrow();
    QString downArrow();
    QString rightTriangle();
    QString degreeSign();
    QString delta();
    QColor textColourForBaseColour(const QColor &);
    QString variantTypeToSqlType(int);
    QString variableTypeToSqlType(const QString &);
    MTDictionary getTableFieldNames(const QString &, QSqlDatabase *);
    void copyTable(const QString &, QSqlDatabase *, QSqlDatabase *, const QString & = QString());
    void addColumn(const QString &, const QString &, QSqlDatabase *);
    void renameColumn(const QString &, const QString &, const QString &, QSqlDatabase *);
    void dropColumn(const QString &, const QString &, QSqlDatabase *);
    QString DBInfoValueForKey(const QString &);
    QSqlError setDBInfoValueForKey(const QString &, const QString &);
    double getCircuitRefrigerantAmount(const QString &, const QString &, double);
    extern QMap<QString, MTDictionary> parsed_expressions;
    MTDictionary parseExpression(const QString &, QStringList &);
    double evaluateExpression(QVariantMap &, const MTDictionary &, const QString &, const QString &, bool * = 0);
    QString compareValues(double, double, double = 0.0, const QString & = QString());
    QString toolTipLink(const QString &, const QString &, const QString &, const QString & = QString(), const QString & = QString(), bool = true);
    // Dictionaries
    const MTDictionary & databaseTables();
    const MTDictionary & variableTypes();
    const MTDictionary & variableNames();
    const QString variableType(const QString &, bool * = 0);
    const MTDictionary & fieldsOfApplication();
    const MTDictionary & oils();
    const MTDictionary & attributeValues();
    // List
    QString listRefrigerantsToString();
    MTDictionary listInspectors();
    MTDictionary listAssemblyRecordItemCategories();
    MTDictionary listAssemblyRecordTypes();
    MTDictionary listAllVariables();
    MTDictionary listDataTypes();
    MTDictionary listOperators(const QString &);
    QStringList listVariableIds(bool = false);
    QStringList listSupportedFunctions();
    // Data types
    enum DataTypes {
        String = 0,
        Integer = 1,
        Numeric = 2,
        Boolean = 3,
        Text = 4
    };
}

#endif // GLOBAL_H
