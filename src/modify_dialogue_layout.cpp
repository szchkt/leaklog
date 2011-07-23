#include "modify_dialogue_layout.h"

#include "input_widgets.h"

ModifyDialogueLayout::ModifyDialogueLayout(QList<MDAbstractInputWidget *> * md_inputwidgets, QGridLayout * md_grid_main)
{
    this->md_inputwidgets = md_inputwidgets;
    this->md_grid_main = md_grid_main;

    md_grid_main->setHorizontalSpacing(9);
    md_grid_main->setVerticalSpacing(6);
    md_grid_main->setContentsMargins(9, 9, 9, 9);
}

void ModifyDialogueLayout::layout()
{
    for (int i = 0; i < md_inputwidgets->count(); ++i) {
        addWidget(md_inputwidgets->at(i)->label()->widget(), i, 0);
        addWidget(md_inputwidgets->at(i)->widget(), i, 1, 1, 3);
    }
}

ModifyDialogueColumnLayout::ModifyDialogueColumnLayout(QList<MDAbstractInputWidget *> * inputwidgets, QGridLayout * grid_main, int rows_in_column)
    : ModifyDialogueLayout(inputwidgets, grid_main)
{
    this->rows_in_column = rows_in_column;
}

void ModifyDialogueColumnLayout::layout()
{
    int count = 0;
    for (int i = 0; i < md_inputwidgets->count(); ++i) { if (md_inputwidgets->at(i)->showInForm()) count++; }
    int num_cols = count / rows_in_column + 1;
    int num_rows = count / num_cols + (count % num_cols > 0 ? 1 : 0);
    int i = 0;
    for (int c = 0; c < num_cols; ++c) {
        for (int r = 0; r < num_rows; ++r) {
            if (i >= md_inputwidgets->count()) { return; }
            if (!md_inputwidgets->at(i)->showInForm()) { r--; i++; continue; }
            addWidget(md_inputwidgets->at(i)->label()->widget(), r, 2 * c);
            addWidget(md_inputwidgets->at(i)->widget(), r, (2 * c) + 1);
            i++;
        }
    }
}

void ModifyDialogueLayout::addWidget(QWidget * widget, int row, int column, Qt::Alignment alignment)
{
    md_grid_main->addWidget(widget, row, column, alignment);
}

void ModifyDialogueLayout::addWidget(QWidget * widget, int fromRow, int fromColumn, int rowSpan, int columnSpan, Qt::Alignment alignment)
{
    md_grid_main->addWidget(widget, fromRow, fromColumn, rowSpan, columnSpan, alignment);
}
