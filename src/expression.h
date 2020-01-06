/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2020 Matus & Michal Tomlein

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

#ifndef EXPRESSION_H
#define EXPRESSION_H

#include "fparser/fparser.hh"

#include <QSharedPointer>
#include <QVariantMap>

class Expression
{
public:
    Expression(const QString &expression);
    virtual ~Expression();

    inline bool isValid() const { return var_names != NULL; }

    double evaluate(const QVariantMap &inspection, const QString &customer_uuid, const QString &circuit_uuid, bool *ok = NULL, bool *null_var = NULL) const;
    double evaluate(const QVariantMap &inspection, const QVariantMap &circuit_attributes, bool *ok = NULL, bool *null_var = NULL) const;

private:
    QSharedPointer<FunctionParser> fparser;
    QSharedPointer<QStringList> var_names;
};

#endif // EXPRESSION_H
