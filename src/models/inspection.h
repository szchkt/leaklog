/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2023 Matus & Michal Tomlein

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

#ifndef INSPECTION_H
#define INSPECTION_H

#include "dbrecord.h"

class MDComboBox;
class Customer;
class Circuit;
class InspectionCompressor;
class InspectionFile;

class Inspection : public DBRecord
{
    Q_OBJECT

public:
    enum Type {
        RegularInspection = 0,
        NominalInspection = 1,
        Repair = 2,
        InspectionAfterRepair = 3,

        StrengthTightnessTest = 4,
        VacuumTest = 5,

        CircuitMoved = -1,
        SkippedInspection = -2,
    };

    static QString titleForInspectionType(Type type);
    static bool showDescriptionForInspectionType(Type type);
    static QString descriptionForInspectionType(Type type, const QString &type_data);

    static QString tableName();
    static inline MTRecordQuery<Inspection> query(const QVariantMap &parents = QVariantMap()) { return MTRecordQuery<Inspection>(tableName(), parents); }
    static MTQuery queryByInspector(const QString &inspector_uuid);
    static const ColumnList &columns();

    Inspection(const QString &uuid = QString(), const QVariantMap &savedValues = QVariantMap());

    void initEditDialogue(EditDialogueWidgets *);

    int scope() { return m_scope; }

    inline QString customerUUID() { return stringValue("customer_uuid"); }
    inline void setCustomerUUID(const QString &value) { setValue("customer_uuid", value); }
    Customer customer();
    inline QString circuitUUID() { return stringValue("circuit_uuid"); }
    inline void setCircuitUUID(const QString &value) { setValue("circuit_uuid", value); }
    Circuit circuit();
    inline QString date() { return stringValue("date"); }
    inline void setDate(const QString &value) { setValue("date", value); }
    inline bool isNominal() { return intValue("inspection_type") == NominalInspection; }
    inline bool isRepair() { return intValue("inspection_type") == Repair; }
    inline bool isOutsideInterval() { return intValue("outside_interval"); }
    inline void setOutsideInterval(bool value) { setValue("outside_interval", (int)value); }
    inline Type type() { return (Type)intValue("inspection_type"); }
    inline void setType(Type value) { setValue("inspection_type", (int)value); }
    inline QString typeData() { return stringValue("inspection_type_data"); }
    inline void setTypeData(const QString &value) { setValue("inspection_type_data", value); }
    QJsonObject data();
    void setData(const QJsonObject &value);

    MTRecordQuery<InspectionCompressor> compressors() const;
    MTRecordQuery<InspectionFile> files() const;

    bool remove() const;

public slots:
    void showSecondNominalInspectionWarning(MDComboBox *, int);

private:
    int m_scope;
};

#endif // INSPECTION_H
