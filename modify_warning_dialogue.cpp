/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008 Matus & Michal Tomlein

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

#include "main_window.h"

AttributeFilter::AttributeFilter(AttributeFilters * parent):
QWidget(parent)
{
    this->parent = parent;
    QHBoxLayout * hlayout = new QHBoxLayout(this);
    hlayout->setSpacing(6);
    hlayout->setContentsMargins(0, 0, 0, 0);
    attribute = new QComboBox(this);
    hlayout->addWidget(attribute);
    function = new QComboBox(this);
    hlayout->addWidget(function);
    value = new QLineEdit(this);
    hlayout->addWidget(value);
    QToolButton * remove = new QToolButton(this);
    remove->setIcon(QIcon(QString::fromUtf8(":/images/images/remove16.png")));
    QObject::connect(remove, SIGNAL(clicked()), this, SLOT(remove()));
    hlayout->addWidget(remove);
}

void AttributeFilter::remove() { emit removeFilter(this); }

AttributeFilters::AttributeFilters(QWidget * parent):
QWidget(parent)
{
    af_attributes.insert("hermetic", tr("Hermetically sealed"));
    af_attributes.insert("year", tr("Year of purchase"));
    af_attributes.insert("commissioning", tr("Date of commissioning"));
    af_attributes.insert("refrigerant_amount", tr("Amount of refrigerant"));
    af_attributes.insert("oil_amount", tr("Amount of oil"));
    af_attributes.insert("life", tr("Service life"));
    af_attributes.insert("runtime", tr("Run-time per day"));
    af_attributes.insert("utilisation", tr("Rate of utilisation"));
    af_functions.insert("less", "<");
    af_functions.insert("lessorequal", "<=");
    af_functions.insert("equal", "=");
    af_functions.insert("moreorequal", ">=");
    af_functions.insert("more", ">");
    af_vlayout_main = new QVBoxLayout(this);
#ifdef Q_WS_MAC
    af_vlayout_main->setSpacing(0);
#else
    af_vlayout_main->setSpacing(6);
#endif
    af_vlayout_main->setContentsMargins(0, 0, 0, 0);
}

int AttributeFilters::count() { return af_filters.count(); }

QString AttributeFilters::attribute(int i) { return af_attributes.firstKey(af_filters.at(i)->attribute->currentText()); }

QString AttributeFilters::function(int i) { return af_functions.firstKey(af_filters.at(i)->function->currentText()); }

QString AttributeFilters::value(int i) { return af_filters.at(i)->value->text(); }

void AttributeFilters::add(const QString & attribute, const QString & function, const QString & value)
{
    AttributeFilter * filter = add();
    for (int i = 0; i < filter->attribute->count(); ++i) {
        if (af_attributes.firstKey(filter->attribute->itemText(i)) == attribute) { filter->attribute->setCurrentIndex(i); break; }
    }
    for (int i = 0; i < filter->function->count(); ++i) {
        if (af_functions.firstKey(filter->function->itemText(i)) == function) { filter->function->setCurrentIndex(i); break; }
    }
    filter->value->setText(value);
}

AttributeFilter * AttributeFilters::add()
{
    AttributeFilter * filter = new AttributeFilter(this);
    filter->attribute->addItems(af_attributes.values());
    filter->function->addItems(af_functions.values());
    filter->function->setCurrentIndex(af_functions.indexOfValue("="));
    af_vlayout_main->addWidget(filter);
    af_filters << filter;
    QObject::connect(filter, SIGNAL(removeFilter(AttributeFilter *)), this, SLOT(remove(AttributeFilter *)));
    return filter;
}

void AttributeFilters::remove(AttributeFilter * filter)
{
    int i = af_filters.indexOf(filter);
    if (i >= 0) { delete af_filters.takeAt(i); }
}

