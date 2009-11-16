/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2009 Matus & Michal Tomlein

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

#ifndef RECORDS_H
#define RECORDS_H

#include "global.h"
#include "mtcolourcombobox.h"
#include "mtcheckboxgroup.h"

#include <QCheckBox>
#include <QSpinBox>
#include <QDateEdit>
#include <QPlainTextEdit>

class ModifyDialogue;

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

protected:
    static QPalette paletteForColour(const QString &);
    static MTLabel * createLabel(QWidget * parent, const QString &);

private:
    QString iw_id;
    MTLabel * iw_label;
    QWidget * iw_widget;
};

class MDLineEdit : public QLineEdit, public MDInputWidget
{
    Q_OBJECT

public:
    MDLineEdit(const QString &, const QString &, QWidget *, const QString &, const QString & = QString(), const QString & = QString(), bool = true);

    QVariant variantValue();
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

private:
    MTDictionary cb_items;
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

class Modifiable
{
public:
    virtual ~Modifiable() {}

    virtual void initModifyDialogue(ModifyDialogue *) = 0;
};

class DBRecord : public MTRecord, public Modifiable
{
    Q_OBJECT

public:
    DBRecord();
    DBRecord(const QString &, const QString &, const QString &, const MTDictionary &);
};

class Customer : public DBRecord
{
    Q_OBJECT

public:
    Customer(const QString &);

    void initModifyDialogue(ModifyDialogue *);
};

class Circuit : public DBRecord
{
    Q_OBJECT

public:
    Circuit(const QString &, const QString &);

    void initModifyDialogue(ModifyDialogue *);
};

class Inspection : public DBRecord
{
    Q_OBJECT

public:
    Inspection(const QString &, const QString &, const QString &);

    void initModifyDialogue(ModifyDialogue *);
};

class Repair : public DBRecord
{
    Q_OBJECT

public:
    Repair(const QString &);

    void initModifyDialogue(ModifyDialogue *);
};

class VariableRecord : public DBRecord
{
    Q_OBJECT

public:
    enum Type { VARIABLE = 0, SUBVARIABLE = 1 };

    VariableRecord(Type, const QString &, const QString & = QString());

    void initModifyDialogue(ModifyDialogue *);

private:
    Type v_type;
};

class Table : public DBRecord
{
    Q_OBJECT

public:
    Table(const QString &, const QString & = QString());

    void initModifyDialogue(ModifyDialogue *);
};

class Inspector : public DBRecord
{
    Q_OBJECT

public:
    Inspector(const QString &);

    void initModifyDialogue(ModifyDialogue *);
};

class ServiceCompany : public DBRecord
{
    Q_OBJECT

public:
    ServiceCompany(const QString &);

    void initModifyDialogue(ModifyDialogue *);
};

class RecordOfRefrigerantManagement : public DBRecord
{
    Q_OBJECT

public:
    RecordOfRefrigerantManagement(const QString &);

    void initModifyDialogue(ModifyDialogue *);
};

class WarningRecord : public DBRecord
{
    Q_OBJECT

public:
    WarningRecord(const QString &);

    void initModifyDialogue(ModifyDialogue *);
};

#endif // RECORDS_H
