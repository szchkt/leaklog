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

    QToolButton * add_new_btn = new QToolButton;
    add_new_btn->setIcon(QIcon(QString::fromUtf8(":/images/images/add16.png")));
    QObject::connect(add_new_btn, SIGNAL(clicked()), this, SLOT(addNewRow()));
    layout->addWidget(add_new_btn);

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
