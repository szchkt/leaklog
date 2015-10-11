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

#ifndef CIRCUIT_H
#define CIRCUIT_H

#include "dbrecord.h"

class Customer;
class Compressor;
class CircuitUnit;
class Inspection;

class Circuit : public DBRecord
{
    Q_OBJECT

public:
    Circuit(const QString &uuid = QString());
    Circuit(const MTDictionary &parents);
    Circuit(const Circuit &other): DBRecord(other) {}

    void initEditDialogue(EditDialogueWidgets *);
    bool checkValues(const QVariantMap &, QWidget * = 0);

    QString customerUUID();
    Customer customer();
    QString circuitID();
    QString circuitName();
    bool disused();
    QString placeOfOperation();
    QString building();
    QString device();
    bool hermetic();
    QString manufacturer();
    QString type();
    QString serialNumber();
    int year();
    QString dateOfCommissioning();
    QString dateOfDecommissioning();
    QString field();
    QString refrigerant();
    double refrigerantAmount();
    QString oil();
    double oilAmount();
    bool leakDetectorInstalled();
    double runtime();
    double utilisation();
    int inspectionInterval();

    Compressor compressors();
    CircuitUnit units();
    Inspection inspections();

    static QString tableName();
    static const ColumnList &columns();
    static const MTDictionary &attributes();
    static int numBasicAttributes();
};

#endif // CIRCUIT_H
