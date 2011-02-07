#ifndef MODIFYDIALOGUETABLE_H
#define MODIFYDIALOGUETABLE_H

class QGridLayout;
class QLineEdit;
class QComboBox;

#include <QGroupBox>
#include <QAction>

#include "mtdictionary.h"

class ModifyDialogueTableRow;

class ModifyDialogueTableGroupBox : public QGroupBox
{
    Q_OBJECT

public:
    ModifyDialogueTableGroupBox(const QString &, const MTDictionary &, QWidget *);

    void addRow(const QString &, const MTDictionary &, bool);
    void addRow(ModifyDialogueTableRow *, const QString &);
    QList<MTDictionary> allValues();

private slots:
    void activateRow();

private:
    void createHeader();
    QLayout * addRowControlsLayout();

    QGridLayout * grid;
    QComboBox * add_row_cb;

    MTDictionary header;
    QList<ModifyDialogueTableRow *> rows;
    int visible_rows;
};

class ModifyDialogueTableRow
{
public:
    ModifyDialogueTableRow(const MTDictionary &, bool);

    void addWidget(const QString &, QLineEdit *);
    const MTDictionary & dictValues();
    bool isInTable() { return in_table; }
    void setInTable(bool in_table) { this->in_table = in_table; }

    const QString & itemTypeId();
    const MTDictionary & dict() { return values; }

private:
    QMap<QString, QLineEdit *> * widgets;
    MTDictionary values;
    bool in_table;
};

#endif // MODIFYDIALOGUETABLE_H
