/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2025 Matus & Michal Tomlein

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
#include "dbfile.h"

#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QSpinBox>
#include <QDateEdit>
#include <QPlainTextEdit>
#include <QGroupBox>
#include <QPushButton>

class QRadioButton;
class QGridLayout;

class MainWindow;

class WheelEventEater : public QObject
{
    Q_OBJECT

public:
    WheelEventEater(QObject *parent): QObject(parent) {}

protected:
    bool eventFilter(QObject *obj, QEvent *event) {
        if (event->type() == QEvent::Wheel) {
            event->ignore();
            return true;
        }
        return QObject::eventFilter(obj, event);
    }
};

class MTObject
{
public:
    virtual ~MTObject() {}

    bool addConnection(const QObject *sender, const char *signal, const char *method)
    { return dynamic_cast<QObject *>(this)->connect(sender, signal, method); }
};

class MTLabeledWidget : public MTObject
{
public:
    MTLabeledWidget(const QString &, QWidget *);

    void setDefaultText(const QString &text) { labeltext = text; }
    void setAlternativeText(const QString &alt) { altlabeltext = alt; }
    QWidget *widget() const { return w; }

    virtual void setLabelText(const QString &) = 0;

    bool wasChanged() const { return changed; }
    void toggleChanged() { changed = !changed; }

    void toggleAlternativeText(bool);

    void setVisible(bool visible) { w->setVisible(visible); }

protected:
    QString labeltext;
    QString altlabeltext;
    QWidget *w;

    bool changed;
};

class MTLabel : public QLabel, public MTLabeledWidget
{
    Q_OBJECT

public:
    MTLabel(const QString &text, QWidget *parent);

    void setLabelText(const QString &t) { MTLabel::setText(t); }

public slots:
    void toggleAlternativeText(bool alt) { MTLabeledWidget::toggleAlternativeText(alt); }
};

class MTButtonLabel : public QPushButton, public MTLabeledWidget
{
    Q_OBJECT

public:
    MTButtonLabel(const QString &text, QWidget *parent);

    void setLabelText(const QString &t) { QPushButton::setText(t); }

    QSize sizeHint() const;

public slots:
    void toggleAlternativeText(bool alt) { MTLabeledWidget::toggleAlternativeText(alt); }
    void changeState();
    void setChanged() { if (!wasChanged()) changeState(); }
};

class MDAbstractInputWidget
{
public:
    MDAbstractInputWidget(const QString &, QWidget *);
    virtual ~MDAbstractInputWidget() {}

    virtual QVariant variantValue() const = 0;
    virtual void setVariantValue(const QVariant &) = 0;

    inline QString id() const { return iw_id; }
    inline void setId(const QString &id) { iw_id = id; }
    inline bool hasGroupLabel() const { return iw_group_label != NULL; }
    MTLabeledWidget *groupLabel();
    inline MTLabeledWidget *label() const { return iw_label; }
    inline QWidget *widget() const { return iw_widget; }

    inline QString groupId() const { return iw_group_id; }
    inline void setGroupId(const QString &id) { iw_group_id = id; }

    inline QString colour() const { return iw_colour; }
    inline void setColour(const QString &colour) { iw_colour = colour; }

    int rowSpan() const { return row_span; }
    void setRowSpan(int row_span) { this->row_span = row_span; }

    bool skipSave() const { return skip_save; }
    void setSkipSave(bool skip) { skip_save = skip; }

    void setVisible(bool visible) {
        iw_label->setVisible(visible);
        iw_widget->setVisible(visible);
    }

protected:
    MainWindow *parentWindow() const;

    static QPalette paletteForColour(const QString &);

    MTLabeledWidget *iw_label;

private:
    int row_span;
    bool skip_save;
    QString iw_id;
    MTLabeledWidget *iw_group_label;
    QWidget *iw_widget;
    QString iw_group_id;
    QString iw_colour;
};

class MDNullableInputWidget : public MDAbstractInputWidget
{
public:
    MDNullableInputWidget(const QString &, const QString &, QWidget *, QWidget *);

protected:
    static QWidget *createLabel(QWidget *parent, const QString &);
};

class MDInputWidget : public MDAbstractInputWidget
{
public:
    MDInputWidget(const QString &, const QString &, QWidget *, QWidget *);
};

class MDLineEdit : public QLineEdit, public MDInputWidget
{
    Q_OBJECT

public:
    enum {
        CompanyID = -1
    };

    MDLineEdit(const QString &, const QString &, QWidget *, const QString &, int = 0, const QString & = QString(), bool = true);

    QVariant variantValue() const;
    void setVariantValue(const QVariant &);

    void setNullValue(const QVariant &);

protected:
    QVariant nullvalue;
};

class MDCompanyIDEdit : public MDLineEdit
{
    Q_OBJECT

public:
    MDCompanyIDEdit(const QString &, const QString &, QWidget *, const QString &);

    QVariant variantValue() const;
    void setVariantValue(const QVariant &);
};

class MDCheckBox : public MTCheckBox, public MDInputWidget
{
    Q_OBJECT

public:
    MDCheckBox(const QString &, const QString &, QWidget *, bool, bool = true);

    QVariant variantValue() const;
    void setVariantValue(const QVariant &);
};

