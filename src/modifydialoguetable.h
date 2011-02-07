#ifndef MODIFYDIALOGUETABLE_H
#define MODIFYDIALOGUETABLE_H

class QGridLayout;

#include <QGroupBox>

#include "mtdictionary.h"

class ModifyDialogueTableRow;

class ModifyDialogueTableGroupBox : public QGroupBox
{
public:
    ModifyDialogueTableGroupBox(const QString &, const MTDictionary &, QWidget *);

    void addRow(const QString &, const MTDictionary &);

private:
    void createHeader();

    MTDictionary header;
    QGridLayout * grid;
    QList<ModifyDialogueTableRow *> rows;
    int cols;
};

class ModifyDialogueTableRow
{
public:
    void addWidget(QWidget *);

private:
    QList<QWidget *> widgets;
};

#endif // MODIFYDIALOGUETABLE_H
