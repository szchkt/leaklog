#ifndef MODIFY_INSPECTOR_DIALOGUE_H
#define MODIFY_INSPECTOR_DIALOGUE_H

#include "tabbed_modify_dialogue.h"

class ModifyInspectorDialogue;
class ModifyInspectorDialogueTab;

class ModifyInspectorDialogue : public TabbedModifyDialogue
{
    Q_OBJECT

public:
    ModifyInspectorDialogue(DBRecord *, QWidget * = NULL);
};

class ModifyInspectorDialogueTab : public ModifyDialogueTab
{
    Q_OBJECT

public:
    ModifyInspectorDialogueTab(QList<MDAbstractInputWidget *>, QWidget * = NULL);

    void save(int);
    QWidget * widget() { return this; }

private:
    void init();

    QList<MDAbstractInputWidget *> inputwidgets;
};

#endif // MODIFY_INSPECTOR_DIALOGUE_H
