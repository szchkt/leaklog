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

#ifndef EDIT_INSPECTION_DIALOGUE_ASSEMBLY_RECORD_H
#define EDIT_INSPECTION_DIALOGUE_ASSEMBLY_RECORD_H

#include "tabbededitdialogue.h"

class EditDialogueGroupsLayout;
class EditInspectionDialogueAccess;

class EditInspectionDialogueAssemblyRecordTab : public EditDialogueTab
{
    Q_OBJECT

public:
    EditInspectionDialogueAssemblyRecordTab(int, MDLineEdit *, MDComboBox *, EditInspectionDialogueAccess *, const QString &, const QString &, QWidget * = NULL);

    void save(const QString &);
    QString saveNewItemType(const MTDictionary &);

private slots:
    void loadItemInputWidgets(bool = false);
    void recordTypeChanged();
    void assemblyRecordNumberChanged();

private:
    void init();
    MTDictionary listAssemblyRecordItemTypes();
    const QVariant assemblyRecordType();
    const QVariant assemblyRecordId();

    EditDialogueGroupsLayout *groups_layout;
    MDComboBox *ar_type_w;
    MDLineEdit *arno_w;
    QString original_arno;
    QString current_arno;
    QString customer_uuid;
    QString circuit_uuid;
    bool arno_being_changed;
    EditInspectionDialogueAccess *inspection_dialogue_access;
};

#endif // EDIT_INSPECTION_DIALOGUE_ASSEMBLY_RECORD_H
