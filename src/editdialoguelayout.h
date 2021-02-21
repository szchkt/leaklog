/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2021 Matus & Michal Tomlein

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

#ifndef EDIT_DIALOGUE_LAYOUT_H
#define EDIT_DIALOGUE_LAYOUT_H

#include <QList>
#include <QGridLayout>

class QWidget;

class MDAbstractInputWidget;

class EditDialogueLayout
{
public:
    EditDialogueLayout(QList<MDAbstractInputWidget *> *, QGridLayout *);

    virtual void layout();
    void addWidget(QWidget *widget, int row, int column, Qt::Alignment alignment = Qt::Alignment());
    void addWidget(QWidget *widget, int fromRow, int fromColumn, int rowSpan, int columnSpan, Qt::Alignment alignment = Qt::Alignment());

protected:
    QGridLayout *md_grid_main;
    QList<MDAbstractInputWidget *> *md_inputwidgets;

};

class EditDialogueColumnLayout : public EditDialogueLayout
{
public:
    EditDialogueColumnLayout(QList<MDAbstractInputWidget *> *, QGridLayout *, int = 20);

    void layout();

private:
    int rows_in_column;
};

#endif // EDIT_DIALOGUE_LAYOUT_H
