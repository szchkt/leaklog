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

#ifndef VARIABLES_H
#define VARIABLES_H

#include "mtsqlqueryresult.h"

class Variables : public MTSqlQueryResult
{
    Q_OBJECT

public:
    Variables(QSqlDatabase = QSqlDatabase(), bool = true);

protected:
    virtual void saveResult();

    void initVariables(const QString & = QString());
    void initVariable(const QString &, const QString &, const QString &, const QString &, bool, double, const QString &);
    void initVariable(const QString &, const QString &, const QString &);
    void initSubvariable(const QString &, const QString &, const QString &, const QString &, const QString &, const QString &, bool, double);

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

#endif // VARIABLES_H