class MDSpinBox : public QSpinBox, public MDInputWidget
{
    Q_OBJECT

public:
    MDSpinBox(const QString &, const QString &, QWidget *, int, int, int, const QString & = QString(), const QString & = QString(), bool = true);

    QVariant variantValue() const;
    void setVariantValue(const QVariant &);
};

class MDDoubleSpinBox : public QDoubleSpinBox, public MDInputWidget
{
    Q_OBJECT

public:
    MDDoubleSpinBox(const QString &, const QString &, QWidget *, double, double, double, const QString & = QString(), const QString & = QString());

    QVariant variantValue() const;
    void setVariantValue(const QVariant &);

public slots:
    void clear() { setValue(0.0); }
};

class MDNullableDoubleSpinBox : public QDoubleSpinBox, public MDNullableInputWidget
{
    Q_OBJECT

public:
    MDNullableDoubleSpinBox(const QString &, const QString &, QWidget *, double, double, const QVariant &, const QString & = QString(), const QString & = QString());

    QVariant variantValue() const;
    void setVariantValue(const QVariant &);

public slots:
    void clear() { setValue(0.0); }

private slots:
    void labelClicked();
};

class MDComboBox : public QComboBox, public MDInputWidget
{
    Q_OBJECT

public:
    MDComboBox(const QString &, const QString &, QWidget *, const QString &, const MTDictionary &, const QString & = QString(), bool = true);

    QVariant variantValue() const;
    void setVariantValue(const QVariant &);

    void setNullValue(const QVariant &);

signals:
    void currentIndexChanged(MDComboBox *, int);
    void toggled(bool);

protected slots:
    void emitToggled(int);

private:
    QVariant nullvalue;
};

class MDColourComboBox : public MTColourComboBox, public MDInputWidget
{
    Q_OBJECT

public:
    MDColourComboBox(const QString &, const QString &, QWidget *, const QString &);

    QVariant variantValue() const;
    void setVariantValue(const QVariant &);
};

class MDDateTimeEdit : public QDateTimeEdit, public MDInputWidget
{
    Q_OBJECT

public:
    MDDateTimeEdit(const QString &, const QString &, QWidget *, const QString &);

    QVariant variantValue() const;
    void setVariantValue(const QVariant &);
};

class MDDateEdit : public QDateEdit, public MDInputWidget
{
    Q_OBJECT

public:
    MDDateEdit(const QString &, const QString &, QWidget *, const QString &);

    QVariant variantValue() const;
    void setVariantValue(const QVariant &);

public slots:
    void setEnabled(bool);
};

class MDAddressEdit : public MTAddressEdit, public MDInputWidget
{
    Q_OBJECT

public:
    MDAddressEdit(const QString &, const QString &, QWidget *, const QString &);

    QVariant variantValue() const;
    void setVariantValue(const QVariant &);
};

class MDHighlightedPlainTextEdit : public QPlainTextEdit, public MDInputWidget
{
    Q_OBJECT

public:
    MDHighlightedPlainTextEdit(const QString &, const QString &, QWidget *, const QString &, const QStringList &, bool = true);

    QVariant variantValue() const;
    void setVariantValue(const QVariant &);
};

class MDPlainTextEdit : public QPlainTextEdit, public MDInputWidget
{
    Q_OBJECT

public:
    MDPlainTextEdit(const QString &, const QString &, QWidget *, const QString &, const QString & = QString(), bool = true);

    QVariant variantValue() const;
    void setVariantValue(const QVariant &);

public slots:
    void setEnabled(bool);
};

class MDGroupedCheckBoxes : public QGroupBox, public MDInputWidget
{
    Q_OBJECT

public:
    MDGroupedCheckBoxes(const QString &, const QString &, QWidget *, int);
    ~MDGroupedCheckBoxes();

    void addCheckBox(int, const QString &);

    QVariant variantValue() const;
    void setVariantValue(const QVariant &) {}

private:
    QList<MDCheckBox *> checkboxes;
    int grouped_value;
};

class MDFileChooser : public DBFileChooser, public MDInputWidget
{
    Q_OBJECT

public:
    MDFileChooser(const QString &, const QString &, QWidget *, const QString &);

    QVariant variantValue() const;
    void setVariantValue(const QVariant &) {}
};

class MDGroupedInputWidgets : public QFrame, public MDInputWidget
{
    Q_OBJECT

public:
    MDGroupedInputWidgets(const QString &, QWidget *);

    void addWidget(MDAbstractInputWidget *);

    QVariant variantValue() const { return QVariant(QVariant::String); }
    void setVariantValue(const QVariant &) {}

protected:
    QGridLayout *grid;
};

class MDRadioButtonGroup : public QGroupBox, public MDInputWidget
{
    Q_OBJECT
public:
    MDRadioButtonGroup(const QString &, const QString &, QWidget *, const QString &);

    void addRadioButton(const QString &, const QString &);

    QVariant variantValue() const;
    void setVariantValue(const QVariant &) {}

private:
    QMap<QRadioButton *, QString> radiobuttons;
    QString selected;
};

class MDHiddenIdField : public MDInputWidget
{
public:
    MDHiddenIdField(const QString &, QWidget *, const QVariant &);

    QVariant variantValue() const;
    void setVariantValue(const QVariant &);

private:
    QVariant value;
};

#endif // INPUT_WIDGETS_H
