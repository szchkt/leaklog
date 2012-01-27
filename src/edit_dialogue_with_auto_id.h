#ifndef EDIT_DIALOGUE_WITH_AUTO_ID_H
#define EDIT_DIALOGUE_WITH_AUTO_ID_H

#include "edit_dialogue.h"

class EditDialogueWithAutoId : public EditDialogue
{
public:
    EditDialogueWithAutoId(DBRecord * record, QWidget * parent = NULL, int max_id = -1) :
        EditDialogue(record, parent), m_max_id(max_id) {}

protected slots:
    virtual void save();

private:
    int m_max_id;
};

#endif // EDIT_DIALOGUE_WITH_AUTO_ID_H
