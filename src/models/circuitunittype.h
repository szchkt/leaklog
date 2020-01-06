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

#ifndef CIRCUITUNITTYPE_H
#define CIRCUITUNITTYPE_H

#include "dbrecord.h"

class CircuitUnitType : public DBRecord
{
    Q_OBJECT

public:
    enum Location {
        External = 0,
        Internal = 1
    };

    CircuitUnitType(const QString &uuid = QString());

    void initEditDialogue(EditDialogueWidgets *);
    bool checkValues(QWidget * = 0);
    static const QString locationToString(int);

    inline QString manufacturer() { return stringValue("manufacturer"); }
    inline QString type() { return stringValue("type"); }
    inline QString refrigerant() { return stringValue("refrigerant"); }
    inline double refrigerantAmount() { return doubleValue("refrigerant_amount"); }
    inline double acquisitionPrice() { return doubleValue("acquisition_price"); }
    inline double listPrice() { return doubleValue("list_price"); }
    inline Location location() { return (Location)intValue("location"); }
    inline QString unit() { return stringValue("unit"); }
    inline QString oil() { return stringValue("oil"); }
    inline double oilAmount() { return doubleValue("oil_amount"); }
    inline double output() { return doubleValue("output"); }
    inline QString outputUnit() { return stringValue("output_unit"); }
    inline double outputT0Tc() { return doubleValue("output_t0_tc"); }
    inline QString notes() { return stringValue("notes"); }
    inline double discount() { return doubleValue("discount"); }

    static QString tableName();
    static inline MTRecordQuery<CircuitUnitType> query(const QVariantMap &parents = QVariantMap()) { return MTRecordQuery<CircuitUnitType>(tableName(), parents); }
    static const ColumnList &columns();
    static const MTDictionary &attributes();
};

#endif // CIRCUITUNITTYPE_H
