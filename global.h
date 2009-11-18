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

#include "fparser/fparser.hh"
#include "refrigerants.h"
#include "mtaddress.h"
#include "mtsqlqueryresult.h"

#include <QApplication>
#include <QVariant>
#include <QSet>
#include <QStringList>
#include <QColor>
#include <QWebPage>
#include <QNetworkRequest>
#include <QTextStream>
#include <QSyntaxHighlighter>
#include <QHash>
#include <QTextCharFormat>

#include <cmath>

#define LEAKLOG_VERSION "0.9.5"
#define F_LEAKLOG_VERSION 0.905
#define LEAKLOG_PREVIEW_VERSION 0
#define DB_VERSION "0.9.5"
#define F_DB_VERSION 0.905

#define REAL_NUMBER_PRECISION 2
#define REAL_NUMBER_PRECISION_EXP 100.0

namespace Global {
    QString toString(const QVariant &);
    QString escapeString(QString, bool = false, bool = false);
    QString upArrow();
    QString downArrow();
    QString degreeSign();
    QString delta();
    QColor textColourForBaseColour(const QColor &);
    QString variantTypeToSqlType(QVariant::Type);
    MTDictionary getTableFieldNames(const QString &, QSqlDatabase *);
    void copyTable(const QString &, QSqlDatabase *, QSqlDatabase *, const QString & = QString());
    void addColumn(const QString &, const QString &, QSqlDatabase *);
    void renameColumn(const QString &, const QString &, const QString &, QSqlDatabase *);
    void dropColumn(const QString &, const QString &, QSqlDatabase *);
    extern QMap<QString, MTDictionary> parsed_expressions;
    MTDictionary parseExpression(const QString &, QStringList &);
    double evaluateExpression(StringVariantMap &, const MTDictionary &, const QString &, const QString &, bool * = NULL);
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

class Variables : public MTSqlQueryResult
{
    Q_OBJECT

public:
    Variables(QSqlDatabase = QSqlDatabase(), bool = true);

protected:
    virtual void saveResult();

    void initVariables(const QString & = QString());
    void initVariable(const QString &, const QString &, const QString &, const QString &, const QString &, bool, double, const QString &);
    void initVariable(const QString &, const QString &, const QString &);
    void initSubvariable(const QString &, const QString &, const QString &, const QString &, const QString &, const QString &, const QString &, bool, double);

    MTDictionary dict_varnames;
    QMultiMap<QString, int> var_indices;
};

class Variable : public Variables
{
    Q_OBJECT

public:
    Variable(const QString & = QString(), QSqlDatabase = QSqlDatabase());

protected:
    void saveResult();

private:
    QString var_id;
};

class Subvariable : public Variables
{
    Q_OBJECT

public:
    Subvariable(const QString &, const QString & = QString(), QSqlDatabase = QSqlDatabase());

protected:
    void saveResult();

private:
    QString var_id;
};

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

    static void initWarnings(QSqlDatabase, ListOfStringVariantMaps *, int, int = -1, bool = false);

protected:
    void saveResult();

    static void initWarning(QSqlDatabase, ListOfStringVariantMaps *, const QString &, const QString &, const QString &, int, bool);
    static void initFilter(ListOfStringVariantMaps *, const QString &, const QString &, const QString &, const QString &);
    static void initCondition(ListOfStringVariantMaps *, const QString &, const QString &, const QString &, const QString &);

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

class MTWebPage : public QWebPage
{
    Q_OBJECT

public:
    MTWebPage(QObject * parent = 0): QWebPage(parent) {}

protected:
    bool acceptNavigationRequest(QWebFrame *, const QNetworkRequest &, NavigationType);
};

class MTVariant
{
public:
    enum Type { Default = 0, Address = 128 };
    MTVariant(const QVariant & v = QVariant(), Type t = Default): v_value(v), v_type(t) {}

    inline void setType(Type t) { v_type = t; }
    inline Type type() const { return v_type; }
    inline QVariant::Type variantType() const { return v_value.type(); }
    inline void setValue(const QVariant & v) { v_value = v; }
    inline QVariant value() const { return v_value; }

    inline QString toString() const {
        return v_value.toString();
    }
    QString toHtml() const {
        switch (v_type) {
            case Address: return MTAddress(v_value.toString()).toHtml(); break;
            case Default: break;
        }
        return Global::escapeString(v_value.toString());
    }

private:
    QVariant v_value;
    Type v_type;
};

class MTTextStream : public QTextStream
{
public:
    MTTextStream(QString * string, QIODevice::OpenMode openMode = QIODevice::ReadWrite): QTextStream(string, openMode) {}

    inline MTTextStream & operator<<(double f) {
        long double ld = (long double)f;
        if (round(ld) == ld) this->QTextStream::operator<<(f);
        else this->QTextStream::operator<<((double)(round(ld * REAL_NUMBER_PRECISION_EXP) / REAL_NUMBER_PRECISION_EXP));
        return *this;
    }
    inline MTTextStream & operator<<(const char * string) { this->QTextStream::operator<<(string); return *this; }
    inline MTTextStream & operator<<(const QString & string) { this->QTextStream::operator<<(string); return *this; }
    inline MTTextStream & operator<<(const MTVariant & variant) { this->QTextStream::operator<<(variant.toHtml()); return *this; }
};

class Highlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    Highlighter(QStringList, QTextDocument * = 0);

protected:
    void highlightBlock(const QString &);

private:
    struct HighlightingRule
    {
        QRegExp pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;
    QTextCharFormat keywordFormat;
};

#endif // GLOBAL_H
