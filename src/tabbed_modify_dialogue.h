#ifndef TABBED_MODIFY_DIALOGUE_H
#define TABBED_MODIFY_DIALOGUE_H

#include "modify_dialogue.h"

class QTabWidget;
class QTreeWidget;
class QLineEdit;
class QComboBox;
class QGroupBox;
class QScrollArea;

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
    QList<ModifyDialogueTab *> tabs;
};

class ModifyDialogueTab : public QWidget
{
    Q_OBJECT

public:
    ModifyDialogueTab(QWidget * = NULL);

    void setLayout(QLayout *);

    const QString & name() { return tab_name; }
    virtual void save(const QVariant &) = 0;

    virtual QWidget * widget();
    QScrollArea * createScrollArea();

protected:
    void setName(const QString tab_name) { this->tab_name = tab_name; }

    QString tab_name;
};

#endif // TABBED_MODIFY_DIALOGUE_H
