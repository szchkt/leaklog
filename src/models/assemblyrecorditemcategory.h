/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2017 Matus & Michal Tomlein

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

#ifndef ASSEMBLYRECORDITEMCATEGORY_H
#define ASSEMBLYRECORDITEMCATEGORY_H

#include "dbrecord.h"

class AssemblyRecordTypeCategory;

class AssemblyRecordItemCategory : public DBRecord
{
    Q_OBJECT

public:
    enum DisplayOptions {
        ShowValue = 1,
        ShowListPrice = 2,
        ShowAcquisitionPrice = 4,
        ShowDiscount = 8,
        ShowTotal = 16
    };
    enum DisplayPosition {
        DisplayAtTop = 0,
        DisplayAtBottom = 1
    };

    AssemblyRecordItemCategory(const QString &uuid = QString());

    void initEditDialogue(EditDialogueWidgets *);

    bool isPredefined();
    QString name();
    DisplayOptions displayOptions();
    DisplayPosition displayPosition();

    MTRecordQuery<AssemblyRecordTypeCategory> typeCategories();

    static QString tableName();
    static inline MTRecordQuery<AssemblyRecordItemCategory> query(const MTDictionary &parents = MTDictionary()) { return MTRecordQuery<AssemblyRecordItemCategory>(tableName(), parents); }
    static const ColumnList &columns();
    static const MTDictionary &attributes();
};

#endif // ASSEMBLYRECORDITEMCATEGORY_H
