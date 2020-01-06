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

#ifndef INSPECTIONIMAGE_H
#define INSPECTIONIMAGE_H

#include "dbrecord.h"

class Inspection;
class File;

class InspectionFile : public MTRecord
{
public:
    InspectionFile(const QString &uuid = QString(), const QVariantMap &savedValues = QVariantMap());

    inline QString inspectionUUID() { return stringValue("inspection_uuid"); }
    inline void setInspectionUUID(const QString &value) { setValue("inspection_uuid", value); }
    inline QString fileUUID() { return stringValue("file_uuid"); }
    inline void setFileUUID(const QString &value) { setValue("file_uuid", value); }
    inline QString description() { return stringValue("description"); }
    inline void setDescription(const QString &value) { setValue("description", value); }

    Inspection inspection();
    File file();

    static QString tableName();
    static inline MTRecordQuery<InspectionFile> query(const QVariantMap &parents = QVariantMap()) { return MTRecordQuery<InspectionFile>(tableName(), parents); }
    static const ColumnList &columns();
};

#endif // INSPECTIONIMAGE_H
