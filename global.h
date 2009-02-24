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
#include "mtdictionary.h"
#include "refrigerants.h"

#include <QApplication>
#include <QVariant>
#include <QSet>
#include <QStringList>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QSqlField>
#include <QColor>
#include <QWebPage>
#include <QNetworkRequest>
#include <QTextStream>

#include <memory>
#include <cmath>

using std::auto_ptr;

#define StringVariantMap QMap<QString, QVariant>

#define ListOfStringVariantMaps QList<StringVariantMap>

#define MapOfStringVariantMaps QMap<QString, StringVariantMap>

#define MultiMapOfStringVariantMaps QMultiMap<QString, StringVariantMap>

#define ListOfStringVariantMapsPtr auto_ptr<ListOfStringVariantMaps>

#define MultiMapOfStringVariantMapsPtr auto_ptr<MultiMapOfStringVariantMaps>

#define LEAKLOG_VERSION "0.9.3"
#define F_LEAKLOG_VERSION 0.903
#define DB_VERSION "0.9.3"
#define F_DB_VERSION 0.903

#define REAL_NUMBER_PRECISION 2
#define REAL_NUMBER_PRECISION_EXP 100.0

namespace Global {
    QString toString(const QVariant &);
    QString escapeString(QString);
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
    MTDictionary parseExpression(const QString &, QStringList *);
    double evaluateExpression(StringVariantMap &, const MTDictionary &, const QString &, const QString &, bool * = NULL);
    QString compareValues(double, double, double = 0.0, const QString & = QString());
    QString toolTipLink(const QString &, const QString &, const QString &, const QString & = QString(), const QString & = QString());
    // Dictionaries
    MTDictionary get_dict_dbtables();
    MTDictionary get_dict_vartypes();
    MTDictionary get_dict_varnames();
    MTDictionary get_dict_attrvalues();
    MTDictionary get_dict_attrnames();
    // List
    QString listRefrigerantsToString();
    QString listInspectorsToString();
    QStringList listVariableIds(bool = false);
}

class MTRecord : public QObject
{
    Q_OBJECT

public:
    MTRecord() {};
    MTRecord(const QString &, const QString &, const MTDictionary &);
    MTRecord(const MTRecord &);
    MTRecord & operator=(const MTRecord &);
    void setType(const QString & type) { r_type = type; };
    inline QString type() { return r_type; };
    inline QString id() { return r_id; };
    inline MTDictionary * parents() { return &r_parents; };
    bool exists();
    QSqlQuery select(const QString & = "*");
    StringVariantMap list(const QString & = "*");
    ListOfStringVariantMapsPtr listAll(const QString & = "*");
    MultiMapOfStringVariantMapsPtr mapAll(const QString &, const QString & = "*");
    bool update(const StringVariantMap &, bool = false);
    bool remove();

protected:
    QString tableForRecordType(const QString &);
    QString idFieldForRecordType(const QString &);

private:
    QString r_type;
    QString r_id;
    MTDictionary r_parents;
};

class MTSqlQueryResult : public QObject
{
    Q_OBJECT

public:
    MTSqlQueryResult(const QString &, QSqlDatabase = QSqlDatabase());
    MTSqlQueryResult(QSqlDatabase = QSqlDatabase());
    ~MTSqlQueryResult();

    void bindValue(const QString &, const QVariant &, QSql::ParamType = QSql::In);
    QVariant boundValue(const QString &) const;
    bool exec(const QString &);
    bool exec();
    bool next();
    bool prepare(const QString &);
    QSqlQuery * query();
    QSqlRecord record() const;
    QVariant value(int) const;
    QVariant value(const QString &) const;
    int count() const;

protected:
    int * pos();
    ListOfStringVariantMaps * result();
    virtual void saveResult();

private:
    QSqlQuery * _query;
    ListOfStringVariantMaps _result;
    int _pos;
};

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
    MTWebPage(QObject * parent = 0): QWebPage(parent) {};

protected:
    bool acceptNavigationRequest(QWebFrame *, const QNetworkRequest &, NavigationType);
};

class MTTextStream : public QTextStream
{
public:
    MTTextStream(QString * string, QIODevice::OpenMode openMode = QIODevice::ReadWrite): QTextStream(string, openMode) {};

    inline MTTextStream & operator<<(double f) {
        long double ld = (long double)f;
        if (round(ld) == ld) this->QTextStream::operator<<(f);
        else this->QTextStream::operator<<((double)(round(ld * REAL_NUMBER_PRECISION_EXP) / REAL_NUMBER_PRECISION_EXP));
        return *this;
    };
    inline MTTextStream & operator<<(const char * string) { this->QTextStream::operator<<(string); return *this; };
    inline MTTextStream & operator<<(const QString & string) { this->QTextStream::operator<<(string); return *this; };
};

#endif // GLOBAL_H
