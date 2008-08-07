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

Highlighter::Highlighter(QStringList used_ids, QTextDocument * parent):
QSyntaxHighlighter(parent)
{
    HighlightingRule rule;
    keywordFormat.setFontWeight(QFont::Bold);
    QStringList keywordPatterns;
    for (int i = 0; i < used_ids.count(); ++i) {
        keywordPatterns << QString("\\b%1\\b").arg(used_ids.at(i));
    }
    foreach (QString pattern, keywordPatterns) {
        rule.pattern = QRegExp(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }
}

void Highlighter::highlightBlock(const QString & text)
{
    foreach (HighlightingRule rule, highlightingRules) {
        QRegExp expression(rule.pattern);
        int index = text.indexOf(expression);
        while (index >= 0) {
            int length = expression.matchedLength();
            setFormat(index, length, rule.format);
            index = text.indexOf(expression, index + length);
        }
    }
}

void ModifyDialogue::init(const QDomElement & element, const QStringList & used_ids, MainWindow * parent)
{
    md_parent = parent;
    md_element = element;
    md_used_ids = used_ids;
    QVBoxLayout * md_vlayout_main = new QVBoxLayout(this);
    md_vlayout_main->setSpacing(9);
    md_vlayout_main->setContentsMargins(6, 6, 6, 6);
    md_grid_main = new QGridLayout;
    md_grid_main->setHorizontalSpacing(9);
    md_grid_main->setVerticalSpacing(6);
    md_grid_main->setContentsMargins(0, 0, 0, 0);
    md_vlayout_main->addLayout(md_grid_main);
    QDialogButtonBox * md_bb = new QDialogButtonBox(this);
    md_bb->setStandardButtons(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
    md_bb->button(QDialogButtonBox::Save)->setText(tr("Save"));
    md_bb->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));
    QObject::connect(md_bb, SIGNAL(accepted()), this, SLOT(save()));
    QObject::connect(md_bb, SIGNAL(rejected()), this, SLOT(reject()));
    md_vlayout_main->addWidget(md_bb);
    this->resize(20, 20);
}

ModifyDialogue::ModifyDialogue(const QDomElement & element, const QStringList & used_ids, MainWindow * parent):
QDialog(parent)
{
    init(element, used_ids, parent);
}

