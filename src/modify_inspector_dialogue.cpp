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

#include "modify_inspector_dialogue.h"

#include "input_widgets.h"

#include <QTabWidget>

ModifyInspectorDialogue::ModifyInspectorDialogue(DBRecord * record, QWidget * parent)
    : TabbedModifyDialogue(record, parent)
{
    main_tabw->setTabText(0, tr("Inspector"));

    QList<MDAbstractInputWidget *> tab_inputwidgets;
    tab_inputwidgets << inputWidget("list_price");
    tab_inputwidgets << inputWidget("acquisition_price");

    addTab(new ModifyInspectorDialogueTab(tab_inputwidgets, this));
}

ModifyInspectorDialogueTab::ModifyInspectorDialogueTab(QList<MDAbstractInputWidget *> inputwidgets, QWidget * parent)
    : ModifyDialogueTab(parent)
{
    setName(tr("Assembly record"));

    this->inputwidgets = inputwidgets;

    init();
}

void ModifyInspectorDialogueTab::init()
{
    for (int i = 0; i < inputwidgets.count(); ++i) {
        inputwidgets.at(i)->setShowInForm(true);
    }

    QGridLayout * grid = new QGridLayout;
    ModifyDialogueColumnLayout(&inputwidgets, grid).layout();

    setLayout(grid);
}

void ModifyInspectorDialogueTab::save(const QVariant &)
{
}
