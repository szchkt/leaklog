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

#ifndef EDIT_CIRCUIT_DIALOGUE_H
#define EDIT_CIRCUIT_DIALOGUE_H

#include "tabbededitdialogue.h"

#include <QVariantMap>

class EditCircuitDialogueNotesTab : public EditDialogueTab
{
    Q_OBJECT

public:
    EditCircuitDialogueNotesTab(MDAbstractInputWidget *notes, QWidget * = NULL);

    void save(const QString &uuid);
    QWidget *widget() { return this; }
};

class EditCircuitDialogue : public TabbedEditDialogue
{
    Q_OBJECT

public:
    EditCircuitDialogue(DBRecord *, UndoStack *, QWidget * = NULL);

private slots:
    void updateCircuit(const QVariantMap &);
};

#endif // EDIT_CIRCUIT_DIALOGUE_H
