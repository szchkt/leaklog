#ifndef MODIFY_INSPECTION_DIALOGUE_ASSEMBLY_RECORD_H
#define MODIFY_INSPECTION_DIALOGUE_ASSEMBLY_RECORD_H

#include "tabbed_modify_dialogue.h"

class ModifyDialogueGroupsLayout;

class ModifyInspectionDialogueAssemblyRecordTab : public ModifyDialogueTab
{
    Q_OBJECT

public:
    ModifyInspectionDialogueAssemblyRecordTab(int, MDLineEdit *, MDComboBox *, const QString &, const QString &, QWidget * = NULL);

    void save(const QVariant &);
    int saveNewItemType(const MTDictionary &);

private slots:
    void loadItemInputWidgets(bool = false);
    void recordTypeChanged();
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
    QString customer_id;
    QString circuit_id;
};

#endif // MODIFY_INSPECTION_DIALOGUE_ASSEMBLY_RECORD_H
