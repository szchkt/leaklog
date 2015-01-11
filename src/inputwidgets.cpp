/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2015 Matus & Michal Tomlein

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

#include "inputwidgets.h"
#include "highlighter.h"
#include "global.h"
#include "mainwindow.h"

#include <QVBoxLayout>
#include <QFileDialog>
#include <QRadioButton>
#include <QCalendarWidget>

using namespace Global;

#define BUTTON_STYLE "margin: 4; text-align: right;"

MTLabeledWidget::MTLabeledWidget(const QString &text, QWidget *w)
{
    changed = false;
    labeltext = text;
    altlabeltext = text;
    this->w = w;
}

MTLabel::MTLabel(const QString &text, QWidget *parent):
QLabel(text, parent),
MTLabeledWidget(text, this)
{}

MTButtonLabel::MTButtonLabel(const QString &text, QWidget *parent):
QPushButton(text, parent),
MTLabeledWidget(text, this)
{
    setStyleSheet(QString("QPushButton { %1 }").arg(BUTTON_STYLE));

    QObject::connect(this, SIGNAL(clicked()), this, SLOT(changeState()));
}

QSize MTButtonLabel::sizeHint() const
{
    QSize hint = QPushButton::sizeHint();
    hint.setWidth(hint.width() + 20);
    return hint;
}

void MTButtonLabel::changeState()
{
    toggleChanged();
    setStyleSheet(QString("QPushButton { %1 %2 }").arg(BUTTON_STYLE).arg(wasChanged() ? "font-weight: bold;" : ""));
}

void MTLabeledWidget::toggleAlternativeText(bool alt)
{
    if (alt)
        setLabelText(altlabeltext);
    else
        setLabelText(labeltext);
}

MDAbstractInputWidget::MDAbstractInputWidget(const QString &id, QWidget *widget)
{
    iw_id = id;
    iw_widget = widget;
    show = true;
    skip_save = false;
}

MainWindow *MDAbstractInputWidget::parentWindow() const
{
    QWidget *parent_widget = iw_widget->parentWidget();
    MainWindow *main_window;
    while (parent_widget) {
        main_window = qobject_cast<MainWindow *>(parent_widget);
        if (main_window)
            return main_window;
        parent_widget = parent_widget->parentWidget();
    }
    return NULL;
}

QPalette MDAbstractInputWidget::paletteForColour(const QString &colour)
{
    QPalette palette;
    palette.setColor(QPalette::Active, QPalette::Base, QColor(colour));
    palette.setColor(QPalette::Active, QPalette::Text, textColourForBaseColour(colour));
    palette.setColor(QPalette::Inactive, QPalette::Base, QColor(colour));
    palette.setColor(QPalette::Inactive, QPalette::Text, textColourForBaseColour(colour));
    return palette;
}

MDNullableInputWidget::MDNullableInputWidget(const QString &id, const QString &labeltext, QWidget *parent, QWidget *widget):
MDAbstractInputWidget(id, widget)
{
    MTButtonLabel *btn = new MTButtonLabel(labeltext, parent);
    btn->setFlat(true);
    //btn->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    iw_label = btn;
}

MDInputWidget::MDInputWidget(const QString &id, const QString &labeltext, QWidget *parent, QWidget *widget):
MDAbstractInputWidget(id, widget)
{
    MTLabel *lbl = new MTLabel(labeltext, parent);
    lbl->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    iw_label = lbl;
}

MDLineEdit::MDLineEdit(const QString &id, const QString &labeltext, QWidget *parent, const QString &value, int maxintvalue, const QString &colour, bool enabled):
QLineEdit(parent),
MDInputWidget(id, labeltext, parent, this)
{
    if (!colour.isEmpty()) { setPalette(paletteForColour(colour)); }
    setMinimumSize(150, sizeHint().height());
    setEnabled(enabled);
    if (maxintvalue) { setValidator(new QIntValidator(0, maxintvalue, parent)); }
    setText(value);
}

QVariant MDLineEdit::variantValue() const
{
    return text().isEmpty() ? nullvalue : text();
}

void MDLineEdit::setVariantValue(const QVariant &value)
{
    setText(value.toString());
}

void MDLineEdit::setNullValue(const QVariant &value)
{
    nullvalue = value;
}

MDCheckBox::MDCheckBox(const QString &id, const QString &labeltext, QWidget *parent, bool checked, bool enabled):
MTCheckBox(labeltext, parent),
MDInputWidget(id, "", parent, this)
{
    setEnabled(enabled);
    setChecked(checked);
}

QVariant MDCheckBox::variantValue() const
{
    return isChecked() ? 1 : 0;
}

