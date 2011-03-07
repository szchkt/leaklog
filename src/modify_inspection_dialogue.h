#ifndef MODIFY_INSPECTION_DIALOGUE_H
#define MODIFY_INSPECTION_DIALOGUE_H

#include "tabbed_modify_dialogue.h"

class ModifyDialogueBasicTable;
class ModifyDialogueGroupsLayout;

class ModifyInspectionDialogue : public TabbedModifyDialogue
{
    Q_OBJECT

public:
    ModifyInspectionDialogue(DBRecord *, QWidget * = NULL);

protected:
    const QVariant idFieldValue();
};

class ModifyInspectionDialogueTab : public ModifyDialogueTab
{
    Q_OBJECT

public:
    ModifyInspectionDialogueTab(int, MDLineEdit *, MDComboBox *, const QString &, const QString &, QWidget * = NULL);

    void save(const QVariant &);
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
    QString customer_id;
    QString circuit_id;
};

class ModifyInspectionDialogueImagesTab : public ModifyDialogueTab
{
    Q_OBJECT

public:
    ModifyInspectionDialogueImagesTab(const QString &, const QString &, const QString &);

    void save(const QVariant &);

private:
    void init(const QString &);
    void loadItemInputWidgets(const QString &);

    QString customer_id;
    QString circuit_id;

    ModifyDialogueBasicTable * table;
};

#endif // MODIFY_INSPECTION_DIALOGUE_H
