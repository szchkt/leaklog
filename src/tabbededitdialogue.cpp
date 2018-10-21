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

#include "tabbededitdialogue.h"

#include "records.h"
#include "inputwidgets.h"
#include "global.h"
#include "editdialoguelayout.h"

#include <QTabWidget>
#include <QScrollArea>

TabbedEditDialogue::TabbedEditDialogue(DBRecord *record, UndoStack *undo_stack, QWidget *parent, bool layout)
    : EditDialogue(undo_stack, parent)
{
    main_tabw = new QTabWidget;

    init(record);

    md_record->initEditDialogue(this);

    if (layout)
        EditDialogueColumnLayout(&md_inputwidgets, md_grid_main, md_rows_in_column).layout();
}

TabbedEditDialogue::~TabbedEditDialogue()
{
    delete main_tabw;
}

void TabbedEditDialogue::addMainGridLayout(QVBoxLayout *md_vlayout_main)
{
    QWidget *main_form_tab = new QWidget(main_tabw);
    main_form_tab->setLayout(md_grid_main);

    main_tabw->addTab(main_form_tab, tr("Basic information"));
    md_vlayout_main->addWidget(main_tabw);
}

void TabbedEditDialogue::addTab(EditDialogueTab *tab)
{
    tabs << tab;

    main_tabw->addTab(tab->widget(), tab->name());
}

void TabbedEditDialogue::save()
{
    if (!EditDialogue::save(false)) return;

    for (int i = 0; i < tabs.count(); ++i) {
        tabs.at(i)->save(md_record->uuid());
    }

    accept();
}

EditDialogueTab::EditDialogueTab(QWidget *parent)
    : QWidget(parent)
{
    setAutoFillBackground(false);
}

void EditDialogueTab::setLayout(QLayout *layout)
{
    layout->setContentsMargins(9, 9, 9, 9);
    QWidget::setLayout(layout);
}

QWidget *EditDialogueTab::widget()
{
    QScrollArea *scroll_area = createScrollArea();
    scroll_area->setWidget(this);
    return scroll_area;
}

QScrollArea *EditDialogueTab::createScrollArea()
{
    QScrollArea *scroll_area = new QScrollArea;
    scroll_area->setWidgetResizable(1);
    scroll_area->setFrameStyle(QFrame::NoFrame);
    scroll_area->setAutoFillBackground(true);
    scroll_area->setBackgroundRole(QPalette::NoRole);
    return scroll_area;
}