void MDCheckBox::setVariantValue(const QVariant &value)
{
    setChecked(value.toBool());
}

MDSpinBox::MDSpinBox(const QString &id, const QString &labeltext, QWidget *parent, int minimum, int maximum, int value, const QString &suffix, const QString &
#ifndef Q_OS_MAC
    colour
#endif
    , bool enabled):
QSpinBox(parent),
MDInputWidget(id, labeltext, parent, this)
{
    installEventFilter(new WheelEventEater(this));
#ifndef Q_OS_MAC
    if (!colour.isEmpty()) { setPalette(paletteForColour(colour)); }
#endif
    setEnabled(enabled);
    setMinimum(minimum);
    setMaximum(maximum);
    setValue(value);
    if (!suffix.isEmpty()) { setSuffix(QString(" %1").arg(suffix)); }
}

QVariant MDSpinBox::variantValue() const
{
    return value();
}

void MDSpinBox::setVariantValue(const QVariant &value)
{
    setValue(value.toInt());
}

MDDoubleSpinBox::MDDoubleSpinBox(const QString &id, const QString &labeltext, QWidget *parent, double minimum, double maximum, double value, const QString &suffix, const QString &
#ifndef Q_OS_MAC
    colour
#endif
    ):
QDoubleSpinBox(parent),
MDInputWidget(id, labeltext, parent, this)
{
    installEventFilter(new WheelEventEater(this));
#ifndef Q_OS_MAC
    if (!colour.isEmpty()) { setPalette(paletteForColour(colour)); }
#endif
    setDecimals(3);
    setMinimum(minimum);
    setMaximum(maximum);
    setValue(value);
    if (!suffix.isEmpty()) { setSuffix(QString(" %1").arg(suffix)); }
}

QVariant MDDoubleSpinBox::variantValue() const
{
    return value();
}

void MDDoubleSpinBox::setVariantValue(const QVariant &value)
{
    setValue(value.toDouble());
}

MDNullableDoubleSpinBox::MDNullableDoubleSpinBox(const QString &id, const QString &labeltext, QWidget *parent, double minimum, double maximum, const QVariant &value, const QString &suffix, const QString &
#ifndef Q_OS_MAC
    colour
#endif
    ):
QDoubleSpinBox(parent),
MDNullableInputWidget(id, labeltext, parent, this)
{
    installEventFilter(new WheelEventEater(this));
#ifndef Q_OS_MAC
    if (!colour.isEmpty()) { setPalette(paletteForColour(colour)); }
#endif
    setDecimals(3);
    setMinimum(minimum);
    setMaximum(maximum);
    if (!value.isNull()) {
        setValue(value.toDouble());
        ((MTButtonLabel *)label())->changeState();
    }
    if (!suffix.isEmpty()) { setSuffix(QString(" %1").arg(suffix)); }

    QObject::connect(this, SIGNAL(valueChanged(double)), (MTButtonLabel *)label(), SLOT(setChanged()));
    QObject::connect((MTButtonLabel *)label(), SIGNAL(clicked()), this, SLOT(labelClicked()));
}

QVariant MDNullableDoubleSpinBox::variantValue() const
{
    return label()->wasChanged() ? value() : QVariant();
}

void MDNullableDoubleSpinBox::setVariantValue(const QVariant &value)
{
    setValue(value.toDouble());
}

void MDNullableDoubleSpinBox::labelClicked()
{
    if (((MTButtonLabel *)label())->wasChanged()) {
        setFocus();
        selectAll();
    } else {
        QObject::disconnect(this, SIGNAL(valueChanged(double)), (MTButtonLabel *)label(), SLOT(setChanged()));
        clear();
        QObject::connect(this, SIGNAL(valueChanged(double)), (MTButtonLabel *)label(), SLOT(setChanged()));
    }
}

MDComboBox::MDComboBox(const QString &id, const QString &labeltext, QWidget *parent, const QString &value, const MTDictionary &items, const QString &
#ifndef Q_OS_MAC
    colour
#endif
    , bool enabled):
QComboBox(parent),
MDInputWidget(id, labeltext, parent, this)
{
    installEventFilter(new WheelEventEater(this));
#ifndef Q_OS_MAC
    if (!colour.isEmpty()) { setPalette(paletteForColour(colour)); }
#endif
    setEnabled(enabled);
    QStringList list; int n = -1;
    for (int i = 0; i < items.count(); ++i) {
        if (!list.contains(items.value(i))) {
            list << items.value(i);
            addItem(items.value(i), items.key(i));
        }
        if (items.key(i) == value)
            n = list.indexOf(items.value(i));
    }
    if (n < 0) {
        if (value.isEmpty()) {
            n = 0;
        } else {
            addItem(tr("Unknown value (%1)").arg(value), value);
            n = count() - 1;
        }
    }
    setCurrentIndex(n);
}

