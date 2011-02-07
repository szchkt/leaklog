/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2010 Matus & Michal Tomlein

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

#ifndef INPUT_WIDGETS_H
#define INPUT_WIDGETS_H

#include "mtcolourcombobox.h"
#include "mtcheckboxgroup.h"
#include "mtaddress.h"
#include "mtdictionary.h"

#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QSpinBox>
#include <QDateEdit>
#include <QPlainTextEdit>
#include <QGroupBox>

class DBFile;

class MTLabel : public QLabel
{
    Q_OBJECT

public:
    MTLabel(const QString & text, QWidget * parent);

    void setAlternativeText(const QString & alt) { altlabeltext = alt; }

public slots:
    void toggleAlternativeText(bool);

private:
    QString labeltext;
    QString altlabeltext;
};

class MDInputWidget
{
public:
    MDInputWidget(const QString &, const QString &, QWidget *, QWidget *);
    virtual ~MDInputWidget() {}

    virtual QVariant variantValue() = 0;

    inline QString id() { return iw_id; }
    inline MTLabel * label() { return iw_label; }
    inline QWidget * widget() { return iw_widget; }

    bool showInForm() { return show; }
    void setShowInForm(bool show) { this->show = show; }

protected:
    static QPalette paletteForColour(const QString &);
    static MTLabel * createLabel(QWidget * parent, const QString &);

private:
    bool show;
    QString iw_id;
    MTLabel * iw_label;
    QWidget * iw_widget;
};

class MDLineEdit : public QLineEdit, public MDInputWidget
{
    Q_OBJECT

public:
    MDLineEdit(const QString &, const QString &, QWidget *, const QString &, int = 0, const QString & = QString(), bool = true);

    QVariant variantValue();

    void setNullValue(const QVariant &);

private:
    QVariant nullvalue;
};

class MDCheckBox : public MTCheckBox, public MDInputWidget
{
    Q_OBJECT

public:
    MDCheckBox(const QString &, const QString &, QWidget *, bool, bool = true);

    QVariant variantValue();
};

class MDSpinBox : public QSpinBox, public MDInputWidget
{
    Q_OBJECT

public:
    MDSpinBox(const QString &, const QString &, QWidget *, int, int, int, const QString & = QString(), const QString & = QString(), bool = true);

    QVariant variantValue();
};

class MDDoubleSpinBox : public QDoubleSpinBox, public MDInputWidget
{
    Q_OBJECT

public:
    MDDoubleSpinBox(const QString &, const QString &, QWidget *, double, double, double, const QString & = QString(), const QString & = QString());

    QVariant variantValue();

public slots:
    void clear() { setValue(0.0); }
};

class MDComboBox : public QComboBox, public MDInputWidget
{
    Q_OBJECT

public:
    MDComboBox(const QString &, const QString &, QWidget *, const QString &, const MTDictionary &, const QString & = QString(), bool = true);

    QVariant variantValue();

    void setNullValue(const QVariant &);

private:
    MTDictionary cb_items;
    QVariant nullvalue;
};

class MDColourComboBox : public MTColourComboBox, public MDInputWidget
{
    Q_OBJECT

public:
    MDColourComboBox(const QString &, const QString &, QWidget *, const QString &);

    QVariant variantValue();
};

class MDDateTimeEdit : public QDateTimeEdit, public MDInputWidget
{
    Q_OBJECT

public:
    MDDateTimeEdit(const QString &, const QString &, QWidget *, const QString &);

    QVariant variantValue();
};

class MDDateEdit : public QDateEdit, public MDInputWidget
{
    Q_OBJECT

public:
    MDDateEdit(const QString &, const QString &, QWidget *, const QString &);

    QVariant variantValue();
};

class MDAddressEdit : public MTAddressEdit, public MDInputWidget
{
    Q_OBJECT

public:
    MDAddressEdit(const QString &, const QString &, QWidget *, const QString &);

    QVariant variantValue();
};

class MDHighlightedPlainTextEdit : public QPlainTextEdit, public MDInputWidget
{
    Q_OBJECT

public:
    MDHighlightedPlainTextEdit(const QString &, const QString &, QWidget *, const QString &, const QStringList &, bool = true);

    QVariant variantValue();
};

class MDPlainTextEdit : public QPlainTextEdit, public MDInputWidget
{
    Q_OBJECT

public:
    MDPlainTextEdit(const QString &, const QString &, QWidget *, const QString &, const QString & = QString(), bool = true);

    QVariant variantValue();
};

class MDGroupedCheckBoxes : public QGroupBox, public MDInputWidget
{
    Q_OBJECT

public:
    MDGroupedCheckBoxes(const QString &, const QString &, QWidget *, int);
    ~MDGroupedCheckBoxes();

    void addCheckBox(int, const QString &);

    QVariant variantValue();

private:
    QList<MDCheckBox *> checkboxes;
    int grouped_value;
};

class MDFileChooser : public QWidget, public MDInputWidget
{
    Q_OBJECT

public:
    MDFileChooser(const QString &, const QString &, QWidget *, int);

    QVariant variantValue();

private slots:
    void browse();

private:
    QLabel * name_lbl;
    DBFile * db_file;
};

#endif // INPUT_WIDGETS_H
