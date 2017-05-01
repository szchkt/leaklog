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

#ifndef VARIABLE_H
#define VARIABLE_H

#include "dbrecord.h"

class VariableRecord : public DBRecord
{
    Q_OBJECT

public:
    VariableRecord(const QString &id = QString(), const QVariantMap &savedValues = QVariantMap());

    void initEditDialogue(EditDialogueWidgets *);

    inline QString variableID() { return stringValue("id"); }
    inline void setVariableID(const QString &value) { setValue("id", value); }
    inline QString parentID() { return stringValue("parent_id"); }
    inline void setParentID(const QString &value) { setValue("parent_id", value); }
    inline QString name() { return stringValue("name"); }
    inline void setName(const QString &value) { setValue("name", value); }
    inline QString type() { return stringValue("type"); }
    inline void setType(const QString &value) { setValue("type", value); }
    inline QString unit() { return stringValue("unit"); }
    inline void setUnit(const QString &value) { setValue("unit", value); }
    inline int scope() { return intValue("scope"); }
    inline void setScope(int value) { setValue("scope", value); }
    inline QString valueExpression() { return stringValue("value"); }
    inline void setValueExpression(const QString &value) { setValue("value", value); }
    inline bool compareNom() { return intValue("compare_nom"); }
    inline void setCompareNom(bool value) { setValue("compare_nom", (int)value); }
    inline double tolerance() { return doubleValue("tolerance"); }
    inline void setTolerance(double value) { setValue("tolerance", value); }
    inline QString colBg() { return stringValue("col_bg"); }
    inline void setColBg(const QString &value) { setValue("col_bg", value); }

    static QString tableName();
    static inline MTRecordQuery<VariableRecord> query(const MTDictionary &parents = MTDictionary()) { return MTRecordQuery<VariableRecord>(tableName(), parents); }
    static const ColumnList &columns();

protected:
    bool isJournaled() const;
};

#endif // VARIABLE_H
