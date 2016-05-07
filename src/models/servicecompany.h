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

#ifndef SERVICECOMPANY_H
#define SERVICECOMPANY_H

#include "dbrecord.h"

class MTAddress;

class ServiceCompany : public DBRecord
{
    Q_OBJECT

public:
    ServiceCompany(const QString &uuid);

    void initEditDialogue(EditDialogueWidgets *);

    inline QString imageFileUUID() { return stringValue("image_file_uuid"); }
    inline void setImageFileUUID(const QString &value) { setValue("image_file_uuid", value); }
    inline QString companyID() { return stringValue("id"); }
    inline void setCompanyID(const QString &value) { setValue("id", value); }
    inline QString name() { return stringValue("name"); }
    inline void setName(const QString &value) { setValue("name", value); }
    MTAddress address();
    void setAddress(const MTAddress &value);
    inline void setAddress(const QString &value) { setValue("address", value); }
    inline QString mail() { return stringValue("mail"); }
    inline void setMail(const QString &value) { setValue("mail", value); }
    inline QString phone() { return stringValue("phone"); }
    inline void setPhone(const QString &value) { setValue("phone", value); }
    inline QString website() { return stringValue("website"); }
    inline void setWebsite(const QString &value) { setValue("website", value); }

    static QString tableName();
    static inline MTRecordQuery<ServiceCompany> query(const MTDictionary &parents = MTDictionary()) { return MTRecordQuery<ServiceCompany>(tableName(), parents); }
    static const ColumnList &columns();
    static const MTDictionary &attributes();
};

#endif // SERVICECOMPANY_H
