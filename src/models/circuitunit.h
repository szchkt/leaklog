/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2018 Matus & Michal Tomlein

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

#ifndef CIRCUITUNIT_H
#define CIRCUITUNIT_H

#include "dbrecord.h"

class Circuit;
class CircuitUnitType;

class CircuitUnit : public MTRecord
{
public:
    CircuitUnit(const QString &uuid = QString(), const QVariantMap &savedValues = QVariantMap());

    inline QString circuitUUID() { return stringValue("circuit_uuid"); }
    Circuit circuit();
    inline void setCircuitUUID(const QString &value) { setValue("circuit_uuid", value); }
    inline QString unitTypeUUID() { return stringValue("unit_type_uuid"); }
    CircuitUnitType unitType();
    inline void setUnitTypeUUID(const QString &value) { setValue("unit_type_uuid", value); }
    inline QString serialNumber() { return stringValue("sn"); }
    inline void setSerialNumber(const QString &value) { setValue("sn", value); }

    static QString tableName();
    static inline MTRecordQuery<CircuitUnit> query(const QVariantMap &parents = QVariantMap()) { return MTRecordQuery<CircuitUnit>(tableName(), parents); }
    static const ColumnList &columns();
};

#endif // CIRCUITUNIT_H
