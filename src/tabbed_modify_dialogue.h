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

    ModifyDialogueGroupsLayout * groups_layout;
    MDComboBox * ar_type_w;
    MDLineEdit * arno_w;
};

class ModifyDialogueGroupsLayout : public QWidget
{
    Q_OBJECT

public:
    ModifyDialogueGroupsLayout(QWidget *);

    void addHeaderItem(int, const QString &, const QString &);
    void addItem(const QString &, const QString &, const MTDictionary &, int, bool);
    ModifyDialogueTableGroupBox * createGroup(const QString &, int);

    QList<MTDictionary> allValues();

private:
    QMap<QString, ModifyDialogueTableGroupBox *> * groups;
    QList<ModifyDialogueGroupHeaderItem *> header_items;
    QList<MDInputWidget *> item_inputwidgets;
    QVBoxLayout * layout;
};

class ModifyDialogueGroupHeaderItem
{
public:
    ModifyDialogueGroupHeaderItem(int, const QString &, const QString &);

    int id() { return item_id; }
    const QString & name() { return item_name; }
    const QString & fullName() { return item_full_name; }

private:
    int item_id;
    QString item_name;
    QString item_full_name;
};

#endif // TABBED_MODIFY_DIALOGUE_H
