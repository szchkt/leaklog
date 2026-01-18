/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2026 Matus & Michal Tomlein

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

#ifndef EDIT_INSPECTION_DIALOGUE_LAYOUT_H
#define EDIT_INSPECTION_DIALOGUE_LAYOUT_H

template<class Key, class T>
class QMap;

#include "editdialoguelayout.h"

#include <QTreeWidget>

class UniformRowColourTreeWidget : public QTreeWidget
{
    Q_OBJECT

public:
    enum ItemDataRole {
        BackgroundColourRole = Qt::UserRole + 1
    };

    UniformRowColourTreeWidget(QWidget *parent = 0): QTreeWidget(parent) {}

protected:
    void drawRow(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

class EditInspectionDialogueLayout : public EditDialogueLayout
{
public:
    EditInspectionDialogueLayout(QList<MDAbstractInputWidget *> *, QMap<QString, QString> *, QGridLayout *);

    void layout();

private:
    QMap<QString, QString> *groups;
};

#endif // EDIT_INSPECTION_DIALOGUE_LAYOUT_H
