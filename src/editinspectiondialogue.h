/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2025 Matus & Michal Tomlein

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

#ifndef EDIT_INSPECTION_DIALOGUE_H
#define EDIT_INSPECTION_DIALOGUE_H

#include "tabbededitdialogue.h"

class EditDialogueBasicTable;
class EditDialogueGroupsLayout;
class EditInspectionDialogueCompressors;
class Inspection;

class QSplitter;

class EditInspectionDialogue : public TabbedEditDialogue
{
    Q_OBJECT

public:
    EditInspectionDialogue(Inspection *record, UndoStack *undo_stack, QWidget *parent = NULL, const QString &duplicate_from = QString(), const QStringList &circuit_uuids = QStringList());
    virtual ~EditInspectionDialogue();

    virtual void save();

protected:
    bool saveOther();

    QStringList circuit_uuids;
    EditInspectionDialogueCompressors *compressors;
    QSplitter *splitter;
};

class EditInspectionDialogueImagesTab : public EditDialogueTab
{
    Q_OBJECT

public:
    EditInspectionDialogueImagesTab(const QString &inspection_uuid);

    void save(const QString &inspection_uuid);

private:
    void loadItemInputWidgets(const QString &inspection_uuid);

    EditDialogueBasicTable *table;
};

#endif // EDIT_INSPECTION_DIALOGUE_H
