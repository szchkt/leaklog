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

#ifndef CIRCUITSVIEW_H
#define CIRCUITSVIEW_H

#include "customersview.h"

#include <QVariantMap>

class HTMLDiv;
class HTMLTableRow;

class CircuitsView : public CustomersView
{
    Q_OBJECT

public:
    CircuitsView(ViewTabSettings *settings);

    QString renderHTML();

    QString title() const;

protected:
    void writeCircuitsTable(MTTextStream &out, const QString &customer_id, const QString &circuit_id = QString(), int cols_in_row = -1);
    HTMLDiv *writeCircuitsTable(const QString &customer_id, const QString &circuit_id = QString(), int cols_in_row = -1, HTMLTable *table = NULL);
    void writeCircuitsHeader(const QString &customer_id, const QString &circuit_id, int cols_in_row, bool show_date_updated, bool show_owner, bool disused, HTMLTableRow *thead);
    void writeCircuitRow(const QVariantMap &circuit, const QString &customer_id, const QString &circuit_id, int cols_in_row, bool show_date_updated, bool show_owner, HTMLTable *table);
    HTMLTable *circuitCompressorsTable(const QString &, const QString &, HTMLTable * = NULL);
    HTMLTable *circuitUnitsTable(const QString &, const QString &, HTMLTable * = NULL);
};

#endif // CIRCUITSVIEW_H
