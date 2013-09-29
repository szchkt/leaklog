#ifndef EDIT_INSPECTION_DIALOGUE_ACCESS_H
#define EDIT_INSPECTION_DIALOGUE_ACCESS_H

#include <QVariant>

class EditInspectionDialogue;

class EditInspectionDialogueAccess
{
public:
    EditInspectionDialogueAccess(EditInspectionDialogue *);

    const QVariant getVariableValue(const QString &);

private:
    EditInspectionDialogue *m_inspection_dialogue;
};

#endif // EDIT_INSPECTION_DIALOGUE_ACCESS_H
