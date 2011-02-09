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
class ARInputWidget;

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

    void addWidget(const QString &, ARInputWidget *);
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
    QMap<QString, ARInputWidget *> widgets;
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

class ARInputWidget
{
public:
    ARInputWidget(QWidget * w) { this->w = w; }

    virtual QVariant variantValue() = 0;
    QWidget * widget() { return w; }

private:
    QWidget * w;
};

class ARLineEdit : public QLineEdit, public ARInputWidget
{
public:
    ARLineEdit(const QString & text, QWidget * parent) : QLineEdit(text, parent), ARInputWidget(this) {}

    QVariant variantValue() { return text(); }
};

class ARSpinBox : public QSpinBox, public ARInputWidget
{
public:
    ARSpinBox(QWidget * parent) : QSpinBox(parent), ARInputWidget(this) {}

    QVariant variantValue() { return value(); }
};

class ARDoubleSpinBox : public QDoubleSpinBox, public ARInputWidget
{
    Q_OBJECT

public:
    ARDoubleSpinBox(QWidget * parent) : QDoubleSpinBox(parent), ARInputWidget(this) {}

    QVariant variantValue() { return value(); }

public slots:
    void clear() { setValue(0.0); }
};

class ARPlainTextEdit : public QPlainTextEdit, public ARInputWidget
{
    Q_OBJECT

public:
    ARPlainTextEdit(const QString & text, QWidget * parent) : QPlainTextEdit(text, parent), ARInputWidget(this) {}

    QVariant variantValue() { return this->toPlainText(); }
};

class ARCheckBox : public QCheckBox, public ARInputWidget
{
    Q_OBJECT

public:
    ARCheckBox(bool checked, QWidget * parent) : QCheckBox(parent), ARInputWidget(this) { setChecked(checked); }

    QVariant variantValue() { return this->isChecked(); }
};

#endif // MODIFYDIALOGUETABLE_H
