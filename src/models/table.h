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

#ifndef TABLE_H
#define TABLE_H

#include "dbrecord.h"

class Table : public DBRecord
{
    Q_OBJECT

public:
    Table(const QString &uuid = QString());
    Table(const MTDictionary &parents);

    void initEditDialogue(EditDialogueWidgets *);

    inline QString name() { return stringValue("name"); }
    inline void setName(const QString &value) { setValue("name", value); }
    inline int position() { return intValue("position"); }
    inline void setPosition(int value) { setValue("position", value); }
    inline bool highlightNominal() { return intValue("highlight_nominal"); }
    inline void setHighlightNominal(bool value) { setValue("highlight_nominal", value); }
    inline int scope() { return intValue("scope"); }
    inline void setScope(int value) { setValue("scope", value); }
    inline QStringList variables() { return stringValue("variables").split(';', QString::SkipEmptyParts); }
    inline void setVariables(const QStringList &value) { setValue("variables", value.join(';')); }
    inline void setVariables(const QString &value) { setValue("variables", value); }
    inline QStringList summedVariables() { return stringValue("sum").split(';', QString::SkipEmptyParts); }
    inline void setSummedVariables(const QStringList &value) { setValue("sum", value.join(';')); }
    inline void setSummedVariables(const QString &value) { setValue("sum", value); }
    inline QStringList averagedVariables() { return stringValue("avg").split(';', QString::SkipEmptyParts); }
    inline void setAveragedVariables(const QStringList &value) { setValue("avg", value.join(';')); }
    inline void setAveragedVariables(const QString &value) { setValue("avg", value); }

    static QString tableName();
    static const ColumnList &columns();
};

#endif // TABLE_H
