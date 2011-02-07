#include "modifydialoguetable.h"

#include <QGridLayout>
#include <QLineEdit>
#include <QLabel>

ModifyDialogueTableGroupBox::ModifyDialogueTableGroupBox(const QString &name, const MTDictionary & header, QWidget * parent):
        QGroupBox(name, parent)
{
    this->header = header;
    grid = new QGridLayout(this);
    setLayout(grid);
    cols = 0;

    createHeader();
}

void ModifyDialogueTableGroupBox::createHeader()
{
    grid->addWidget(new QLabel(tr("Name")), 0, 0);

    for (int i = 0; i < header.count(); ++i) {
        grid->addWidget(new QLabel(header.value(i)), 0, i + 1);
    }
}

void ModifyDialogueTableGroupBox::addRow(const QString & name, const MTDictionary & values)
{
    rows.append(new ModifyDialogueTableRow);
    grid->addWidget(new QLabel(name), rows.count(), 0);

    for (int i = 0; i < header.count(); ++i) {
        QLineEdit * le = new QLineEdit(values.value(header.key(i)));
        rows.last()->addWidget(le);
        grid->addWidget(le, rows.count(), i + 1);
    }
}

void ModifyDialogueTableRow::addWidget(QWidget * w)
{
    widgets.append(w);
}
