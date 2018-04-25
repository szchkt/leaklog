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

#ifndef EDIT_ASSEMBLY_RECORD_DIALOGUE_H
#define EDIT_ASSEMBLY_RECORD_DIALOGUE_H

#include "tabbededitdialogue.h"

class EditAssemblyRecordDialogue : public TabbedEditDialogue
{
    Q_OBJECT

public:
    EditAssemblyRecordDialogue(DBRecord *, UndoStack *, QWidget * = NULL);

protected slots:
    virtual void save();
};

class EditAssemblyRecordDialogueTab : public EditDialogueTab
{
    Q_OBJECT

public:
    EditAssemblyRecordDialogueTab(int, QWidget * = NULL);

    void save(const QVariant &);
    QWidget *widget() { return this; }

private:
    void init();

    int record_id;
    QTreeWidget *tree;
};

#endif // EDIT_ASSEMBLY_RECORD_DIALOGUE_H
