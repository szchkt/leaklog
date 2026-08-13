/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2026 Matus & Michal Tomlein

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

#ifndef BUSINESSPARTNER_H
#define BUSINESSPARTNER_H

#include "dbrecord.h"

class BusinessPartner : public DBRecord
{
    Q_OBJECT

public:
    BusinessPartner(const QString &uuid = QString(), const QVariantMap &savedValues = QVariantMap());

    void initEditDialogue(EditDialogueWidgets *);

    inline QString name() { return stringValue("name"); }
    inline void setName(const QString &value) { setValue("name", value); }
    inline QString companyID() { return stringValue("company_id"); }
    inline void setCompanyID(const QString &value) { setValue("company_id", value); }
    inline QString companyVATIN() { return stringValue("company_vatin"); }
    inline void setCompanyVATIN(const QString &value) { setValue("company_vatin", value); }
    inline QString address() { return stringValue("address"); }
    inline void setAddress(const QString &value) { setValue("address", value); }
    inline QString mail() { return stringValue("mail"); }
    inline void setMail(const QString &value) { setValue("mail", value); }
    inline QString phone() { return stringValue("phone"); }
    inline void setPhone(const QString &value) { setValue("phone", value); }
    inline QString notes() { return stringValue("notes"); }
    inline void setNotes(const QString &value) { setValue("notes", value); }
    static QString tableName();
    static inline MTRecordQuery<BusinessPartner> query(const QVariantMap &parents = QVariantMap()) { return MTRecordQuery<BusinessPartner>(tableName(), parents); }
    static const ColumnList &columns();
    static const MTDictionary &attributes();
};

#endif // BUSINESSPARTNER_H
