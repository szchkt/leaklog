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

#include "importcsvdialogue.h"

#include "csvparser/mtcsvparser.h"
#include "mtaddress.h"
#include "mtrecord.h"
#include "mtquery.h"

#include <QFile>
#include <QTextStream>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QTextCodec>
#endif
#include <QHeaderView>
#include <QMessageBox>
#include <QInputDialog>
#include <QDate>
#include <QMenu>

ImportCsvDialogue::ImportCsvDialogue(const QString &path, QList<ImportDialogueTable *> &tables, QWidget *parent):
QDialog(parent, Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowMaximizeButtonHint),
file_path(path)
{
    setupUi(this);
    this->tables = tables;
    id_bb->button(QDialogButtonBox::Ok)->setEnabled(false);
    id_bb->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));
    tw_content->verticalHeader()->setDefaultSectionSize(20);
    for (int i = 0; i < tables.count(); ++i) {
        cb_table->addItem(tables.at(i)->name(), tables.at(i)->id());
    }
    cb_encoding->addItem(tr("Unicode (UTF-8)"), "UTF-8");
    cb_encoding->addItem(tr("Central European (Windows 1250)"), "CP 1250");
    cb_encoding->addItem(tr("System default"), "System");
    cb_separator->addItem(tr("Comma"), ',');
    cb_separator->addItem(tr("Semicolon"), ';');
    cb_separator->addItem(tr("Tab"), '\t');
    loadTableColumns(0);
    trw_columns->header()->setStretchLastSection(false);
    trw_columns->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    trw_columns->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    QObject::connect(btn_reload, SIGNAL(clicked()), this, SLOT(load()));
    QObject::connect(cb_table, SIGNAL(currentIndexChanged(int)), this, SLOT(loadTableColumns(int)));
    QObject::connect(trw_columns, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(changeColumnIndex(QTreeWidgetItem *)));
};

ImportCsvDialogue::~ImportCsvDialogue()
{
    for (int i = tables.count() - 1; i >= 0; --i) {
        delete tables.takeAt(i);
    }
}

void ImportCsvDialogue::load()
{
    QString encoding = cb_encoding->currentData().toString();
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    std::optional<QStringConverter::Encoding> codec;
    if (encoding != "System") {
        codec = QStringConverter::encodingForName(encoding.toUtf8());
    }
#else
    QTextCodec *codec = NULL;
    if (encoding != "System") {
        codec = QTextCodec::codecForName(encoding.toUtf8());
    } else {
        codec = QTextCodec::codecForLocale();
    }
#endif

    QFile file(file_path);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::critical(this, tr("Import CSV - Leaklog"), tr("Cannot read file %1:\n%2.").arg(file_path).arg(file.errorString()));
        reject();
        return;
    }

    QTextStream in(&file);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    if (codec) {
        in.setEncoding(*codec);
    }
#else
    in.setCodec(codec);
#endif

    MTCSVParser parser(&in);
    parser.setSkipLines(spb_skip_lines->value());
    parser.setFieldSeparator(cb_separator->currentData().toChar());

    file_content.clear();
    int num_columns = 0, num_rows = 0;
    while (parser.hasNextRow()) {
        QStringList row = parser.nextRow();
        file_content << row;
        if (num_columns < row.count())
            num_columns = row.count();
        num_rows++;
    }

    tw_content->clear();
    tw_content->setColumnCount(num_columns);
    tw_content->setRowCount(num_rows);
    for (int r = 0; r < num_rows; ++r) {
        for (int c = 0; c < num_columns; ++c) {
            tw_content->setItem(r, c, new QTableWidgetItem(c >= file_content.at(r).count() ? "-----" : file_content.at(r).at(c)));
        }
    }

    updateHeader();

    id_bb->button(QDialogButtonBox::Ok)->setEnabled(true);
}

