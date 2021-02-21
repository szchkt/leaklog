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

#include "editdialoguelayout.h"

#include "inputwidgets.h"

EditDialogueLayout::EditDialogueLayout(QList<MDAbstractInputWidget *> *md_inputwidgets, QGridLayout *md_grid_main)
{
    this->md_inputwidgets = md_inputwidgets;
    this->md_grid_main = md_grid_main;

    md_grid_main->setHorizontalSpacing(9);
    md_grid_main->setVerticalSpacing(6);
    md_grid_main->setContentsMargins(9, 9, 9, 9);
}

void EditDialogueLayout::layout()
{
    for (int i = 0; i < md_inputwidgets->count(); ++i) {
        addWidget(md_inputwidgets->at(i)->label()->widget(), i, 0);
        addWidget(md_inputwidgets->at(i)->widget(), i, 1, 1, 3);
    }
}

EditDialogueColumnLayout::EditDialogueColumnLayout(QList<MDAbstractInputWidget *> *inputwidgets, QGridLayout *grid_main, int rows_in_column)
    : EditDialogueLayout(inputwidgets, grid_main)
{
    this->rows_in_column = rows_in_column;
}

void EditDialogueColumnLayout::layout()
{
    int count = 0;
    for (int i = 0; i < md_inputwidgets->count(); ++i) {
        count += md_inputwidgets->at(i)->rowSpan();
    }
    int num_cols = count / rows_in_column + 1;
    int num_rows = count / num_cols + (count % num_cols > 0 ? 1 : 0);
    int i = 0;
    for (int c = 0; c < num_cols; ++c) {
        for (int r = 0; r < num_rows;) {
            if (i >= md_inputwidgets->count()) { return; }
            int row_span = md_inputwidgets->at(i)->rowSpan();
            if (!row_span) { i++; continue; }
            addWidget(md_inputwidgets->at(i)->label()->widget(), r, 2 * c);
            addWidget(md_inputwidgets->at(i)->widget(), r, (2 * c) + 1, row_span, 1);
            r += row_span;
            i++;
        }
    }
}

void EditDialogueLayout::addWidget(QWidget *widget, int row, int column, Qt::Alignment alignment)
{
    md_grid_main->addWidget(widget, row, column, alignment);
}

void EditDialogueLayout::addWidget(QWidget *widget, int fromRow, int fromColumn, int rowSpan, int columnSpan, Qt::Alignment alignment)
{
    md_grid_main->addWidget(widget, fromRow, fromColumn, rowSpan, columnSpan, alignment);
}