Condition::Condition(Conditions * parent):
QWidget(parent)
{
    this->parent = parent;
    QHBoxLayout * hlayout = new QHBoxLayout(this);
    hlayout->setSpacing(6);
    hlayout->setContentsMargins(0, 0, 0, 0);
    expression_ins = new QPlainTextEdit(this);
    hlayout->addWidget(expression_ins);
    function = new QComboBox(this);
    hlayout->addWidget(function);
    expression_nom = new QPlainTextEdit(this);
    hlayout->addWidget(expression_nom);
    QToolButton * remove = new QToolButton(this);
    remove->setIcon(QIcon(QString::fromUtf8(":/images/images/remove16.png")));
    QObject::connect(remove, SIGNAL(clicked()), this, SLOT(remove()));
    hlayout->addWidget(remove);
}

void Condition::remove() { emit removeCondition(this); }

Conditions::Conditions(const QStringList & used_ids, QWidget * parent):
QWidget(parent)
{
    c_used_ids = used_ids;
    c_functions.insert("less", "<");
    c_functions.insert("lessorequal", "<=");
    c_functions.insert("equal", "=");
    c_functions.insert("moreorequal", ">=");
    c_functions.insert("more", ">");
    c_vlayout_main = new QVBoxLayout(this);
#ifdef Q_WS_MAC
    c_vlayout_main->setSpacing(0);
#else
    c_vlayout_main->setSpacing(6);
#endif
    c_vlayout_main->setContentsMargins(0, 0, 0, 0);
}

int Conditions::count() { return c_conditions.count(); }

QString Conditions::expressionIns(int i) { return c_conditions.at(i)->expression_ins->toPlainText(); }

QString Conditions::function(int i) { return c_functions.firstKey(c_conditions.at(i)->function->currentText()); }

QString Conditions::expressionNom(int i) { return c_conditions.at(i)->expression_nom->toPlainText(); }

void Conditions::add(const QString & exp_ins, const QString & function, const QString & exp_nom)
{
    Condition * condition = add();
    condition->expression_ins->setPlainText(exp_ins);
    condition->expression_nom->setPlainText(exp_nom);
    for (int i = 0; i < condition->function->count(); ++i) {
        if (c_functions.firstKey(condition->function->itemText(i)) == function) { condition->function->setCurrentIndex(i); break; }
    }
}

Condition * Conditions::add()
{
    Condition * condition = new Condition(this);
    condition->expression_nom->setPalette(searchLineEditPalettes.search_active_palette);
    new Highlighter(c_used_ids, condition->expression_ins->document());
    new Highlighter(c_used_ids, condition->expression_nom->document());
    condition->function->addItems(c_functions.values());
    condition->function->setCurrentIndex(c_functions.indexOfValue("="));
    c_vlayout_main->addWidget(condition);
    c_conditions << condition;
    QObject::connect(condition, SIGNAL(removeCondition(Condition *)), this, SLOT(remove(Condition *)));
    return condition;
}

void Conditions::remove(Condition * condition)
{
    int i = c_conditions.indexOf(condition);
    if (i >= 0) { delete c_conditions.takeAt(i); }
}

