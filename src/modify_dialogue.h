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

#ifndef MODIFY_DIALOGUE_H
#define MODIFY_DIALOGUE_H

#include <QDialog>
#include <QGridLayout>

class DBRecord;
class MDInputWidget;

class ModifyDialogue : public QDialog
{
    Q_OBJECT

public:
    ModifyDialogue(DBRecord *, QWidget * = NULL);

    virtual void setWindowTitle(const QString &);

    inline DBRecord * record() { return md_record; }

private slots:
    virtual void save();

protected:
    ModifyDialogue(QWidget * = NULL);
    void init(DBRecord *);
    void layoutInputWidgets();
    virtual void addMainGridLayout(QVBoxLayout *);

    void addInputWidget(MDInputWidget * iw) { md_inputwidgets << iw; }
    int inputWidgetCount() { return md_inputwidgets.count(); }
    void addWidget(QWidget * widget, int row, int column, Qt::Alignment alignment = 0);
    void addWidget(QWidget * widget, int fromRow, int fromColumn, int rowSpan, int columnSpan, Qt::Alignment alignment = 0);

    void setUsedIds(const QStringList & ids) { md_used_ids = ids; }

    QList<MDInputWidget *> md_inputwidgets;
    DBRecord * md_record;
    QStringList md_used_ids;
    QGridLayout * md_grid_main;

    friend class Customer;
    friend class Circuit;
    friend class Inspection;
    friend class Repair;
    friend class VariableRecord;
    friend class Table;
    friend class Inspector;
    friend class ServiceCompany;
    friend class RecordOfRefrigerantManagement;
    friend class WarningRecord;
    friend class AssemblyRecordType;
    friend class AssemblyRecordItemType;
};

#endif // MODIFY_DIALOGUE_H
