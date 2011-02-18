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

#include "import_csv_dialogue.h"

#include "csvparser/mtcsvparser.h"
#include "mtaddress.h"
#include "mtrecord.h"

#include <QFile>
#include <QTextStream>
#include <QTextCodec>
#include <QHeaderView>
#include <QMessageBox>
#include <QInputDialog>
#include <QDate>

#undef QT_TRANSLATE_NOOP
#define QT_TRANSLATE_NOOP(context, sourceText) context, sourceText

ImportCsvDialogue::ImportCsvDialogue(const QString & path, QList<ImportDialogueTable *> & tables, QWidget * parent):
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
    QObject::connect(btn_reload, SIGNAL(clicked()), this, SLOT(load()));
    QObject::connect(cb_table, SIGNAL(currentIndexChanged(int)), this, SLOT(loadTableColumns(int)));
    QObject::connect(trw_columns, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(changeColumnIndex(QTreeWidgetItem *)));
};

void ImportCsvDialogue::load()
{
    QString encoding = cb_encoding->itemData(cb_encoding->currentIndex(), Qt::UserRole).toString();
    QTextCodec * codec = NULL;
    if (encoding != "System") {
        codec = QTextCodec::codecForName(encoding.toUtf8());
    } else {
        codec = QTextCodec::codecForLocale();
    }

    QFile file(file_path);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::critical(this, tr("Import CSV - Leaklog"), tr("Cannot read file %1:\n%2.").arg(file_path).arg(file.errorString()));
        reject();
        return;
    }

    QTextStream in(&file);
    in.setCodec(codec);

    MTCSVParser parser(&in);
    parser.setSkipLines(spb_skip_lines->value());
    parser.setFieldSeparator(cb_separator->itemData(cb_separator->currentIndex(), Qt::UserRole).toChar());

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

QTreeWidgetItem * columnItem(const QString & name, const QString & data, int & index) {
    QTreeWidgetItem * item = new QTreeWidgetItem(QStringList() << name << QString::number(index));
    item->setData(0, Qt::UserRole, data);
    index++;
    return item;
}

void ImportCsvDialogue::loadTableColumns(int index)
{
    trw_columns->clear();
    ImportDialogueTable * table = tables.at(index);
    current_table = table;
    if (!table) return;

    int n = 1;
    for (int i = 0; i < table->count(); ++i) {
        trw_columns->addTopLevelItem(columnItem(table->at(i)->name(), table->at(i)->id(), n));
    }

    updateHeader();
}

void ImportCsvDialogue::changeColumnIndex(QTreeWidgetItem * item)
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
    ImportDialogueTableRow * row;
    foreach (QStringList values, fileContent()) {
        row = new ImportDialogueTableRow;
        for (int i = 0; i < current_table->count(); ++i) {
            int index = trw_columns->topLevelItem(i)->text(1).toInt() - 1;
            if (index < 0) continue;
            row->addValue(current_table->at(i), values.at(index));
        }
        if (!current_table->save(row)) num_failed++;
        delete row;
    }
    return num_failed;
}

ImportDialogueTableColumn * ImportDialogueTable::addColumn(const QString & name, const QString & id, int type)
{
    ImportDialogueTableColumn * col = new ImportDialogueTableColumn(name, id, type);
    columns.append(col);

    return col;
}

ImportDialogueTable * ImportDialogueTable::addChildTable(const QString & name, const QString & id, const QStringList & parent_cols)
{
    ImportDialogueTable * table = new ImportDialogueTable(name, id);
    ImportDialogueTableColumn * col;

    for (int i = 0; i < parent_cols.count(); ++i) {
        col = NULL;
        for (int n = 0; n < columns.count(); ++n) {
            if (columns.at(n)->id() == parent_cols.at(i)) {
                col = columns.at(n);
                break;
            }
        }
        if (col) table->addParentColumn(col);
    }

    child_tables.append(table);

    return table;
}

void ImportDialogueTable::addParentColumn(ImportDialogueTableColumn * col)
{
    parent_columns.append(col);
}

