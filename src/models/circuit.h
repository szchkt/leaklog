/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2021 Matus & Michal Tomlein

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
    enum Status {
        ExcludedFromAgenda = -1,
        Commissioned = 0,
        Decommissioned = 1
    };

    Circuit(const QString &uuid = QString(), const QVariantMap &savedValues = QVariantMap());
    Circuit(const Circuit &other): DBRecord(other) {}
    Circuit &operator=(const Circuit &other) { DBRecord::operator=(other); return *this; };

    void initEditDialogue(EditDialogueWidgets *);
    bool checkValues(QWidget * = 0);

    Customer customer();

    inline QString customerUUID() { return stringValue("customer_uuid"); }
    inline void setCustomerUUID(const QString &value) { setValue("customer_uuid", value); }
    inline QString serviceCompanyUUID() { return stringValue("service_company_uuid"); }
    inline void setServiceCompanyUUID(const QString &value) { setValue("service_company_uuid", value); }
    inline QString externalUUID() { return stringValue("external_uuid"); }
    inline void setExternalUUID(const QString &value) { setValue("external_uuid", value); }
    inline bool isStarred() { return intValue("starred"); }
    inline void setStarred(bool value) { setValue("starred", (int)value); }
    inline QString circuitID() { return stringValue("id").rightJustified(5, '0'); }
    inline void setCircuitID(const QString &value) { setValue("id", value.toInt()); }
    inline void setCircuitID(int value) { setValue("id", value); }
    inline QString circuitName() { return stringValue("name"); }
    inline void setCircuitName(const QString &value) { setValue("name", value); }
    inline Status status() { return (Status)intValue("disused"); }
    inline void setStatus(Status value) { setValue("disused", value); }
    inline QString placeOfOperation() { return stringValue("operation"); }
    inline void setPlaceOfOperation(const QString &value) { setValue("operation", value); }
    inline QString building() { return stringValue("building"); }
    inline void setBuilding(const QString &value) { setValue("building", value); }
    inline QString device() { return stringValue("device"); }
    inline void setDevice(const QString &value) { setValue("device", value); }
    inline bool hermetic() { return intValue("hermetic"); }
    inline void setHermetic(bool value) { setValue("hermetic", (int)value); }
    inline QString manufacturer() { return stringValue("manufacturer"); }
    inline void setManufacturer(const QString &value) { setValue("manufacturer", value); }
    inline QString type() { return stringValue("type"); }
    inline void setType(const QString &value) { setValue("type", value); }
    inline QString serialNumber() { return stringValue("sn"); }
    inline void setSerialNumber(const QString &value) { setValue("sn", value); }
    inline int year() { return intValue("year"); }
    inline void setYear(int value) { setValue("year", value); }
    inline QString dateOfCommissioning() { return stringValue("commissioning"); }
    inline void setDateOfCommissioning(const QString &value) { setValue("commissioning", value); }
    inline QString dateOfDecommissioning() { return stringValue("decommissioning"); }
    inline void setDateOfDecommissioning(const QString &value) { setValue("decommissioning", value); }
    inline QString reasonForDecommissioning() { return stringValue("decommissioning_reason"); }
    inline void setReasonForDecommissioning(const QString &value) { setValue("decommissioning_reason", value); }
    inline QString field() { return stringValue("field"); }
    inline void setField(const QString &value) { setValue("field", value); }
    inline QString refrigerant() { return stringValue("refrigerant"); }
    inline void setRefrigerant(const QString &value) { setValue("refrigerant", value); }
    inline double refrigerantAmount() { return doubleValue("refrigerant_amount"); }
    inline void setRefrigerantAmount(double value) { setValue("refrigerant_amount", value); }
    inline QString oil() { return stringValue("oil"); }
    inline void setOil(const QString &value) { setValue("oil", value); }
    inline double oilAmount() { return doubleValue("oil_amount"); }
    inline void setOilAmount(double value) { setValue("oil_amount", value); }
    inline bool leakDetectorInstalled() { return intValue("leak_detector"); }
    inline void setLeakDetectorInstalled(bool value) { setValue("leak_detector", (int)value); }
    inline double runtime() { return doubleValue("runtime"); }
    inline void setRuntime(double value) { setValue("runtime", value); }
    inline double utilisation() { return doubleValue("utilisation"); }
    inline void setUtilisation(double value) { setValue("utilisation", value); }
    inline int inspectionInterval() { return intValue("inspection_interval"); }
    inline void setInspectionInterval(int value) { setValue("inspection_interval", value); }
    inline QString notes() { return stringValue("notes"); }
    inline void setNotes(const QString &value) { setValue("notes", value); }
    inline QString operatorLink() { return stringValue("operator_link"); }
    inline void setOperatorLink(const QString &value) { setValue("operator_link", value); }

    MTRecordQuery<Compressor> compressors() const;
    MTRecordQuery<CircuitUnit> units() const;
    MTRecordQuery<Inspection> inspections() const;

    static QString tableName();
    static inline MTRecordQuery<Circuit> query(const QVariantMap &parents = QVariantMap()) { return MTRecordQuery<Circuit>(tableName(), parents); }
    static const ColumnList &columns();
    static const MTDictionary &attributes();
    bool remove() const;
};

#endif // CIRCUIT_H
