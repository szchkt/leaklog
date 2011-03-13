/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2011 Matus & Michal Tomlein

 Leaklog is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public Licence
 as published by the Free Software Foundation; either version 2
 of the Licence, or (at your option) any later version.

 Leaklog is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public Licence for more details.

 You should have received a copy of the GNU General Public Licence
 along with Leaklog; if not, write to the Free Software Foundation,
 Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
********************************************************************/

#ifndef MODIFYDIALOGUETABLE_H
#define MODIFYDIALOGUETABLE_H

#include <QGroupBox>
#include <QLineEdit>
#include <QAction>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QPlainTextEdit>
#include <QLabel>

#include "mtdictionary.h"
#include "dbfile.h"

class QGridLayout;
class QComboBox;
class QLabel;
class QToolButton;
class QVBoxLayout;

class ModifyDialogueTableRow;
class ModifyDialogueTableCell;
class ModifyDialogueBasicTableRow;
class MDTInputWidget;

class ModifyDialogueTable : public QGroupBox
{
    Q_OBJECT

public:
    ModifyDialogueTable(const QString &, const QList<ModifyDialogueTableCell *> &, QWidget *);
    ~ModifyDialogueTable();

    void addRow(const QMap<QString, ModifyDialogueTableCell *> &, bool = true);
    virtual void addRow(ModifyDialogueTableRow *);
    QList<MTDictionary> allValues();

public slots:
    void addNewRow();

private slots:
    void rowRemoved(ModifyDialogueTableRow *);

protected:
    void createHeader();
    virtual void addHiddenRow(ModifyDialogueTableRow *) = 0;
    virtual QList<ModifyDialogueTableCell *> hiddenAttributes() = 0;

    QVBoxLayout * layout;
    QGridLayout * grid;

    QList<ModifyDialogueTableCell *> header;
    QList<ModifyDialogueTableRow *> rows;
    int visible_rows;
};

class ModifyDialogueAdvancedTable : public ModifyDialogueTable
{
    Q_OBJECT

public:
    ModifyDialogueAdvancedTable(const QString &, int, const QList<ModifyDialogueTableCell *> &, QWidget *);

protected slots:
    virtual void activateRow();

private:
    QLayout * addRowControlsLayout();
    void addHiddenRow(ModifyDialogueTableRow *);
    QList<ModifyDialogueTableCell *> hiddenAttributes();

    QComboBox * add_row_cb;

    int smallest_index;
    int category_id;
};

class ModifyDialogueBasicTable : public ModifyDialogueTable
{
    Q_OBJECT

public:
    ModifyDialogueBasicTable(const QString &, const QList<ModifyDialogueTableCell *> &, QWidget *);

private slots:
    void activateRow() {}

private:
    void addHiddenRow(ModifyDialogueTableRow *) {}
    QList<ModifyDialogueTableCell *> hiddenAttributes() { return QList<ModifyDialogueTableCell *>(); }
};

class ModifyDialogueTableWithAdjustableTotal : public ModifyDialogueAdvancedTable
{
    Q_OBJECT

public:
    ModifyDialogueTableWithAdjustableTotal(const QString &, int, const QList<ModifyDialogueTableCell *> &, QWidget *);

    void addRow(ModifyDialogueTableRow *);

protected slots:
    void reloadTotal();
    void calculatePricesFromTotal();
    void activateRow();

private:
    QDoubleSpinBox * total_w;
};

class ModifyDialogueTableCell
{
public:
    ModifyDialogueTableCell(const QVariant & _value, int _data_type = -1) {
        this->_value = _value;
        this->_data_type = _data_type;
    }
    ModifyDialogueTableCell(const QVariant & _value, const QString & _id, int _data_type = -1, const QString & unit = QString()) {
        this->_value = _value;
        this->_unit = unit;
        this->_unit = _unit.prepend(" ");
        this->_id = _id;
        this->_data_type = _data_type;
    }

    void setId(const QString & id) { this->_id = id; }
    const QString & id() { return _id; }