ModifyDialogue::ModifyDialogue(const QDomElement & element, const QStringList & used_ids, bool nominal_allowed, MainWindow * parent):
QDialog(parent)
{
    init(element, used_ids, parent);
    if (md_element.nodeName() == "customer") {
        md_dict.insert("customer", tr("Customer"));
        md_dict.insert("id", tr("ID"));
        md_dict_input.insert("id", "le");
        md_dict.insert("company", tr("Company"));
        md_dict_input.insert("company", "le");
        md_dict.insert("name", tr("Contact person"));
        md_dict_input.insert("name", "le");
        md_dict.insert("address", tr("Address"));
        md_dict_input.insert("address", "pte");
        md_dict.insert("mail", tr("E-mail"));
        md_dict_input.insert("mail", "le");
        md_dict.insert("phone", tr("Phone"));
        md_dict_input.insert("phone", "le");
    } else if (md_element.nodeName() == "circuit") {
        md_dict.insert("circuit", tr("Cooling circuit"));
        md_dict.insert("id", tr("ID"));
        md_dict_input.insert("id", "le");
        md_dict.insert("hermetic", tr("Hermetically sealed"));
        md_dict_input.insert("hermetic", "chb");
        md_dict.insert("manufacturer", tr("Manufacturer"));
        md_dict_input.insert("manufacturer", "pte");
        md_dict.insert("type", tr("Type"));
        md_dict_input.insert("type", "le");
        md_dict.insert("sn", tr("Serial number"));
        md_dict_input.insert("sn", "le");
        md_dict.insert("year", tr("Year of purchase"));
        md_dict_input.insert("year", QString("spb;1900;%1;2999").arg(QDate::currentDate().year()));
        md_dict.insert("commissioning", tr("Date of commissioning"));
        md_dict_input.insert("commissioning", "de");
        md_dict.insert("field", tr("Field of application"));
        QStringList fields;
        fields << tr("Car air conditioning") + "||car";
        fields << tr("Low-rise residential buildings") + "||lowrise";
        fields << tr("High-rise residential buildings") + "||highrise";
        fields << tr("Commercial buildings") + "||commercial";
        fields << tr("Institutional buildings") + "||institutional";
        fields << tr("Industrial spaces") + "||industrial";
        fields << tr("Transportation") + "||transportation";
        fields << tr("Air conditioning") + "||airconditioning";
        fields << tr("Heat pumps") + "||heatpumps";
        md_dict_input.insert("field", QString("cb;%1").arg(fields.join(";")));
        md_dict.insert("refrigerant", tr("Refrigerant"));
        md_dict_input.insert("refrigerant", "cb;R134a;R404a");
        md_dict.insert("refrigerant_amount", tr("Amount of refrigerant"));
        md_dict_input.insert("refrigerant_amount", QString("spb;0;0;999999; %1").arg(tr("kg")));
        md_dict.insert("oil", tr("Oil"));
        md_dict_input.insert("oil", "cb;Synthetic;Mineral");
        md_dict.insert("oil_amount", tr("Amount of oil"));
        md_dict_input.insert("oil_amount", QString("spb;0;0;999999; %1").arg(tr("kg")));
        md_dict.insert("life", tr("Service life"));
        md_dict_input.insert("life", QString("dspb;0.0;0.0;999999.9; %1").arg(tr("years")));
        md_dict.insert("runtime", tr("Run-time per day"));
        md_dict_input.insert("runtime", QString("dspb;0.0;0.0;24.0; %1").arg(tr("hours")));
        md_dict.insert("utilisation", tr("Rate of utilisation"));
        md_dict_input.insert("utilisation", QString("dspb;0.0;0.0;100.0; %1").arg(tr("%")));
    } else if (md_element.nodeName() == "inspection") {
        md_dict.insert("inspection", tr("Inspection"));
        md_dict.insert("date", tr("Date"));
        md_dict_input.insert("date", "dte");
        md_dict.insert("nominal", tr("Nominal"));
        md_dict_input.insert("nominal", "chb");
        QDomElement el_variables = md_parent->document.documentElement().firstChildElement("variables");
        if (!el_variables.isNull()) {
            QDomElement variable = el_variables.firstChildElement("var");
            while (!variable.isNull()) {
                QDomElement subvariable = variable.firstChildElement("var");
                if (subvariable.isNull()) {
                    md_dict_vars.insert(variable.attribute("id"), variable.attribute("name"));
                    if (variable.attribute("type") == "int") {
                        md_dict_input.insert(variable.attribute("id"), QString("spb;-999999999;0.0;999999999; %1").arg(variable.attribute("unit")));
                    } else if (variable.attribute("type") == "float") {
                        md_dict_input.insert(variable.attribute("id"), QString("dspb;-999999999.9;0.0;999999999.9; %1").arg(variable.attribute("unit")));
                    } else if (variable.attribute("type") == "string") {
                        md_dict_input.insert(variable.attribute("id"), "le");
                    }
                }
                while (!subvariable.isNull()) {
                    md_dict_vars.insert(QString("%1/%2").arg(variable.attribute("id")).arg(subvariable.attribute("id")), tr("%1: %2").arg(variable.attribute("name")).arg(subvariable.attribute("name")));
                    if (subvariable.attribute("type") == "int") {
                        md_dict_input.insert(QString("%1/%2").arg(variable.attribute("id")).arg(subvariable.attribute("id")), QString("spb;-999999999;0.0;999999999; %1").arg(subvariable.attribute("unit")));
                    } else if (subvariable.attribute("type") == "float") {
                        md_dict_input.insert(QString("%1/%2").arg(variable.attribute("id")).arg(subvariable.attribute("id")), QString("dspb;-999999999.9;0.0;999999999.9; %1").arg(subvariable.attribute("unit")));
                    } else if (subvariable.attribute("type") == "string") {
                        md_dict_input.insert(QString("%1/%2").arg(variable.attribute("id")).arg(subvariable.attribute("id")), "le");
                    }
                    subvariable = subvariable.nextSiblingElement();
                }
                variable = variable.nextSiblingElement();
            }
        }
    } else if (md_element.nodeName() == "var") {
        md_used_ids << "refrigerant_amount" << "oil_amount" << "sum";
        md_dict.insert("var", tr("Variable"));
        md_dict.insert("id", tr("ID"));
        md_dict_input.insert("id", "le");
        md_dict.insert("name", tr("Name"));
        md_dict_input.insert("name", "le");
        md_dict.insert("unit", tr("Unit"));
        md_dict_input.insert("unit", "le");
        md_dict.insert("type", tr("Type"));
        QStringList types;
        types << tr("Integer") + "||int";
        types << tr("Real number") + "||float";
        types << tr("String") + "||string";
        md_dict_input.insert("type", QString("cb;%1").arg(types.join(";")));
        md_dict.insert("value", tr("Value"));
        md_dict_input.insert("value", "pteh");
        md_dict.insert("compare_nom", tr("Compare value with the nominal one"));
        md_dict_input.insert("compare_nom", "chb");
        md_dict.insert("col_bg", tr("Colour"));
        md_dict_input.insert("col_bg", "ccb");
    } else if (md_element.nodeName() == "table") {
        md_dict.insert("table", tr("Table"));
        md_dict.insert("id", tr("ID"));
        md_dict_input.insert("id", "le");
        md_dict.insert("highlight_nominal", tr("Highlight the nominal inspection"));
        md_dict_input.insert("highlight_nominal", "chb");
    }
    // ------------
    if (!md_element.attribute("id").isEmpty()) {
        this->setWindowTitle(tr("%1: %2").arg(md_dict.value(md_element.nodeName())).arg(md_element.attribute("id")));
    } else if (!md_element.attribute("date").isEmpty()) {
        this->setWindowTitle(tr("%1: %2").arg(md_dict.value(md_element.nodeName())).arg(md_element.attribute("date")));
    } else {
        this->setWindowTitle(md_dict.value(md_element.nodeName()));
    }
    MTDictionary dict(md_dict);
    for (int i = 0; i < md_dict_vars.count(); ++i) {
        dict.insert(md_dict_vars.key(i), md_dict_vars.value(i));
    }
    QLabel * md_lbl_var = NULL; QWidget * md_w_var = NULL;
    int i = 0; QStringList inputtype, var_id; QString value;
    int num_cols = (dict.count() - 1) / 20 + 1;
    int num_rows = (dict.count() - 1) / num_cols + ((dict.count() - 1) % num_cols > 0 ? 1 : 0);
    for (int c = 0; c < num_cols; ++c) {
        for (int r = 0; r < num_rows; ++r) {
            if (i >= dict.count()) { break; }
            if (dict.key(i) == md_element.nodeName()) { i++; r--; continue; }
            if (md_dict_vars.contains(dict.key(i))) {
                value.clear(); QDomElement var;
                var_id = dict.key(i).split("/");
                QDomElement variable = md_element.firstChildElement("var");
                while (!variable.isNull()) {
                    if (variable.attribute("id") == var_id.at(0)) {
                        if (var_id.count() < 2) { value = variable.text(); }
                        else { var = variable; }
                        break;
                    }
                    variable = variable.nextSiblingElement();
                }
                if (!var.isNull()) {
                    QDomElement subvariable = var.firstChildElement("var");
                    while (!subvariable.isNull()) {
                        if (subvariable.attribute("id") == var_id.at(1)) {
                            value = subvariable.text(); break;
                        }
                        subvariable = subvariable.nextSiblingElement();
                    }
                }
            } else if (dict.key(i) == "value" && md_element.nodeName() == "var") {
                value = loadExpression(md_element, "value");
            } else {
                value = md_element.attribute(dict.key(i));
            }
            inputtype = md_dict_input.value(dict.key(i)).split(";");
            if (inputtype.at(0) != "chb") {
                md_lbl_var = new QLabel(tr("%1:").arg(dict.value(i)), this);
                md_lbl_var->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                md_grid_main->addWidget(md_lbl_var, r, 2 * c);
            }
            md_w_var = createInputWidget(inputtype, dict.value(i), value);
            if (dict.key(i) == "nominal") { md_w_var->setEnabled(nominal_allowed); }
            md_grid_main->addWidget(md_w_var, r, (2 * c) + 1);
            md_vars.insert(dict.key(i), md_w_var);
            i++;
        }
    }
}

