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

#include "editdialoguetable.h"

#include "records.h"
#include "global.h"

#include <QGridLayout>
#include <QLineEdit>
#include <QLabel>
#include <QMap>
#include <QComboBox>
#include <QToolButton>
#include <QPushButton>
#include <QTreeWidget>
#include <QHeaderView>

EditDialogueTable::EditDialogueTable(const QString &name, const QList<EditDialogueTableCell *> &header, QWidget *parent):
        QWidget(parent)
{
    this->header = header;

    layout = new QVBoxLayout(this);

    title_layout = new QHBoxLayout;
    title_layout->addWidget(new QLabel(QString("%1:").arg(name), this));
    title_layout->addStretch();
    layout->addLayout(title_layout);

    tree = new QTreeWidget(this);
    tree->setColumnCount(header.count() + 1);
    tree->setIndentation(0);
    QStringList header_labels;
    for (int i = 0; i < header.count(); ++i) {
        header_labels << header.at(i)->value().toString();
    }
    header_labels << "";
    tree->setHeaderLabels(header_labels);
    tree->setSelectionMode(QAbstractItemView::NoSelection);
    for (int i = 0; i < header.count(); ++i) {
        tree->header()->setSectionResizeMode(i, QHeaderView::Stretch);
    }
    tree->header()->setStretchLastSection(false);
    tree->header()->setSectionResizeMode(header.count(), QHeaderView::Custom);
    tree->header()->resizeSection(header.count(), 24);
    tree->setColumnWidth(header.count(), 24);

    layout->addWidget(tree);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(2);
}

EditDialogueTable::~EditDialogueTable()
{
    for (int i = rows.count() - 1; i >= 0; --i) {
        delete rows.takeAt(i);
    }
    for (int i = header.count() - 1; i >= 0; --i) {
        delete header.takeAt(i);
    }

    delete tree;
}

void EditDialogueTable::addRow(const QMap<QString, EditDialogueTableCell *> &values, bool display, EditDialogueTable::RowType row_type)
{
    EditDialogueTableRow *row = new EditDialogueTableRow(values, display, row_type, tree);
    QObject::connect(row, SIGNAL(removed(EditDialogueTableRow*)), this, SLOT(rowRemoved(EditDialogueTableRow*)));
    rows.append(row);

    if (!display) {
        addHiddenRow(row);
    } else {
        addRow(row);
    }
}

void EditDialogueTable::addRow(EditDialogueTableRow *row)
{
    MDTInputWidget *iw;

    EditDialogueTableCell *cell;
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
            iw = new MDTFileChooser(cell->value().toString(), this);
            break;

        default:
            iw = new MDTLabel(cell->value().toString(), this);
            break;
        }

        row->addWidget(header.at(i)->id(), iw);
        tree->setItemWidget(row->treeItem(), i, iw->widget());
    }
    tree->setItemWidget(row->treeItem(), header.count(), row->removeButton());
}

