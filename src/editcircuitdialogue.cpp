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

#include "editcircuitdialogue.h"

#include "global.h"
#include "records.h"
#include "mtdictionary.h"
#include "inputwidgets.h"
#include "editcircuitdialoguecompressorstab.h"
#include "editcircuitdialogueunitstab.h"

EditCircuitDialogueNotesTab::EditCircuitDialogueNotesTab(MDAbstractInputWidget *notes, QWidget *parent)
    : EditDialogueTab(parent)
{
    setName(tr("Notes"));

    static_cast<QLabel *>(notes->label()->widget())->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(9, 9, 9, 9);
    layout->setSpacing(9);

    layout->addWidget(notes->label()->widget());
    layout->addWidget(notes->widget());
}

void EditCircuitDialogueNotesTab::save(const QVariant &)
{
}

EditCircuitDialogue::EditCircuitDialogue(DBRecord *record, UndoStack *undo_stack, QWidget *parent)
    : TabbedEditDialogue(record, undo_stack, parent)
{
    main_tabw->setTabText(0, tr("Cooling circuit"));

    EditCircuitDialogueCompressorsTab *compressors_tab = new EditCircuitDialogueCompressorsTab(md_record->parent("parent"), idFieldValue().toString(), this);
    addTab(compressors_tab);

    EditCircuitDialogueUnitsTab *units_tab = new EditCircuitDialogueUnitsTab(md_record->parent("parent"), idFieldValue().toString(), this);
    QObject::connect(units_tab, SIGNAL(updateCircuit(const QVariantMap &)), this, SLOT(updateCircuit(const QVariantMap &)));
    addTab(units_tab);

    EditCircuitDialogueNotesTab *notes_tab = new EditCircuitDialogueNotesTab(inputWidget("notes"), this);
    addTab(notes_tab);
}

void EditCircuitDialogue::updateCircuit(const QVariantMap &values)
{
    for (auto i = values.constBegin(); i != values.constEnd(); ++i) {
        MDAbstractInputWidget *md = inputWidget(i.key());
        if (md)
            md->setVariantValue(i.value());
    }
}