bool ImportDialogueTable::save(ImportDialogueTableRow * row)
{
    QVariantMap set;
    QString string_value;
    bool ok = true;
    int int_value;
    double numeric_value;
    QDate date_value;
    QList<QString> id_columns;

    QString address_street, address_city, address_postal_code;

    for (int i = 0; i < columns.count(); ++i) {
        if (!row->contains(columns.at(i))) continue;

        switch (columns.at(i)->type()) {
        case ImportDialogueTableColumn::ID:
            id_columns.append(columns.at(i)->id());
            set.insert(columns.at(i)->id(), row->value(columns.at(i)));
            break;

        case ImportDialogueTableColumn::Integer:
            int_value = row->value(columns.at(i)).toInt(&ok);
            /*if (foreign_keys.contains(column)) {
                if (!ok) break;
                QStringList foreign_key = foreign_keys.value(column);
                MTRecord record(foreign_key.value(0), foreign_key.value(1), QString::number(value), MTDictionary());
                if (!record.exists()) {
                    ok = false;
                    break;
                }
                parents.insert(column, QString::number(value));
            } else {*/
                if (ok)
                    set.insert(columns.at(i)->id(), int_value);
                else
                    ok = true;
            //}
            break;

        case ImportDialogueTableColumn::Text:
            set.insert(columns.at(i)->id(), row->value(columns.at(i)));
            break;

        case ImportDialogueTableColumn::Numeric:
            string_value = row->value(columns.at(i)).toString().simplified().remove(' ');
            if (string_value.contains(',') && !string_value.contains('.'))
                string_value.replace(',', '.');
            while (string_value.count('.') > 1)
                string_value.remove(string_value.indexOf('.'), 1);
            numeric_value = string_value.toDouble(&ok);
            if (ok)
                set.insert(columns.at(i)->id(), numeric_value);
            break;

        case ImportDialogueTableColumn::Boolean:
            string_value = row->value(columns.at(i)).toString().toLower().simplified().remove(' ');
            int_value = ((string_value.toInt(&ok) && ok) || (!ok &&
                            (string_value == QObject::tr("Yes").toLower() ||
                         string_value == "yes" || string_value == "true")));
            set.insert(columns.at(i)->id(), int_value);
            break;

        case ImportDialogueTableColumn::Date:
            string_value = row->value(columns.at(i)).toString().simplified().remove(' ');
            date_value = QDate::fromString(string_value, "yyyy.MM.dd");
            if (!date_value.isValid())
                date_value = QDate::fromString(string_value, "d.M.yyyy");
            if (date_value.isValid())
                set.insert(columns.at(i)->id(), date_value.toString("yyyy.MM.dd"));
            break;

        case ImportDialogueTableColumn::Select:
            string_value = row->value(columns.at(i)).toString().toLower().simplified();
            set.insert(columns.at(i)->id(), columns.at(i)->selectValue(row->value(columns.at(i)).toString()));
            break;

        case ImportDialogueTableColumn::AddressCity:
            address_city = row->value(columns.at(i)).toString();
            break;

        case ImportDialogueTableColumn::AddressStreet:
            address_street = row->value(columns.at(i)).toString();
            break;

        case ImportDialogueTableColumn::AddressPostalCode:
            address_postal_code = row->value(columns.at(i)).toString();
            break;

        default:
            break;
        }
    }

    if (!address_city.isEmpty() || !address_street.isEmpty() || !address_postal_code.isEmpty()) {
        MTAddress address;
        address.setStreet(address_street);
        address.setCity(address_city);
        address.setPostalCode(address_postal_code);
        set.insert("address", address.toString());
    }

    MTRecord record(id(), id_columns.first(), set.value(id_columns.first()).toString(), MTDictionary());
    ok = ok && record.update(set);

    for (int i = 0; i < child_tables.count(); ++i) {
        child_tables.at(i)->save(row);
    }

    return ok;
}

QVariant ImportDialogueTableRow::value(ImportDialogueTableColumn * col)
{
    return cells.value(col);
}

void ImportDialogueTableRow::addValue(ImportDialogueTableColumn * key, const QVariant & value)
{
    cells.insert(key, value);
}