void EditDialogueTable::rowRemoved(EditDialogueTableRow *row)
{
    int index = tree->indexOfTopLevelItem(row->takeTreeItem());
    if (index >= 0) {
        delete tree->takeTopLevelItem(index);
    }
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

void EditDialogueTable::addNewRow()
{
    QMap<QString, EditDialogueTableCell *> cells_map;
    for (int i = 0; i < header.count(); ++i) {
        cells_map.insert(header.at(i)->id(), new EditDialogueTableCell(QVariant(), header.at(i)->dataType()));
    }
    QList<EditDialogueTableCell *> other_cells = hiddenAttributes();
    for (int i = 0; i < other_cells.count(); ++i) {
        cells_map.insert(other_cells.at(i)->id(), other_cells.at(i));
    }
    addRow(cells_map, true);
}

QList<MTDictionary> EditDialogueTable::allValues() const
{
    QList<MTDictionary> values;

    for (int i = 0; i < rows.count(); ++i) {
        if (rows.at(i)->isInTable())
            values.append(rows.at(i)->dictValues());
    }

    return values;
}

EditDialogueAdvancedTable::EditDialogueAdvancedTable(const QString &name, const QString &category_uuid, const QList<EditDialogueTableCell *> &header, QWidget *parent):
    EditDialogueTable(name, header, parent),
    category_uuid(category_uuid)
{
    smallest_index = -1;

    layout->addLayout(addRowControlsLayout());
}

QLayout *EditDialogueAdvancedTable::addRowControlsLayout()
{
    QHBoxLayout *layout = new QHBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(6);
    add_row_cb = new QComboBox(this);
    add_row_cb->setMaximumWidth(500);

    layout->addWidget(add_row_cb);

    QPushButton *add_btn = new QPushButton(tr("Add"), this);
    QObject::connect(add_btn, SIGNAL(clicked()), this, SLOT(activateRow()));
    layout->addWidget(add_btn);

    layout->addStretch();

    QToolButton *add_new_btn = new QToolButton(this);
    add_new_btn->setIcon(QIcon(QString::fromUtf8(":/images/images/add16.png")));
    add_new_btn->setText(tr("Add custom item"));
    add_new_btn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    QObject::connect(add_new_btn, SIGNAL(clicked()), this, SLOT(addNewRow()));
    layout->addWidget(add_new_btn);

    return layout;
}

void EditDialogueAdvancedTable::addHiddenRow(EditDialogueTableRow *row)
{
    add_row_cb->addItem(row->value("name"), row->valuesMap().value("ar_item_type_uuid")->value());
}

void EditDialogueAdvancedTable::activateRow()
{
    EditDialogueTableRow *row = NULL;
    QString item_type_uuid = add_row_cb->itemData(add_row_cb->currentIndex()).toString();

    for (int i = 0; i < rows.count(); ++i) {
        if (!rows.at(i)->isInTable() && rows.at(i)->value("ar_item_type_uuid") == item_type_uuid) {
            row = rows.at(i);
            break;
        }
    }

    if (!row) return;
    row->setInTable(true);
    addRow(row);
    add_row_cb->removeItem(add_row_cb->currentIndex());
}

QList<EditDialogueTableCell *> EditDialogueAdvancedTable::hiddenAttributes()
{
    QList<EditDialogueTableCell *> attrs;
    EditDialogueTableCell *cell = new EditDialogueTableCell(category_uuid);
    cell->setId("ar_item_category_uuid");
    attrs.append(cell);
    cell = new EditDialogueTableCell(smallest_index--);
    cell->setId("ar_item_type_uuid");
    attrs.append(cell);
    cell = new EditDialogueTableCell(AssemblyRecordItem::AssemblyRecordItemTypes);
    cell->setId("source");
    attrs.append(cell);

    return attrs;
}

EditDialogueBasicTable::EditDialogueBasicTable(const QString &name, const QList<EditDialogueTableCell *> &header, QWidget *parent):
    EditDialogueTable(name, header, parent)
{
    this->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);

    QToolButton *add_btn = new QToolButton;
    add_btn->setIcon(QIcon(QString::fromUtf8(":/images/images/add16.png")));
    QObject::connect(add_btn, SIGNAL(clicked()), this, SLOT(addNewRow()));
    title_layout->addWidget(add_btn);
}

EditDialogueTableWithAdjustableTotal::EditDialogueTableWithAdjustableTotal(const QString &name, const QString &category_uuid, const QList<EditDialogueTableCell *> &header, QWidget *parent):
    EditDialogueAdvancedTable(name, category_uuid, header, parent)
{
    total_w = new QDoubleSpinBox(this);
    total_w->setMaximum(99999999.9);

    QLabel *lbl = new QLabel(tr("Total list price:"), this);
    title_layout->addWidget(lbl);
    title_layout->addWidget(total_w);

    QObject::connect(total_w, SIGNAL(editingFinished()), this, SLOT(calculatePricesFromTotal()));
}

void EditDialogueTableWithAdjustableTotal::calculatePricesFromTotal()
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

void EditDialogueTableWithAdjustableTotal::reloadTotal()
{
    double current_total = 0.0;

    for (int i = 0; i < rows.count(); ++i) {
        current_total += rows.at(i)->total();
    }

    total_w->setValue(current_total);
}

void EditDialogueTableWithAdjustableTotal::activateRow()
{
    EditDialogueAdvancedTable::activateRow();
    reloadTotal();
}

void EditDialogueTableWithAdjustableTotal::addRow(EditDialogueTableRow *row)
{
    QObject::connect(row, SIGNAL(valuesChanged()), this, SLOT(reloadTotal()));
    EditDialogueTable::addRow(row);
    reloadTotal();
}

EditDialogueTableRow::EditDialogueTableRow(const QMap<QString, EditDialogueTableCell *> &values, bool in_table, EditDialogueTable::RowType row_type, QTreeWidget *tree):
    m_tree(tree),
    m_tree_item(NULL)
{
    this->values = values;
    this->in_table = in_table;
    this->row_type = row_type;
    remove_btn = NULL;
}

