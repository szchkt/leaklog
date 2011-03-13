/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2011 Matus & Michal Tomlein

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

#include "modify_dialogue_table.h"

#include "records.h"
#include "global.h"

#include <QGridLayout>
#include <QLineEdit>
#include <QLabel>
#include <QMap>
#include <QComboBox>
#include <QToolButton>
#include <QPushButton>

ModifyDialogueTable::ModifyDialogueTable(const QString & name, const QList<ModifyDialogueTableCell *> & header, QWidget * parent):
        QGroupBox(name, parent)
{
    this->header = header;
    visible_rows = 0;

    layout = new QVBoxLayout(this);
    grid = new QGridLayout;
    grid->setContentsMargins(0, 0, 0, 0);
    layout->addLayout(grid);
    layout->setContentsMargins(9, 9, 9, 9);
    layout->setSpacing(6);

    createHeader();
}

ModifyDialogueTable::~ModifyDialogueTable()
{
    for (int i = rows.count() - 1; i >= 0; --i) {
        delete rows.takeAt(i);
    }
    for (int i = header.count() - 1; i >= 0; --i) {
        delete header.takeAt(i);
    }

    delete grid;
}

void ModifyDialogueTable::createHeader()
{
    for (int i = 0; i < header.count(); ++i) {
        grid->addWidget(new QLabel(QString("<b>%1</b>").arg(header.at(i)->value().toString())), visible_rows, i);
    }
    visible_rows++;
}

void ModifyDialogueTable::addRow(const QMap<QString, ModifyDialogueTableCell *> & values, bool display)
{
    ModifyDialogueTableRow * row = new ModifyDialogueTableRow(values, display);
    QObject::connect(row, SIGNAL(removed(ModifyDialogueTableRow*)), this, SLOT(rowRemoved(ModifyDialogueTableRow*)));
    rows.append(row);

    if (!display) {
        addHiddenRow(row);
    } else {
        addRow(row);
    }
}

void ModifyDialogueTable::addRow(ModifyDialogueTableRow * row)
{
    MDTInputWidget * iw;

    ModifyDialogueTableCell * cell;
    for (int i = 0; i < header.count(); ++i) {
        cell = row->valuesMap().value(header.at(i)->id());

        switch (cell->dataType()) {
        case Global::Boolean:
            iw = new MDTCheckBox(cell->value().toBool(), this);
            break;

        case Global::String:
            iw = new MDTLineEdit(cell->value().toString(), this);
            break;

        case Global::Text:
            iw = new MDTPlainTextEdit(cell->value().toString(), this);
            break;

        case Global::Integer:
            iw = new MDTSpinBox(this);
            ((MDTSpinBox *) iw)->setValue(cell->value().toInt());
            ((MDTDoubleSpinBox *) iw)->setSuffix(cell->unit());
            break;

        case Global::Numeric:
            iw = new MDTDoubleSpinBox(this);
            ((MDTDoubleSpinBox *) iw)->setValue(cell->value().toDouble());
            ((MDTDoubleSpinBox *) iw)->setSuffix(cell->unit());
            break;

        case Global::File:
            iw = new MDTFileChooser(cell->value().toInt(), this);
            break;

        default:
            iw = new MDTLabel(cell->value().toString(), this);
            break;
        }

        row->addWidget(header.at(i)->id(), iw);
        grid->addWidget(iw->widget(), visible_rows, i);
    }
    grid->addWidget(row->removeButton(), visible_rows, header.count());
    visible_rows++;
}

void ModifyDialogueTable::rowRemoved(ModifyDialogueTableRow * row)
{
    if (row->toBeDeleted()) {
        for (int i = 0; i < rows.count(); ++i) {
            if (rows.at(i) == row) {
                delete rows.takeAt(i);
                break;
            }
        }
    }
    else addHiddenRow(row);
}

void ModifyDialogueTable::addNewRow()
{
    QMap<QString, ModifyDialogueTableCell *> cells_map;
    for (int i = 0; i < header.count(); ++i) {
        cells_map.insert(header.at(i)->id(), new ModifyDialogueTableCell(QVariant(), header.at(i)->dataType()));
    }
    QList<ModifyDialogueTableCell *> other_cells = hiddenAttributes();
    for (int i = 0; i < other_cells.count(); ++i) {
        cells_map.insert(other_cells.at(i)->id(), other_cells.at(i));
    }
    addRow(cells_map, true);
}

QList<MTDictionary> ModifyDialogueTable::allValues()
{
    QList<MTDictionary> values;

    for (int i = 0; i < rows.count(); ++i) {
        if (rows.at(i)->isInTable())
            values.append(rows.at(i)->dictValues());
    }

    return values;
}

