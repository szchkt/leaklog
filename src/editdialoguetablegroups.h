/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2018 Matus & Michal Tomlein

 Leaklog is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public Licence
 as published by the Free Software Foundation; either version 2
 of the Licence, or (at your option) any later version.

 Leaklog is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public Licence for more details.

 You should have received a copy of the GNU General Public Licence
 along with Leaklog; if not, write to the Free Software Foundation,
 Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
********************************************************************/

#ifndef EDIT_DIALOGUE_TABLE_GROUPS_H
#define EDIT_DIALOGUE_TABLE_GROUPS_H

#include <QString>
#include <QWidget>

class QVBoxLayout;

class EditDialogueTableCell;
class EditDialogueAdvancedTable;
class EditDialogueGroupHeaderItem;

class EditDialogueGroupsLayout : public QWidget
{
    Q_OBJECT

public:
    EditDialogueGroupsLayout(QWidget *);
    ~EditDialogueGroupsLayout();

    void addHeaderItem(int, const QString &, const QString &, int);
    void addItem(const QString &, const QString &, QMap<QString, EditDialogueTableCell *> &, int, bool);
    EditDialogueAdvancedTable *createGroup(const QString &, const QString &, int);

    QList<QVariantMap> allValues();

    void clear();

private:
    QMap<QString, EditDialogueAdvancedTable *> *groups;
    QList<EditDialogueGroupHeaderItem *> header_items;
    QVBoxLayout *layout;
};

class EditDialogueGroupHeaderItem
{
public:
    EditDialogueGroupHeaderItem(int, const QString &, const QString &, int);

    int id() { return item_id; }
    EditDialogueTableCell *tableCell();

private:
    int item_id;
    QString item_name;
    QString item_full_name;
    int data_type;
};

#endif // EDIT_DIALOGUE_TABLE_GROUPS_H
