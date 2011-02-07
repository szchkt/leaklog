/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2010 Matus & Michal Tomlein

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

#include "modify_dialogue.h"
#include "input_widgets.h"
#include "records.h"

#include <QPushButton>
#include <QDialogButtonBox>
#include <QMessageBox>

void ModifyDialogue::init(DBRecord * record)
{
    md_record = record;
    QVBoxLayout * md_vlayout_main = new QVBoxLayout(this);
    md_vlayout_main->setSpacing(9);
    md_vlayout_main->setContentsMargins(9, 9, 9, 9);
    md_grid_main = new QGridLayout;
    addMainGridLayout(md_vlayout_main);
    QDialogButtonBox * md_bb = new QDialogButtonBox(this);
    md_bb->setStandardButtons(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
    md_bb->button(QDialogButtonBox::Save)->setText(tr("Save"));
    md_bb->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));
    QObject::connect(md_bb, SIGNAL(accepted()), this, SLOT(save()));
    QObject::connect(md_bb, SIGNAL(rejected()), this, SLOT(reject()));
    md_vlayout_main->addWidget(md_bb);
    this->resize(20, 20);
}

void ModifyDialogue::addMainGridLayout(QVBoxLayout * md_vlayout_main)
{
    md_vlayout_main->addLayout(md_grid_main);
}

ModifyDialogue::ModifyDialogue(QWidget * parent):
QDialog(parent)
{}

ModifyDialogue::ModifyDialogue(DBRecord * record, QWidget * parent):
QDialog(parent)
{
    init(record);

    md_record->initModifyDialogue(this);

    ModifyDialogueColumnLayout(&md_inputwidgets, md_grid_main).layout();
}

void ModifyDialogue::setWindowTitle(const QString & title)
{
    if (!md_record->id().isEmpty()) {
        this->QDialog::setWindowTitle(tr("%1: %2").arg(title).arg(md_record->id()));
    } else {
        this->QDialog::setWindowTitle(title);
    }
}

void ModifyDialogue::save()
{
    save(true);
}

bool ModifyDialogue::save(bool call_accept)
{
    QVariantMap values; QString id; QVariant value;
    for (QList<MDInputWidget *>::const_iterator i = md_inputwidgets.constBegin(); i != md_inputwidgets.constEnd(); ++i) {
        id = (*i)->id();
        value = (*i)->variantValue();
        if (id == md_record->idField()) {
            if (value.toString().isEmpty()) {
                QMessageBox::information(NULL, tr("Save changes"), tr("Invalid ID."));
                return false;
            } else if (md_used_ids.contains(value.toString())) {
                if (id == "date") {
                    QMessageBox::information(NULL, tr("Save changes"), tr("This date is not available. Please choose a different date."));
                } else {
                    QMessageBox::information(NULL, tr("Save changes"), tr("This ID is not available. Please choose a different ID."));
                }
                return false;
            }
        }
        values.insert(id, value);
    }
    md_record->update(values, true);

    if (call_accept) accept();
    return true;
}

const QVariant ModifyDialogue::idFieldValue()
{
    for (int i = 0; i < md_inputwidgets.count(); ++i) {
        if (md_inputwidgets.at(i)->id() == "id") {
            return md_inputwidgets.at(i)->variantValue();
        }
    }
    return QVariant();
}

MDInputWidget * ModifyDialogue::inputWidget(const QString id)
{
    for (int i = 0; i < md_inputwidgets.count(); ++i) {
        if (md_inputwidgets.at(i)->id() == id) {
            return md_inputwidgets.at(i);
        }
    }
    return NULL;
}

ModifyDialogueLayout::ModifyDialogueLayout(QList<MDInputWidget *> * md_inputwidgets, QGridLayout * md_grid_main)
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
        addWidget(md_inputwidgets->at(i)->label(), i, 0);
        addWidget(md_inputwidgets->at(i)->widget(), i, 1, 1, 3);
    }
}

ModifyDialogueColumnLayout::ModifyDialogueColumnLayout(QList<MDInputWidget *> * inputwidgets, QGridLayout * grid_main, int rows_in_column)
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
            addWidget(md_inputwidgets->at(i)->label(), r, 2 * c);
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
