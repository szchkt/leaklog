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

#include "input_widgets.h"
#include "highlighter.h"
#include "global.h"

using namespace Global;

MTLabel::MTLabel(const QString & text, QWidget * parent):
QLabel(text, parent) {
    labeltext = text;
    altlabeltext = text;
}

void MTLabel::toggleAlternativeText(bool alt)
{
    if (alt)
        setText(altlabeltext);
    else
        setText(labeltext);
}

MDInputWidget::MDInputWidget(const QString & id, const QString & labeltext, QWidget * parent, QWidget * widget)
{
    iw_id = id;
    iw_label = createLabel(parent, labeltext);
    iw_widget = widget;
}

QPalette MDInputWidget::paletteForColour(const QString & colour)
{
    QPalette palette;
    palette.setColor(QPalette::Active, QPalette::Base, QColor::QColor(colour));
    palette.setColor(QPalette::Active, QPalette::Text, textColourForBaseColour(colour));
    palette.setColor(QPalette::Inactive, QPalette::Base, QColor::QColor(colour));
    palette.setColor(QPalette::Inactive, QPalette::Text, textColourForBaseColour(colour));
    return palette;
}

MTLabel * MDInputWidget::createLabel(QWidget * parent, const QString & text)
{
    MTLabel * lbl = new MTLabel(text, parent);
    lbl->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    return lbl;
}

MDLineEdit::MDLineEdit(const QString & id, const QString & labeltext, QWidget * parent, const QString & value, int maxintvalue, const QString & colour, bool enabled):
QLineEdit(parent),
MDInputWidget(id, labeltext, parent, this)
{
    if (!colour.isEmpty()) { setPalette(paletteForColour(colour)); }
    setMinimumSize(150, sizeHint().height());
    setEnabled(enabled);
    if (maxintvalue) { setValidator(new QIntValidator(0, maxintvalue, parent)); }
    setText(value);
}

QVariant MDLineEdit::variantValue()
{
    return text().isEmpty() ? nullvalue : text();
}

void MDLineEdit::setNullValue(const QVariant & value)
{
    nullvalue = value;
}

MDCheckBox::MDCheckBox(const QString & id, const QString & labeltext, QWidget * parent, bool checked, bool enabled):
MTCheckBox(labeltext, parent),
MDInputWidget(id, "", parent, this)
{
    setEnabled(enabled);
    setChecked(checked);
}

QVariant MDCheckBox::variantValue()
{
    return isChecked() ? 1 : 0;
}

MDSpinBox::MDSpinBox(const QString & id, const QString & labeltext, QWidget * parent, int minimum, int maximum, int value, const QString & suffix, const QString &
#ifndef Q_WS_MAC
    colour
#endif
    , bool enabled):
QSpinBox(parent),
MDInputWidget(id, labeltext, parent, this)
{
#ifndef Q_WS_MAC
    if (!colour.isEmpty()) { setPalette(paletteForColour(colour)); }
#endif
    setEnabled(enabled);
    setMinimum(minimum);
    setMaximum(maximum);
    setValue(value);
    if (!suffix.isEmpty()) { setSuffix(QString(" %1").arg(suffix)); }
}

QVariant MDSpinBox::variantValue()
{
    return value();
}

MDDoubleSpinBox::MDDoubleSpinBox(const QString & id, const QString & labeltext, QWidget * parent, double minimum, double maximum, double value, const QString & suffix, const QString &
#ifndef Q_WS_MAC
    colour
#endif
    ):
QDoubleSpinBox(parent),
MDInputWidget(id, labeltext, parent, this)
{
#ifndef Q_WS_MAC
    if (!colour.isEmpty()) { setPalette(paletteForColour(colour)); }
#endif
    setDecimals(3);
    setMinimum(minimum);
    setMaximum(maximum);
    setValue(value);
    if (!suffix.isEmpty()) { setSuffix(QString(" %1").arg(suffix)); }
}

QVariant MDDoubleSpinBox::variantValue()
{
    return value();
}

