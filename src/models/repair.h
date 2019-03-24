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

#ifndef REPAIR_H
#define REPAIR_H

#include "dbrecord.h"

class Repair : public DBRecord
{
    Q_OBJECT

public:
    Repair(const QString &uuid = QString(), const QVariantMap &savedValues = QVariantMap());

    void initEditDialogue(EditDialogueWidgets *);
    bool checkValues(QWidget * = 0);

    inline QString customerUUID() { return stringValue("customer_uuid"); }
    inline void setCustomerUUID(const QString &value) { setValue("customer_uuid", value); }
    inline QString inspectorUUID() { return stringValue("inspector_uuid"); }
    inline void setInspectorUUID(const QString &value) { setValue("inspector_uuid", value); }
    inline QString date() { return stringValue("date"); }
    inline void setDate(const QString &value) { setValue("date", value); }
    inline QString customer() { return stringValue("customer"); }
    inline void setCustomer(const QString &value) { setValue("customer", value); }
    inline QString device() { return stringValue("device"); }
    inline void setDevice(const QString &value) { setValue("device", value); }
    inline QString field() { return stringValue("field"); }
    inline void setField(const QString &value) { setValue("field", value); }
    inline QString refrigerant() { return stringValue("refrigerant"); }
    inline void setRefrigerant(const QString &value) { setValue("refrigerant", value); }
    inline double refrigerantAmount() { return doubleValue("refrigerant_amount"); }
    inline void setRefrigerantAmount(double value) { setValue("refrigerant_amount", value); }
    inline double refrigerantAddition() { return doubleValue("refr_add_am"); }
    inline void setRefrigerantAddition(double value) { setValue("refr_add_am", value); }
    inline double refrigerantRecovery() { return doubleValue("refr_reco"); }
    inline void setRefrigerantRecovery(double value) { setValue("refr_reco", value); }
    inline QString arno() { return stringValue("arno"); }
    inline void setArno(const QString &value) { setValue("arno", value); }

    static QString tableName();
    static inline MTRecordQuery<Repair> query(const QVariantMap &parents = QVariantMap()) { return MTRecordQuery<Repair>(tableName(), parents); }
    static const ColumnList &columns();
    static const MTDictionary &attributes();
};

#endif // REPAIR_H
