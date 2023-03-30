/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2023 Matus & Michal Tomlein

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

#include "editwarningdialogue.h"
#include "inputwidgets.h"
#include "global.h"
#include "records.h"
#include "warnings.h"
#include "highlighter.h"
#include "editdialoguelayout.h"
#include "undostack.h"

#include <QToolButton>
#include <QMessageBox>
#include <QComboBox>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QApplication>

AttributeFilter::AttributeFilter(AttributeFilters *parent):
QWidget(parent)
{
    this->parent = parent;
    QHBoxLayout *hlayout = new QHBoxLayout(this);
    hlayout->setSpacing(6);
    hlayout->setContentsMargins(0, 0, 0, 0);
    attribute = new QComboBox(this);
    hlayout->addWidget(attribute);
    function = new QComboBox(this);
    hlayout->addWidget(function);
    value = new QLineEdit(this);
    hlayout->addWidget(value);
    QToolButton *remove = new QToolButton(this);
    remove->setIcon(QIcon(QString::fromUtf8(":/images/images/remove16.png")));
    QObject::connect(remove, SIGNAL(clicked()), this, SLOT(remove()));
    hlayout->addWidget(remove);
}

void AttributeFilter::remove() { emit removeFilter(this); }

AttributeFilters::AttributeFilters(QWidget *parent):
QWidget(parent)
{
    af_attributes.insert("hermetic", QApplication::translate("Circuit", "Hermetically sealed"));
    af_attributes.insert("year", QApplication::translate("Circuit", "Year of purchase"));
    af_attributes.insert("commissioning", QApplication::translate("Circuit", "Date of commissioning"));
    af_attributes.insert("refrigerant", QApplication::translate("Circuit", "Refrigerant"));
    af_attributes.insert("refrigerant_amount", QApplication::translate("Circuit", "Refrigerant amount"));
    af_attributes.insert("co2_equivalent", QApplication::translate("MainWindow", "CO\342\202\202 equivalent"));
    af_attributes.insert("gwp", QApplication::translate("MainWindow", "GWP"));
    af_attributes.insert("oil_amount", QApplication::translate("Circuit", "Oil amount"));
    af_attributes.insert("leak_detector", QApplication::translate("Circuit", "Fixed leakage detector installed"));
    af_attributes.insert("runtime", QApplication::translate("Circuit", "Run-time per day"));
    af_attributes.insert("utilisation", QApplication::translate("Circuit", "Rate of utilisation"));
    af_attributes.insert("inspection_interval", QApplication::translate("Circuit", "Inspection interval"));
    af_functions << "<";
    af_functions << "<=";
    af_functions << "=";
    af_functions << ">=";
    af_functions << ">";
    af_vlayout_main = new QVBoxLayout(this);
#ifdef Q_OS_MAC
    af_vlayout_main->setSpacing(0);
#else
    af_vlayout_main->setSpacing(6);
#endif
    af_vlayout_main->setContentsMargins(0, 0, 0, 0);
}

int AttributeFilters::count() { return af_filters.count(); }

QString AttributeFilters::attribute(int i) { return af_attributes.firstKey(af_filters.at(i)->attribute->currentText()); }

QString AttributeFilters::function(int i) { return af_filters.at(i)->function->currentText(); }

QString AttributeFilters::value(int i) { return af_filters.at(i)->value->text(); }

void AttributeFilters::add(const QString &attribute, const QString &function, const QString &value)
{
    AttributeFilter *filter = add();
    for (int i = 0; i < filter->attribute->count(); ++i) {
        if (af_attributes.firstKey(filter->attribute->itemText(i)) == attribute) { filter->attribute->setCurrentIndex(i); break; }
    }
    for (int i = 0; i < filter->function->count(); ++i) {
        if (filter->function->itemText(i) == function) { filter->function->setCurrentIndex(i); break; }
    }
    filter->value->setText(value);
}

