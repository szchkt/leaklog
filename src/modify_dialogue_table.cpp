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
    MDTInputWidget * iw;
    if (row->itemTypeId().toInt() < 0) {
        iw = new MDTLineEdit(row->valuesMap().value("name")->value().toString(), this);
        row->addWidget("name", iw);
        grid->addWidget(iw->widget(), visible_rows, 0);
    }
    else grid->addWidget(row->label(name), visible_rows, 0);

    ModifyDialogueTableCell * cell;
    for (int i = 0; i < header.count(); ++i) {
        cell = row->valuesMap().value(header.key(i));

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
            break;

        case Global::Numeric:
            iw = new MDTDoubleSpinBox(this);
            ((MDTDoubleSpinBox *) iw)->setValue(cell->value().toDouble());
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
    cells_map.insert("name", new ModifyDialogueTableCell(tr("New item"), Global::String));
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

void ModifyDialogueTableRow::addWidget(const QString & name, MDTInputWidget * le)
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
         }
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

ModifyDialogueBasicTable::ModifyDialogueBasicTable(const QString & name, const MTDictionary & header, QWidget * parent):
        QGroupBox(name, parent)
{
    this->header = header;
    visible_rows = 0;
    this->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);

    QVBoxLayout * layout = new QVBoxLayout(this);
    grid = new QGridLayout;
    layout->addLayout(grid);
    layout->addStretch();
    layout->setContentsMargins(6, 6, 6, 6);
    grid->setContentsMargins(0, 0, 0, 0);

    for (int i = 0; i < header.count(); ++i) {
        grid->addWidget(new QLabel(QString("<b>%1</b>").arg(header.value(i))), visible_rows, i);
    }

    QToolButton * add_btn = new QToolButton;
    add_btn->setIcon(QIcon(QString::fromUtf8(":/images/images/add16.png")));
    QObject::connect(add_btn, SIGNAL(clicked()), this, SLOT(addNewRow()));
    grid->addWidget(add_btn, visible_rows, header.count());
    visible_rows++;
}

void ModifyDialogueBasicTable::addNewRow()
{
    QMap<QString, QVariant> values;
    for (int i = 0; i < header.count(); ++i) {
        values.insert(header.key(i), QVariant());
    }
    addRow(values);
}

void ModifyDialogueBasicTable::addRow(const QMap<QString, QVariant> & values)
{
    ModifyDialogueBasicTableRow * row = new ModifyDialogueBasicTableRow(values);
    QObject::connect(row, SIGNAL(removed(ModifyDialogueBasicTableRow*)), this, SLOT(rowRemoved(ModifyDialogueBasicTableRow*)));
    rows.append(row);
    QLineEdit * iw;

    for (int i = 0; i < header.count(); ++i) {
        iw = new QLineEdit(values.value(header.key(i)).toString(), this);
        if (!iw) continue;

        row->addWidget(header.key(i), iw);
        grid->addWidget(iw, visible_rows, i);
    }
    grid->addWidget(row->removeButton(), visible_rows, header.count());
    visible_rows++;
}

void ModifyDialogueBasicTable::rowRemoved(ModifyDialogueBasicTableRow * row)
{
    for (int i = 0; i < rows.count(); ++i) {
        if (rows.at(i) == row) {
            delete rows.takeAt(i);
            break;
        }
    }
}

QList<MTDictionary> ModifyDialogueBasicTable::allValues()
{
    QList<MTDictionary> values;

    for (int i = 0; i < rows.count(); ++i) {
        values.append(rows.at(i)->dictValues());
    }

    return values;
}

ModifyDialogueBasicTableRow::ModifyDialogueBasicTableRow(const QMap<QString, QVariant> & values)
{
    this->values = values;
    remove_btn = NULL;
}

void ModifyDialogueBasicTableRow::addWidget(const QString & name, QLineEdit * le)
{
    widgets.insert(name, le);
}

QToolButton * ModifyDialogueBasicTableRow::removeButton()
{
    if (!remove_btn) {
        remove_btn = new QToolButton;
        remove_btn->setIcon(QIcon(QString::fromUtf8(":/images/images/remove16.png")));
        QObject::connect(remove_btn, SIGNAL(clicked()), this, SLOT(remove()));
    }
    return remove_btn;
}

MTDictionary ModifyDialogueBasicTableRow::dictValues()
{
    MTDictionary dict;
    QMapIterator<QString, QVariant> i(values);
    while (i.hasNext()) {
        i.next();
        if (widgets.contains(i.key())) {
            dict.setValue(i.key(), widgets.value(i.key())->text());
        } else {
            dict.setValue(i.key(), i.value().toString());
        }
    }
    return dict;
}

void ModifyDialogueBasicTableRow::remove()
{
    QMapIterator<QString, QLineEdit *> i(widgets);
    while (i.hasNext()) {
        i.next();
        delete widgets.take(i.key());
    }

     if (remove_btn) {
         delete remove_btn;
         remove_btn = NULL;
     }

     emit removed(this);
}
