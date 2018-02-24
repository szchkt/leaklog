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

struct CircuitsColumns
{
    int date_updated : 1;
    int owner : 1;
    int notes : 1;
    int operation : 1;
    int building : 1;
    int device : 1;
    int manufacturer : 1;
    int type : 1;
    int sn : 1;
    int year : 1;
    int commissioning : 1;
    int field : 1;
    int oil : 1;
};

class CircuitsView : public CustomersView
{
    Q_OBJECT

public:
    CircuitsView(ViewTabSettings *settings);

    QString renderHTML(bool for_export = false);

    QString title() const;

protected:
    void writeCircuitsTable(MTTextStream &out, const QString &customer_uuid, const QString &circuit_uuid = QString(), int cols_in_row = -1);
    HTMLDiv *writeCircuitsTable(const QString &customer_uuid, const QString &circuit_uuid = QString(), int cols_in_row = -1, HTMLTable *table = NULL);
    void writeCircuitsHeader(const QString &customer_uuid, const QString &circuit_uuid, int cols_in_row, CircuitsColumns columns, bool disused, HTMLTableRow *thead);
    void writeCircuitRow(const QVariantMap &circuit, const QString &customer_uuid, const QString &circuit_uuid, int cols_in_row, CircuitsColumns columns, HTMLTable *table);
    HTMLTable *circuitCompressorsTable(const QString &circuit_uuid, HTMLTable * = NULL);
    HTMLTable *circuitUnitsTable(const QString &circuit_uuid, HTMLTable * = NULL);
};

#endif // CIRCUITSVIEW_H
