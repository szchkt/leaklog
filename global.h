/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2009 Matus & Michal Tomlein

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
class QString;
class QStringList;
class QVariant;
template<class Key, class T>
class QMap;
class QColor;
class QSqlDatabase;
class QTextStream;

namespace Global {
    QString escapeString(QString, bool = false, bool = false);
    QString upArrow();
    QString downArrow();
    QString degreeSign();
    QString delta();
    QColor textColourForBaseColour(const QColor &);
    QString variantTypeToSqlType(int);
    MTDictionary getTableFieldNames(const QString &, QSqlDatabase *);
    void copyTable(const QString &, QSqlDatabase *, QSqlDatabase *, const QString & = QString());
    void addColumn(const QString &, const QString &, QSqlDatabase *);
    void renameColumn(const QString &, const QString &, const QString &, QSqlDatabase *);
    void dropColumn(const QString &, const QString &, QSqlDatabase *);
    extern QMap<QString, MTDictionary> parsed_expressions;
    MTDictionary parseExpression(const QString &, QStringList &);
    double evaluateExpression(StringVariantMap &, const MTDictionary &, const QString &, const QString &, bool * = 0);
    QString compareValues(double, double, double = 0.0, const QString & = QString());
    QString toolTipLink(const QString &, const QString &, const QString &, const QString & = QString(), const QString & = QString(), bool = true);
    // Dictionaries
    MTDictionary get_dict_dbtables();
    MTDictionary get_dict_vartypes();
    MTDictionary get_dict_varnames();
    MTDictionary get_dict_fields();
    MTDictionary get_dict_oils();
    MTDictionary get_dict_attrvalues();
    MTDictionary get_dict_attrnames();
    // List
    QString listRefrigerantsToString();
    MTDictionary listInspectors();
    QStringList listVariableIds(bool = false);
    QStringList listSupportedFunctions();
}

#endif // GLOBAL_H
