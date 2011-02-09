#include "modifydialoguetable.h"

#include "records.h"
#include "global.h"

#include <QGridLayout>
#include <QLineEdit>
#include <QLabel>
#include <QMap>
#include <QComboBox>
#include <QToolButton>
#include <QPushButton>

ModifyDialogueTableGroupBox::ModifyDialogueTableGroupBox(const QString &name, int category_id, const MTDictionary & header, QWidget * parent):
        QGroupBox(name, parent)
{
    this->header = header;
    this->category_id = category_id;
    visible_rows = 0;
    smallest_index = -1;

    QVBoxLayout * layout = new QVBoxLayout(this);
    grid = new QGridLayout;
    grid->setContentsMargins(0, 0, 0, 0);
    layout->addLayout(grid);
    layout->addLayout(addRowControlsLayout());
    layout->setContentsMargins(9, 9, 9, 9);
    layout->setSpacing(6);

    createHeader();
}

ModifyDialogueTableGroupBox::~ModifyDialogueTableGroupBox()
{
    for (int i = rows.count() - 1; i >= 0; --i) {
        delete rows.takeAt(i);
    }

    delete add_row_cb;
    delete grid;
}

void ModifyDialogueTableGroupBox::createHeader()
{
    grid->addWidget(new QLabel(QString("<b>%1</b>").arg(tr("Name")), 0, 0));

    for (int i = 0; i < header.count(); ++i) {
        grid->addWidget(new QLabel(QString("<b>%1</b>").arg(header.value(i))), visible_rows, i + 1);
    }
    visible_rows++;
}

QLayout * ModifyDialogueTableGroupBox::addRowControlsLayout()
{
    QHBoxLayout * layout = new QHBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    add_row_cb = new QComboBox;

    layout->addWidget(add_row_cb);

    QPushButton * add_btn = new QPushButton(tr("Add"));
    QObject::connect(add_btn, SIGNAL(clicked()), this, SLOT(activateRow()));
    layout->addWidget(add_btn);

    layout->addStretch();

    QToolButton * add_new_btn = new QToolButton;
    add_new_btn->setIcon(QIcon(QString::fromUtf8(":/images/images/add16.png")));
    QObject::connect(add_new_btn, SIGNAL(clicked()), this, SLOT(addNewRow()));
    layout->addWidget(add_new_btn);

    return layout;
}

void ModifyDialogueTableGroupBox::addRow(const QString & name, const QMap<QString, ModifyDialogueTableCell *> & values, bool display)
{
    ModifyDialogueTableRow * row = new ModifyDialogueTableRow(values, display);
    QObject::connect(row, SIGNAL(removed(ModifyDialogueTableRow*, bool)), this, SLOT(rowRemoved(ModifyDialogueTableRow*, bool)));
    rows.append(row);

    if (!display) {
        add_row_cb->addItem(name, row->itemTypeId());
    } else {
        addRow(row, name);
    }
}

void ModifyDialogueTableGroupBox::addRow(ModifyDialogueTableRow * row, const QString & name)
{
    ARInputWidget * iw;
    if (row->itemTypeId().toInt() < 0) {
        iw = new ARLineEdit(row->valuesMap().value("name")->value().toString(), this);
        row->addWidget("name", iw);
        grid->addWidget(iw->widget(), visible_rows, 0);
    }
    else grid->addWidget(row->label(name), visible_rows, 0);

    ModifyDialogueTableCell * cell;
    for (int i = 0; i < header.count(); ++i) {
        cell = row->valuesMap().value(header.key(i));
        switch (cell->dataType()) {
        case Global::Boolean:
            iw = new ARCheckBox(cell->value().toBool(), this);
            break;

        case Global::String:
            iw = new ARLineEdit(cell->value().toString(), this);
            break;

        case Global::Integer:
            iw = new ARSpinBox(this);
            ((ARSpinBox *) iw)->setValue(cell->value().toInt());
            break;

        case Global::Numeric:
            iw = new ARDoubleSpinBox(this);
            ((ARDoubleSpinBox *) iw)->setValue(cell->value().toDouble());
            break;

        default:
            iw = NULL;
            continue;
            break;
        }
        if (!cell->enabled()) iw->widget()->setEnabled(false);
        row->addWidget(header.key(i), iw);
        grid->addWidget(iw->widget(), visible_rows, i + 1);
    }
    grid->addWidget(row->removeButton(), visible_rows, header.count() + 1);
    visible_rows++;
}

