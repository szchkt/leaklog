#include "edit_inspection_dialogue_access.h"

#include "edit_inspection_dialogue.h"
#include "input_widgets.h"

EditInspectionDialogueAccess::EditInspectionDialogueAccess(EditInspectionDialogue * parent)
: m_inspection_dialogue(parent)
{
}

const QVariant EditInspectionDialogueAccess::getVariableValue(const QString & name)
{
    return m_inspection_dialogue->inputWidget(name)->variantValue();
}
