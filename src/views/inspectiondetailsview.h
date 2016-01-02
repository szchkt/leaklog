/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2015 Matus & Michal Tomlein

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

#ifndef INSPECTIONDETAILSVIEW_H
#define INSPECTIONDETAILSVIEW_H

#include "circuitsview.h"

#include <QVariantMap>

namespace VariableEvaluation {
class Variable;
class EvaluationContext;
}
class Warnings;

class InspectionDetailsView : public CircuitsView
{
    Q_OBJECT

public:
    InspectionDetailsView(ViewTabSettings *settings);

    QString renderHTML();

    QString title() const;

protected:
    void showVariableInInspectionTable(VariableEvaluation::Variable *, VariableEvaluation::EvaluationContext &, const QVariantMap &, HTMLTable *);
    QStringList listWarnings(Warnings &, const QVariantMap &, const QVariantMap &, const QVariantMap &);
    bool checkWarningConditions(Warnings &warnings, const QVariantMap &circuit_attributes, const QVariantMap &nominal_ins, const QVariantMap &inspection);
    QString tableVarValue(const QString &, const QString &, const QString &, const QString &, bool, double, bool = false);
};

#endif // INSPECTIONDETAILSVIEW_H
