#ifndef MODIFY_ASSEMBLY_RECORD_DIALOGUE_H
#define MODIFY_ASSEMBLY_RECORD_DIALOGUE_H

#include "tabbed_modify_dialogue.h"

class ModifyAssemblyRecordDialogue : public TabbedModifyDialogue
{
    Q_OBJECT

public:
    ModifyAssemblyRecordDialogue(DBRecord *, QWidget * = NULL);

};

class ModifyAssemblyRecordDialogueTab : public ModifyDialogueTab
{
    Q_OBJECT

public:
    ModifyAssemblyRecordDialogueTab(int, QWidget * = NULL);

    void save(const QVariant &);
    QWidget * widget() { return this; }

private:
    void init();

    int record_id;
    QTreeWidget * tree;
};

#endif // MODIFY_ASSEMBLY_RECORD_DIALOGUE_H
