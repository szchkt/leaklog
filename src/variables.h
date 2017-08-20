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

#ifndef VARIABLES_H
#define VARIABLES_H

#include "mtsqlqueryresult.h"

#include <QDateTime>

class EditDialogueWidgets;
class Inspection;
class MDCheckBox;

class VariableContractBase
{
public:
    enum Fields {
        ParentUUID,
        UUID,
        ID,
        Name,
        Type,
        Unit,
        ScopeValue,
        Value,
        CompareNom,
        Tolerance,
        ColBg,
        FieldCount
    };

    QString parentUUID() const {
        return variantValue(ParentUUID).toString();
    }

    QString uuid() const {
        return variantValue(UUID).toString();
    }

    QString id() const {
        return variantValue(ID).toString();
    }

    QString name() const {
        return variantValue(Name).toString();
    }

    QString type() const {
        return variantValue(Type).toString();
    }

    QString unit() const {
        return variantValue(Unit).toString();
    }

    int scope() const {
        return variantValue(ScopeValue).toInt();
    }

    QString valueExpression() const {
        return variantValue(Value).toString();
    }

    int compareNom() const {
        return variantValue(CompareNom).toInt();
    }

    double tolerance() const {
        return variantValue(Tolerance).toDouble();
    }

    QString colBg() const {
        return variantValue(ColBg).toString();
    }

protected:
    virtual QVariant variantValue(int) const = 0;
};

class VariableContract : public QMap<int, QVariant>, public VariableContractBase
{
public:
    VariableContract(): QMap<int, QVariant>() {}
    VariableContract(const QMap<int, QVariant> &other): QMap<int, QVariant>(other) {}

protected:
    QVariant variantValue(int i) const {
        return QMap<int, QVariant>::value(i);
    }
};

class Variables : public MTSqlQueryResultBase<int>, public VariableContractBase
{
public:
    Variables(QSqlDatabase = QSqlDatabase(), int = 0xFFFF);
    static Variables *defaultVariables(int = 0xFFFF);

    VariableContract variableForID(const QString &id);
    VariableContract variableForUUID(const QString &uuid);
    inline VariableContract parentVariable() {
        return variableForUUID(parentUUID());
    }

    void initEditDialogueWidgets(EditDialogueWidgets *, const QVariantMap &, Inspection * = NULL, const QDateTime & = QDateTime(), MDCheckBox * = NULL, MDCheckBox * = NULL);

protected:
    Variables(int);
    Variables(QSqlDatabase, const QString &, int = 0xFFFF);

    void saveResult();

    void initVariables();
    void initVariable(const QString &, int, const QString &, const QString &, bool, double, const QString &);
    void initVariable(const QString &, int, const QString &);
    void initSubvariable(const QString &, int, const QString &, const QString &, const QString &, const QString &, bool, double);

    QVariant variantValue(int i) const {
        return MTSqlQueryResultBase<int>::value(i);
    }

    QString ns;
    QMap<QString, int> var_indices;
    int m_scope;
    QString m_filter;
};

class Variable : public Variables
{
public:
    enum Scope {
        Inspection = 1,
        Compressor = 2
    };

    Variable(const QString & = QString(), QSqlDatabase = QSqlDatabase());

protected:
    void saveResult();
};

#endif // VARIABLES_H
