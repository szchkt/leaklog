#ifndef MODIFY_CIRCUIT_DIALOGUE_H
#define MODIFY_CIRCUIT_DIALOGUE_H

#include "tabbed_modify_dialogue.h"

class ModifyCircuitDialogue : public TabbedModifyDialogue
{
public:
    ModifyCircuitDialogue(DBRecord *, QWidget * = NULL);
};

class ModifyCircuitDialogueUnitsTab : public ModifyDialogueTab
{
    Q_OBJECT

public:
    ModifyCircuitDialogueUnitsTab(QWidget * = NULL);

    void save(int);
};

#endif // MODIFY_CIRCUIT_DIALOGUE_H
