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

#ifndef IMPORT_CSV_DIALOGUE_H
#define IMPORT_CSV_DIALOGUE_H

#include "ui_import_csv_dialogue.h"

class ImportDialogueTable;
class ImportDialogueTableColumn;
class ImportDialogueTableRow;

class ImportCsvDialogue : public QDialog, private Ui::ImportCsvDialogue
{
    Q_OBJECT

public:
    ImportCsvDialogue(const QString & path, QList<ImportDialogueTable *> &, QWidget * parent = NULL);

    inline QList<QStringList> & fileContent() { return file_content; }
    inline QString table() const { return cb_table->itemData(cb_table->currentIndex(), Qt::UserRole).toString(); }
    QMap<QString, int> columnIndexMap();
    int save();

private slots:
    void load();
    void loadTableColumns(int);
    void changeColumnIndex(QTreeWidgetItem *);

private:
    void updateHeader();

    QString file_path;
    QList<QStringList> file_content;
    QList<ImportDialogueTable *> tables;
    ImportDialogueTable * current_table;
};

class ImportDialogueTable
{
public:
    ImportDialogueTable(const QString & t_name, const QString & t_id)
        {  this->t_name = t_name; this->t_id = t_id; }

    const QString & name() { return t_name; }
    const QString & id() { return t_id; }

    ImportDialogueTableColumn * addColumn(const QString &, const QString &, int);
    ImportDialogueTable * addChildTable(const QString &, const QString &, const QStringList &);

    void addParentColumn(ImportDialogueTableColumn *);

    bool save(ImportDialogueTableRow *);

    int count() { return columns.count(); }
    ImportDialogueTableColumn * at(int i) { return columns.at(i); }

private:
    QString t_name;
    QString t_id;

    QList<ImportDialogueTableColumn *> columns;
    QList<ImportDialogueTableColumn *> parent_columns;
    QList<ImportDialogueTable *> child_tables;
};

class ImportDialogueTableColumn
{
public:
    enum Type {
        ID,
        Integer,
        Numeric,
        Boolean,
        Text,
        Date,
        Select,
        AddressStreet,
        AddressCity,
        AddressPostalCode
    };

    ImportDialogueTableColumn(const QString & c_name, const QString & c_id, int c_type)
        {   this->c_name = c_name; this->c_id = c_id; this->c_type = c_type; }

    const QString & name() { return c_name; }
    const QString & id() { return c_id; }
    int type() { return c_type; }

    void addSelectValue(const QString & key, const QString & value) { select_vals.insert(key, value); }
    const QString selectValue(const QString & key) { return select_vals.value(key, key); }

private:
    QString c_name;
    QString c_id;

    QMap<QString, QString> select_vals;

    int c_type;
    int index;
};

class ImportDialogueTableRow
{
public:
    void addValue(ImportDialogueTableColumn *, const QVariant &);
    QVariant value(ImportDialogueTableColumn *);

    bool contains(ImportDialogueTableColumn * col) { return cells.contains(col); }

private:
    QMap<ImportDialogueTableColumn *, QVariant> cells;
};

#endif // IMPORT_CSV_DIALOGUE_H
