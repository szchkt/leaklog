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

class ImportCsvDialogue : public QDialog, private Ui::ImportCsvDialogue
{
    Q_OBJECT

public:
    ImportCsvDialogue(const QString & path, QWidget * parent = NULL);

    inline QList<QStringList> & fileContent() { return file_content; }
    inline QString table() const { return cb_table->itemData(cb_table->currentIndex(), Qt::UserRole).toString(); }
    QMap<QString, int> columnIndexMap();

private slots:
    void load();
    void loadTableColumns(int);
    void changeColumnIndex(QTreeWidgetItem *);

private:
    QString file_path;
    QList<QStringList> file_content;
};

#endif // IMPORT_CSV_DIALOGUE_H
