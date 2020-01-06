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

#ifndef INSPECTOR_H
#define INSPECTOR_H

#include "dbrecord.h"

class Inspector : public DBRecord
{
    Q_OBJECT

public:
    Inspector(const QString &uuid = QString());

    void initEditDialogue(EditDialogueWidgets *);

    QString certificateNumber();
    QString certificateCountry();
    QString personName();
    QString mail();
    QString phone();
    double listPrice();
    double acquisitionPrice();

    static QString tableName();
    static inline MTRecordQuery<Inspector> query(const QVariantMap &parents = QVariantMap()) { return MTRecordQuery<Inspector>(tableName(), parents); }
    static const ColumnList &columns();
    static const MTDictionary &attributes();
};

#endif // INSPECTOR_H