MDComboBox::MDComboBox(const QString & id, const QString & labeltext, QWidget * parent, const QString & value, const MTDictionary & items, const QString &
#ifndef Q_WS_MAC
    colour
#endif
    , bool enabled):
QComboBox(parent),
MDInputWidget(id, labeltext, parent, this)
{
    cb_items = items;
#ifndef Q_WS_MAC
    if (!colour.isEmpty()) { setPalette(paletteForColour(colour)); }
#endif
    setEnabled(enabled);
    QStringList list; int n = 0;
    for (int i = 0; i < items.count(); ++i) {
        if (!list.contains(items.key(i))) {
            list << items.key(i);
            addItem(items.key(i));
        }
        if (items.value(i) == value) { n = list.indexOf(items.key(i)); }
    }
    setCurrentIndex(n);
}

QVariant MDComboBox::variantValue()
{
    QString value = cb_items.value(currentText());
    return value.isEmpty() ? nullvalue : value;
}

void MDComboBox::setNullValue(const QVariant & value)
{
    nullvalue = value;
}

MDColourComboBox::MDColourComboBox(const QString & id, const QString & labeltext, QWidget * parent, const QString & value):
MTColourComboBox(parent),
MDInputWidget(id, labeltext, parent, this)
{
    for (int i = 0; i < count(); ++i) {
        if (itemText(i) == value) { setCurrentIndex(i); break; }
    }
}

QVariant MDColourComboBox::variantValue()
{
    return currentText();
}

MDDateTimeEdit::MDDateTimeEdit(const QString & id, const QString & labeltext, QWidget * parent, const QString & value):
QDateTimeEdit(parent),
MDInputWidget(id, labeltext, parent, this)
{
    setDisplayFormat("yyyy.MM.dd-hh:mm");
    setDateTime(value.isEmpty() ? QDateTime::currentDateTime() : QDateTime::fromString(value, "yyyy.MM.dd-hh:mm"));
}

QVariant MDDateTimeEdit::variantValue()
{
    return dateTime().toString("yyyy.MM.dd-hh:mm");
}

MDDateEdit::MDDateEdit(const QString & id, const QString & labeltext, QWidget * parent, const QString & value):
QDateEdit(parent),
MDInputWidget(id, labeltext, parent, this)
{
    setDisplayFormat("yyyy.MM.dd");
    setDate(value.isEmpty() ? QDate::currentDate() : QDate::fromString(value, "yyyy.MM.dd"));
}

QVariant MDDateEdit::variantValue()
{
    return date().toString("yyyy.MM.dd");
}

MDAddressEdit::MDAddressEdit(const QString & id, const QString & labeltext, QWidget * parent, const QString & value):
MTAddressEdit(parent),
MDInputWidget(id, labeltext, parent, this)
{
    setMinimumSize(200, sizeHint().height());
    setAddress(MTAddress(value));
}

QVariant MDAddressEdit::variantValue()
{
    return address().toString();
}

MDHighlightedPlainTextEdit::MDHighlightedPlainTextEdit(const QString & id, const QString & labeltext, QWidget * parent, const QString & value, const QStringList & ids, bool enabled):
QPlainTextEdit(parent),
MDInputWidget(id, labeltext, parent, this)
{
    setEnabled(enabled);
    setMinimumSize(200, 30);
    setPlainText(value);
    new Highlighter(ids, document());
}

QVariant MDHighlightedPlainTextEdit::variantValue()
{
    return toPlainText();
}

MDPlainTextEdit::MDPlainTextEdit(const QString & id, const QString & labeltext, QWidget * parent, const QString & value, const QString & colour, bool enabled):
QPlainTextEdit(parent),
MDInputWidget(id, labeltext, parent, this)
{
    setEnabled(enabled);
    if (!colour.isEmpty()) { setPalette(paletteForColour(colour)); }
    setMinimumSize(200, 30);
    setPlainText(value);
}

QVariant MDPlainTextEdit::variantValue()
{
    return toPlainText();
}