QWidget * ModifyDialogue::createInputWidget(const QStringList & inputtype, const QString & name, const QString & value)
{
    if (inputtype.at(0) == "le") {
        QLineEdit * md_le_var = new QLineEdit(this);
        md_le_var->setText(value);
        return md_le_var;
    } else if (inputtype.at(0) == "chb") {
        QCheckBox * md_chb_var = new QCheckBox(name, this);
        md_chb_var->setChecked(value == "true");
        return md_chb_var;
    } else if (inputtype.at(0) == "spb") {
        QSpinBox * md_spb_var = new QSpinBox(this);
        if (inputtype.count() > 1) { md_spb_var->setMinimum(inputtype.at(1).toInt()); }
        if (inputtype.count() > 3) { md_spb_var->setMaximum(inputtype.at(3).toInt()); }
        if (inputtype.count() > 2) { md_spb_var->setValue((value.isEmpty() ? inputtype.at(2) : value).toInt()); }
        else { md_spb_var->setValue(value.toInt()); }
        if (inputtype.count() > 4) { md_spb_var->setSuffix(inputtype.at(4)); }
        return md_spb_var;
    } else if (inputtype.at(0) == "dspb") {
        QDoubleSpinBox * md_dspb_var = new QDoubleSpinBox(this);
        if (inputtype.count() > 1) { md_dspb_var->setMinimum(inputtype.at(1).toDouble()); }
        if (inputtype.count() > 3) { md_dspb_var->setMaximum(inputtype.at(3).toDouble()); }
        if (inputtype.count() > 2) { md_dspb_var->setValue((value.isEmpty() ? inputtype.at(2) : value).toDouble()); }
        else { md_dspb_var->setValue(value.toDouble()); }
        if (inputtype.count() > 4) { md_dspb_var->setSuffix(inputtype.at(4)); }
        return md_dspb_var;
    } else if (inputtype.at(0) == "cb") {
        QComboBox * md_cb_var = new QComboBox(this); int n = -1;
        for (int j = 1; j < inputtype.count(); ++j) {
            md_cb_var->addItem(inputtype.at(j).split("||").first());
            if (inputtype.at(j).split("||").last() == value) { n = j - 1; }
        }
        md_cb_var->setCurrentIndex(n);
        return md_cb_var;
    } else if (inputtype.at(0) == "ccb") {
        MTColourComboBox * md_ccb_var = new MTColourComboBox(this);
        for (int j = 0; j < md_ccb_var->count(); ++j) {
            if (md_ccb_var->itemText(j) == value) { md_ccb_var->setCurrentIndex(j); break; }
        }
        return md_ccb_var;
    } else if (inputtype.at(0) == "dte") {
        QDateTimeEdit * md_dte_var = new QDateTimeEdit(this);
        md_dte_var->setDateTime(QDateTime::fromString(value, "yyyy.MM.dd-hh:mm"));
        return md_dte_var;
    } else if (inputtype.at(0) == "de") {
        QDateEdit * md_de_var = new QDateEdit(this);
        md_de_var->setDate(QDate::fromString(value, "yyyy.MM.dd"));
        return md_de_var;
    } else if (inputtype.at(0) == "pteh") {
        QPlainTextEdit * md_pte_var = new QPlainTextEdit(this);
        md_pte_var->setMinimumSize(200, 30);
        md_pte_var->setPlainText(value);
        new Highlighter(md_used_ids, md_pte_var->document());
        return md_pte_var;
    } else {
        QPlainTextEdit * md_pte_var = new QPlainTextEdit(this);
        md_pte_var->setMinimumSize(200, 30);
        md_pte_var->setPlainText(value);
        return md_pte_var;
    }
    return new QWidget;
}

