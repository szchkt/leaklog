#ifndef MODIFYDIALOGUETABLE_H
#define MODIFYDIALOGUETABLE_H

#include <QGroupBox>
#include <QAction>

#include "mtdictionary.h"

class QGridLayout;
class QLineEdit;
class QComboBox;
class QLabel;
class QToolButton;

class ModifyDialogueTableRow;
class ModifyDialogueTableCell;

class ModifyDialogueTableGroupBox : public QGroupBox
{
    Q_OBJECT

public:
    ModifyDialogueTableGroupBox(const QString &, int, const MTDictionary &, QWidget *);
    ~ModifyDialogueTableGroupBox();

    void addRow(const QString &, const QMap<QString, ModifyDialogueTableCell *> &, bool);
    void addRow(ModifyDialogueTableRow *, const QString &);
    QList<MTDictionary> allValues();

private slots:
    void activateRow();
    void rowRemoved(ModifyDialogueTableRow *, bool);
    void addNewRow();

private:
    void createHeader();
    QLayout * addRowControlsLayout();

    QGridLayout * grid;
    QComboBox * add_row_cb;

    MTDictionary header;
    QList<ModifyDialogueTableRow *> rows;
    int visible_rows;
    int smallest_index;
    int category_id;
};

class ModifyDialogueTableRow : public QObject
{
    Q_OBJECT

public:
    ModifyDialogueTableRow(const QMap<QString, ModifyDialogueTableCell *> &, bool);
    ~ModifyDialogueTableRow();

    void addWidget(const QString &, QLineEdit *);
    const MTDictionary dictValues();
    bool isInTable() { return in_table; }
    void setInTable(bool in_table) { this->in_table = in_table; }

    const QString itemTypeId();
    const QMap<QString, ModifyDialogueTableCell *> & valuesMap() { return values; }

    QToolButton * removeButton();
    QLabel * label(const QString &);
    const QString & name() { return row_name; }

private slots:
    void remove(bool = true);

signals:
    void removed(ModifyDialogueTableRow *, bool);

private:
    QToolButton * remove_btn;
    QLabel * lbl;
    QString row_name;
    QMap<QString, QLineEdit *> widgets;
    QMap<QString, ModifyDialogueTableCell *> values;
    bool in_table;
};

class ModifyDialogueTableCell
{
public:
    ModifyDialogueTableCell(const QVariant & _value, int _data_type = -1, bool _enabled = true) {
        this->_value = _value;
        this->_data_type = _data_type;
        this->_enabled = _enabled;
    }

    const QVariant & value() { return _value; }
    int dataType() { return _data_type; }
    bool enabled() { return _enabled; }

private:
    QVariant _value;
    int _data_type;
    bool _enabled;
};

#endif // MODIFYDIALOGUETABLE_H
