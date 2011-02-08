#ifndef MODIFYDIALOGUETABLE_H
#define MODIFYDIALOGUETABLE_H

class QGridLayout;
class QLineEdit;
class QComboBox;
class QLabel;
class QToolButton;

#include <QGroupBox>
#include <QAction>

#include "mtdictionary.h"

class ModifyDialogueTableRow;

class ModifyDialogueTableGroupBox : public QGroupBox
{
    Q_OBJECT

public:
    ModifyDialogueTableGroupBox(const QString &, const MTDictionary &, QWidget *);
    ~ModifyDialogueTableGroupBox();

    void addRow(const QString &, const MTDictionary &, bool);
    void addRow(ModifyDialogueTableRow *, const QString &);
    QList<MTDictionary> allValues();

private slots:
    void activateRow();
    void rowRemoved(ModifyDialogueTableRow *);

private:
    void createHeader();
    QLayout * addRowControlsLayout();

    QGridLayout * grid;
    QComboBox * add_row_cb;

    MTDictionary header;
    QList<ModifyDialogueTableRow *> rows;
    int visible_rows;
};

class ModifyDialogueTableRow : public QObject
{
    Q_OBJECT

public:
    ModifyDialogueTableRow(const MTDictionary &, bool);
    ~ModifyDialogueTableRow();

    void addWidget(const QString &, QLineEdit *);
    const MTDictionary & dictValues();
    bool isInTable() { return in_table; }
    void setInTable(bool in_table) { this->in_table = in_table; }

    const QString & itemTypeId();
    const MTDictionary & dict() { return values; }

    QToolButton * removeButton();
    QLabel * label(const QString &);
    const QString & name() { return row_name; }

private slots:
    void remove(bool = true);

signals:
    void removed(ModifyDialogueTableRow *);

private:
    QMap<QString, QLineEdit *> * widgets;
    QToolButton * remove_btn;
    QLabel * lbl;
    QString row_name;
    MTDictionary values;
    bool in_table;
};

#endif // MODIFYDIALOGUETABLE_H
