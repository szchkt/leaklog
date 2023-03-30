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

#ifndef CUSTOMER_H
#define CUSTOMER_H

#include "dbrecord.h"

class MTAddress;
class Repair;
class Circuit;
class Person;

class Customer : public DBRecord
{
    Q_OBJECT

public:
    enum OperatorType {
        OperatorTypeServiceCompany = -1,
        OperatorTypeCustomer,
        OperatorTypeOther
    };

    Customer(const QString &uuid = QString());

    void initEditDialogue(EditDialogueWidgets *);
    void readOperatorValues();

    inline bool isStarred() { return intValue("starred"); }
    inline void setStarred(bool value) { setValue("starred", (int)value); }
    inline QString companyID() { return stringValue("id"); }
    inline void setCompanyID(const QString &value) { setValue("id", value); }
    inline QString companyName() { return stringValue("company"); }
    inline void setCompanyName(const QString &value) { setValue("company", value); }
    MTAddress address();
    inline QString mail() { return stringValue("mail"); }
    inline void setMail(const QString &value) { setValue("mail", value); }
    inline QString phone() { return stringValue("phone"); }
    inline void setPhone(const QString &value) { setValue("phone", value); }
    inline QString websiteURL() { return stringValue("website_url"); }
    inline void setWebsiteURL(const QString &value) { setValue("website_url", value); }
    inline QString mapsURL() { return stringValue("maps_url"); }
    inline void setMapsURL(const QString &value) { setValue("maps_url", value); }
    inline OperatorType operatorType() { return (OperatorType)intValue("operator_type"); }
    inline void setOperatorType(OperatorType value) { setValue("operator_type", value); }
    inline QString operatorCompanyID() { return stringValue("operator_id"); }
    inline void setOperatorCompanyID(const QString &value) { setValue("operator_id", value); }
    inline QString operatorCompanyName() { return stringValue("operator_company"); }
    inline void setOperatorCompanyName(const QString &value) { setValue("operator_company", value); }
    MTAddress operatorAddress();
    inline QString operatorMail() { return stringValue("operator_mail"); }
    inline void setOperatorMail(const QString &value) { setValue("operator_mail", value); }
    inline QString operatorPhone() { return stringValue("operator_phone"); }
    inline void setOperatorPhone(const QString &value) { setValue("operator_phone", value); }
    inline QString notes() { return stringValue("notes"); }
    inline void setNotes(const QString &value) { setValue("notes", value); }

    MTRecordQuery<Repair> repairs() const;
    MTRecordQuery<Circuit> circuits() const;
    MTRecordQuery<Person> persons() const;

    static QString tableName();
    static inline MTRecordQuery<Customer> query(const QVariantMap &parents = QVariantMap()) { return MTRecordQuery<Customer>(tableName(), parents); }
    static const ColumnList &columns();
    static const MTDictionary &attributes();
    bool remove() const;
};

#endif // CUSTOMER_H
