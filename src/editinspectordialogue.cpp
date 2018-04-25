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

#include "editinspectordialogue.h"

#include "inputwidgets.h"
#include "editdialoguelayout.h"

#include <QTabWidget>

EditInspectorDialogue::EditInspectorDialogue(DBRecord *record, UndoStack *undo_stack, QWidget *parent)
    : TabbedEditDialogue(record, undo_stack, parent)
{
    main_tabw->setTabText(0, tr("Inspector"));

    QList<MDAbstractInputWidget *> tab_inputwidgets;
    tab_inputwidgets << inputWidget("acquisition_price");
    tab_inputwidgets << inputWidget("list_price");

    addTab(new EditInspectorDialogueTab(tab_inputwidgets, this));
}

EditInspectorDialogueTab::EditInspectorDialogueTab(QList<MDAbstractInputWidget *> inputwidgets, QWidget *parent)
    : EditDialogueTab(parent)
{
    setName(tr("Assembly record"));

    this->inputwidgets = inputwidgets;

    init();
}

void EditInspectorDialogueTab::init()
{
    for (int i = 0; i < inputwidgets.count(); ++i) {
        inputwidgets.at(i)->setRowSpan(1);
    }

    QGridLayout *grid = new QGridLayout;
    EditDialogueColumnLayout(&inputwidgets, grid).layout();

    setLayout(grid);
}

void EditInspectorDialogueTab::save(const QVariant &)
{
}
