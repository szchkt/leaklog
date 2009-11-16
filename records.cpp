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

#include "modify_dialogue.h"

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

QLabel * MDInputWidget::createLabel(QWidget * parent, const QString & text)
{
    QLabel * lbl = new QLabel(text, parent);
    lbl->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    return lbl;
}

MDLineEdit::MDLineEdit(const QString & id, const QString & labeltext, QWidget * parent, const QString & value, const QString & inputmask, const QString & colour, bool enabled):
QLineEdit(parent),
MDInputWidget(id, labeltext, parent, this)
{
    if (!colour.isEmpty()) { setPalette(paletteForColour(colour)); }
    setMinimumSize(150, sizeHint().height());
    setEnabled(enabled);
    if (!inputmask.isEmpty()) { setInputMask(inputmask); }
    setText(value);
}

QVariant MDLineEdit::variantValue()
{
    return text();
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
    return cb_items.value(currentText());
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

DBRecord::DBRecord():
MTRecord()
{}

DBRecord::DBRecord(const QString & type, const QString & id_field, const QString & id, const MTDictionary & parents):
MTRecord(type, id_field, id, parents)
{}

Customer::Customer(const QString & id):
DBRecord("customers", "id", id, MTDictionary())
{}

void Customer::initModifyDialogue(ModifyDialogue * md)
{
    md->setWindowTitle(tr("Customer"));
    StringVariantMap attributes;
    if (!id().isEmpty()) {
        attributes = list();
    }
    md->addInputWidget(new MDLineEdit("id", tr("ID:"), md, attributes.value("id").toString(), "00000000"));
    md->addInputWidget(new MDLineEdit("company", tr("Company:"), md, attributes.value("company").toString()));
    md->addInputWidget(new MDLineEdit("contact_person", tr("Contact person:"), md, attributes.value("contact_person").toString()));
    md->addInputWidget(new MDAddressEdit("address", tr("Address:"), md, attributes.value("address").toString()));
    md->addInputWidget(new MDLineEdit("mail", tr("E-mail:"), md, attributes.value("mail").toString()));
    md->addInputWidget(new MDLineEdit("phone", tr("Phone:"), md, attributes.value("phone").toString()));
    QStringList used_ids; QSqlQuery query_used_ids;
    query_used_ids.setForwardOnly(true);
    query_used_ids.prepare("SELECT id FROM customers" + QString(id().isEmpty() ? "" : " WHERE id <> :id"));
    if (!id().isEmpty()) { query_used_ids.bindValue(":id", id()); }
    if (query_used_ids.exec()) {
        while (query_used_ids.next()) {
            used_ids << query_used_ids.value(0).toString();
        }
    }
    md->setUsedIds(used_ids);
}

Circuit::Circuit(const QString & parent, const QString & id):
DBRecord("circuits", "id", id, MTDictionary("parent", parent))
{}

void Circuit::initModifyDialogue(ModifyDialogue * md)
{
    MTDictionary refrigerants(listRefrigerantsToString().split(';'));

    md->setWindowTitle(tr("Customer: %1 > Cooling circuit").arg(parents()->value("parent").rightJustified(8, '0')));
    StringVariantMap attributes;
    if (!id().isEmpty()) {
        attributes = list();
    } else {
        attributes.insert("year", QDate::currentDate().year());
    }
    md->addInputWidget(new MDLineEdit("id", tr("ID:"), md, attributes.value("id").toString(), "0000"));
    md->addInputWidget(new MDLineEdit("name", tr("Circuit name:"), md, attributes.value("name").toString()));
    md->addInputWidget(new MDCheckBox("disused", tr("Disused"), md, attributes.value("disused").toInt()));
    md->addInputWidget(new MDLineEdit("operation", tr("Place of operation:"), md, attributes.value("operation").toString()));
    md->addInputWidget(new MDLineEdit("building", tr("Building:"), md, attributes.value("building").toString()));
    md->addInputWidget(new MDLineEdit("device", tr("Device:"), md, attributes.value("device").toString()));
    md->addInputWidget(new MDCheckBox("hermetic", tr("Hermetically sealed"), md, attributes.value("hermetic").toInt()));
    md->addInputWidget(new MDPlainTextEdit("manufacturer", tr("Manufacturer:"), md, attributes.value("manufacturer").toString()));
    md->addInputWidget(new MDLineEdit("type", tr("Type:"), md, attributes.value("type").toString()));
    md->addInputWidget(new MDLineEdit("sn", tr("Serial number:"), md, attributes.value("sn").toString()));
    md->addInputWidget(new MDSpinBox("year", tr("Year of purchase:"), md, 1900, 2999, attributes.value("year").toInt()));
    md->addInputWidget(new MDDateEdit("commissioning", tr("Date of commissioning:"), md, attributes.value("commissioning").toString()));
    md->addInputWidget(new MDComboBox("field", tr("Field of application:"), md, attributes.value("field").toString(), get_dict_fields()));
    md->addInputWidget(new MDComboBox("refrigerant", tr("Refrigerant:"), md, attributes.value("refrigerant").toString(), refrigerants));
    md->addInputWidget(new MDDoubleSpinBox("refrigerant_amount", tr("Amount of refrigerant:"), md, 0.0, 999999.9, attributes.value("refrigerant_amount").toDouble(), tr("kg")));
    md->addInputWidget(new MDComboBox("oil", tr("Oil:"), md, attributes.value("oil").toString(), get_dict_oils()));
    md->addInputWidget(new MDDoubleSpinBox("oil_amount", tr("Amount of oil:"), md, 0.0, 999999.9, attributes.value("oil_amount").toDouble(), tr("kg")));
    md->addInputWidget(new MDCheckBox("leak_detector", tr("Fixed leakage detector installed"), md, attributes.value("leak_detector").toInt()));
    md->addInputWidget(new MDDoubleSpinBox("runtime", tr("Run-time per day:"), md, 0.0, 24.0, attributes.value("runtime").toDouble(), tr("hours")));
    md->addInputWidget(new MDDoubleSpinBox("utilisation", tr("Rate of utilisation:"), md, 0.0, 100.0, attributes.value("utilisation").toDouble(), tr("%")));
    md->addInputWidget(new MDSpinBox("inspection_interval", tr("Inspection interval:"), md, 0, 999999, attributes.value("inspection_interval").toInt(), tr("days")));
    QStringList used_ids; QSqlQuery query_used_ids;
    query_used_ids.setForwardOnly(true);
    query_used_ids.prepare("SELECT id FROM circuits WHERE parent = :parent" + QString(id().isEmpty() ? "" : " AND id <> :id"));
    query_used_ids.bindValue(":parent", parents()->value("parent"));
    if (!id().isEmpty()) { query_used_ids.bindValue(":id", id()); }
    if (query_used_ids.exec()) {
        while (query_used_ids.next()) {
            used_ids << query_used_ids.value(0).toString();
        }
    }
    md->setUsedIds(used_ids);
}

Inspection::Inspection(const QString & customer, const QString & circuit, const QString & date):
DBRecord("inspections", "date", date, MTDictionary(QStringList() << "customer" << "circuit", QStringList() << customer << circuit))
{}

void Inspection::initModifyDialogue(ModifyDialogue * md)
{
    md->setWindowTitle(tr("Customer: %1 > Cooling circuit: %2 > Inspection")
                       .arg(parents()->value("customer").rightJustified(8, '0'))
                       .arg(parents()->value("circuit").rightJustified(4, '0')));
    StringVariantMap attributes;
    if (!id().isEmpty()) {
        attributes = list();
    }
    bool nominal_allowed = true;
    QStringList used_ids; QSqlQuery query_used_ids;
    query_used_ids.setForwardOnly(true);
    query_used_ids.prepare("SELECT date, nominal FROM inspections WHERE customer = :customer AND circuit = :circuit" + QString(id().isEmpty() ? "" : " AND date <> :date"));
    query_used_ids.bindValue(":customer", parents()->value("customer"));
    query_used_ids.bindValue(":circuit", parents()->value("circuit"));
    if (!id().isEmpty()) { query_used_ids.bindValue(":date", id()); }
    if (query_used_ids.exec()) {
        while (query_used_ids.next()) {
            used_ids << query_used_ids.value(0).toString();
            if (nominal_allowed && query_used_ids.value(1).toInt()) { nominal_allowed = false; }
        }
    }
    md->setUsedIds(used_ids);
    md->addInputWidget(new MDDateTimeEdit("date", tr("Date:"), md, attributes.value("date").toString()));
    MTCheckBoxGroup * chbgrp_i_type = new MTCheckBoxGroup(md);
    MDCheckBox * chb_nominal = new MDCheckBox("nominal", tr("Nominal inspection"), md, attributes.value("nominal").toInt(), nominal_allowed);
    md->addInputWidget(chb_nominal);
    chbgrp_i_type->addCheckBox((MTCheckBox *)chb_nominal->widget());
    MDCheckBox * chb_repair = new MDCheckBox("repair", tr("Repair"), md, attributes.value("repair").toInt(), true);
    md->addInputWidget(chb_repair);
    chbgrp_i_type->addCheckBox((MTCheckBox *)chb_repair->widget());
    Variables query; QString var_id, var_name, var_type, subvar_id, subvar_name, subvar_type;
    while (query.next()) {
        var_id = query.value("VAR_ID").toString();
        subvar_id = query.value("SUBVAR_ID").toString();
        if (subvar_id.isEmpty()) {
            if (!query.value("VAR_VALUE").toString().isEmpty()) { continue; }
            var_name = tr("%1:").arg(query.value("VAR_NAME").toString());
            var_type = query.value("VAR_TYPE").toString();
            if (var_id == "inspector") {
                md->addInputWidget(new MDComboBox(var_id, var_name, md,
                    attributes.value(var_id).toString(), listInspectors(), query.value("VAR_COL_BG").toString()));
            } else if (var_type == "int") {
                md->addInputWidget(new MDSpinBox(var_id, var_name, md, -999999999, 999999999,
                    attributes.value(var_id).toInt(), query.value("VAR_UNIT").toString(), query.value("VAR_COL_BG").toString()));
            } else if (var_type == "float") {
                md->addInputWidget(new MDDoubleSpinBox(var_id, var_name, md, -999999999.9, 999999999.9,
                    attributes.value(var_id).toDouble(), query.value("VAR_UNIT").toString(), query.value("VAR_COL_BG").toString()));
            } else if (var_type == "string") {
                md->addInputWidget(new MDLineEdit(var_id, var_name, md,
                    attributes.value(var_id).toString(), "", query.value("VAR_COL_BG").toString()));
            } else if (var_type == "text") {
                md->addInputWidget(new MDPlainTextEdit(var_id, var_name, md,
                    attributes.value(var_id).toString(), query.value("VAR_COL_BG").toString()));
            } else if (var_type == "bool") {
                MDCheckBox * chb = new MDCheckBox(var_id, "", md, attributes.value(var_id).toInt());
                chb->label()->setText(var_name);
                md->addInputWidget(chb);
            } else {
                md->addInputWidget(new MDLineEdit(var_id, var_name, md,
                    attributes.value(var_id).toString(), "", query.value("VAR_COL_BG").toString()));
            }
        } else {
            if (!query.value("SUBVAR_VALUE").toString().isEmpty()) { continue; }
            subvar_name = tr("%1: %2:").arg(query.value("VAR_NAME").toString()).arg(query.value("SUBVAR_NAME").toString());
            subvar_type = query.value("SUBVAR_TYPE").toString();
            if (subvar_type == "int") {
                md->addInputWidget(new MDSpinBox(subvar_id, subvar_name, md, -999999999, 999999999,
                    attributes.value(subvar_id).toInt(), query.value("SUBVAR_UNIT").toString(), query.value("VAR_COL_BG").toString()));
            } else if (subvar_type == "float") {
                md->addInputWidget(new MDDoubleSpinBox(subvar_id, subvar_name, md, -999999999.9, 999999999.9,
                    attributes.value(subvar_id).toDouble(), query.value("SUBVAR_UNIT").toString(), query.value("VAR_COL_BG").toString()));
            } else if (subvar_type == "string") {
                md->addInputWidget(new MDLineEdit(subvar_id, subvar_name, md,
                    attributes.value(subvar_id).toString(), "", query.value("VAR_COL_BG").toString()));
            } else if (subvar_type == "text") {
                md->addInputWidget(new MDPlainTextEdit(subvar_id, subvar_name, md,
                    attributes.value(subvar_id).toString(), query.value("VAR_COL_BG").toString()));
            } else if (subvar_type == "bool") {
                MDCheckBox * chb = new MDCheckBox(subvar_id, query.value("SUBVAR_NAME").toString(), md,
                    attributes.value(subvar_id).toInt());
                chb->label()->setText(tr("%1:").arg(query.value("VAR_NAME").toString()));
                md->addInputWidget(chb);
            } else {
                md->addInputWidget(new MDLineEdit(subvar_id, subvar_name, md,
                    attributes.value(subvar_id).toString(), "", query.value("VAR_COL_BG").toString()));
            }
        }
    }
}

Repair::Repair(const QString & date):
DBRecord("repairs", "date", date, MTDictionary())
{}

void Repair::initModifyDialogue(ModifyDialogue * md)
{
    MTDictionary refrigerants(listRefrigerantsToString().split(';'));

    md->setWindowTitle(tr("Repair"));
    StringVariantMap attributes;
    if (!id().isEmpty()) {
        attributes = list();
    }
    md->addInputWidget(new MDDateTimeEdit("date", tr("Date:"), md, attributes.value("date").toString()));
    md->addInputWidget(new MDLineEdit("customer", tr("Customer:"), md, attributes.value("customer").toString()));
    md->addInputWidget(new MDLineEdit("device", tr("Device:"), md, attributes.value("device").toString()));
    md->addInputWidget(new MDComboBox("field", tr("Field of application:"), md, attributes.value("field").toString(), get_dict_fields()));
    md->addInputWidget(new MDComboBox("refrigerant", tr("Refrigerant:"), md, attributes.value("refrigerant").toString(), refrigerants));
    md->addInputWidget(new MDComboBox("repairman", tr("Repairman:"), md, attributes.value("repairman").toString(), listInspectors()));
    md->addInputWidget(new MDLineEdit("arno", tr("Assembly record No.:"), md, attributes.value("arno").toString()));
    md->addInputWidget(new MDDoubleSpinBox("refrigerant_amount", tr("Amount of refrigerant:"), md, 0.0, 999999.9, attributes.value("refrigerant_amount").toDouble(), tr("kg")));
    md->addInputWidget(new MDDoubleSpinBox("refr_add_am", tr("%1: %2:").arg(tr("Refrigerant addition")).arg(tr("New")), md, -999999999.9, 999999999.9, attributes.value("refr_add_am").toDouble(), tr("kg")));
    md->addInputWidget(new MDDoubleSpinBox("refr_add_am_recy", tr("%1: %2:").arg(tr("Refrigerant addition")).arg(tr("Recovered")), md, -999999999.9, 999999999.9, attributes.value("refr_add_am_recy").toDouble(), tr("kg")));
    md->addInputWidget(new MDDoubleSpinBox("refr_reco", tr("%1: %2:").arg(tr("Refrigerant recovery")).arg(tr("Store")), md, -999999999.9, 999999999.9, attributes.value("refr_reco").toDouble(), tr("kg")));
    md->addInputWidget(new MDDoubleSpinBox("refr_reco_cust", tr("%1: %2:").arg(tr("Refrigerant recovery")).arg(tr("Customer")), md, -999999999.9, 999999999.9, attributes.value("refr_reco_cust").toDouble(), tr("kg")));
    QStringList used_ids; QSqlQuery query_used_ids;
    query_used_ids.setForwardOnly(true);
    query_used_ids.prepare("SELECT date FROM repairs WHERE" + QString(id().isEmpty() ? "" : " date <> :date"));
    if (!id().isEmpty()) { query_used_ids.bindValue(":date", id()); }
    if (query_used_ids.exec()) {
        while (query_used_ids.next()) {
            used_ids << query_used_ids.value(0).toString();
        }
    }
    md->setUsedIds(used_ids);
}

VariableRecord::VariableRecord(Type type, const QString & var_id, const QString & subvar_id):
DBRecord(type ? "subvariables" : "variables", "id", type ? subvar_id : var_id, type ? MTDictionary("parent", var_id) : MTDictionary())
{ v_type = type; }

void VariableRecord::initModifyDialogue(ModifyDialogue * md)
{
    switch (v_type) {
        case SUBVARIABLE: md->setWindowTitle(tr("Subvariable")); break;
        default: md->setWindowTitle(tr("Variable")); break;
    }
    StringVariantMap attributes; bool enable_all = true;
    if (!id().isEmpty()) {
        if (v_type == SUBVARIABLE) {
            Subvariable query(parents()->value("parent"), id());
            if (query.next()) {
                attributes.insert("id", query.value("SUBVAR_ID"));
                attributes.insert("name", query.value("SUBVAR_NAME"));
                attributes.insert("type", query.value("SUBVAR_TYPE"));
                attributes.insert("unit", query.value("SUBVAR_UNIT"));
                attributes.insert("value", query.value("SUBVAR_VALUE"));
                attributes.insert("compare_nom", query.value("SUBVAR_COMPARE_NOM"));
                attributes.insert("tolerance", query.value("SUBVAR_TOLERANCE"));
            }
        } else {
            Variable query(id());
            if (query.next()) {
                attributes.insert("id", query.value("VAR_ID"));
                attributes.insert("name", query.value("VAR_NAME"));
                attributes.insert("type", query.value("VAR_TYPE"));
                attributes.insert("unit", query.value("VAR_UNIT"));
                attributes.insert("value", query.value("VAR_VALUE"));
                attributes.insert("compare_nom", query.value("VAR_COMPARE_NOM"));
                attributes.insert("tolerance", query.value("VAR_TOLERANCE"));
                attributes.insert("col_bg", query.value("VAR_COL_BG"));
            }
        }
        if (get_dict_varnames().contains(id())) { enable_all = false; }
    }
    QStringList used_ids;
    used_ids << "refrigerant_amount" << "oil_amount" << "sum" << "p_to_t";
    used_ids << listSupportedFunctions();
    used_ids << listVariableIds(true);
    if (!id().isEmpty()) { used_ids.removeAll(id()); }
    md->setUsedIds(used_ids);
    md->addInputWidget(new MDLineEdit("id", tr("ID:"), md, attributes.value("id").toString(), "", "", enable_all));
    md->addInputWidget(new MDLineEdit("name", tr("Name:"), md, attributes.value("name").toString(), "", "", enable_all));
    md->addInputWidget(new MDLineEdit("unit", tr("Unit:"), md, attributes.value("unit").toString(), "", "", enable_all));
    md->addInputWidget(new MDComboBox("type", tr("Type:"), md, attributes.value("type").toString(), get_dict_vartypes().swapKeysAndValues(), "", enable_all));
    md->addInputWidget(new MDHighlightedPlainTextEdit("value", tr("Value:"), md, attributes.value("value").toString(), used_ids, enable_all));
    md->addInputWidget(new MDCheckBox("compare_nom", tr("Compare value with the nominal one"), md, attributes.value("compare_nom").toInt()));
    md->addInputWidget(new MDDoubleSpinBox("tolerance", tr("Tolerance:"), md, 0.0, 999999.9, attributes.value("tolerance").toDouble()));
    if (v_type == VARIABLE) {
        md->addInputWidget(new MDColourComboBox("col_bg", tr("Colour:"), md, attributes.value("col_bg").toString()));
    }
}

Table::Table(const QString & id, const QString & uid):
DBRecord("tables", uid.isEmpty() ? "id" : "uid", uid.isEmpty() ? id : uid, MTDictionary())
{}

void Table::initModifyDialogue(ModifyDialogue * md)
{
    md->setWindowTitle(tr("Table"));
    StringVariantMap attributes;
    if (!id().isEmpty()) {
        attributes = list();
    }
    md->addInputWidget(new MDLineEdit("id", tr("Name:"), md, attributes.value("id").toString()));
    md->addInputWidget(new MDCheckBox("highlight_nominal", tr("Highlight the nominal inspection"), md, attributes.value("highlight_nominal").toInt()));
    QStringList used_ids; QSqlQuery query_used_ids;
    query_used_ids.setForwardOnly(true);
    query_used_ids.prepare("SELECT id FROM tables" + QString(id().isEmpty() ? "" : " WHERE id <> :id"));
    if (!id().isEmpty()) { query_used_ids.bindValue(":id", id()); }
    if (query_used_ids.exec()) {
        while (query_used_ids.next()) {
            used_ids << query_used_ids.value(0).toString();
        }
    }
    md->setUsedIds(used_ids);
}

Inspector::Inspector(const QString & id):
DBRecord("inspectors", "id", id, MTDictionary())
{}

void Inspector::initModifyDialogue(ModifyDialogue * md)
{
    md->setWindowTitle(tr("Inspector"));
    StringVariantMap attributes;
    if (!id().isEmpty()) {
        attributes = list();
    }
    md->addInputWidget(new MDLineEdit("id", tr("ID:"), md, attributes.value("id").toString(), "0000"));
    md->addInputWidget(new MDLineEdit("person", tr("Certified person:"), md, attributes.value("person").toString()));
    md->addInputWidget(new MDLineEdit("company", tr("Certified company:"), md, attributes.value("company").toString()));
    md->addInputWidget(new MDLineEdit("person_reg_num", tr("Person registry number:"), md, attributes.value("person_reg_num").toString()));
    md->addInputWidget(new MDLineEdit("phone", tr("Phone:"), md, attributes.value("phone").toString()));
    QStringList used_ids; QSqlQuery query_used_ids;
    query_used_ids.setForwardOnly(true);
    query_used_ids.prepare("SELECT id FROM inspectors" + QString(id().isEmpty() ? "" : " WHERE id <> :id"));
    if (!id().isEmpty()) { query_used_ids.bindValue(":id", id()); }
    if (query_used_ids.exec()) {
        while (query_used_ids.next()) {
            used_ids << query_used_ids.value(0).toString();
        }
    }
    md->setUsedIds(used_ids);
}

ServiceCompany::ServiceCompany(const QString & id):
DBRecord("service_companies", "id", id, MTDictionary())
{}

void ServiceCompany::initModifyDialogue(ModifyDialogue * md)
{
    md->setWindowTitle(tr("Service company"));
    StringVariantMap attributes;
    if (!id().isEmpty()) {
        attributes = list();
    }
    md->addInputWidget(new MDLineEdit("name", tr("Name:"), md, attributes.value("name").toString()));
    md->addInputWidget(new MDLineEdit("id", tr("ID:"), md, attributes.value("id").toString(), "00000000"));
    md->addInputWidget(new MDAddressEdit("address", tr("Address:"), md, attributes.value("address").toString()));
    md->addInputWidget(new MDLineEdit("phone", tr("Phone:"), md, attributes.value("phone").toString()));
    md->addInputWidget(new MDLineEdit("mail", tr("E-mail:"), md, attributes.value("mail").toString()));
    md->addInputWidget(new MDLineEdit("website", tr("Website:"), md, attributes.value("website").toString()));
    QStringList used_ids; QSqlQuery query_used_ids;
    query_used_ids.setForwardOnly(true);
    query_used_ids.prepare("SELECT id FROM service_companies" + QString(id().isEmpty() ? "" : " WHERE id <> :id"));
    if (!id().isEmpty()) { query_used_ids.bindValue(":id", id()); }
    if (query_used_ids.exec()) {
        while (query_used_ids.next()) {
            used_ids << query_used_ids.value(0).toString();
        }
    }
    md->setUsedIds(used_ids);
}

RecordOfRefrigerantManagement::RecordOfRefrigerantManagement(const QString & date):
DBRecord("refrigerant_management", "date", date, MTDictionary())
{}

void RecordOfRefrigerantManagement::initModifyDialogue(ModifyDialogue * md)
{
    MTDictionary refrigerants(listRefrigerantsToString().split(';'));

    md->setWindowTitle(tr("Record of refrigerant management"));
    StringVariantMap attributes;
    if (!id().isEmpty()) {
        attributes = list();
    }
    md->addInputWidget(new MDDateTimeEdit("date", tr("Date:"), md, attributes.value("date").toString()));
    md->addInputWidget(new MDComboBox("refrigerant", tr("Refrigerant:"), md, attributes.value("refrigerant").toString(), refrigerants));
    md->addInputWidget(new MDDoubleSpinBox("purchased", tr("Purchased (new):"), md, 0.0, 999999999.9, attributes.value("purchased").toDouble(), tr("kg")));
    md->addInputWidget(new MDDoubleSpinBox("purchased_reco", tr("Purchased (recovered):"), md, 0.0, 999999999.9, attributes.value("purchased_reco").toDouble(), tr("kg")));
    md->addInputWidget(new MDDoubleSpinBox("sold", tr("Sold (new):"), md, 0.0, 999999999.9, attributes.value("sold").toDouble(), tr("kg")));
    md->addInputWidget(new MDDoubleSpinBox("sold_reco", tr("Sold (recovered):"), md, 0.0, 999999999.9, attributes.value("sold_reco").toDouble(), tr("kg")));
    md->addInputWidget(new MDDoubleSpinBox("refr_rege", tr("Reclaimed:"), md, 0.0, 999999999.9, attributes.value("refr_rege").toDouble(), tr("kg")));
    md->addInputWidget(new MDDoubleSpinBox("refr_disp", tr("Disposed of:"), md, 0.0, 999999999.9, attributes.value("refr_disp").toDouble(), tr("kg")));
    md->addInputWidget(new MDDoubleSpinBox("leaked", tr("Leaked (new):"), md, 0.0, 999999999.9, attributes.value("leaked").toDouble(), tr("kg")));
    md->addInputWidget(new MDDoubleSpinBox("leaked_reco", tr("Leaked (recovered):"), md, 0.0, 999999999.9, attributes.value("leaked_reco").toDouble(), tr("kg")));
    QStringList used_ids; QSqlQuery query_used_ids;
    query_used_ids.setForwardOnly(true);
    query_used_ids.prepare("SELECT date FROM refrigerant_management" + QString(id().isEmpty() ? "" : " WHERE date <> :date"));
    if (!id().isEmpty()) { query_used_ids.bindValue(":date", id()); }
    if (query_used_ids.exec()) {
        while (query_used_ids.next()) {
            used_ids << query_used_ids.value(0).toString();
        }
    }
    md->setUsedIds(used_ids);
}

WarningRecord::WarningRecord(const QString & id):
DBRecord("warnings", "id", id, MTDictionary())
{}

void WarningRecord::initModifyDialogue(ModifyDialogue * md)
{
    md->setWindowTitle(tr("Warning"));
    StringVariantMap attributes; bool enable_all = true;
    if (!id().isEmpty()) {
        Warning query(id().toInt());
        if (query.next()) {
            for (int i = 0; i < query.record().count(); ++i) {
                attributes.insert(query.record().fieldName(i), query.value(query.record().fieldName(i)));
            }
        }
        if (!attributes.value("name").toString().isEmpty()) {
            md->setWindowTitle(tr("%1: %2").arg(tr("Warning")).arg(attributes.value("name").toString()));
        }
        enable_all = id().toInt() < 1000;
    }
    md->addInputWidget(new MDCheckBox("enabled", tr("Enabled"), md, attributes.value("enabled").toInt()));
    md->addInputWidget(new MDLineEdit("name", tr("Name:"), md, attributes.value("name").toString(), "", "", enable_all));
    md->addInputWidget(new MDLineEdit("description", tr("Description:"), md, attributes.value("description").toString(), "", "", enable_all));
    md->addInputWidget(new MDSpinBox("delay", tr("Delay:"), md, 0, 999999, attributes.value("delay").toInt(), tr("days"), "", enable_all));
    QStringList used_ids;
    used_ids << "refrigerant_amount" << "oil_amount" << "sum" << "p_to_t";
    used_ids << listSupportedFunctions();
    used_ids << listVariableIds();
    md->setUsedIds(used_ids);
}
