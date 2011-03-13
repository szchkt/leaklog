/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2011 Matus & Michal Tomlein

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
