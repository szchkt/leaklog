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
    af_attributes.insert("leak_detector", tr("Fixed leakage detector installed"));
    af_attributes.insert("life", tr("Service life"));
    af_attributes.insert("runtime", tr("Run-time per day"));
    af_attributes.insert("utilisation", tr("Rate of utilisation"));
    af_attributes.insert("inspection_interval", tr("Inspection interval"));
    af_functions << "<";
    af_functions << "<=";
    af_functions << "=";
    af_functions << ">=";
    af_functions << ">";
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

QString AttributeFilters::function(int i) { return af_filters.at(i)->function->currentText(); }

QString AttributeFilters::value(int i) { return af_filters.at(i)->value->text(); }

void AttributeFilters::add(const QString & attribute, const QString & function, const QString & value)
{
    AttributeFilter * filter = add();
    for (int i = 0; i < filter->attribute->count(); ++i) {
        if (af_attributes.firstKey(filter->attribute->itemText(i)) == attribute) { filter->attribute->setCurrentIndex(i); break; }
    }
    for (int i = 0; i < filter->function->count(); ++i) {
        if (filter->function->itemText(i) == function) { filter->function->setCurrentIndex(i); break; }
    }
    filter->value->setText(value);
}

AttributeFilter * AttributeFilters::add()
{
    AttributeFilter * filter = new AttributeFilter(this);
    filter->attribute->addItems(af_attributes.values());
    filter->function->addItems(af_functions);
    filter->function->setCurrentIndex(af_functions.indexOf("="));
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
    expression_ins->setMinimumSize(200, 30);
    hlayout->addWidget(expression_ins);
    function = new QComboBox(this);
    hlayout->addWidget(function);
    expression_nom = new QPlainTextEdit(this);
    expression_nom->setMinimumSize(200, 30);
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
    c_functions << "<";
    c_functions << "<=";
    c_functions << "=";
    c_functions << ">=";
    c_functions << ">";
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

QString Conditions::function(int i) { return c_conditions.at(i)->function->currentText(); }

QString Conditions::expressionNom(int i) { return c_conditions.at(i)->expression_nom->toPlainText(); }

void Conditions::add(const QString & exp_ins, const QString & function, const QString & exp_nom)
{
    Condition * condition = add();
    condition->expression_ins->setPlainText(exp_ins);
    condition->expression_nom->setPlainText(exp_nom);
    for (int i = 0; i < condition->function->count(); ++i) {
        if (condition->function->itemText(i) == function) { condition->function->setCurrentIndex(i); break; }
    }
}

Condition * Conditions::add()
{
    Condition * condition = new Condition(this);
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

void Conditions::remove(Condition * condition)
{
    int i = c_conditions.indexOf(condition);
    if (i >= 0) { delete c_conditions.takeAt(i); }
}

ModifyWarningDialogue::ModifyWarningDialogue(const MTRecord & record, const QStringList & used_ids, QWidget * parent):
ModifyDialogue(record, used_ids, parent)
{
    md_used_ids << "refrigerant_amount" << "oil_amount" << "sum" << "p_to_t";
    md_dict.insert("warning", tr("Warning"));
    md_dict.insert("enabled", tr("Enabled"));
    md_dict_input.insert("enabled", "chb");
    md_dict.insert("name", tr("Name"));
    md_dict_input.insert("name", "le");
    md_dict.insert("description", tr("Description"));
    md_dict_input.insert("description", "le");
    md_dict.insert("delay", tr("Delay"));
    md_dict_input.insert("delay", QString("spb;0;0;999999; %1").arg(tr("days")));
    MTDictionary md_dict_values;
    if (!md_record.id().isEmpty()) {
        Warning query(md_record.id().toInt());
        if (query.next()) {
            for (int i = 0; i < query.record().count(); ++i) {
                md_dict_values.insert(query.record().fieldName(i), query.value(query.record().fieldName(i)).toString());
            }
        }
    }
    // ------------
    if (md_dict_values.contains("name") && !md_dict_values.value("name").isEmpty()) {
        this->setWindowTitle(tr("%1: %2").arg(md_dict.value(md_record.type())).arg(md_dict_values.value("name")));
    } else {
        this->setWindowTitle(md_dict.value(md_record.type()));
    }
    QLabel * md_lbl_var = NULL; QWidget * md_w_var = NULL;
    int r = 0; QStringList inputtype; QString value;
    bool disable_input = md_record.id().toInt() >= 1000;
    for (int i = 0; i < md_dict.count(); ++i) {
        if (md_dict.key(i) == md_record.type()) { continue; }
        value = md_dict_values.contains(md_dict.key(i)) ? md_dict_values.value(md_dict.key(i)) : "";
        inputtype = md_dict_input.value(md_dict.key(i)).split(";");
        if (inputtype.at(0) != "chb") {
            md_lbl_var = new QLabel(tr("%1:").arg(md_dict.value(i)), this);
            md_lbl_var->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
            md_grid_main->addWidget(md_lbl_var, r, 0);
        }
        md_w_var = createInputWidget(inputtype, md_dict.value(i), value);
        md_w_var->setDisabled(disable_input && md_dict.key(i) != "enabled");
        md_grid_main->addWidget(md_w_var, r, 1, 1, 3);
        md_vars.insert(md_dict.key(i), md_w_var);
        r++;
    }
    md_grid_main->addWidget(new QLabel(tr("Circuit filter:"), this), r, 0, 1, 3);
    QToolButton * md_tbtn_add_filter = new QToolButton(this);
    md_tbtn_add_filter->setDisabled(disable_input);
    md_tbtn_add_filter->setIcon(QIcon(QString::fromUtf8(":/images/images/add16.png")));
    md_grid_main->addWidget(md_tbtn_add_filter, r, 3);
    md_filters = new AttributeFilters(this);
    md_filters->setDisabled(disable_input);
    QObject::connect(md_tbtn_add_filter, SIGNAL(clicked()), md_filters, SLOT(add()));
    md_grid_main->addWidget(md_filters, r + 1, 0, 1, 4);
    md_grid_main->addWidget(new QLabel(tr("Conditions:"), this), r + 2, 0, 1, 3);
    QToolButton * md_tbtn_add_condition = new QToolButton(this);
    md_tbtn_add_condition->setDisabled(disable_input);
    md_tbtn_add_condition->setIcon(QIcon(QString::fromUtf8(":/images/images/add16.png")));
    md_grid_main->addWidget(md_tbtn_add_condition, r + 2, 3);
    md_conditions = new Conditions(md_used_ids, this);
    md_conditions->setDisabled(disable_input);
    QObject::connect(md_tbtn_add_condition, SIGNAL(clicked()), md_conditions, SLOT(add()));
    md_grid_main->addWidget(md_conditions, r + 3, 0, 1, 4);
    if (!md_record.id().isEmpty()) {
        WarningFilters filters(md_record.id().toInt());
        while (filters.next()) {
            md_filters->add(filters.value("circuit_attribute").toString(), filters.value("function").toString(), filters.value("value").toString());
        }
        WarningConditions conditions(md_record.id().toInt());
        while (conditions.next()) {
            md_conditions->add(conditions.value("value_ins").toString(), conditions.value("function").toString(), conditions.value("value_nom").toString());
        }
    }
    this->resize(450, 20);
}

void ModifyWarningDialogue::save()
{
    QMap<QString, QVariant> values; QStringList inputtype;
    QMapIterator<QString, QWidget *> i(md_vars);
    while (i.hasNext()) { i.next();
        inputtype = md_dict_input.value(i.key()).split(";");
        values.insert(i.key(), getInputFromWidget(i.value(), inputtype, i.key()));
    }
    if (!md_record.id().isEmpty()) {
        QSqlQuery delete_filters;
        delete_filters.prepare("DELETE FROM warnings_filters WHERE parent = :parent");
        delete_filters.bindValue(":parent", md_record.id());
        delete_filters.exec();
        QSqlQuery delete_conditions;
        delete_conditions.prepare("DELETE FROM warnings_conditions WHERE parent = :parent");
        delete_conditions.bindValue(":parent", md_record.id());
        delete_conditions.exec();
    }
    md_record.update(values);
    for (int i = 0; i < md_filters->count(); ++i) {
        QSqlQuery insert_filter;
        insert_filter.prepare("INSERT INTO warnings_filters (parent, circuit_attribute, function, value) VALUES (:parent, :circuit_attribute, :function, :value)");
        insert_filter.bindValue(":parent", md_record.id());
        insert_filter.bindValue(":circuit_attribute", md_filters->attribute(i));
        insert_filter.bindValue(":function", md_filters->function(i));
        insert_filter.bindValue(":value", md_filters->value(i));
        insert_filter.exec();
    }
    for (int i = 0; i < md_conditions->count(); ++i) {
        QSqlQuery insert_condition;
        insert_condition.prepare("INSERT INTO warnings_conditions (parent, value_ins, function, value_nom) VALUES (:parent, :value_ins, :function, :value_nom)");
        insert_condition.bindValue(":parent", md_record.id());
        insert_condition.bindValue(":value_ins", md_conditions->expressionIns(i));
        insert_condition.bindValue(":function", md_conditions->function(i));
        insert_condition.bindValue(":value_nom", md_conditions->expressionNom(i));
        insert_condition.exec();
    }
    accept();
}
