/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2019 Matus & Michal Tomlein

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

#ifndef COMPRESSOR_H
#define COMPRESSOR_H

#include "dbrecord.h"

class Circuit;

class Compressor : public MTRecord
{
public:
    Compressor(const QString &uuid = QString(), const QVariantMap &savedValues = QVariantMap());

    inline QString circuitUUID() { return stringValue("circuit_uuid"); }
    inline void setCircuitUUID(const QString &value) { setValue("circuit_uuid", value); }
    Circuit circuit();
    inline QString name() { return stringValue("name"); }
    inline void setName(const QString &value) { setValue("name", value); }
    inline QString manufacturer() { return stringValue("manufacturer"); }
    inline void setManufacturer(const QString &value) { setValue("manufacturer", value); }
    inline QString type() { return stringValue("type"); }
    inline void setType(const QString &value) { setValue("type", value); }
    inline QString serialNumber() { return stringValue("sn"); }
    inline void setSerialNumber(const QString &value) { setValue("sn", value); }

    static QString tableName();
    static inline MTRecordQuery<Compressor> query(const QVariantMap &parents = QVariantMap()) { return MTRecordQuery<Compressor>(tableName(), parents); }
    static const ColumnList &columns();
    static const MTDictionary &attributes();
};

#endif // COMPRESSOR_H
