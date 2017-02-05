/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2017 Matus & Michal Tomlein

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

#ifndef EDITDIALOGUETABLE_H
#define EDITDIALOGUETABLE_H

#include <QGroupBox>
#include <QLineEdit>
#include <QAction>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QPlainTextEdit>
#include <QLabel>
#include <QToolButton>

#include "mtdictionary.h"
#include "dbfile.h"

class QGridLayout;
class QComboBox;
class QLabel;
class QToolButton;
class QVBoxLayout;
class QTreeWidget;
class QTreeWidgetItem;
class QHBoxLayout;

class EditDialogueTableRow;
class EditDialogueTableCell;
class EditDialogueBasicTableRow;
class MDTInputWidget;

class EditDialogueTable : public QWidget
{
    Q_OBJECT

public:
    enum RowType {
        Default,
        Removable,
        Hidable
    };

    EditDialogueTable(const QString &, const QList<EditDialogueTableCell *> &, QWidget *);
    ~EditDialogueTable();

    void addRow(const QMap<QString, EditDialogueTableCell *> &values, bool display = true, RowType row_type = Removable);
    virtual void addRow(EditDialogueTableRow *);
    QList<MTDictionary> allValues() const;

    int rowsCount() const { return rows.count(); }

public slots:
    void addNewRow();

private slots:
    void rowRemoved(EditDialogueTableRow *);

protected:
    virtual void addHiddenRow(EditDialogueTableRow *) = 0;
    virtual QList<EditDialogueTableCell *> hiddenAttributes() = 0;

    QVBoxLayout *layout;
    QTreeWidget *tree;
    QHBoxLayout *title_layout;

    QList<EditDialogueTableCell *> header;
    QList<EditDialogueTableRow *> rows;
};

class EditDialogueAdvancedTable : public EditDialogueTable
{
    Q_OBJECT

public:
    EditDialogueAdvancedTable(const QString &, int, const QList<EditDialogueTableCell *> &, QWidget *);

protected slots:
    virtual void activateRow();

private:
    QLayout *addRowControlsLayout();
    void addHiddenRow(EditDialogueTableRow *);
    QList<EditDialogueTableCell *> hiddenAttributes();

    QComboBox *add_row_cb;

    int smallest_index;
    int category_id;
};

class EditDialogueBasicTable : public EditDialogueTable
{
    Q_OBJECT

public:
    EditDialogueBasicTable(const QString &, const QList<EditDialogueTableCell *> &, QWidget *);

private slots:
    void activateRow() {}

private:
    void addHiddenRow(EditDialogueTableRow *) {}
    QList<EditDialogueTableCell *> hiddenAttributes() { return QList<EditDialogueTableCell *>(); }
};

class EditDialogueTableWithAdjustableTotal : public EditDialogueAdvancedTable
{
    Q_OBJECT

public:
    EditDialogueTableWithAdjustableTotal(const QString &, int, const QList<EditDialogueTableCell *> &, QWidget *);

    void addRow(EditDialogueTableRow *);

protected slots:
    void reloadTotal();
    void calculatePricesFromTotal();
    void activateRow();

private:
    QDoubleSpinBox *total_w;
};

class EditDialogueTableCell
{
public:
    EditDialogueTableCell(const QVariant &value, int data_type = -1) {
        _value = value;
        _data_type = data_type;
    }
    EditDialogueTableCell(const QVariant &value, const QString &id, int data_type = -1, const QString &unit = QString()) {
        _value = value;
        _unit = unit;
        if (!_unit.isEmpty())
            _unit.prepend(" ");
        _id = id;
        _data_type = data_type;
    }

    void setId(const QString &id) { _id = id; }
    const QString &id() const { return _id; }

    const QVariant &value() const { return _value; }
    int dataType() const { return _data_type; }
    const QString &unit() const { return _unit; }

private:
    QString _id;
    QString _unit;
    QVariant _value;
    int _data_type;
};

class EditDialogueTableRow : public QObject
{
    Q_OBJECT

public:
    EditDialogueTableRow(const QMap<QString, EditDialogueTableCell *> &values, bool display, EditDialogueTable::RowType row_type, QTreeWidget *tree);
    ~EditDialogueTableRow();

    void addWidget(const QString &, MDTInputWidget *);
    const MTDictionary dictValues() const;
    bool isInTable() const { return in_table; }
    void setInTable(bool in_table) { this->in_table = in_table; }

    const QString itemTypeId() const;
    const QString value(const QString &name) const;
    const QMap<QString, EditDialogueTableCell *> &valuesMap() const { return values; }

    QToolButton *removeButton();
    const QString &name() const { return row_name; }