AttributeFilter *AttributeFilters::add()
{
    AttributeFilter *filter = new AttributeFilter(this);
    filter->attribute->addItems(af_attributes.values());
    filter->function->addItems(af_functions);
    filter->function->setCurrentIndex(af_functions.indexOf("="));
    af_vlayout_main->addWidget(filter);
    af_filters << filter;
    QObject::connect(filter, SIGNAL(removeFilter(AttributeFilter *)), this, SLOT(remove(AttributeFilter *)));
    return filter;
}

void AttributeFilters::remove(AttributeFilter *filter)
{
    int i = af_filters.indexOf(filter);
    if (i >= 0) { delete af_filters.takeAt(i); }
}

Condition::Condition(Conditions *parent):
QWidget(parent)
{
    this->parent = parent;
    QHBoxLayout *hlayout = new QHBoxLayout(this);
    hlayout->setSpacing(6);
    hlayout->setContentsMargins(0, 0, 0, 0);
    expression_ins = new QPlainTextEdit(this);
    expression_ins->setMinimumSize(200, 30);
    hlayout->addWidget(expression_ins);
    function = new QComboBox(this);
    hlayout->addWidget(function);
    expression_nom = new QPlainTextEdit(this);
    expression_nom->setMinimumSize(200, 30);
    hlayout->addWidget(expression_nom);
    QToolButton *remove = new QToolButton(this);
    remove->setIcon(QIcon(QString::fromUtf8(":/images/images/remove16.png")));
    QObject::connect(remove, SIGNAL(clicked()), this, SLOT(remove()));
    hlayout->addWidget(remove);
}

void Condition::remove() { emit removeCondition(this); }

Conditions::Conditions(const QStringList &used_ids, QWidget *parent):
QWidget(parent)
{
    c_used_ids = used_ids;
    c_functions << "<";
    c_functions << "<=";
    c_functions << "=";
    c_functions << ">=";
    c_functions << ">";
    c_vlayout_main = new QVBoxLayout(this);
#ifdef Q_OS_MAC
    c_vlayout_main->setSpacing(0);
#else
    c_vlayout_main->setSpacing(6);
#endif
    c_vlayout_main->setContentsMargins(0, 0, 0, 0);
}

int Conditions::count() { return c_conditions.count(); }

QString Conditions::expressionIns(int i) { return c_conditions.at(i)->expression_ins->toPlainText(); }

QString Conditions::function(int i) { return c_conditions.at(i)->function->currentText(); }

QString Conditions::expressionNom(int i) { return c_conditions.at(i)->expression_nom->toPlainText(); }

void Conditions::add(const QString &exp_ins, const QString &function, const QString &exp_nom)
{
    Condition *condition = add();
    condition->expression_ins->setPlainText(exp_ins);
    condition->expression_nom->setPlainText(exp_nom);
    for (int i = 0; i < condition->function->count(); ++i) {
        if (condition->function->itemText(i) == function) { condition->function->setCurrentIndex(i); break; }
    }
}

Condition *Conditions::add()
{
    Condition *condition = new Condition(this);
    condition->expression_nom->setPalette(searchLineEditPalettes.search_active_palette);
    new Highlighter(c_used_ids, condition->expression_ins->document());
    new Highlighter(c_used_ids, condition->expression_nom->document());
    condition->function->addItems(c_functions);
    condition->function->setCurrentIndex(c_functions.indexOf("="));
    c_vlayout_main->addWidget(condition);
    c_conditions << condition;
    QObject::connect(condition, SIGNAL(removeCondition(Condition *)), this, SLOT(remove(Condition *)));
    return condition;
}

void Conditions::remove(Condition *condition)
{
    int i = c_conditions.indexOf(condition);
    if (i >= 0) { delete c_conditions.takeAt(i); }
}

