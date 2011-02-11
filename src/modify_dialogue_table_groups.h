#ifndef MODIFY_DIALOGUE_TABLE_GROUPS_H
#define MODIFY_DIALOGUE_TABLE_GROUPS_H

#include <QString>
#include <QWidget>

#include "mtdictionary.h"

class QVBoxLayout;

class ModifyDialogueTableCell;
class ModifyDialogueAdvancedTable;
class ModifyDialogueGroupHeaderItem;

class ModifyDialogueGroupsLayout : public QWidget
{
    Q_OBJECT

public:
    ModifyDialogueGroupsLayout(QWidget *);
    ~ModifyDialogueGroupsLayout();

    void addHeaderItem(int, const QString &, const QString &, int);
    void addItem(const QString &, int, QMap<QString, ModifyDialogueTableCell *> &, int, bool);
    ModifyDialogueAdvancedTable * createGroup(const QString &, int, int);

    QList<MTDictionary> allValues();

    void clear();

private:
    QMap<QString, ModifyDialogueAdvancedTable *> * groups;
    QList<ModifyDialogueGroupHeaderItem *> header_items;
    QVBoxLayout * layout;
};

class ModifyDialogueGroupHeaderItem
{
public:
    ModifyDialogueGroupHeaderItem(int, const QString &, const QString &, int);

    int id() { return item_id; }
    ModifyDialogueTableCell * tableCell();

private:
    int item_id;
    QString item_name;
    QString item_full_name;
    int data_type;
};

#endif // MODIFY_DIALOGUE_TABLE_GROUPS_H