ModifyWarningDialogue::ModifyWarningDialogue(const QDomElement & element, const QStringList & used_ids, MainWindow * parent):
ModifyDialogue(element, used_ids, parent)
{
    md_used_ids << "refrigerant_amount" << "oil_amount" << "sum";
    md_dict.insert("warning", tr("Warning"));
    md_dict.insert("id", tr("Name"));
    md_dict_input.insert("id", "le");
    md_dict.insert("description", tr("Description"));
    md_dict_input.insert("description", "le");
    // ------------
    if (!md_element.attribute("id").isEmpty()) {
        this->setWindowTitle(tr("%1: %2").arg(md_dict.value(md_element.nodeName())).arg(md_element.attribute("id")));
    } else {
        this->setWindowTitle(md_dict.value(md_element.nodeName()));
    }
    QLabel * md_lbl_var = NULL; QWidget * md_w_var = NULL;
    int r = 0; QStringList inputtype; QString value;
    for (int i = 0; i < md_dict.count(); ++i) {
        if (md_dict.key(i) == md_element.nodeName()) { continue; }
        value = md_element.attribute(md_dict.key(i));
        inputtype = md_dict_input.value(md_dict.key(i)).split(";");
        if (inputtype.at(0) != "chb") {
            md_lbl_var = new QLabel(tr("%1:").arg(md_dict.value(i)), this);
            md_lbl_var->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
            md_grid_main->addWidget(md_lbl_var, r, 0);
        }
        md_w_var = createInputWidget(inputtype, md_dict.value(i), value);
        md_grid_main->addWidget(md_w_var, r, 1, 1, 3);
        md_vars.insert(md_dict.key(i), md_w_var);
        r++;
    }
    md_grid_main->addWidget(new QLabel(tr("Circuit filter:"), this), r, 0, 1, 3);
    QToolButton * md_tbtn_add_filter = new QToolButton(this);
    md_tbtn_add_filter->setIcon(QIcon(QString::fromUtf8(":/images/images/add16.png")));
    md_grid_main->addWidget(md_tbtn_add_filter, r, 3);
    md_filters = new AttributeFilters(this);
    QObject::connect(md_tbtn_add_filter, SIGNAL(clicked()), md_filters, SLOT(add()));
    md_grid_main->addWidget(md_filters, r + 1, 0, 1, 4);
    md_grid_main->addWidget(new QLabel(tr("Conditions:"), this), r + 2, 0, 1, 3);
    QToolButton * md_tbtn_add_condition = new QToolButton(this);
    md_tbtn_add_condition->setIcon(QIcon(QString::fromUtf8(":/images/images/add16.png")));
    md_grid_main->addWidget(md_tbtn_add_condition, r + 2, 3);
    md_conditions = new Conditions(md_used_ids, this);
    QObject::connect(md_tbtn_add_condition, SIGNAL(clicked()), md_conditions, SLOT(add()));
    md_grid_main->addWidget(md_conditions, r + 3, 0, 1, 4);
    QDomNodeList conditions = md_element.elementsByTagName("condition");
    for (int i = 0; i < conditions.count(); ++i) {
        QDomElement condition = conditions.at(i).toElement();
        if (condition.hasAttribute("cc_attr")) {
            md_filters->add(condition.attribute("cc_attr"), condition.attribute("f"), condition.text());
        } else {
            md_conditions->add(loadExpression(condition, "value_ins"), condition.attribute("f"), loadExpression(condition, "value_nom"));
        }
    }
    this->resize(450, 20);
}

void ModifyWarningDialogue::save()
{
    QStringList used_ids = md_used_ids;
    md_used_ids.clear();
    MTDictionary values; QStringList inputtype;
    QMapIterator<QString, QWidget *> i(md_vars);
    while (i.hasNext()) { i.next();
        inputtype = md_dict_input.value(i.key()).split(";");
        QString value = getInputFromWidget(i.value(), inputtype, i.key());
        values.insert(i.key(), value);
    }
    for (int i = 0; i < values.count(); ++i) {
        md_element.setAttribute(values.key(i), values.value(i));
    }
    md_used_ids = used_ids;
    while (md_element.hasChildNodes()) { md_element.removeChild(md_element.firstChild()); }
    for (int i = 0; i < md_filters->count(); ++i) {
        QDomElement condition = md_parent->document.createElement("condition");
        condition.setAttribute("cc_attr", md_filters->attribute(i));
        condition.setAttribute("f", md_filters->function(i));
        QDomText text = md_parent->document.createTextNode(md_filters->value(i));
        condition.appendChild(text);
        md_element.appendChild(condition);
    }
    for (int i = 0; i < md_conditions->count(); ++i) {
        QDomElement condition = md_parent->document.createElement("condition");
        condition.setAttribute("f", md_conditions->function(i));
        saveExpression(md_conditions->expressionIns(i), condition, "value_ins");
        saveExpression(md_conditions->expressionNom(i), condition, "value_nom");
        md_element.appendChild(condition);
    }
    accept();
}
