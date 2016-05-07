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

#ifndef ASSEMBLYRECORDITEMTYPE_H
#define ASSEMBLYRECORDITEMTYPE_H

#include "dbrecord.h"
#include "global.h"

class AssemblyRecordItemType : public DBRecord
{
    Q_OBJECT

public:
    AssemblyRecordItemType(const QString &uuid = QString());

    void initEditDialogue(EditDialogueWidgets *);

    QString name();
    double acquisitionPrice();
    double listPrice();
    int ean();
    QString unit();
    QString inspectionVariableID();
    Global::DataType valueDataType();
    double discount();
    bool autoShow();

    static QString tableName();
    static inline MTRecordQuery<AssemblyRecordItemType> query(const MTDictionary &parents = MTDictionary()) { return MTRecordQuery<AssemblyRecordItemType>(tableName(), parents); }
    static const ColumnList &columns();
    static const MTDictionary &attributes();
};

#endif // ASSEMBLYRECORDITEMTYPE_H