void ImportCsvDialogue::updateHeader()
{
    bool all_zeros = true;
    for (int i = 0; i < trw_columns->topLevelItemCount(); ++i) {
        if (trw_columns->topLevelItem(i)->text(1).toInt() > tw_content->columnCount())
            trw_columns->topLevelItem(i)->setText(1, "0");
        if (all_zeros && trw_columns->topLevelItem(i)->text(1).toInt())
            all_zeros = false;
    }
    if (all_zeros) {
        int count = qMin(tw_content->columnCount(), trw_columns->topLevelItemCount());
        for (int i = 0; i < count; ++i)
            trw_columns->topLevelItem(i)->setText(1, QString::number(i + 1));
    }
    bool found;
    for (int c = 0; c < tw_content->columnCount(); ++c) {
        found = false;
        for (int i = 0; i < trw_columns->topLevelItemCount(); ++i) {
            if (trw_columns->topLevelItem(i)->text(1).toInt() == c + 1) {
                if (tw_content->horizontalHeaderItem(c))
                    tw_content->horizontalHeaderItem(c)->setText(trw_columns->topLevelItem(i)->text(0));
                else
                    tw_content->setHorizontalHeaderItem(c, new QTableWidgetItem(trw_columns->topLevelItem(i)->text(0)));
                found = true;
                break;
            }
        }
        if (!found) {
            if (tw_content->horizontalHeaderItem(c))
                tw_content->horizontalHeaderItem(c)->setText(QString::number(c + 1));
            else
                tw_content->setHorizontalHeaderItem(c, new QTableWidgetItem(QString::number(c + 1)));
        }
    }
}

QTreeWidgetItem *columnItem(const QString &name, const QString &data, int &index) {
    QTreeWidgetItem *item = new QTreeWidgetItem(QStringList() << name << QString::number(index));
    item->setData(0, Qt::UserRole, data);
    index++;
    return item;
}

void ImportCsvDialogue::loadTableColumns(int index)
{
    trw_columns->clear();
    ImportDialogueTable *table = tables.at(index);
    current_table = table;
    if (!table) return;

    int n = 1;
    for (int i = 0; i < table->count(); ++i) {
        trw_columns->addTopLevelItem(columnItem(table->at(i)->name(), table->at(i)->id(), n));
    }

    for (int i = 0; i < table->childTablesCount(); ++i) {
        for (int k = 0; k < table->childTableAt(i)->count(); ++k) {
            trw_columns->addTopLevelItem(columnItem(table->childTableAt(i)->at(k)->name(), table->childTableAt(i)->at(k)->id(), n));
        }
    }

    QMenu *menu = btn_add_linked_table->menu();
    if (menu) {
        menu->clear();
    } else {
        menu = new QMenu(btn_add_linked_table);
        btn_add_linked_table->setMenu(menu);
        btn_add_linked_table->setPopupMode(QToolButton::InstantPopup);
        QObject::connect(menu, SIGNAL(triggered(QAction*)), this, SLOT(addLinkedTable(QAction*)));
    }

    for (int i = 0; i < table->childTemplatesCount(); ++i) {
        QAction *action = new QAction(table->childTemplateAt(i)->name(), menu);
        action->setData(i);
        menu->addAction(action);
    }

    updateHeader();
}

void ImportCsvDialogue::changeColumnIndex(QTreeWidgetItem *item)
{
    item->setText(1, QString::number(
            QInputDialog::getInt(this, tr("Change column index"),
                                 tr("Index of column %1 (zero if not present):").arg(item->text(0)),
                                 item->text(1).toInt(), 0, tw_content->columnCount())));
    updateHeader();
}

QMap<QString, int> ImportCsvDialogue::columnIndexMap()
{
    QMap<QString, int> column_index_map;
    for (int i = 0; i < trw_columns->topLevelItemCount(); ++i) {
        column_index_map.insert(trw_columns->topLevelItem(i)->data(0, Qt::UserRole).toString(),
                                trw_columns->topLevelItem(i)->text(1).toInt() - 1);
    }
    return column_index_map;
}

