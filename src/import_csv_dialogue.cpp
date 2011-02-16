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

#include <QFile>
#include <QTextStream>
#include <QTextCodec>
#include <QHeaderView>
#include <QMessageBox>
#include <QInputDialog>

#undef QT_TRANSLATE_NOOP
#define QT_TRANSLATE_NOOP(context, sourceText) context, sourceText

ImportCsvDialogue::ImportCsvDialogue(const QString & path, QWidget * parent):
QDialog(parent, Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowMaximizeButtonHint),
file_path(path)
{
    setupUi(this);
    id_bb->button(QDialogButtonBox::Ok)->setEnabled(false);
    id_bb->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));
    tw_content->verticalHeader()->setDefaultSectionSize(20);
    cb_table->addItem(tr("Customers"), "customers");
    cb_table->addItem(tr("Circuits"), "circuits");
    cb_table->addItem(tr("Circuit unit types"), "circuit_unit_types");
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

QTreeWidgetItem * columnItem(const char * context, const char * text, const QString & data, int & index) {
    QTreeWidgetItem * item = new QTreeWidgetItem(QStringList() << QApplication::translate(context, text) << QString::number(index));
    item->setData(0, Qt::UserRole, data);
    index++;
    return item;
}

void ImportCsvDialogue::loadTableColumns(int index)
{
    trw_columns->clear();
    QString table = cb_table->itemData(index, Qt::UserRole).toString();
    int i = 1;
    if (table == "customers") {
        trw_columns->addTopLevelItem(columnItem(QT_TRANSLATE_NOOP("Customer", "ID"), "id", i));
        trw_columns->addTopLevelItem(columnItem(QT_TRANSLATE_NOOP("Customer", "Company"), "company", i));
        trw_columns->addTopLevelItem(columnItem(QT_TRANSLATE_NOOP("Customer", "Contact person"), "contact_person", i));
        trw_columns->addTopLevelItem(columnItem(QT_TRANSLATE_NOOP("MTAddressEdit", "Street"), "street", i));
        trw_columns->addTopLevelItem(columnItem(QT_TRANSLATE_NOOP("MTAddressEdit", "City"), "city", i));
        trw_columns->addTopLevelItem(columnItem(QT_TRANSLATE_NOOP("MTAddressEdit", "Postal code"), "postal_code", i));
        trw_columns->addTopLevelItem(columnItem(QT_TRANSLATE_NOOP("Customer", "E-mail"), "mail", i));
        trw_columns->addTopLevelItem(columnItem(QT_TRANSLATE_NOOP("Customer", "Phone"), "phone", i));
    } else if (table == "circuits") {
        trw_columns->addTopLevelItem(columnItem(QT_TRANSLATE_NOOP("Circuit", "Customer ID"), "parent", i));
        trw_columns->addTopLevelItem(columnItem(QT_TRANSLATE_NOOP("Circuit", "ID"), "id", i));
        trw_columns->addTopLevelItem(columnItem(QT_TRANSLATE_NOOP("Circuit", "Circuit name"), "name", i));
        trw_columns->addTopLevelItem(columnItem(QT_TRANSLATE_NOOP("Circuit", "Disused"), "disused", i));
        trw_columns->addTopLevelItem(columnItem(QT_TRANSLATE_NOOP("Circuit", "Place of operation"), "operation", i));
        trw_columns->addTopLevelItem(columnItem(QT_TRANSLATE_NOOP("Circuit", "Building"), "building", i));
        trw_columns->addTopLevelItem(columnItem(QT_TRANSLATE_NOOP("Circuit", "Device"), "device", i));
        trw_columns->addTopLevelItem(columnItem(QT_TRANSLATE_NOOP("Circuit", "Hermetically sealed"), "hermetic", i));
        trw_columns->addTopLevelItem(columnItem(QT_TRANSLATE_NOOP("Circuit", "Manufacturer"), "manufacturer", i));
        trw_columns->addTopLevelItem(columnItem(QT_TRANSLATE_NOOP("Circuit", "Type"), "type", i));
        trw_columns->addTopLevelItem(columnItem(QT_TRANSLATE_NOOP("Circuit", "Serial number"), "sn", i));
        trw_columns->addTopLevelItem(columnItem(QT_TRANSLATE_NOOP("Circuit", "Year of purchase"), "year", i));
        trw_columns->addTopLevelItem(columnItem(QT_TRANSLATE_NOOP("Circuit", "Date of commissioning"), "commissioning", i));
        trw_columns->addTopLevelItem(columnItem(QT_TRANSLATE_NOOP("Circuit", "Field of application"), "field", i));
        trw_columns->addTopLevelItem(columnItem(QT_TRANSLATE_NOOP("Circuit", "Refrigerant"), "refrigerant", i));
        trw_columns->addTopLevelItem(columnItem(QT_TRANSLATE_NOOP("Circuit", "Amount of refrigerant"), "refrigerant_amount", i));
        trw_columns->addTopLevelItem(columnItem(QT_TRANSLATE_NOOP("Circuit", "Oil"), "oil", i));
        trw_columns->addTopLevelItem(columnItem(QT_TRANSLATE_NOOP("Circuit", "Amount of oil"), "oil_amount", i));
        trw_columns->addTopLevelItem(columnItem(QT_TRANSLATE_NOOP("Circuit", "Fixed leakage detector installed"), "leak_detector", i));
        trw_columns->addTopLevelItem(columnItem(QT_TRANSLATE_NOOP("Circuit", "Run-time per day"), "runtime", i));
        trw_columns->addTopLevelItem(columnItem(QT_TRANSLATE_NOOP("Circuit", "Rate of utilisation"), "utilisation", i));
        trw_columns->addTopLevelItem(columnItem(QT_TRANSLATE_NOOP("Circuit", "Inspection interval"), "inspection_interval", i));
    } else if (table == "circuit_unit_types") {
        trw_columns->addTopLevelItem(columnItem(QT_TRANSLATE_NOOP("CircuitUnitType", "ID"), "id", i));
        trw_columns->addTopLevelItem(columnItem(QT_TRANSLATE_NOOP("CircuitUnitType", "Manufacturer"), "manufacturer", i));
        trw_columns->addTopLevelItem(columnItem(QT_TRANSLATE_NOOP("CircuitUnitType", "Type"), "type", i));
        trw_columns->addTopLevelItem(columnItem(QT_TRANSLATE_NOOP("CircuitUnitType", "Refrigerant"), "refrigerant", i));
        trw_columns->addTopLevelItem(columnItem(QT_TRANSLATE_NOOP("CircuitUnitType", "Amount of refrigerant"), "refrigerant_amount", i));
        trw_columns->addTopLevelItem(columnItem(QT_TRANSLATE_NOOP("CircuitUnitType", "Oil"), "oil", i));
        trw_columns->addTopLevelItem(columnItem(QT_TRANSLATE_NOOP("CircuitUnitType", "Amount of oil"), "oil_amount", i));
        trw_columns->addTopLevelItem(columnItem(QT_TRANSLATE_NOOP("CircuitUnitType", "Acquisition price"), "acquisition_price", i));
        trw_columns->addTopLevelItem(columnItem(QT_TRANSLATE_NOOP("CircuitUnitType", "List price"), "list_price", i));
        trw_columns->addTopLevelItem(columnItem(QT_TRANSLATE_NOOP("CircuitUnitType", "Location"), "location", i));
        trw_columns->addTopLevelItem(columnItem(QT_TRANSLATE_NOOP("CircuitUnitType", "Category ID"), "category_id", i));
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