QVariant MDComboBox::variantValue() const
{
    QString value = itemData(currentIndex()).toString();
    return value.isEmpty() ? nullvalue : value;
}

void MDComboBox::setVariantValue(const QVariant &value)
{
    for (int i = 0; i < count(); ++i) {
        if (itemData(i) == value) {
            setCurrentIndex(i);
            return;
        }
    }
    if (!value.toString().isEmpty()) {
        addItem(tr("Unknown value (%1)").arg(value.toString()), value);
        setCurrentIndex(count() - 1);
    }
}

void MDComboBox::setNullValue(const QVariant &value)
{
    nullvalue = value;
}

MDColourComboBox::MDColourComboBox(const QString &id, const QString &labeltext, QWidget *parent, const QString &value):
MTColourComboBox(parent),
MDInputWidget(id, labeltext, parent, this)
{
    installEventFilter(new WheelEventEater(this));
    for (int i = 0; i < count(); ++i) {
        if (itemText(i) == value) { setCurrentIndex(i); break; }
    }
}

QVariant MDColourComboBox::variantValue() const
{
    return currentText();
}

void MDColourComboBox::setVariantValue(const QVariant &value)
{
    for (int i = 0; i < count(); ++i) {
        if (itemText(i) == value.toString()) {
            setCurrentIndex(i);
            break;
        }
    }
}

MDDateTimeEdit::MDDateTimeEdit(const QString &id, const QString &labeltext, QWidget *parent, const QString &value):
QDateTimeEdit(parent),
MDInputWidget(id, labeltext, parent, this)
{
    installEventFilter(new WheelEventEater(this));
    MainWindow *main_window = parentWindow();
    if (main_window)
        setDisplayFormat(main_window->settings().dateTimeFormatString());
    else
        setDisplayFormat(DATE_TIME_FORMAT);
    setDateTime(value.isEmpty() ? QDateTime::currentDateTime() : QDateTime::fromString(value, DATE_TIME_FORMAT));
    setCalendarPopup(true);
    calendarWidget()->setLocale(QLocale());
#if QT_VERSION < QT_VERSION_CHECK(4, 8, 0)
    calendarWidget()->setFirstDayOfWeek(Qt::Monday);
#else
    calendarWidget()->setFirstDayOfWeek(QLocale().firstDayOfWeek());
#endif
}

QVariant MDDateTimeEdit::variantValue() const
{
    return dateTime().toString(DATE_TIME_FORMAT);
}

void MDDateTimeEdit::setVariantValue(const QVariant &value)
{
    setDateTime(QDateTime::fromString(value.toString(), DATE_TIME_FORMAT));
}

MDDateEdit::MDDateEdit(const QString &id, const QString &labeltext, QWidget *parent, const QString &value):
QDateEdit(parent),
MDInputWidget(id, labeltext, parent, this)
{
    installEventFilter(new WheelEventEater(this));
    MainWindow *main_window = parentWindow();
    if (main_window)
        setDisplayFormat(main_window->settings().dateFormatString());
    else
        setDisplayFormat(DATE_FORMAT);
    setDate(value.isEmpty() ? QDate::currentDate() : QDate::fromString(value, DATE_FORMAT));
    setCalendarPopup(true);
    calendarWidget()->setLocale(QLocale());
#if QT_VERSION < QT_VERSION_CHECK(4, 8, 0)
    calendarWidget()->setFirstDayOfWeek(Qt::Monday);
#else
    calendarWidget()->setFirstDayOfWeek(QLocale().firstDayOfWeek());
#endif
}

QVariant MDDateEdit::variantValue() const
{
    return date().toString(DATE_FORMAT);
}

void MDDateEdit::setVariantValue(const QVariant &value)
{
    setDate(QDate::fromString(value.toString(), DATE_FORMAT));
}

MDAddressEdit::MDAddressEdit(const QString &id, const QString &labeltext, QWidget *parent, const QString &value):
MTAddressEdit(parent),
MDInputWidget(id, labeltext, parent, this)
{
    setMinimumSize(200, sizeHint().height());
    setAddress(MTAddress(value));
}

QVariant MDAddressEdit::variantValue() const
{
    return address().toString();
}

void MDAddressEdit::setVariantValue(const QVariant &value)
{
    setAddress(MTAddress(value.toString()));
}

