#ifndef TABBED_MODIFY_DIALOGUE_H
#define TABBED_MODIFY_DIALOGUE_H

#include "modify_dialogue.h"

class QTabWidget;
class QTreeWidget;
class QLineEdit;
class QComboBox;
class QGroupBox;

class ModifyDialogueTab;
class ModifyDialogueGroupsLayout;
class MDLineEdit;
class MDComboBox;
class MTDictionary;
class ModifyDialogueTableGroupBox;
class ModifyDialogueGroupHeaderItem;
class ModifyDialogueTableCell;

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

    void setLayout(QLayout *);

    const QString & name() { return tab_name; }
    virtual void save(int) = 0;

protected:
    void setName(const QString tab_name) { this->tab_name = tab_name; }

    QString tab_name;
};

#endif // TABBED_MODIFY_DIALOGUE_H
