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

#ifndef ASSEMBLYRECORDITEM_H
#define ASSEMBLYRECORDITEM_H

#include "dbrecord.h"

class AssemblyRecordItem : public MTRecord
{
public:
    enum Source {
        AssemblyRecordItemTypes = 0,
        CircuitUnitTypes = 1,
        Inspectors = 2
    };

    AssemblyRecordItem(const QString &uuid = QString(), const QVariantMap &savedValues = QVariantMap());

    inline void setValue(const QString &field, const QVariant &value) { MTRecord::setValue(field, value); }

    inline QString itemTypeUUID() { return stringValue("ar_item_type_uuid"); }
    inline void setItemTypeUUID(const QString &value) { setValue("ar_item_type_uuid", value); }
    inline QString itemCategoryUUID() { return stringValue("ar_item_category_uuid"); }
    inline void setItemCategoryUUID(const QString &value) { setValue("ar_item_category_uuid", value); }
    inline QString arno() { return stringValue("arno"); }
    inline void setArno(const QString &value) { setValue("arno", value); }
    inline QString value() { return stringValue("value"); }
    inline void setValue(const QString &value) { setValue("value", value); }
    inline double acquisitionPrice() { return doubleValue("acquisition_price"); }
    inline void setAcquisitionPrice(double value) { setValue("acquisition_price", value); }
    inline double listPrice() { return doubleValue("list_price"); }
    inline void setListPrice(double value) { setValue("list_price", value); }
    inline Source source() { return (Source)intValue("source"); }
    inline void setSource(Source value) { setValue("source", value); }
    inline QString name() { return stringValue("name"); }
    inline void setName(const QString &value) { setValue("name", value); }
    inline QString unit() { return stringValue("unit"); }
    inline void setUnit(const QString &value) { setValue("unit", value); }
    inline double discount() { return doubleValue("discount"); }
    inline void setDiscount(double value) { setValue("discount", value); }

    static QString tableName();
    static inline MTRecordQuery<AssemblyRecordItem> query(const QVariantMap &parents = QVariantMap()) { return MTRecordQuery<AssemblyRecordItem>(tableName(), parents); }
    static MTQuery queryByInspector(const QString &inspector_uuid);
    static const ColumnList &columns();
    static const MTDictionary &attributes();
};

#endif // ASSEMBLYRECORDITEM_H