    bool toBeDeleted() const { return value("item_type_id").toInt() < 0; }

    double total() const;
    double listPrice() const;
    double acquisitionOrListPrice() const;
    void setListPrice(double);
    QVariant widgetValue(const QString &) const;

    QTreeWidgetItem *treeItem();
    QTreeWidgetItem *takeTreeItem();

private slots:
    void remove();
    void toggleHidden();

signals:
    void removed(EditDialogueTableRow *);
    void valuesChanged();

private:
    QToolButton *remove_btn;
    QString row_name;
    QMap<QString, MDTInputWidget *> widgets;
    QMap<QString, EditDialogueTableCell *> values;
    QTreeWidget *m_tree;
    QTreeWidgetItem *m_tree_item;
    bool in_table;
    EditDialogueTable::RowType row_type;
};

class MDTInputWidget
{
public:
    MDTInputWidget(QWidget *w) { this->w = w; }
    virtual ~MDTInputWidget() {}

    virtual QVariant variantValue() const = 0;
    virtual void setVariantValue(const QVariant &) = 0;
    QWidget *widget() const { return w; }

private:
    QWidget *w;
};

class MDTLineEdit : public QLineEdit, public MDTInputWidget
{
public:
    MDTLineEdit(const QString &text, QWidget *parent) : QLineEdit(text, parent), MDTInputWidget(this) {}
    virtual ~MDTLineEdit() {}

    QVariant variantValue() const { return text(); }
    void setVariantValue(const QVariant &val) { setText(val.toString()); }
};

class MDTSpinBox : public QSpinBox, public MDTInputWidget
{
public:
    MDTSpinBox(QWidget *parent) : QSpinBox(parent), MDTInputWidget(this) { setMaximum(99999999); }
    virtual ~MDTSpinBox() {}

    QVariant variantValue() const { return value(); }
    void setVariantValue(const QVariant &val) { setValue(val.toInt()); }
};

class MDTDoubleSpinBox : public QDoubleSpinBox, public MDTInputWidget
{
    Q_OBJECT

public:
    MDTDoubleSpinBox(QWidget *parent) : QDoubleSpinBox(parent), MDTInputWidget(this) { setMaximum(99999999.0); }
    virtual ~MDTDoubleSpinBox() {}

    QVariant variantValue() const { return value(); }
    void setVariantValue(const QVariant &val) { setValue(val.toDouble()); }

public slots:
    void clear() { setValue(0.0); }
};

class MDTPlainTextEdit : public QPlainTextEdit, public MDTInputWidget
{
    Q_OBJECT

public:
    MDTPlainTextEdit(const QString &text, QWidget *parent) : QPlainTextEdit(text, parent), MDTInputWidget(this) {}
    virtual ~MDTPlainTextEdit() {}

    QVariant variantValue() const { return toPlainText(); }
    void setVariantValue(const QVariant &val) { setPlainText(val.toString()); }
};

class MDTCheckBox : public QCheckBox, public MDTInputWidget
{
    Q_OBJECT

public:
    MDTCheckBox(bool checked, QWidget *parent) : QCheckBox(parent), MDTInputWidget(this) { setChecked(checked); }
    virtual ~MDTCheckBox() {}

    QVariant variantValue() const { return isChecked() ? 1 : 0; }
    void setVariantValue(const QVariant &val) { setChecked(val.toBool()); }
};

class MDTLabel : public QLineEdit, public MDTInputWidget
{
    Q_OBJECT

public:
    MDTLabel(const QString &text, QWidget *parent) : QLineEdit(text, parent), MDTInputWidget(this) { setReadOnly(true); }
    virtual ~MDTLabel() {}

    QVariant variantValue() const { return text(); }
    void setVariantValue(const QVariant &val) { setText(val.toString()); }
};

class MDTFileChooser : public DBFileChooser, public MDTInputWidget
{
    Q_OBJECT

public:
    MDTFileChooser(int value, QWidget *parent) : DBFileChooser(parent, value), MDTInputWidget(this) {}
    virtual ~MDTFileChooser() {}

    QVariant variantValue() const { return DBFileChooser::variantValue(); }
    void setVariantValue(const QVariant &) {}
};

class MDTToolButton : public QToolButton, public MDTInputWidget
{
    Q_OBJECT

public:
    MDTToolButton(QWidget *parent = NULL) : QToolButton(parent), MDTInputWidget(this) {}
    virtual ~MDTToolButton() {}

    QVariant variantValue() const { return (int)isChecked(); }
    void setVariantValue(const QVariant &val) { setChecked(val.toInt()); }
};

#endif // EDITDIALOGUETABLE_H
