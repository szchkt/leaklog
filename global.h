/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008 Matus & Michal Tomlein

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

#include <QApplication>
#include <QVariant>
#include <QSet>
#include <QStringList>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>

namespace Global {
    QString toString(const QVariant &);
    //QString escapeDoubleQuotes(const QString &);
    QString upArrow();
    QString downArrow();
    QString degreeSign();
    QString delta();
    void copyTable(const QString &, QSqlDatabase *, QSqlDatabase *, const QString & = QString());
    QStringList getTableFieldNames(const QString &, QSqlDatabase *);
    void addColumn(const QString &, const QString &, QSqlDatabase *);
    void renameColumn(const QString &, const QString &, const QString &, QSqlDatabase *);
    void dropColumn(const QString &, const QString &, QSqlDatabase *);
    MTDictionary parseExpression(const QString &, QStringList *);
    double evaluateExpression(QMap<QString, QVariant> &, const MTDictionary &, const QString &, const QString &, bool * = NULL);
    QString compareValues(double, double, double = 0.0);
    // Dictionaries
    MTDictionary get_dict_dbtables();
    MTDictionary get_dict_vartypes();
    MTDictionary get_dict_varnames();
    MTDictionary get_dict_attrvalues();
    MTDictionary get_dict_attrnames();
    // Variables
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
    QMap<QString, QVariant> list(const QString & = "*");
    QList<QMap<QString, QVariant> > listAll(const QString & = "*");
    bool update(const QMap<QString, QVariant> &, bool = false);
    bool remove();

protected:
    QString tableForRecordType(const QString &);

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

protected:
    int * pos();
    QList<QMap<QString, QVariant> > * result();
    virtual void saveResult();

private:
    QSqlQuery * _query;
    QList<QMap<QString, QVariant> > _result;
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
    QMap<QString, int> var_indices;
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

#endif // GLOBAL_H