QString ModifyDialogue::getInputFromWidget(QWidget * input_widget, const QStringList & inputtype, const QString & key)
{
    QString value;
    if (inputtype.at(0) == "chb") {
        value = ((QCheckBox *)input_widget)->isChecked() ? "true" : "false";
    } else if (inputtype.at(0) == "le") {
        value = ((QLineEdit *)input_widget)->text();
        if (key == "id") {
            if (value.isEmpty()) {
                QMessageBox::information(this, tr("Save changes"), tr("Invalid ID."));
                return QString();
            }
            if (md_used_ids.contains(value)) {
                QMessageBox::information(this, tr("Save changes"), tr("This ID is not available. Please choose a different ID."));
                return QString();
            }
        }
    } else if (inputtype.at(0) == "spb") {
        value = QString("%1").arg(((QSpinBox *)input_widget)->value());
    } else if (inputtype.at(0) == "dspb") {
        value = QString("%1").arg(((QDoubleSpinBox *)input_widget)->value());
    } else if (inputtype.at(0) == "cb") {
        MTDictionary item_values;
        for (int j = 1; j < inputtype.count(); ++j) {
            item_values.insert(inputtype.at(j).split("||").first(), inputtype.at(j).split("||").last());
        }
        value = item_values.value(((QComboBox *)input_widget)->currentText());
    } else if (inputtype.at(0) == "ccb") {
        value = ((MTColourComboBox *)input_widget)->currentText();
    } else if (inputtype.at(0) == "dte") {
        value = ((QDateTimeEdit *)input_widget)->dateTime().toString("yyyy.MM.dd-hh:mm");
        if (key == "date") {
            if (md_used_ids.contains(value)) {
                QMessageBox::information(this, tr("Save changes"), tr("This date is not available. Please choose a different date."));
                return QString();
            }
        }
    } else if (inputtype.at(0) == "de") {
        value = ((QDateEdit *)input_widget)->date().toString("yyyy.MM.dd");
    } else {
        value = ((QPlainTextEdit *)input_widget)->toPlainText();
    }
    return value.isNull() ? QString("") : value;
}