int ImportCsvDialogue::save()
{
    int num_failed = 0;
    ImportDialogueTableRow *row;
    foreach (QStringList values, fileContent()) {
        row = new ImportDialogueTableRow;
        int i;
        for (i = 0; i < current_table->count(); ++i) {
            int index = trw_columns->topLevelItem(i)->text(1).toInt() - 1;
            if (index < 0) continue;
            row->addValue(current_table->at(i), values.at(index));
        }

        for (int n = 0; n < current_table->childTablesCount(); ++n) {
            for (int k = 0; k < current_table->childTableAt(n)->count(); ++k, ++i) {
                int index = trw_columns->topLevelItem(i)->text(1).toInt() - 1;
                if (index < 0) continue;
                row->addValue(current_table->childTableAt(n)->at(k), values.at(index));
            }
        }

        if (!current_table->save(row)) num_failed++;

        delete row;
    }
    return num_failed;
}

void ImportCsvDialogue::addLinkedTable(QAction *action)
{
    ImportDialogueTable *table = current_table->addChildTable(action->data().toInt());
    if (!table) return;

    for (int i = 0; i < table->count(); ++i) {
        int n = 0;
        trw_columns->addTopLevelItem(columnItem(table->at(i)->name(), table->at(i)->id(), n));
    }

    updateHeader();
}

ImportDialogueTable::~ImportDialogueTable()
{
    int i;
    for (i = columns.count() - 1; i >= 0; --i) {
        delete columns.takeAt(i);
    }
    for (i = child_tables.count() - 1; i >= 0; --i) {
        delete child_tables.takeAt(i);
    }
    for (i = child_templates.count() - 1; i >= 0; --i) {
        delete child_templates.takeAt(i);
    }
}

ImportDialogueTableColumn *ImportDialogueTable::addColumn(const QString &name, const QString &id, int type)
{
    ImportDialogueTableColumn *col = new ImportDialogueTableColumn(name, id, (ImportDialogueTableColumn::Type)type);
    columns.append(col);

    return col;
}

ImportDialogueTableColumn *ImportDialogueTable::addForeignKeyColumn(const QString &name, const QString &id, const QString &foreign_key_table, const QString &foreign_key_column)
{
    ImportDialogueTableColumn *col = addColumn(name, id, ImportDialogueTableColumn::ForeignKey);
    col->setForeignKey(foreign_key_table, foreign_key_column);
    return col;
}

void ImportDialogueTable::addColumn(ImportDialogueTableColumn *column)
{
    columns.append(column);
}

ImportDialogueTableTemplate *ImportDialogueTable::addChildTableTemplate(const QString &name, const QString &id, const QString &parent_column)
{
    ImportDialogueTableTemplate *table = new ImportDialogueTableTemplate(name, id);
    table->setParentColumn(parent_column);

    child_templates.append(table);

    return table;
}

ImportDialogueTable *ImportDialogueTable::addChildTable(int i)
{
    ImportDialogueTableTemplate *table_template = childTemplateAt(i);
    if (!table_template) return NULL;

    ImportDialogueTable *table = table_template->table();
    child_tables.append(table);

    return table;
}