EditWarningDialogue::EditWarningDialogue(WarningRecord *record, UndoStack *undo_stack, QWidget *parent):
EditDialogue(undo_stack, parent)
{
    init(record);

    record->initEditDialogue(this);

    int r = md_inputwidgets.count();
    /*for (int i = 0; i < r; ++i) {
        addWidget(md_inputwidgets.at(i)->label(), i, 0);
        addWidget(md_inputwidgets.at(i)->widget(), i, 1, 1, 3);
    }*/
    EditDialogueLayout md_layout(&md_inputwidgets, md_grid_main);
    md_layout.layout();
    bool disable_input = Warnings::isPredefined(md_record->uuid());
    md_layout.addWidget(new QLabel(tr("Circuit filter:"), this), r, 0, 1, 3);
    QToolButton *tbtn_add_filter = new QToolButton(this);
    tbtn_add_filter->setDisabled(disable_input);
    tbtn_add_filter->setIcon(QIcon(QString::fromUtf8(":/images/images/add16.png")));
    md_layout.addWidget(tbtn_add_filter, r, 3);
    md_filters = new AttributeFilters(this);
    md_filters->setDisabled(disable_input);
    QObject::connect(tbtn_add_filter, SIGNAL(clicked()), md_filters, SLOT(add()));
    md_layout.addWidget(md_filters, r + 1, 0, 1, 4);
    md_layout.addWidget(new QLabel(tr("Conditions:"), this), r + 2, 0, 1, 3);
    QToolButton *tbtn_add_condition = new QToolButton(this);
    tbtn_add_condition->setDisabled(disable_input);
    tbtn_add_condition->setIcon(QIcon(QString::fromUtf8(":/images/images/add16.png")));
    md_layout.addWidget(tbtn_add_condition, r + 2, 3);

    QStringList used_ids;
    used_ids << "refrigerant_amount" << "oil_amount" << "sum" << "p_to_t" << "p_to_t_vap" << "gwp" << "co2_equivalent";
    used_ids << Global::listSupportedFunctions();
    used_ids << Global::listVariableIds();
    md_conditions = new Conditions(used_ids, this);
    md_conditions->setDisabled(disable_input);
    QObject::connect(tbtn_add_condition, SIGNAL(clicked()), md_conditions, SLOT(add()));
    md_layout.addWidget(md_conditions, r + 3, 0, 1, 4);
    if (!md_record->uuid().isEmpty()) {
        WarningFilters filters(md_record->uuid());
        while (filters.next()) {
            md_filters->add(filters.value("circuit_attribute").toString(), filters.value("function").toString(), filters.value("value").toString());
        }
        WarningConditions conditions(md_record->uuid());
        while (conditions.next()) {
            md_conditions->add(conditions.value("value_ins").toString(), conditions.value("function").toString(), conditions.value("value_nom").toString());
        }
    }
    this->resize(450, 20);
}

void EditWarningDialogue::setWindowTitle(const QString &title)
{
    this->QDialog::setWindowTitle(title);
}

void EditWarningDialogue::save()
{
    md_undo_stack->savepoint();

    if (!md_record->uuid().isEmpty()) {
        WarningFilter::query({{"warning_uuid", md_record->uuid()}}).removeAll();
        WarningCondition::query({{"warning_uuid", md_record->uuid()}}).removeAll();
    }

    for (QList<MDAbstractInputWidget *>::const_iterator i = md_inputwidgets.constBegin(); i != md_inputwidgets.constEnd(); ++i) {
        md_record->setValue((*i)->id(), (*i)->variantValue());
    }

    md_record->save();

    if (!Warnings::isPredefined(md_record->uuid())) {
        for (int i = 0; i < md_filters->count(); ++i) {
            WarningFilter filter;
            filter.setValue("warning_uuid", md_record->uuid());
            filter.setValue("circuit_attribute", md_filters->attribute(i));
            filter.setValue("function", md_filters->function(i));
            filter.setValue("value", md_filters->value(i));
            filter.save();
        }
        for (int i = 0; i < md_conditions->count(); ++i) {
            WarningCondition condition;
            condition.setValue("warning_uuid", md_record->uuid());
            condition.setValue("value_ins", md_conditions->expressionIns(i));
            condition.setValue("function", md_conditions->function(i));
            condition.setValue("value_nom", md_conditions->expressionNom(i));
            condition.save();
        }
    }

    accept();
}
