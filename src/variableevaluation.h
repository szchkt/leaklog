/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2012 Matus & Michal Tomlein

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

#ifndef VARIABLE_PARSER_H
#define VARIABLE_PARSER_H

#include "defs.h"

#include <QVariantMap>
#include <QStringList>

namespace VariableEvaluation {

    class Variable;

    class EvaluationContext
    {
    public:
        EvaluationContext(int = 0);
        EvaluationContext(const QString &, const QString &, int = 0);
        ~EvaluationContext();

        void setNominalInspection(const QVariantMap & nominal_ins) { this->nominal_ins = nominal_ins; }
        QVariantMap & nominalInspection() { return nominal_ins; }

        Variable * variable(const QString & name) const { return vars_map.value(name); }
        QString evaluate(Variable *, QVariantMap &, QString &);
        QString evaluate(const QString &, QVariantMap &, QString &);

        QStringList & usedIDs() { return used_ids; }

        QList<Variable *> listVariables() const { return vars_list; }

        QString variableName(Variable *, bool = false) const;

    private:
        void init();

        QMap<QString, Variable *> vars_map;
        QList<Variable *> vars_list;
        QStringList used_ids;
        QString customer_id;
        QString circuit_id;
        QVariantMap circuit;
        QVariantMap nominal_ins;
        int vars_scope;
        MultiMapOfVariantMaps persons;
        MultiMapOfVariantMaps inspectors;

        friend class Variable;
    };

    class Variable
    {
    public:
        void setName(const QString & name) { _name = name; }
        QString name() const { return _name; }
        void setType(const QString & type) { _type = type; }
        QString type() const { return _type; }
        void setUnit(const QString & unit) { _unit = unit; }
        QString unit() const { return _unit; }
        void setCompareNom(int compare_nom) { _compare_nom = compare_nom; }
        int compareNom() const { return _compare_nom; }
        void setColBg(const QString & col_bg) { _col_bg = col_bg; }
        QString colBg() const { return _col_bg; }
        void setTolerance(double tolerance) { _tolerance = tolerance; }
        double tolerance() const { return _tolerance; }
        void setValue(const QString & value) { _value = value; }
        QString value() const { return _value; }
        void setID(const QString & id) { _id = id; }
        QString id() const { return _id; }
        void setParentID(const QString & parent_id) { _parent_id = parent_id; }
        QString parentID() const { return _parent_id; }

        void addSubvariable(Variable * var) { subvars.append(var); }
        QList<Variable *> subvariables() const { return subvars; }
        int countSubvariables() const { return subvars.count(); }

        QString evaluate(EvaluationContext &, QVariantMap &, QString &);

    private:
        QString _name;
        QString _type;
        QString _unit;
        int _compare_nom;
        QString _col_bg;
        double _tolerance;
        QString _value;
        QString _id;
        QString _parent_id;

        QList<Variable *> subvars;
    };
}

#endif // VARIABLE_PARSER_H
