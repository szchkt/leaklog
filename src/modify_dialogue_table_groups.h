#ifndef MODIFY_DIALOGUE_TABLE_GROUPS_H
#define MODIFY_DIALOGUE_TABLE_GROUPS_H

#include <QString>
#include <QWidget>

#include "mtdictionary.h"

class QVBoxLayout;

class ModifyDialogueTableCell;
class ModifyDialogueTableGroupBox;
class ModifyDialogueGroupHeaderItem;

class ModifyDialogueGroupsLayout : public QWidget
{
    Q_OBJECT

public:
    ModifyDialogueGroupsLayout(QWidget *);
    ~ModifyDialogueGroupsLayout();

    void addHeaderItem(int, const QString &, const QString &);
    void addItem(const QString &, int, const QString &, const QMap<QString, ModifyDialogueTableCell *> &, int, bool);
    ModifyDialogueTableGroupBox * createGroup(const QString &, int, int);

    QList<MTDictionary> allValues();

    void clear();

private:
    QMap<QString, ModifyDialogueTableGroupBox *> * groups;
    QList<ModifyDialogueGroupHeaderItem *> header_items;
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

#endif // MODIFY_DIALOGUE_TABLE_GROUPS_H
