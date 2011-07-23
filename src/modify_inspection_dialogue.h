/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2011 Matus & Michal Tomlein

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

#ifndef MODIFY_INSPECTION_DIALOGUE_H
#define MODIFY_INSPECTION_DIALOGUE_H

#include "tabbed_modify_dialogue.h"

class ModifyDialogueBasicTable;
class ModifyDialogueGroupsLayout;
class ModifyInspectionDialogueCompressors;

class ModifyInspectionDialogue : public TabbedModifyDialogue
{
    Q_OBJECT

public:
    ModifyInspectionDialogue(DBRecord *, QWidget * = NULL);

protected:
    const QVariant idFieldValue();
    ModifyInspectionDialogueCompressors * compressors;
};

class ModifyInspectionDialogueTab : public ModifyDialogueTab
{
    Q_OBJECT

public:
    ModifyInspectionDialogueTab(int, MDLineEdit *, MDComboBox *, const QString &, const QString &, QWidget * = NULL);

    void save(const QVariant &);
    int saveNewItemType(const MTDictionary &);

private slots:
    void loadItemInputWidgets(bool = false);
    void recordTypeChanged();
    void assemblyRecordNumberChanged();

private:
    void init();
    MTDictionary listAssemblyRecordItemTypes();
    const QVariant assemblyRecordType();
    const QVariant assemblyRecordId();

    ModifyDialogueGroupsLayout * groups_layout;
    MDComboBox * ar_type_w;
    MDLineEdit * arno_w;
    QString original_arno;
    QString customer_id;
    QString circuit_id;
};

class ModifyInspectionDialogueImagesTab : public ModifyDialogueTab
{
    Q_OBJECT

public:
    ModifyInspectionDialogueImagesTab(const QString &, const QString &, const QString &);

    void save(const QVariant &);

private:
    void init(const QString &);
    void loadItemInputWidgets(const QString &);

    QString customer_id;
    QString circuit_id;

    ModifyDialogueBasicTable * table;
};

#endif // MODIFY_INSPECTION_DIALOGUE_H