MDHighlightedPlainTextEdit::MDHighlightedPlainTextEdit(const QString &id, const QString &labeltext, QWidget *parent, const QString &value, const QStringList &ids, bool enabled):
QPlainTextEdit(parent),
MDInputWidget(id, labeltext, parent, this)
{
    setEnabled(enabled);
    setMinimumSize(200, 30);
    setPlainText(value);
    new Highlighter(ids, document());
}

QVariant MDHighlightedPlainTextEdit::variantValue() const
{
    return toPlainText();
}

void MDHighlightedPlainTextEdit::setVariantValue(const QVariant &value)
{
    setPlainText(value.toString());
}

MDPlainTextEdit::MDPlainTextEdit(const QString &id, const QString &labeltext, QWidget *parent, const QString &value, const QString &colour, bool enabled):
QPlainTextEdit(parent),
MDInputWidget(id, labeltext, parent, this)
{
    setEnabled(enabled);
    if (!colour.isEmpty()) { setPalette(paletteForColour(colour)); }
    setMinimumSize(200, 30);
    setPlainText(value);
}

QVariant MDPlainTextEdit::variantValue() const
{
    return toPlainText();
}

void MDPlainTextEdit::setVariantValue(const QVariant &value)
{
    setPlainText(value.toString());
}

MDGroupedCheckBoxes::MDGroupedCheckBoxes(const QString &id, const QString &labeltext, QWidget *parent, int grouped_value):
QGroupBox(" ", parent),
MDInputWidget(id, labeltext, parent, this)
{
    this->grouped_value = grouped_value;
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(12);
}

MDGroupedCheckBoxes::~MDGroupedCheckBoxes()
{
    for (int i = checkboxes.count() - 1; i >= 0; --i) {
        delete checkboxes.takeAt(i);
    }
}

void MDGroupedCheckBoxes::addCheckBox(int chb_id, const QString &chb_name)
{
    MDCheckBox *chb = new MDCheckBox(QString::number(chb_id), chb_name, this, grouped_value & chb_id);
    checkboxes.append(chb);

    layout()->addWidget(chb);
}

QVariant MDGroupedCheckBoxes::variantValue() const
{
    int value = 0;

    for (int i = 0; i < checkboxes.count(); ++i) {
        if (checkboxes.at(i)->variantValue().toBool()) {
            value |= checkboxes.at(i)->id().toInt();
        }
    }

    return QVariant(value);
}

MDFileChooser::MDFileChooser(const QString &id, const QString &labeltext, QWidget *parent, int file_id):
DBFileChooser(parent, file_id),
MDInputWidget(id, labeltext, parent, this)
{
}

QVariant MDFileChooser::variantValue() const
{
    return DBFileChooser::variantValue();
}

MDGroupedInputWidgets::MDGroupedInputWidgets(const QString &name, QWidget *parent):
QFrame(parent),
MDInputWidget(QString(), name, parent, this)
{
    setSkipSave(true);
    setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
    grid = new QGridLayout(this);
    grid->setSpacing(6);
    grid->setContentsMargins(6, 6, 6, 6);
}

void MDGroupedInputWidgets::addWidget(MDAbstractInputWidget *iw)
{
    grid->addWidget(iw->label()->widget(), grid->rowCount(), 0);
    grid->addWidget(iw->widget(), grid->rowCount() - 1, 1);
}

MDRadioButtonGroup::MDRadioButtonGroup(const QString &id, const QString &labeltext, QWidget *parent, const QString &value):
        QGroupBox(" ", parent),
        MDInputWidget(id, labeltext, parent, this)
{
    this->selected = value;
    (new QVBoxLayout(this))->setSpacing(12);
}

void MDRadioButtonGroup::addRadioButton(const QString &name, const QString &value)
{
    QRadioButton *radio = new QRadioButton(name, this);
    radio->setChecked(value == selected);
    radiobuttons.insert(radio, value);
    this->layout()->addWidget(radio);
}

QVariant MDRadioButtonGroup::variantValue() const
{
    QMapIterator<QRadioButton *, QString> i(radiobuttons);
    while (i.hasNext()) { i.next();
        if (i.key()->isChecked())
            return i.value();
    }
    return QVariant();
}

MDHiddenIdField::MDHiddenIdField(const QString &id, QWidget *parent, const QVariant &value):
MDInputWidget(id, "", parent, NULL)
{
    setShowInForm(false);
    iw_label->setVisible(false);
    setVariantValue(value);
}

QVariant MDHiddenIdField::variantValue() const
{
    return value;
}

void MDHiddenIdField::setVariantValue(const QVariant &value)
{
    this->value = value;
}
