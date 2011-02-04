#ifndef TABBED_MODIFY_DIALOGUE_H
#define TABBED_MODIFY_DIALOGUE_H

#include "modify_dialogue.h"

class QTabWidget;
class QTreeWidget;
class QLineEdit;
class QComboBox;
class QGroupBox;

class ModifyDialogueTab;
class MDLineEdit;
class MDComboBox;
class MTDictionary;

class TabbedModifyDialogue : public ModifyDialogue
{
    Q_OBJECT

public:
    TabbedModifyDialogue(DBRecord *, QWidget * = NULL);
    ~TabbedModifyDialogue();

protected slots:
    void save();

protected:
    void addMainGridLayout(QVBoxLayout *);

    void addTab(ModifyDialogueTab *);

    QTabWidget * main_tabw;
};

class ModifyDialogueTab : public QWidget
{
public:
    ModifyDialogueTab(QWidget * = NULL);

    const QString & name() { return tab_name; }
    virtual void save(int) = 0;

protected:
    void setName(const QString tab_name) { this->tab_name = tab_name; }

    QString tab_name;
};

class AssemblyRecordModifyDialogue : public TabbedModifyDialogue
{
public:
    AssemblyRecordModifyDialogue(DBRecord *, QWidget * = NULL);

};

class AssemblyRecordModifyDialogueTab : public ModifyDialogueTab
{
public:
    AssemblyRecordModifyDialogueTab(int, QWidget * = NULL);

    void save(int);

private:
    void init();

    int record_id;
    QTreeWidget * tree;
};

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

private slots:
    void loadItemInputWidgets();

private:
    void init();
    MTDictionary listAssemblyRecordItemTypes();
    const QVariant assemblyRecordType();
    const QVariant assemblyRecordId();

    QGridLayout * items_grid;
    QList<MDInputWidget *> item_inputwidgets;
    QLineEdit * id_le;
    QComboBox * assembly_record_type;
    MDComboBox * ar_type_w;
    MDLineEdit * arno_w;
};

#endif // TABBED_MODIFY_DIALOGUE_H
