#ifndef MODIFY_INSPECTION_DIALOGUE_H
#define MODIFY_INSPECTION_DIALOGUE_H

#include "tabbed_modify_dialogue.h"

class ModifyDialogueGroupsLayout;

class ModifyInspectionDialogue : public TabbedModifyDialogue
{
public:
    ModifyInspectionDialogue(DBRecord *, QWidget * = NULL);
};

class ModifyInspectionDialogueTab : public ModifyDialogueTab
{
    Q_OBJECT

public:
    ModifyInspectionDialogueTab(int, MDLineEdit *, MDComboBox *, QWidget * = NULL);

    void save(int);
    int saveNewItemType(const MTDictionary &);

private slots:
    void loadItemInputWidgets();
    void assemblyRecordNumberChanged();

private:
    void init();
    MTDictionary listAssemblyRecordItemTypes();
    const QVariant assemblyRecordType();
    const QVariant assemblyRecordId();

    ModifyDialogueGroupsLayout * groups_layout;
    MDComboBox * ar_type_w;
    MDLineEdit * arno_w;
    QString original_arno;
};

#endif // MODIFY_INSPECTION_DIALOGUE_H