bool ImportDialogueTable::save(ImportDialogueTableRow *row, const QString &parent_uuid)
{
    QVariantMap set;
    bool ok = true;
    MTAddress address;

    bool all_empty = true;

    for (int i = 0; i < columns.count(); ++i) {
        if (!row->contains(columns.at(i))) continue;

        if (row->value(columns.at(i)).isNull()) continue;
        else all_empty = false;

        switch (columns.at(i)->type()) {
        case ImportDialogueTableColumn::ID: {
            set.insert(columns.at(i)->id(), row->value(columns.at(i)));
            QVariantMap record = MTQuery(id(), {
                {columns.at(i)->id(), row->value(columns.at(i))}
            }).list("uuid", "uuid");
            if (!record.isEmpty()) {
                set.insert("uuid", record.value("uuid"));
            }
            break;
        }

        case ImportDialogueTableColumn::ForeignKey: {
            QVariantMap record = MTQuery(columns.at(i)->foreignKeyTable(), {
                {columns.at(i)->foreignKeyColumn(), row->value(columns.at(i))}
            }).list("uuid", "uuid");
            if (record.isEmpty()) {
                ok = false;
            } else {
                set.insert(columns.at(i)->id(), record.value("uuid"));
            }
            break;
        }

        case ImportDialogueTableColumn::Integer: {
            bool ok = true;
            int int_value = row->value(columns.at(i)).toInt(&ok);
            if (ok)
                set.insert(columns.at(i)->id(), int_value);
            break;
        }

        case ImportDialogueTableColumn::Text:
            set.insert(columns.at(i)->id(), row->value(columns.at(i)));
            break;

        case ImportDialogueTableColumn::Numeric: {
            QString string_value = row->value(columns.at(i)).toString().simplified().remove(' ');
            if (string_value.contains(',') && !string_value.contains('.'))
                string_value.replace(',', '.');
            while (string_value.count('.') > 1)
                string_value.remove(string_value.indexOf('.'), 1);
            bool ok = true;
            double numeric_value = string_value.toDouble(&ok);
            if (ok)
                set.insert(columns.at(i)->id(), numeric_value);
            break;
        }

        case ImportDialogueTableColumn::Boolean: {
            QString string_value = row->value(columns.at(i)).toString().toLower().simplified().remove(' ');
            bool ok = true;
            int int_value = !!string_value.toInt(&ok);
            if (!ok) {
                int_value = (string_value == QObject::tr("Yes").toLower() ||
                             string_value == "yes" || string_value == "true");
            }
            set.insert(columns.at(i)->id(), int_value);
            break;
        }

        case ImportDialogueTableColumn::Date: {
            QString string_value = row->value(columns.at(i)).toString().simplified().remove(' ');
            QDate date_value = QDate::fromString(string_value, DATE_FORMAT);
            if (!date_value.isValid())
                date_value = QDate::fromString(string_value, "d.M.yyyy");
            if (date_value.isValid())
                set.insert(columns.at(i)->id(), date_value.toString(DATE_FORMAT));
            break;
        }

        case ImportDialogueTableColumn::Select: {
            QString string_value = row->value(columns.at(i)).toString().toLower().simplified();
            set.insert(columns.at(i)->id(), columns.at(i)->selectValue(row->value(columns.at(i)).toString()));
            break;
        }

        case ImportDialogueTableColumn::AddressCity:
            address.setCity(row->value(columns.at(i)).toString());
            break;

        case ImportDialogueTableColumn::AddressStreet:
            address.setStreet(row->value(columns.at(i)).toString());
            break;

        case ImportDialogueTableColumn::AddressPostalCode:
            address.setPostalCode(row->value(columns.at(i)).toString());
            break;
        }
    }

    if (all_empty) return ok;

    if (!parent_column.isEmpty()) {
        if (parent_uuid.isEmpty()) {
            ok = false;
        } else {
            set.insert(parent_column, parent_uuid);
        }
    }

    if (!address.city().isEmpty() || !address.street().isEmpty() || !address.postalCode().isEmpty()) {
        set.insert("address", address.toString());
    }

    MTRecord record(id(), set.value("uuid").toString());
    ok = ok && record.update(set);

    for (int i = 0; i < child_tables.count(); ++i) {
        ok = ok && child_tables.at(i)->save(row, record.uuid());
    }

    return ok;
}

QVariant ImportDialogueTableRow::value(ImportDialogueTableColumn *col)
{
    return cells.value(col);
}

void ImportDialogueTableRow::addValue(ImportDialogueTableColumn *key, const QVariant &value)
{
    cells.insert(key, value);
}

ImportDialogueTable *ImportDialogueTableTemplate::table()
{
    ImportDialogueTable *table = new ImportDialogueTableTemplate(name(), id());
    for (int i = 0; i < columns.count(); ++i) {
        table->addColumn(new ImportDialogueTableColumn(columns.at(i)));
    }
    table->setParentColumn(parent_column);

    return table;
}
