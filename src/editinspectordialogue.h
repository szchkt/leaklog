/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2026 Matus & Michal Tomlein

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

#ifndef EDIT_INSPECTOR_DIALOGUE_H
#define EDIT_INSPECTOR_DIALOGUE_H

#include "tabbededitdialogue.h"

class EditInspectorDialogue;
class EditInspectorDialogueTab;

class EditInspectorDialogue : public TabbedEditDialogue
{
    Q_OBJECT

public:
    EditInspectorDialogue(DBRecord *, UndoStack *, QWidget * = NULL);
};

class EditInspectorDialogueTab : public EditDialogueTab
{
    Q_OBJECT

public:
    EditInspectorDialogueTab(QList<MDAbstractInputWidget *>, QWidget * = NULL);

    void save(const QString &);
    QWidget *widget() { return this; }

private:
    void init();

    QList<MDAbstractInputWidget *> inputwidgets;
};

#endif // EDIT_INSPECTOR_DIALOGUE_H
