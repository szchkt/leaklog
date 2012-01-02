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

#include "edit_circuit_dialogue.h"

#include "global.h"
#include "records.h"
#include "mtdictionary.h"
#include "input_widgets.h"
#include "edit_circuit_dialogue_compressors_tab.h"
#include "edit_circuit_dialogue_units_tab.h"

EditCircuitDialogue::EditCircuitDialogue(DBRecord * record, QWidget * parent)
    : TabbedEditDialogue(record, parent)
{
    main_tabw->setTabText(0, tr("Cooling circuit"));

    EditCircuitDialogueCompressorsTab * compressors_tab = new EditCircuitDialogueCompressorsTab(md_record->parent("parent"), idFieldValue().toString(), this);
    addTab(compressors_tab);

    EditCircuitDialogueUnitsTab * units_tab = new EditCircuitDialogueUnitsTab(md_record->parent("parent"), idFieldValue().toString(), this);
    QObject::connect(units_tab, SIGNAL(updateCircuit(MTDictionary)), this, SLOT(updateCircuit(MTDictionary)));
    addTab(units_tab);
}

void EditCircuitDialogue::updateCircuit(MTDictionary dict)
{
    MDAbstractInputWidget * md;
    for (int i = 0; i < dict.count(); ++i) {
        md = inputWidget(dict.key(i));
        if (md)
            md->setVariantValue(dict.value(i));
    }
}
