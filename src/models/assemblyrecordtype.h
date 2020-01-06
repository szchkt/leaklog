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

#ifndef ASSEMBLYRECORDTYPE_H
#define ASSEMBLYRECORDTYPE_H

#include "dbrecord.h"

class AssemblyRecordTypeCategory;
class Style;

class AssemblyRecordType : public DBRecord
{
    Q_OBJECT

public:
    enum DisplayOptions {
        ShowServiceCompany = 1,
        ShowCustomer = 2,
        ShowCustomerContactPersons = 4,
        ShowCircuit = 8,
        ShowCompressors = 16,
        ShowCircuitUnits = 32
    };

    AssemblyRecordType(const QString &uuid = QString());

    void initEditDialogue(EditDialogueWidgets *);

    QString name();
    QString description();
    DisplayOptions displayOptions();
    QString nameFormat();
    Style style();

    MTRecordQuery<AssemblyRecordTypeCategory> typeCategories() const;

    static QString tableName();
    static inline MTRecordQuery<AssemblyRecordType> query(const QVariantMap &parents = QVariantMap()) { return MTRecordQuery<AssemblyRecordType>(tableName(), parents); }
    static const ColumnList &columns();
    static const MTDictionary &attributes();
    bool remove() const;
};

#endif // ASSEMBLYRECORDTYPE_H
