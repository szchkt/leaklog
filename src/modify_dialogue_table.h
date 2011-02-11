#ifndef MODIFYDIALOGUETABLE_H
#define MODIFYDIALOGUETABLE_H

#include <QGroupBox>
#include <QLineEdit>
#include <QAction>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QPlainTextEdit>

#include "mtdictionary.h"

class QGridLayout;
class QComboBox;
class QLabel;
class QToolButton;

class ModifyDialogueTableRow;
class ModifyDialogueTableCell;
class ModifyDialogueBasicTableRow;
class MDTInputWidget;

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

    void addWidget(const QString &, MDTInputWidget *);
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
    QMap<QString, MDTInputWidget *> widgets;
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

class ModifyDialogueBasicTable : public QGroupBox
{
    Q_OBJECT

public:
    ModifyDialogueBasicTable(const QString &, const MTDictionary &, QWidget *);

    void addRow(const QMap<QString, QVariant> &);
    QList<MTDictionary> allValues();

public slots:
    void addNewRow();

private slots:
    void rowRemoved(ModifyDialogueBasicTableRow *);

private:
    MTDictionary header;
    int visible_rows;
    QGridLayout * grid;
    QList<ModifyDialogueBasicTableRow *> rows;
};

class ModifyDialogueBasicTableRow : public QObject
{
    Q_OBJECT

public:
    ModifyDialogueBasicTableRow(const QMap<QString, QVariant> &);

    void addWidget(const QString &, QLineEdit *);
    MTDictionary dictValues();

    QToolButton * removeButton();

private slots:
    void remove();

signals:
    void removed(ModifyDialogueBasicTableRow *);

private:
    QToolButton * remove_btn;
    QMap<QString, QLineEdit *> widgets;
    QMap<QString, QVariant> values;
};

class MDTInputWidget
{
public:
    MDTInputWidget(QWidget * w) { this->w = w; }

    virtual QVariant variantValue() = 0;
    QWidget * widget() { return w; }

private:
    QWidget * w;
};

class MDTLineEdit : public QLineEdit, public MDTInputWidget
{
public:
    MDTLineEdit(const QString & text, QWidget * parent) : QLineEdit(text, parent), MDTInputWidget(this) {}

    QVariant variantValue() { return text(); }
};

class MDTSpinBox : public QSpinBox, public MDTInputWidget
{
public:
    MDTSpinBox(QWidget * parent) : QSpinBox(parent), MDTInputWidget(this) {}

    QVariant variantValue() { return value(); }
};

class MDTDoubleSpinBox : public QDoubleSpinBox, public MDTInputWidget
{
    Q_OBJECT

public:
    MDTDoubleSpinBox(QWidget * parent) : QDoubleSpinBox(parent), MDTInputWidget(this) {}

    QVariant variantValue() { return value(); }

public slots:
    void clear() { setValue(0.0); }
};

class MDTPlainTextEdit : public QPlainTextEdit, public MDTInputWidget
{
    Q_OBJECT

public:
    MDTPlainTextEdit(const QString & text, QWidget * parent) : QPlainTextEdit(text, parent), MDTInputWidget(this) {}

    QVariant variantValue() { return this->toPlainText(); }
};

class MDTCheckBox : public QCheckBox, public MDTInputWidget
{
    Q_OBJECT

public:
    MDTCheckBox(bool checked, QWidget * parent) : QCheckBox(parent), MDTInputWidget(this) { setChecked(checked); }

    QVariant variantValue() { return this->isChecked(); }
};

#endif // MODIFYDIALOGUETABLE_H
