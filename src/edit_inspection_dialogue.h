/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2012 Matus & Michal Tomlein

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

#include "tabbed_edit_dialogue.h"

class EditDialogueBasicTable;
class EditDialogueGroupsLayout;
class EditInspectionDialogueCompressors;

class EditInspectionDialogue : public TabbedEditDialogue
{
    Q_OBJECT

public:
    EditInspectionDialogue(DBRecord *, QWidget * = NULL);

protected:
    bool saveOther();

protected:
    const QVariant idFieldValue();
    EditInspectionDialogueCompressors * compressors;
};

class EditInspectionDialogueImagesTab : public EditDialogueTab
{
    Q_OBJECT

public:
    EditInspectionDialogueImagesTab(const QString &, const QString &, const QString &);

    void save(const QVariant &);

private:
    void init(const QString &);
    void loadItemInputWidgets(const QString &);

    QString customer_id;
    QString circuit_id;
    QString original_inspection_id;

    EditDialogueBasicTable * table;
};

#endif // EDIT_INSPECTION_DIALOGUE_H
