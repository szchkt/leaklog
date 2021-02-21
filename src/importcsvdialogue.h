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

#ifndef IMPORT_CSV_DIALOGUE_H
#define IMPORT_CSV_DIALOGUE_H

#include "ui_importcsvdialogue.h"

#include "mtdictionary.h"

class ImportDialogueTable;
class ImportDialogueTableColumn;
class ImportDialogueTableRow;
class ImportDialogueTableTemplate;
class MTAddress;

class ImportCsvDialogue : public QDialog, private Ui::ImportCsvDialogue
{
    Q_OBJECT

public:
    ImportCsvDialogue(const QString &path, QList<ImportDialogueTable *> &, QWidget *parent = NULL);
    ~ImportCsvDialogue();

    inline QList<QStringList> &fileContent() { return file_content; }
    inline QString table() const { return cb_table->currentData().toString(); }
    QMap<QString, int> columnIndexMap();
    int save();

private slots:
    void load();
    void loadTableColumns(int);
    void changeColumnIndex(QTreeWidgetItem *);
    void addLinkedTable(QAction *);

private:
    void updateHeader();

    QString file_path;
    QList<QStringList> file_content;
    QList<ImportDialogueTable *> tables;
    ImportDialogueTable *current_table;
};

class ImportDialogueTable
{
public:
    ImportDialogueTable(const QString &t_name, const QString &t_id) {
        this->t_name = t_name; this->t_id = t_id;
    }
    ~ImportDialogueTable();

    const QString &name() { return t_name; }
    const QString &id() { return t_id; }

    ImportDialogueTableColumn *addColumn(const QString &, const QString &, int);
    ImportDialogueTableColumn *addForeignKeyColumn(const QString &name, const QString &id, const QString &foreign_key_table, const QString &foreign_key_column);
    void addColumn(ImportDialogueTableColumn *);
    ImportDialogueTableTemplate *addChildTableTemplate(const QString &name, const QString &id, const QString &parent_column);
    ImportDialogueTable *addChildTable(int);

    void setParentColumn(const QString &other) { this->parent_column = other; }
    void setColumns(const QList<ImportDialogueTableColumn *> &columns) { this->columns = columns; }

    bool save(ImportDialogueTableRow *row, const QString &parent_uuid = QString());

    int count() { return columns.count(); }
    ImportDialogueTableColumn *at(int i) { return columns.at(i); }

    int childTemplatesCount() { return child_templates.count(); }
    ImportDialogueTableTemplate *childTemplateAt(int i) { return child_templates.at(i); }

    int childTablesCount() { return child_tables.count(); }
    ImportDialogueTable *childTableAt(int i) { return child_tables.at(i); }

protected:
    QString t_name;
    QString t_id;

    QList<ImportDialogueTableColumn *> columns;
    QString parent_column;
    QList<ImportDialogueTable *> child_tables;
    QList<ImportDialogueTableTemplate *> child_templates;
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
        AddressPostalCode,
        ForeignKey
    };

    ImportDialogueTableColumn(const QString &c_name, const QString &c_id, Type c_type) {
        this->c_name = c_name; this->c_id = c_id; this->c_type = c_type;
    }
    ImportDialogueTableColumn(ImportDialogueTableColumn *other) {
        this->c_name = other->name(); this->c_id = other->id();
        this->c_type = other->type(); this->select_vals = other->selectValues();
        this->foreign_key_table = other->foreignKeyTable();
        this->foreign_key_column = other->foreignKeyColumn();
    }

    const QString &name() { return c_name; }
    const QString &id() { return c_id; }
    Type type() { return c_type; }

    void addSelectValue(const QString &key, const QString &value) { select_vals.insert(key, value); }
    const QString selectValue(const QString &key) { return select_vals.value(key, key); }
    const QMap<QString, QString> &selectValues() { return select_vals; }

    void setForeignKey(const QString &table, const QString &column) {
        this->foreign_key_table = table;
        this->foreign_key_column = column;
    }
    const QString &foreignKeyTable() { return foreign_key_table; }
    const QString &foreignKeyColumn() { return foreign_key_column; }

private:
    QString c_name;
    QString c_id;

    QString foreign_key_table;
    QString foreign_key_column;

    QMap<QString, QString> select_vals;

    Type c_type;
};

class ImportDialogueTableRow
{
public:
    void addValue(ImportDialogueTableColumn *, const QVariant &);
    QVariant value(ImportDialogueTableColumn *);

    bool contains(ImportDialogueTableColumn *col) { return cells.contains(col); }

private:
    QMap<ImportDialogueTableColumn *, QVariant> cells;
};

class ImportDialogueTableTemplate : public ImportDialogueTable
{
public:
    ImportDialogueTableTemplate(const QString &t_name, const QString &t_id)
        : ImportDialogueTable(t_name, t_id) {}

    ImportDialogueTable *table();
};

#endif // IMPORT_CSV_DIALOGUE_H
