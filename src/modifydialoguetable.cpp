#include "modifydialoguetable.h"

#include <QGridLayout>
#include <QLineEdit>
#include <QLabel>
#include <QMap>
#include <QComboBox>
#include <QPushButton>

ModifyDialogueTableGroupBox::ModifyDialogueTableGroupBox(const QString &name, const MTDictionary & header, QWidget * parent):
        QGroupBox(name, parent)
{
    this->header = header;
    visible_rows = 0;

    QVBoxLayout * layout = new QVBoxLayout(this);
    grid = new QGridLayout(this);
    layout->addLayout(grid);
    layout->addLayout(addRowControlsLayout());
    setLayout(layout);

    createHeader();
}

void ModifyDialogueTableGroupBox::createHeader()
{
    grid->addWidget(new QLabel(tr("Name")), 0, 0);

    for (int i = 0; i < header.count(); ++i) {
        grid->addWidget(new QLabel(header.value(i)), visible_rows, i + 1);
    }
    visible_rows++;
}

QLayout * ModifyDialogueTableGroupBox::addRowControlsLayout()
{
    QHBoxLayout * layout = new QHBoxLayout;
    add_row_cb = new QComboBox;

    layout->addWidget(add_row_cb);

    QPushButton * add_btn = new QPushButton(tr("Add"));
    QObject::connect(add_btn, SIGNAL(clicked()), this, SLOT(activateRow()));
    layout->addWidget(add_btn);

    layout->addStretch();

    return layout;
}

void ModifyDialogueTableGroupBox::addRow(const QString & name, const MTDictionary & values, bool display)
{
    rows.append(new ModifyDialogueTableRow(values, display));
    if (!display) {
        add_row_cb->addItem(name, rows.last()->itemTypeId());
    } else {
        addRow(rows.last(), name);
    }
}

void ModifyDialogueTableGroupBox::addRow(ModifyDialogueTableRow * row, const QString & name)
{
    grid->addWidget(new QLabel(name), visible_rows, 0);

    for (int i = 0; i < header.count(); ++i) {
        QLineEdit * le = new QLineEdit(row->dict().value(header.key(i)));
        row->addWidget(header.key(i), le);
        grid->addWidget(le, visible_rows, i + 1);
    }
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

QList<MTDictionary> ModifyDialogueTableGroupBox::allValues()
{
    QList<MTDictionary> values;

    for (int i = 0; i < rows.count(); ++i) {
        if (rows.at(i)->isInTable())
            values.append(rows.at(i)->dictValues());
    }

    return values;
}

ModifyDialogueTableRow::ModifyDialogueTableRow(const MTDictionary & values, bool in_table)
{
    this->values = values;
    this->in_table = in_table;
    widgets = new QMap<QString, QLineEdit *>;
}

void ModifyDialogueTableRow::addWidget(const QString & name, QLineEdit * le)
{
    widgets->insert(name, le);
}

const MTDictionary & ModifyDialogueTableRow::dictValues()
{
    for (int i = 0; i < values.count(); ++i) {
        if (widgets->contains(values.key(i))) {
            values.setValue(values.key(i), widgets->value(values.key(i))->text());
        }
    }
    return values;
}

const QString & ModifyDialogueTableRow::itemTypeId()
{
    return values.value("item_type_id");
}
