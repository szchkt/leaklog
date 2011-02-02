#ifndef TABBED_MODIFY_DIALOGUE_H
#define TABBED_MODIFY_DIALOGUE_H

#include "modify_dialogue.h"

class QTabWidget;

class TabbedModifyDialogue : public ModifyDialogue
{
public:
    TabbedModifyDialogue(DBRecord *, QWidget * = NULL);

protected:
    void addMainGridLayout(QVBoxLayout *);

    QTabWidget * main_tabw;
};

#endif // TABBED_MODIFY_DIALOGUE_H