void ModifyDialogueTableGroupBox::activateRow()
{
    ModifyDialogueTableRow * row = NULL;
    QString type_id = add_row_cb->itemData(add_row_cb->currentIndex()).toString();

    for (int i = 0; i < rows.count(); ++i) {
        if (!rows.at(i)->isInTable() && rows.at(i)->itemTypeId() == type_id) {
            row = rows.at(i);
            break;
        }
    }

    if (!row) return;
    row->setInTable(true);
    addRow(row, add_row_cb->currentText());
    add_row_cb->removeItem(add_row_cb->currentIndex());
}

void ModifyDialogueTableGroupBox::addNewRow()
{
    QMap<QString, ModifyDialogueTableCell *> cells_map;
    cells_map.insert("value", new ModifyDialogueTableCell(QVariant(), Global::String));
    cells_map.insert("item_type_id", new ModifyDialogueTableCell(smallest_index--));
    cells_map.insert("acquisition_price", new ModifyDialogueTableCell(QVariant(), Global::Numeric));
    cells_map.insert("list_price", new ModifyDialogueTableCell(QVariant(), Global::Numeric));
    cells_map.insert("name", new ModifyDialogueTableCell(tr("New item")));
    cells_map.insert("category_id", new ModifyDialogueTableCell(category_id));
    addRow(tr("New item"), cells_map, true);
}

void ModifyDialogueTableGroupBox::rowRemoved(ModifyDialogueTableRow * row, bool deleted)
{
    if (deleted) {
        for (int i = 0; i < rows.count(); ++i) {
            if (rows.at(i) == row) {
                delete rows.takeAt(i);
                break;
            }
        }
    }
    else add_row_cb->addItem(row->name(), row->itemTypeId());
}

QList<MTDictionary> ModifyDialogueTableGroupBox::allValues()
{
    QList<MTDictionary> values;

    for (int i = 0; i < rows.count(); ++i) {
        if (rows.at(i)->isInTable())
            values.append(rows.at(i)->dictValues());
    }

    return values;
}

ModifyDialogueTableRow::ModifyDialogueTableRow(const QMap<QString, ModifyDialogueTableCell *> & values, bool in_table)
{
    this->values = values;
    this->in_table = in_table;
    remove_btn = NULL;
    lbl = NULL;
}

ModifyDialogueTableRow::~ModifyDialogueTableRow()
{
    if (lbl) delete lbl;
    if (remove_btn) delete remove_btn;

    QMapIterator<QString, ModifyDialogueTableCell *> i(values);
    while (i.hasNext()) {
        i.next();
        delete values.take(i.key());
    }
}

void ModifyDialogueTableRow::addWidget(const QString & name, ARInputWidget * le)
{
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

void ModifyDialogueTableRow::remove(bool emit_sig)
{
     QMapIterator<QString, ARInputWidget *> i(widgets);
     while (i.hasNext()) {
         i.next();
         delete widgets.take(i.key());
     }
     if (lbl) {
         delete lbl;
         lbl = NULL;
     }
     if (remove_btn) {
         delete remove_btn;
         remove_btn = NULL;
     }
     setInTable(false);

     if (values.value("item_type_id")->value().toInt() < 0) emit removed(this, true);
     else if (emit_sig) emit removed(this, false);
}

QLabel * ModifyDialogueTableRow::label(const QString & name)
{
    if (!lbl) {
        lbl = new QLabel(name);
        row_name = name;
    }
    return lbl;
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