void ModifyDialogue::save()
{
    MTDictionary values, var_values; QStringList inputtype; QString exp;
    QMapIterator<QString, QWidget *> i(md_vars);
    while (i.hasNext()) { i.next();
        inputtype = md_dict_input.value(i.key()).split(";");
        QString value = getInputFromWidget(i.value(), inputtype, i.key());
        if (value.isNull()) { return; }
        if (md_dict_vars.contains(i.key())) {
            var_values.insert(i.key(), value);
        } else if (i.key() == "value" && md_element.nodeName() == "var") {
            exp = value;
        } else {
            values.insert(i.key(), value);
        }
    }
    for (int i = 0; i < values.count(); ++i) {
        md_element.setAttribute(values.key(i), values.value(i));
    }
    QStringList var_id;
    for (int i = 0; i < var_values.count(); ++i) {
        var_id = var_values.key(i).split("/");
        QDomElement var, subvar;
        QDomElement variable = md_element.firstChildElement("var");
        while (!variable.isNull()) {
            if (variable.attribute("id") == var_id.at(0)) {
                var = variable; break;
            }
            variable = variable.nextSiblingElement();
        }
        if (var.isNull()) {
            var = md_parent->document.createElement("var");
            var.setAttribute("id", var_id.at(0));
            md_element.appendChild(var);
        }
        if (var_id.count() > 1) {
            QDomElement subvariable = var.firstChildElement("var");
            while (!subvariable.isNull()) {
                if (subvariable.attribute("id") == var_id.at(1)) {
                    subvar = subvariable; break;
                }
                subvariable = subvariable.nextSiblingElement();
            }
            if (subvar.isNull()) {
                subvar = md_parent->document.createElement("var");
                subvar.setAttribute("id", var_id.at(1));
                var.appendChild(subvar);
            }
        } else { subvar = var; }
        while (subvar.hasChildNodes()) { subvar.removeChild(subvar.firstChild()); }
        QDomText var_value = md_parent->document.createTextNode(var_values.value(i));
        subvar.appendChild(var_value);
    }
    saveExpression(exp, md_element, "value");
    accept();
}