    const QVariant & value() { return _value; }
    int dataType() { return _data_type; }
    const QString & unit() { return _unit; }

private:
    QString _id;
    QString _unit;
    QVariant _value;
    int _data_type;
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
    const QString value(const QString & name);
    const QMap<QString, ModifyDialogueTableCell *> & valuesMap() { return values; }

    QToolButton * removeButton();
    const QString & name() { return row_name; }

    bool toBeDeleted() { return value("item_type_id").toInt() < 0; }

    double total();
    double listPrice();
    double acquisitionOrListPrice();
    void setListPrice(double);
    QVariant widgetValue(const QString &);

private slots:
    void remove();

signals:
    void removed(ModifyDialogueTableRow *);
    void valuesChanged();

private:
    QToolButton * remove_btn;
    QString row_name;
    QMap<QString, MDTInputWidget *> widgets;
    QMap<QString, ModifyDialogueTableCell *> values;
    bool in_table;
};

class MDTInputWidget
{
public:
    MDTInputWidget(QWidget * w) { this->w = w; }

    virtual QVariant variantValue() = 0;
    virtual void setVariantValue(const QVariant &) = 0;
    QWidget * widget() { return w; }

private:
    QWidget * w;
};

class MDTLineEdit : public QLineEdit, public MDTInputWidget
{
public:
    MDTLineEdit(const QString & text, QWidget * parent) : QLineEdit(text, parent), MDTInputWidget(this) {}

    QVariant variantValue() { return text(); }
    void setVariantValue(const QVariant & val) { setText(val.toString()); }
};

class MDTSpinBox : public QSpinBox, public MDTInputWidget
{
public:
    MDTSpinBox(QWidget * parent) : QSpinBox(parent), MDTInputWidget(this) { setMaximum(99999999); }

    QVariant variantValue() { return value(); }
    void setVariantValue(const QVariant & val) { setValue(val.toInt()); }
};

class MDTDoubleSpinBox : public QDoubleSpinBox, public MDTInputWidget
{
    Q_OBJECT

public:
    MDTDoubleSpinBox(QWidget * parent) : QDoubleSpinBox(parent), MDTInputWidget(this) { setMaximum(99999999.0); }

    QVariant variantValue() { return value(); }
    void setVariantValue(const QVariant & val) { setValue(val.toDouble()); }

public slots:
    void clear() { setValue(0.0); }
};

class MDTPlainTextEdit : public QPlainTextEdit, public MDTInputWidget
{
    Q_OBJECT

public:
    MDTPlainTextEdit(const QString & text, QWidget * parent) : QPlainTextEdit(text, parent), MDTInputWidget(this) {}

    QVariant variantValue() { return toPlainText(); }
    void setVariantValue(const QVariant & val) { setPlainText(val.toString()); }
};

class MDTCheckBox : public QCheckBox, public MDTInputWidget
{
    Q_OBJECT

public:
    MDTCheckBox(bool checked, QWidget * parent) : QCheckBox(parent), MDTInputWidget(this) { setChecked(checked); }

    QVariant variantValue() { return isChecked() ? 1 : 0; }
    void setVariantValue(const QVariant & val) { setChecked(val.toBool()); }
};

class MDTLabel : public QLabel, public MDTInputWidget
{
    Q_OBJECT

public:
    MDTLabel(const QString & text, QWidget * parent) : QLabel(text, parent), MDTInputWidget(this) {}

    QVariant variantValue() { return text(); }
    void setVariantValue(const QVariant & val) { setText(val.toString()); }
};

class MDTFileChooser : public DBFileChooser, public MDTInputWidget
{
    Q_OBJECT

public:
    MDTFileChooser(int value, QWidget * parent) : DBFileChooser(parent, value), MDTInputWidget(this) {}

    QVariant variantValue() { return DBFileChooser::variantValue(); }
    void setVariantValue(const QVariant &) {}
};

#endif // MODIFYDIALOGUETABLE_H