ModifyDialogueAdvancedTable::ModifyDialogueAdvancedTable(const QString &name, int category_id, const QList<ModifyDialogueTableCell *> & header, QWidget * parent):
        ModifyDialogueTable(name, header, parent)
{
    this->category_id = category_id;
    smallest_index = -1;

    QToolButton * add_new_btn = new QToolButton;
    add_new_btn->setIcon(QIcon(QString::fromUtf8(":/images/images/add16.png")));
    QObject::connect(add_new_btn, SIGNAL(clicked()), this, SLOT(addNewRow()));
    grid->addWidget(add_new_btn, 0, header.count());

    layout->addLayout(addRowControlsLayout());
}

QLayout * ModifyDialogueAdvancedTable::addRowControlsLayout()
{
    QHBoxLayout * layout = new QHBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    add_row_cb = new QComboBox(this);

    layout->addWidget(add_row_cb);

    QPushButton * add_btn = new QPushButton(tr("Add"));
    QObject::connect(add_btn, SIGNAL(clicked()), this, SLOT(activateRow()));
    layout->addWidget(add_btn);

    layout->addStretch();

    return layout;
}

void ModifyDialogueAdvancedTable::addHiddenRow(ModifyDialogueTableRow * row)
{
    add_row_cb->addItem(row->value("name"), row->valuesMap().value("item_type_id")->value());
}

void ModifyDialogueAdvancedTable::activateRow()
{
    ModifyDialogueTableRow * row = NULL;
    QString type_id = add_row_cb->itemData(add_row_cb->currentIndex()).toString();

    for (int i = 0; i < rows.count(); ++i) {
        if (!rows.at(i)->isInTable() && rows.at(i)->value("item_type_id") == type_id) {
            row = rows.at(i);
            break;
        }
    }

    if (!row) return;
    row->setInTable(true);
    addRow(row);
    add_row_cb->removeItem(add_row_cb->currentIndex());
}

QList<ModifyDialogueTableCell *> ModifyDialogueAdvancedTable::hiddenAttributes()
{
    QList<ModifyDialogueTableCell *> attrs;
    ModifyDialogueTableCell * cell = new ModifyDialogueTableCell(category_id);
    cell->setId("category_id");
    attrs.append(cell);
    cell = new ModifyDialogueTableCell(smallest_index--);
    cell->setId("item_type_id");
    attrs.append(cell);
    cell = new ModifyDialogueTableCell(AssemblyRecordItem::AssemblyRecordItemTypes);
    cell->setId("source");
    attrs.append(cell);

    return attrs;
}

ModifyDialogueBasicTable::ModifyDialogueBasicTable(const QString & name, const QList<ModifyDialogueTableCell *> & header, QWidget * parent):
        ModifyDialogueTable(name, header, parent)
{
    this->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);

    layout->addStretch();

    QToolButton * add_btn = new QToolButton;
    add_btn->setIcon(QIcon(QString::fromUtf8(":/images/images/add16.png")));
    QObject::connect(add_btn, SIGNAL(clicked()), this, SLOT(addNewRow()));
    grid->addWidget(add_btn, 0, header.count());
}

ModifyDialogueTableWithAdjustableTotal::ModifyDialogueTableWithAdjustableTotal(const QString & name, int category_id, const QList<ModifyDialogueTableCell *> & header, QWidget * parent):
    ModifyDialogueAdvancedTable(name, category_id, header, parent)
{
    total_w = new QDoubleSpinBox(this);
    total_w->setMaximum(99999999.9);

    QLayout * bottom_layout = layout->itemAt(layout->count() - 1)->layout();

    QLabel * lbl = new QLabel(QString("<b>%1</b>").arg(tr("Total list price:")), this);
    bottom_layout->addWidget(lbl);
    bottom_layout->addWidget(total_w);

    QObject::connect(total_w, SIGNAL(editingFinished()), this, SLOT(calculatePricesFromTotal()));
}

void ModifyDialogueTableWithAdjustableTotal::calculatePricesFromTotal()
{
   double total = total_w->value();
   double current_total = 0.0;

   QList<double> list_prices;
   for (int i = 0; i < rows.count(); ++i) {
       if (!rows.at(i)->isInTable()) continue;
       list_prices << rows.at(i)->acquisitionOrListPrice();
       current_total += list_prices.last() * rows.at(i)->widgetValue("value").toDouble();
   }

   for (int i = 0; i < list_prices.count(); ++i) {
       rows.at(i)->setListPrice(list_prices.at(i) * total / current_total);
   }
}

void ModifyDialogueTableWithAdjustableTotal::reloadTotal()
{
    double current_total = 0.0;

    for (int i = 0; i < rows.count(); ++i) {
        current_total += rows.at(i)->total();
    }

    total_w->setValue(current_total);
}

void ModifyDialogueTableWithAdjustableTotal::activateRow()
{
    ModifyDialogueAdvancedTable::activateRow();
    reloadTotal();
}

void ModifyDialogueTableWithAdjustableTotal::addRow(ModifyDialogueTableRow * row)
{
    QObject::connect(row, SIGNAL(valuesChanged()), this, SLOT(reloadTotal()));
    ModifyDialogueTable::addRow(row);
    reloadTotal();
}

