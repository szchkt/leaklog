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

#ifndef STYLE_H
#define STYLE_H

#include "dbrecord.h"

class Style : public DBRecord
{
    Q_OBJECT

public:
    Style(const QString &uuid = QString());

    void initEditDialogue(EditDialogueWidgets *);

    inline QString name() { return stringValue("name"); }
    inline void setName(const QString &value) { setValue("name", value); }
    inline QString content() { return stringValue("content"); }
    inline void setContent(const QString &value) { setValue("content", value); }
    inline bool usesDivElements() { return intValue("div_tables"); }
    inline void setUsesDivElements(bool value) { setValue("div_tables", (int)value); }

    static QString tableName();
    static inline MTRecordQuery<Style> query(const QVariantMap &parents = QVariantMap()) { return MTRecordQuery<Style>(tableName(), parents); }
    static const ColumnList &columns();
    static const MTDictionary &attributes();
};

#endif // STYLE_H
