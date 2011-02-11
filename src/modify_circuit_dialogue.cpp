#include "modify_circuit_dialogue.h"

ModifyCircuitDialogue::ModifyCircuitDialogue(DBRecord * record, QWidget * parent)
    : TabbedModifyDialogue(record, parent)
{
    addTab(new ModifyCircuitDialogueUnitsTab);
}

ModifyCircuitDialogueUnitsTab::ModifyCircuitDialogueUnitsTab(QWidget * parent)
    : ModifyDialogueTab(parent)
{
}

void ModifyCircuitDialogueUnitsTab::save(int)
{
}