EditDialogueTableRow::~EditDialogueTableRow()
{
    if (remove_btn) delete remove_btn;

    QMapIterator<QString, EditDialogueTableCell *> i(values);
    while (i.hasNext()) {
        i.next();
        delete values.take(i.key());
    }
}

double EditDialogueTableRow::total() const
{
    if (!isInTable() || !widgets.contains("value"))
        return 0.0;
    else
        return widgets.value("value")->variantValue().toDouble() * listPrice();
}

void EditDialogueTableRow::setListPrice(double lp)
{
    if (widgets.contains("discount")) {
        lp *= 1 + widgets.value("discount")->variantValue().toDouble() / 100;
    }
    widgets.value("list_price")->setVariantValue(lp);
}

double EditDialogueTableRow::listPrice() const
{
    double lp = 0.0;
    if (!widgets.contains("list_price")) return lp;
    lp = widgets.value("list_price")->variantValue().toDouble();

    if (widgets.contains("discount"))
        lp *= 1 - widgets.value("discount")->variantValue().toDouble() / 100;

    return lp;
}

double EditDialogueTableRow::acquisitionOrListPrice() const
{
    if (widgets.contains("acquisition_price"))
        return widgets.value("acquisition_price")->variantValue().toDouble();
    else if (widgets.contains("list_price"))
        return widgets.value("list_price")->variantValue().toDouble();
    else
        return 0.0;
}

QVariant EditDialogueTableRow::widgetValue(const QString &col_id) const
{
    if (widgets.contains(col_id))
        return widgets.value(col_id)->variantValue();
    else
        return QVariant();
}

void EditDialogueTableRow::addWidget(const QString &name, MDTInputWidget *le)
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

MTDictionary EditDialogueTableRow::dictValues() const
{
    MTDictionary dict;
    QMapIterator<QString, EditDialogueTableCell *> i(values);
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

QString EditDialogueTableRow::itemTypeUUID() const
{
    return values.value("ar_item_type_uuid")->value().toString();
}

void EditDialogueTableRow::remove()
{
     QMapIterator<QString, MDTInputWidget *> i(widgets);
     i.toBack();
     while (i.hasPrevious()) {
         i.previous();
         delete widgets.take(i.key());
     }
     if (remove_btn) {
         delete remove_btn;
         remove_btn = NULL;
     }
     setInTable(false);

     emit removed(this);
}

void EditDialogueTableRow::toggleHidden()
{
    if (remove_btn->isChecked())
        remove_btn->setIcon(QIcon(":/images/images/visible_off16.png"));
    else
        remove_btn->setIcon(QIcon(":/images/images/visible_on16.png"));
}

QToolButton *EditDialogueTableRow::removeButton()
{
    if (!remove_btn) {
        MDTToolButton *button = new MDTToolButton;
        if (row_type == EditDialogueTable::Hidable) {
            bool hidden = values.value("hidden")->value().toBool();
            button->setIcon(QIcon(hidden ? ":/images/images/visible_off16.png" : ":/images/images/visible_on16.png"));
            button->setToolTip(tr("Hide"));
            button->setCheckable(true);
            button->setChecked(hidden);
            QObject::connect(button, SIGNAL(clicked()), this, SLOT(toggleHidden()));
            widgets.insert("hidden", button);
        } else {
            button->setIcon(QIcon(":/images/images/remove16.png"));
            button->setToolTip(tr("Remove"));
            QObject::connect(button, SIGNAL(clicked()), this, SLOT(remove()));
        }
        button->setMaximumWidth(24);
        button->setEnabled(row_type != EditDialogueTable::Default);
        remove_btn = button;
    }
    return remove_btn;
}

QString EditDialogueTableRow::value(const QString &name) const
{
    if (values.contains(name))
        return values.value(name)->value().toString();
    else
        return QString();
}

QTreeWidgetItem *EditDialogueTableRow::treeItem()
{
    if (m_tree_item)
        return  m_tree_item;

    m_tree_item = new QTreeWidgetItem(m_tree);
    return m_tree_item;
}

QTreeWidgetItem *EditDialogueTableRow::takeTreeItem()
{
    QTreeWidgetItem *item = m_tree_item;
    m_tree_item = NULL;
    return item;
}
