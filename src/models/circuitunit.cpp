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

#include "circuitunit.h"

#include "circuit.h"
#include "circuitunittype.h"

CircuitUnit::CircuitUnit(const QString &uuid, const QVariantMap &savedValues):
    MTRecord(tableName(), "uuid", uuid, savedValues)
{}

Circuit CircuitUnit::circuit()
{
    return circuitUUID();
}

CircuitUnitType CircuitUnit::unitType()
{
    return unitTypeUUID();
}

QString CircuitUnit::tableName()
{
    return "circuit_units";
}

class CircuitUnitColumns
{
public:
    CircuitUnitColumns() {
        columns << Column("uuid", "UUID PRIMARY KEY");
        columns << Column("circuit_uuid", "UUID");
        columns << Column("unit_type_uuid", "UUID");
        columns << Column("sn", "TEXT");
        columns << Column("date_updated", "TEXT");
        columns << Column("updated_by", "TEXT");
    }

    ColumnList columns;
};

const ColumnList &CircuitUnit::columns()
{
    static CircuitUnitColumns columns;
    return columns.columns;
}
