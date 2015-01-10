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

#ifndef GLOBAL_H
#define GLOBAL_H

#include "defs.h"

#include <QSqlDatabase>

class MTDictionary;
class QStringList;
class QColor;
class QTextStream;

namespace Global {
#ifdef Q_OS_MAC
    int macVersion();
#endif
    double scaleFactor(bool refresh = false);
    QString escapeString(const QVariant &, bool = false, bool = false);
    QString escapeString(QString, bool = false, bool = false);
    QString elideRight(const QString &, int);
    QString upArrow();
    QString downArrow();
    QString rightTriangle();
    QString degreeSign();
    QString delta();
    QString longMonthName(int);
    QColor textColourForBaseColour(const QColor &);
    QString sqlStringForDatabaseType(QString, const QSqlDatabase & = QSqlDatabase::database());
    QString variantTypeToSqlType(int);
    QString variableTypeToSqlType(const QString &);
    MTDictionary getTableFieldNames(const QString &, const QSqlDatabase &);
    void copyTable(const QString &, const QSqlDatabase &, const QSqlDatabase &, const QString & = QString());
    void addColumn(const QString &, const QString &, const QSqlDatabase &);
    void renameColumn(const QString &, const QString &, const QString &, const QSqlDatabase &);
    void dropColumn(const QString &, const QString &, const QSqlDatabase &);
    QString currentUser(const QSqlDatabase & = QSqlDatabase::database());
    bool isDatabaseRemote(const QSqlDatabase & = QSqlDatabase::database());
    bool isOwnerPermissionApplicable(const QString &);
    QString circuitRefrigerantAmountQuery(const QString &return_as = "refrigerant_amount");
    extern QMap<QString, MTDictionary> parsed_expressions;
    MTDictionary parseExpression(const QString &, QStringList &);
    double evaluateExpression(const QVariantMap &, const MTDictionary &, const QString &, const QString &, bool * = 0, bool * = 0);
    double evaluateExpression(const QVariantMap &, const MTDictionary &, const QVariantMap &, bool * = 0, bool * = 0);
    QString compareValues(double, double, double = 0.0, const QString & = QString());
    enum ToolTipLinkItem {
        ToolTipLinkItemView = 1 << 0,
        ToolTipLinkItemEdit = 1 << 1,
        ToolTipLinkItemRemove = 1 << 2
    };
    QString toolTipLink(const QString &type, const QString &text, const QString &l1, const QString &l2 = QString(), const QString &l3 = QString(), int items = ToolTipLinkItemView | ToolTipLinkItemEdit);
    // Dictionaries
    const MTDictionary &databaseTables();
    const MTDictionary &variableTypes();
    const MTDictionary &variableNames();
    const QString variableType(const QString &, bool * = 0);
    const MTDictionary &fieldsOfApplication();
    int fieldOfApplicationToId(const QString &field);
    QString idToFieldOfApplication(int id);
    const MTDictionary &oils();
    const MTDictionary &attributeValues();
    const MTDictionary &permissions();
    double refrigerantGWP(const QString &refrigerant);
    // List
    QString listRefrigerantsToString();
    MTDictionary listInspectors();
    MTDictionary listAssemblyRecordItemCategories(bool = false);
    MTDictionary listAssemblyRecordTypes();
    MTDictionary listAllVariables();
    MTDictionary listDataTypes();
    MTDictionary listOperators(const QString &);
    MTDictionary listStyles();
    QStringList listVariableIds(bool = false);
    QStringList listSupportedFunctions();
    // Data types
    enum DataTypes {
        String = 0,
        Integer = 1,
        Numeric = 2,
        Boolean = 3,
        Text = 4,
        File = 5
    };
}

#endif // GLOBAL_H