QString ModifyDialogue::loadExpression(QDomElement & element, const QString & node_name)
{
    QString exp(""); int last_f_pos = 0;
    QDomElement el_value = element.firstChildElement(node_name);
    if (!el_value.isNull()) {
        QDomElement ec = el_value.firstChildElement("ec");
        while (!ec.isNull()) {
            exp.append(ec.attribute("id"));
            if (ec.hasAttribute("f")) { last_f_pos = exp.count(); exp.append(ec.attribute("f")); }
            exp.append(ec.attribute("cc_attr"));
            if (ec.hasAttribute("sum")) { exp.insert(last_f_pos, "sum"); exp.append(ec.attribute("sum")); }
            ec = ec.nextSiblingElement();
        }
    }
    return exp;
}

void ModifyDialogue::saveExpression(const QString & exp, QDomElement & element, const QString & node_name)
{
    if (!exp.isEmpty()) {
        QSet<int> matched;
        for (int i = 0; i < md_used_ids.count(); ++i) {
            QRegExp expression(QString("\\b%1\\b").arg(md_used_ids.at(i)));
            int index = exp.indexOf(expression);
            while (index >= 0) {
                int length = expression.matchedLength();
                if (!matched.contains(index)) {
                    for (int j = index; j < index + length; j++) { matched << j; }
                }
                index = exp.indexOf(expression, index + length);
            }
        }
        QDomElement el_value = element.firstChildElement(node_name);
        if (el_value.isNull()) {
            el_value = md_parent->document.createElement(node_name);
            element.appendChild(el_value);
        }
        while (el_value.hasChildNodes()) { el_value.removeChild(el_value.firstChild()); }
        QString id_, f_; bool last_id = false; bool last_sum = false;
        for (int i = 0; i < exp.length(); ++i) {
            if (matched.contains(i)) {
                if (!f_.isEmpty()) {
                    QDomElement ec = md_parent->document.createElement("ec");
                    ec.setAttribute("f", f_);
                    el_value.appendChild(ec);
                    f_.clear();
                }
                last_id = true;
                id_.append(exp.at(i));
            } else {
                if (!id_.isEmpty()) {
                    if (id_ == "sum") {
                        last_sum = true;
                    } else {
                        QDomElement ec = md_parent->document.createElement("ec");
                        if (id_ == "refrigerant_amount" || id_ == "oil_amount") {
                            ec.setAttribute("cc_attr", id_);
                        } else {
                            ec.setAttribute(last_sum ? "sum" : "id", id_);
                        }
                        last_sum = false;
                        el_value.appendChild(ec);
                    }
                    id_.clear();
                }
                last_id = false;
                f_.append(exp.at(i));
            }
        }
        if (!f_.isEmpty()) {
            QDomElement ec = md_parent->document.createElement("ec");
            ec.setAttribute("f", f_);
            el_value.appendChild(ec);
        }
        if (!id_.isEmpty()) {
            QDomElement ec = md_parent->document.createElement("ec");
            if (id_ == "refrigerant_amount" || id_ == "oil_amount") {
                ec.setAttribute("cc_attr", id_);
            } else {
                ec.setAttribute(last_sum ? "sum" : "id", id_);
            }
            el_value.appendChild(ec);
        }
    } else if (!exp.isNull()) {
        QDomElement el_value = element.firstChildElement(node_name);
        while (!el_value.isNull()) {
            element.removeChild(el_value);
            el_value = element.firstChildElement(node_name);
        }
    }
}