ModifyDialogueTableRow::ModifyDialogueTableRow(const QMap<QString, ModifyDialogueTableCell *> & values, bool in_table)
{
    this->values = values;
    this->in_table = in_table;
    remove_btn = NULL;
}

ModifyDialogueTableRow::~ModifyDialogueTableRow()
{
    if (remove_btn) delete remove_btn;

    QMapIterator<QString, ModifyDialogueTableCell *> i(values);
    while (i.hasNext()) {
        i.next();
        delete values.take(i.key());
    }
}

double ModifyDialogueTableRow::total()
{
    if (!isInTable() || !widgets.contains("value"))
        return 0.0;
    else
        return widgets.value("value")->variantValue().toDouble() * listPrice();
}

void ModifyDialogueTableRow::setListPrice(double lp)
{
    if (widgets.contains("discount")) {
        lp *= 1 + widgets.value("discount")->variantValue().toDouble() / 100;
    }
    widgets.value("list_price")->setVariantValue(lp);
}

double ModifyDialogueTableRow::listPrice()
{
    double lp = 0.0;
    if (!widgets.contains("list_price")) return lp;
    lp = widgets.value("list_price")->variantValue().toDouble();

    if (widgets.contains("discount"))
        lp *= 1 - widgets.value("discount")->variantValue().toDouble() / 100;

    return lp;
}

double ModifyDialogueTableRow::acquisitionOrListPrice()
{
    if (widgets.contains("acquisition_price"))
        return widgets.value("acquisition_price")->variantValue().toDouble();
    else if (widgets.contains("list_price"))
        return widgets.value("list_price")->variantValue().toDouble();
    else
        return 0.0;
}

QVariant ModifyDialogueTableRow::widgetValue(const QString & col_id)
{
    if (widgets.contains(col_id))
        return widgets.value(col_id)->variantValue();
    else
        return QVariant();
}

void ModifyDialogueTableRow::addWidget(const QString & name, MDTInputWidget * le)
{
    if (values.value(name)->dataType() == Global::Numeric) {
        QObject::connect((MDTDoubleSpinBox *) le, SIGNAL(valueChanged(double)), this, SIGNAL(valuesChanged()));
    } else if (values.value(name)->dataType() == Global::Integer) {
        QObject::connect((MDTSpinBox *) le, SIGNAL(valueChanged(int)), this, SIGNAL(valuesChanged()));
    } else if (values.value(name)->dataType() == Global::String) {
        QObject::connect((MDTSpinBox *) le, SIGNAL(textEdited(const QString &)), this, SIGNAL(valuesChanged()));
    } else if (values.value(name)->dataType() == Global::String) {
        QObject::connect((MDTSpinBox *) le, SIGNAL(textChanged()), this, SIGNAL(valuesChanged()));
    }
    widgets.insert(name, le);
}

const MTDictionary ModifyDialogueTableRow::dictValues()
{
    MTDictionary dict;
    QMapIterator<QString, ModifyDialogueTableCell *> i(values);
    while (i.hasNext()) {
        i.next();
        if (widgets.contains(i.key())) {
            dict.setValue(i.key(), widgets.value(i.key())->variantValue().toString());
        } else {
            dict.setValue(i.key(), i.value()->value().toString());
        }
    }
    return dict;
}

const QString ModifyDialogueTableRow::itemTypeId()
{
    return values.value("item_type_id")->value().toString();
}

void ModifyDialogueTableRow::remove()
{
     QMapIterator<QString, MDTInputWidget *> i(widgets);
     i.toBack();
     while (i.hasPrevious()) {
         i.previous();
         MDTInputWidget * iw = widgets.take(i.key());

         switch (values.value(i.key())->dataType()) {
         case Global::Boolean:
             delete (MDTCheckBox *) iw;
             break;

         case Global::String:
             delete (MDTLineEdit *) iw;
             break;

         case Global::Text:
             delete (MDTPlainTextEdit *) iw;
             break;

         case Global::Integer:
             delete (MDTSpinBox *) iw;
             break;

         case Global::Numeric:
             delete (MDTDoubleSpinBox *) iw;
             break;

         case Global::File:
             delete (MDTFileChooser *) iw;
             break;

         default:
             delete (MDTLabel *) iw;
             break;
         }
     }
     if (remove_btn) {
         delete remove_btn;
         remove_btn = NULL;
     }
     setInTable(false);

     emit removed(this);
}

QToolButton * ModifyDialogueTableRow::removeButton()
{
    if (!remove_btn) {
        remove_btn = new QToolButton;
        remove_btn->setIcon(QIcon(QString::fromUtf8(":/images/images/remove16.png")));
        QObject::connect(remove_btn, SIGNAL(clicked()), this, SLOT(remove()));
    }
    return remove_btn;
}

const QString ModifyDialogueTableRow::value(const QString &name)
{
    if (values.contains(name))
        return values.value(name)->value().toString();
    else
        return QString();
}
