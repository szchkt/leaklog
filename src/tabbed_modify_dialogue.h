#ifndef TABBED_MODIFY_DIALOGUE_H
#define TABBED_MODIFY_DIALOGUE_H

#include "modify_dialogue.h"

class QTabWidget;
class ModifyDialogueTab;

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
    virtual void save() = 0;

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

    void save();

private:
    void init();

    int record_id;
};

#endif // TABBED_MODIFY_DIALOGUE_H
