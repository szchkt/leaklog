/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2016 Matus & Michal Tomlein

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

#ifndef EDIT_CUSTOMER_DIALOGUE_H
#define EDIT_CUSTOMER_DIALOGUE_H

#include "inputwidgets.h"
#include "editdialogue.h"

class Customer;
class EditDialogueBasicTable;
class QButtonGroup;

class OperatorInputWidget : public MDGroupedInputWidgets
{
    Q_OBJECT

public:
    OperatorInputWidget(const QVariantMap &, QWidget *);

    QVariant variantValue() const;
    void setVariantValue(const QVariant &);

    void addToEditDialogue(EditDialogueWidgets &);

private slots:
    void operatorChoiceChanged(int);

private:
    QButtonGroup *operator_choice;
    QList<MDAbstractInputWidget *> input_widgets;
};

class EditCustomerDialogue : public EditDialogue
{
    Q_OBJECT

public:
    EditCustomerDialogue(Customer *, UndoStack *, QWidget * = NULL);

protected slots:
    void save();

private:
    EditDialogueBasicTable *persons_table;

    QStringList former_ids;
};

#endif // EDIT_CUSTOMER_DIALOGUE_H
