/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2015 Matus & Michal Tomlein

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

#ifndef EDIT_DIALOGUE_H
#define EDIT_DIALOGUE_H

#include <QDialog>
#include <QGridLayout>

#include "editdialoguewidgets.h"

class DBRecord;
class MTRecord;
class MDAbstractInputWidget;
class UndoStack;

class EditDialogue : public QDialog, public EditDialogueWidgets
{
    Q_OBJECT

public:
    EditDialogue(DBRecord *, UndoStack *, QWidget * = NULL);

    virtual void setWindowTitle(const QString &);

    inline DBRecord *record() { return md_record; }

    QWidget *widget() { return this; }

protected slots:
    virtual void save();
    bool save(bool);

protected:
    EditDialogue(UndoStack *, QWidget * = NULL);
    void init(DBRecord *);
    virtual void addMainGridLayout(QVBoxLayout *);

    int inputWidgetCount() { return md_inputwidgets.count(); }
    MDAbstractInputWidget *inputWidget(const QString);

    DBRecord *md_record;
    UndoStack *md_undo_stack;
    QGridLayout *md_grid_main;

    friend class Customer;
    friend class Circuit;
    friend class Inspection;
    friend class Repair;
    friend class VariableRecord;
    friend class Table;
    friend class Inspector;
    friend class ServiceCompany;
    friend class RefrigerantRecord;
    friend class WarningRecord;
    friend class AssemblyRecordType;
    friend class AssemblyRecordItemType;
    friend class AssemblyRecordItemCategory;
    friend class EditInspectionDialogueTab;
    friend class CircuitUnitType;
    friend class Style;
    friend class EditInspectionDialogueAccess;
};

#endif // EDIT_DIALOGUE_H
