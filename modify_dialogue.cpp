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

void ModifyDialogue::init(const MTRecord & record, const QStringList & used_ids)
{
    md_record = record;
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

ModifyDialogue::ModifyDialogue(const MTRecord & record, const QStringList & used_ids, QWidget * parent):
QDialog(parent)
{
    init(record, used_ids);
}

ModifyDialogue::ModifyDialogue(const MTRecord & record, QWidget * parent):
QDialog(parent)
{
    init(record, QStringList());
    bool md_nominal_allowed = true;
    QSqlQuery query_used_ids;
    query_used_ids.setForwardOnly(true);
    int _i = 1; QStringList types;
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
    fields << tr("Other") + "||other";
    if (md_record.type() == "customer") {
        md_dict.insert("customer", tr("Customer")); // _i = 1;
        md_dict.insert("id", tr("ID"));
        md_dict_input.insert("id", "le;00000000");
        md_dict.insert("company", tr("Company"));
        md_dict_input.insert("company", "le");
        md_dict.insert("contact_person", tr("Contact person"));
        md_dict_input.insert("contact_person", "le");
        md_dict.insert("address", tr("Address"));
        md_dict_input.insert("address", "pte");
        md_dict.insert("mail", tr("E-mail"));
        md_dict_input.insert("mail", "le");
        md_dict.insert("phone", tr("Phone"));
        md_dict_input.insert("phone", "le");
        query_used_ids.prepare("SELECT id FROM customers" + QString(md_record.id().isEmpty() ? "" : " WHERE id <> :id"));
        if (!md_record.id().isEmpty()) { query_used_ids.bindValue(":id", md_record.id()); }
    } else if (md_record.type() == "circuit") {
        md_dict.insert("circuit", tr("Cooling circuit")); // _i = 1;
        md_dict.insert("id", tr("ID"));
        md_dict_input.insert("id", "le;0000");
        md_dict.insert("disused", tr("Disused"));
        md_dict_input.insert("disused", "chb");
        md_dict.insert("operation", tr("Place of operation"));
        md_dict_input.insert("operation", "le");
        md_dict.insert("building", tr("Building"));
        md_dict_input.insert("building", "le");
        md_dict.insert("device", tr("Device"));
        md_dict_input.insert("device", "le");
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
        md_dict_input.insert("field", QString("cb;%1").arg(fields.join(";")));
        md_dict.insert("refrigerant", tr("Refrigerant"));
        md_dict_input.insert("refrigerant", "cb;R11;R12;R22;R32;R123;R124;R125;R134a;R143a;R227ea;R365mfc;R404A;R407C;R410A;R502;R507");
        md_dict.insert("refrigerant_amount", tr("Amount of refrigerant"));
        md_dict_input.insert("refrigerant_amount", QString("dspb;0.0;0.0;999999.9; %1").arg(tr("kg")));
        md_dict.insert("oil", tr("Oil"));
        QStringList oils;
        oils << tr("MO (Mineral oil)") + "||mo";
        oils << tr("AB (Alkylbenzene oil)") + "||ab";
        oils << tr("POE (Polyolester oil)") + "||poe";
        oils << tr("PAO (Polyalphaolefin oil)") + "||pao";
        oils << tr("PVE (Polyvinylether oil)") + "||pve";
        oils << tr("PAG (Polyglycol oil)") + "||pag";
        md_dict_input.insert("oil", QString("cb;%1").arg(oils.join(";")));
        md_dict.insert("oil_amount", tr("Amount of oil"));
        md_dict_input.insert("oil_amount", QString("dspb;0.0;0.0;999999.9; %1").arg(tr("kg")));
        md_dict.insert("leak_detector", tr("Fixed leakage detector installed"));
        md_dict_input.insert("leak_detector", "chb");
        md_dict.insert("life", tr("Service life"));
        md_dict_input.insert("life", QString("dspb;0.0;0.0;999999.9; %1").arg(tr("years")));
        md_dict.insert("runtime", tr("Run-time per day"));
        md_dict_input.insert("runtime", QString("dspb;0.0;0.0;24.0; %1").arg(tr("hours")));
        md_dict.insert("utilisation", tr("Rate of utilisation"));
        md_dict_input.insert("utilisation", QString("dspb;0.0;0.0;100.0; %1").arg(tr("%")));
        md_dict.insert("inspection_interval", tr("Inspection interval"));
        md_dict_input.insert("inspection_interval", QString("spb;0;0;999999; %1").arg(tr("days")));
        query_used_ids.prepare("SELECT id FROM circuits WHERE parent = :parent" + QString(md_record.id().isEmpty() ? "" : " AND id <> :id"));
        query_used_ids.bindValue(":parent", md_record.parents()->value("parent"));
        if (!md_record.id().isEmpty()) { query_used_ids.bindValue(":id", md_record.id()); }
    } else if (md_record.type() == "inspection" || (md_record.type() == "repair" && !md_record.parents()->isEmpty())) {
        bool is_inspection = md_record.type() == "inspection";
        if (is_inspection) {
            md_dict.insert("inspection", tr("Inspection")); // _i = 1;
        } else {
            md_record.setType("inspection"); // REPAIR -> INSPECTION
            md_dict.insert("inspection", tr("Repair")); // _i = 1;
        }
        md_dict.insert("date", tr("Date"));
        md_dict_input.insert("date", "dte");
        if (is_inspection) {
            md_dict.insert("nominal", tr("Nominal"));
            md_dict_input.insert("nominal", "chb");
        }
        Variables query;
        while (query.next()) {
            if (query.value("SUBVAR_ID").toString().isEmpty()) {
                if (!query.value("VAR_VALUE").toString().isEmpty()) { continue; }
                md_dict.insert(query.value("VAR_ID").toString(), query.value("VAR_NAME").toString());
                if (query.value("VAR_ID").toString() == "inspector") {
                    QString inspectors_string;
                    QSqlQuery inspectors;
                    inspectors.exec("SELECT id, person FROM inspectors");
                    if (inspectors.next()) {
                        while (true) {
                            inspectors_string.append(inspectors.value(1).toString().isEmpty() ? inspectors.value(0).toString() : inspectors.value(1).toString());
                            inspectors_string.append("||" + inspectors.value(0).toString());
                            if (inspectors.next()) { inspectors_string.append(";"); } else { break; }
                        }
                    }
                    md_dict_input.insert(query.value("VAR_ID").toString(), QString("cb;%1").arg(inspectors_string));
                } else if (query.value("VAR_TYPE").toString() == "int") {
                    md_dict_input.insert(query.value("VAR_ID").toString(), QString("spb;-999999999;0.0;999999999; %1").arg(query.value("VAR_UNIT").toString()));
                } else if (query.value("VAR_TYPE").toString() == "float") {
                    md_dict_input.insert(query.value("VAR_ID").toString(), QString("dspb;-999999999.9;0.0;999999999.9; %1").arg(query.value("VAR_UNIT").toString()));
                } else if (query.value("VAR_TYPE").toString() == "string") {
                    md_dict_input.insert(query.value("VAR_ID").toString(), "le");
                } else if (query.value("VAR_TYPE").toString() == "bool") {
                    md_dict_input.insert(query.value("VAR_ID").toString(), QString("cb;%1||1;%2||0").arg(tr("Yes")).arg(tr("No")));
                }
            } else {
                if (!query.value("SUBVAR_VALUE").toString().isEmpty()) { continue; }
                md_dict.insert(QString("%1/%2").arg(query.value("VAR_ID").toString()).arg(query.value("SUBVAR_ID").toString()), tr("%1: %2").arg(query.value("VAR_NAME").toString()).arg(query.value("SUBVAR_NAME").toString()));
                if (query.value("SUBVAR_TYPE").toString() == "int") {
                    md_dict_input.insert(QString("%1/%2").arg(query.value("VAR_ID").toString()).arg(query.value("SUBVAR_ID").toString()), QString("spb;-999999999;0.0;999999999; %1").arg(query.value("SUBVAR_UNIT").toString()));
                } else if (query.value("SUBVAR_TYPE").toString() == "float") {
                    md_dict_input.insert(QString("%1/%2").arg(query.value("VAR_ID").toString()).arg(query.value("SUBVAR_ID").toString()), QString("dspb;-999999999.9;0.0;999999999.9; %1").arg(query.value("SUBVAR_UNIT").toString()));
                } else if (query.value("SUBVAR_TYPE").toString() == "string") {
                    md_dict_input.insert(QString("%1/%2").arg(query.value("VAR_ID").toString()).arg(query.value("SUBVAR_ID").toString()), "le");
                } else if (query.value("SUBVAR_TYPE").toString() == "bool") {
                    md_dict_input.insert(QString("%1/%2").arg(query.value("VAR_ID").toString()).arg(query.value("SUBVAR_ID").toString()), QString("cb;%1||1;%2||0").arg(tr("Yes")).arg(tr("No")));
                }
            }
        }
        query_used_ids.prepare("SELECT date, nominal FROM inspections WHERE customer = :customer AND circuit = :circuit" + QString(md_record.id().isEmpty() ? "" : " AND date <> :date"));
        query_used_ids.bindValue(":customer", md_record.parents()->value("customer"));
        query_used_ids.bindValue(":circuit", md_record.parents()->value("circuit"));
        if (!md_record.id().isEmpty()) { query_used_ids.bindValue(":date", md_record.id()); }
    } else if (md_record.type() == "repair" && md_record.parents()->isEmpty()) {
        md_dict.insert("repair", tr("Repair")); // _i = 1;
        md_dict.insert("date", tr("Date"));
        md_dict_input.insert("date", "dte");
        md_dict.insert("customer", tr("Customer"));
        md_dict_input.insert("customer", "le");
        md_dict.insert("field", tr("Field of application"));
        md_dict_input.insert("field", QString("cb;%1").arg(fields.join(";")));
        md_dict.insert("repairman", tr("Repairman"));
        md_dict_input.insert("repairman", "le");
        md_dict.insert("arno", tr("Assembly record No."));
        md_dict_input.insert("arno", "le");
        md_dict.insert("refrigerant_amount", tr("Amount of refrigerant"));
        md_dict_input.insert("refrigerant_amount", QString("dspb;0.0;0.0;999999.9; %1").arg(tr("kg")));
        md_dict.insert("refr_add_am", tr("Refrigerant addition"));
        md_dict_input.insert("refr_add_am", QString("dspb;-999999999.9;0.0;999999999.9; %1").arg(tr("kg")));
        md_dict.insert("refr_reco", tr("Refrigerant recovery"));
        md_dict_input.insert("refr_reco", QString("dspb;-999999999.9;0.0;999999999.9; %1").arg(tr("kg")));
        md_dict.insert("refr_recy", tr("Refrigerant recycling"));
        md_dict_input.insert("refr_recy", QString("dspb;-999999999.9;0.0;999999999.9; %1").arg(tr("kg")));
        md_dict.insert("refr_disp", tr("Refrigerant disposal"));
        md_dict_input.insert("refr_disp", QString("dspb;-999999999.9;0.0;999999999.9; %1").arg(tr("kg")));
        query_used_ids.prepare("SELECT date FROM repairs WHERE" + QString(md_record.id().isEmpty() ? "" : " date <> :date"));
        if (!md_record.id().isEmpty()) { query_used_ids.bindValue(":date", md_record.id()); }
    } else if (md_record.type() == "variable" || md_record.type() == "subvariable") {
        md_used_ids << "refrigerant_amount" << "oil_amount" << "sum" << "p_to_t";
        md_dict.insert("variable", tr("Variable"));
        md_dict.insert("subvariable", tr("Subvariable")); _i = 2;
        md_dict.insert("id", tr("ID"));
        md_dict_input.insert("id", "le");
        md_dict.insert("name", tr("Name"));
        md_dict_input.insert("name", "le");
        md_dict.insert("unit", tr("Unit"));
        md_dict_input.insert("unit", "le");
        md_dict.insert("type", tr("Type"));
        types << tr("Integer") + "||int";
        types << tr("Real number") + "||float";
        types << tr("String") + "||string";
        types << tr("Boolean") + "||bool";
        md_dict_input.insert("type", QString("cb;%1").arg(types.join(";")));
        md_dict.insert("value", tr("Value"));
        md_dict_input.insert("value", "pteh");
        md_dict.insert("compare_nom", tr("Compare value with the nominal one"));
        md_dict_input.insert("compare_nom", "chb");
        md_dict.insert("tolerance", tr("Tolerance"));
        md_dict_input.insert("tolerance", "dspb;0.0;0.0;999999.9");
        if (md_record.type() == "variable") {
            md_dict.insert("col_bg", tr("Colour"));
            md_dict_input.insert("col_bg", "ccb");
        }
        md_used_ids << listVariableIds(true);
        if (!md_record.id().isEmpty()) { md_used_ids.removeAll(md_record.id()); }
    } else if (md_record.type() == "table") {
        md_dict.insert("table", tr("Table")); // _i = 1;
        md_dict.insert("id", tr("ID"));
        md_dict_input.insert("id", "le");
        md_dict.insert("highlight_nominal", tr("Highlight the nominal inspection"));
        md_dict_input.insert("highlight_nominal", "chb");
        query_used_ids.prepare("SELECT id FROM tables" + QString(md_record.id().isEmpty() ? "" : " WHERE id <> :id"));
        if (!md_record.id().isEmpty()) { query_used_ids.bindValue(":id", md_record.id()); }
    } else if (md_record.type() == "inspector") {
        md_dict.insert("inspector", tr("Inspector")); // _i = 1;
        md_dict.insert("id", tr("ID"));
        md_dict_input.insert("id", "le;0000");
        md_dict.insert("person", tr("Certified person"));
        md_dict_input.insert("person", "le");
        md_dict.insert("company", tr("Certified company"));
        md_dict_input.insert("company", "le");
        md_dict.insert("person_reg_num", tr("Person registry number"));
        md_dict_input.insert("person_reg_num", "le");
        md_dict.insert("company_reg_num", tr("Company registry number"));
        md_dict_input.insert("company_reg_num", "le");
        md_dict.insert("phone", tr("Phone"));
        md_dict_input.insert("phone", "le");
        query_used_ids.prepare("SELECT id FROM inspectors" + QString(md_record.id().isEmpty() ? "" : " WHERE id <> :id"));
        if (!md_record.id().isEmpty()) { query_used_ids.bindValue(":id", md_record.id()); }
    } else if (md_record.type() == "service_company") {
        md_dict.insert("service_company", tr("Service company")); // _i = 1;
        md_dict.insert("certification_num", tr("Certification number"));
        md_dict_input.insert("certification_num", "le");
        md_dict.insert("name", tr("Name"));
        md_dict_input.insert("name", "le");
        md_dict.insert("id", tr("ID"));
        md_dict_input.insert("id", "le;00000000");
        md_dict.insert("address", tr("Address"));
        md_dict_input.insert("address", "pte");
        md_dict.insert("phone", tr("Phone"));
        md_dict_input.insert("phone", "le");
        md_dict.insert("mail", tr("E-mail"));
        md_dict_input.insert("mail", "le");
        md_dict.insert("website", tr("Website"));
        md_dict_input.insert("website", "le");
        query_used_ids.prepare("SELECT id FROM service_companies" + QString(md_record.id().isEmpty() ? "" : " WHERE id <> :id"));
        if (!md_record.id().isEmpty()) { query_used_ids.bindValue(":id", md_record.id()); }
    }
    if (md_record.type() != "variable" && md_record.type() != "subvariable" && query_used_ids.exec()) {
        bool nominal_ = md_record.type() == "inspection";
        while (query_used_ids.next()) {
            md_used_ids << query_used_ids.value(0).toString();
            if (nominal_ && query_used_ids.value(1).toInt()) { md_nominal_allowed = false; }
        }
    }
    MTDictionary md_dict_values;
    if (!md_record.id().isEmpty()) {
        if (md_record.type() == "variable") {
            Variable query(md_record.id());
            if (query.next()) {
                md_dict_values.insert("id", query.value("VAR_ID").toString());
                md_dict_values.insert("name", query.value("VAR_NAME").toString());
                md_dict_values.insert("type", query.value("VAR_TYPE").toString());
                md_dict_values.insert("unit", query.value("VAR_UNIT").toString());
                md_dict_values.insert("value", query.value("VAR_VALUE").toString());
                md_dict_values.insert("compare_nom", query.value("VAR_COMPARE_NOM").toString());
                md_dict_values.insert("tolerance", query.value("VAR_TOLERANCE").toString());
                md_dict_values.insert("col_bg", query.value("VAR_COL_BG").toString());
            }
        } else if (md_record.type() == "subvariable") {
            Subvariable query(md_record.parents()->value("parent"), md_record.id());
            if (query.next()) {
                md_dict_values.insert("id", query.value("SUBVAR_ID").toString());
                md_dict_values.insert("name", query.value("SUBVAR_NAME").toString());
                md_dict_values.insert("type", query.value("SUBVAR_TYPE").toString());
                md_dict_values.insert("unit", query.value("SUBVAR_UNIT").toString());
                md_dict_values.insert("value", query.value("SUBVAR_VALUE").toString());
                md_dict_values.insert("compare_nom", query.value("SUBVAR_COMPARE_NOM").toString());
                md_dict_values.insert("tolerance", query.value("SUBVAR_TOLERANCE").toString());
            }
        } else {
            QSqlQuery query = md_record.select();
            query.exec();
            if (query.next()) {
                for (int i = 0; i < query.record().count(); ++i) {
                    md_dict_values.insert(query.record().fieldName(i), query.value(i).toString());
                }
            }
        }
        if (md_record.type() == "variable" || md_record.type() == "subvariable") {
            if (get_dict_varnames().contains(md_record.id())) {
                md_dict_input.setValue("id", "led");
                md_dict_input.setValue("name", "led");
                md_dict_input.setValue("unit", "led");
                md_dict_input.setValue("type", QString("cbd;%1").arg(types.join(";")));
                md_dict_input.setValue("value", "ptehd");
            }
        }
    }
    // ------------
    if (!md_record.id().isEmpty()) {
        this->setWindowTitle(tr("%1: %2").arg(md_dict.value(md_record.type())).arg(md_record.id()));
    } else if (!md_record.id().isEmpty()) {
        this->setWindowTitle(tr("%1: %2").arg(md_dict.value(md_record.type())).arg(md_record.id()));
    } else {
        this->setWindowTitle(md_dict.value(md_record.type()));
    }
    QLabel * md_lbl_var = NULL; QWidget * md_w_var = NULL;
    QStringList inputtype; QString value;
    int num_cols = (md_dict.count() - _i) / 20 + 1;
    int num_rows = (md_dict.count() - _i) / num_cols + ((md_dict.count() - _i) % num_cols > 0 ? 1 : 0);
    for (int c = 0; c < num_cols; ++c) {
        for (int r = 0; r < num_rows; ++r) {
            if (_i >= md_dict.count()) { break; }
            //if (md_dict.key(_i) == md_record.type()) { _i++; r--; continue; }
            value = md_dict_values.contains(md_dict.key(_i).split("/").last()) ? md_dict_values.value(md_dict.key(_i).split("/").last()) : "";
            inputtype = md_dict_input.value(md_dict.key(_i)).split(";");
            if (inputtype.at(0) != "chb") {
                md_lbl_var = new QLabel(tr("%1:").arg(md_dict.value(_i)), this);
                md_lbl_var->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                md_grid_main->addWidget(md_lbl_var, r, 2 * c);
            }
            md_w_var = createInputWidget(inputtype, md_dict.value(_i), value);
            if (md_dict.key(_i) == "nominal") { md_w_var->setEnabled(md_nominal_allowed); }
            md_grid_main->addWidget(md_w_var, r, (2 * c) + 1);
            md_vars.insert(md_dict.key(_i), md_w_var);
            _i++;
        }
    }
}

QWidget * ModifyDialogue::createInputWidget(const QStringList & inputtype, const QString & name, const QString & value)
{
    if (inputtype.at(0) == "le" || inputtype.at(0) == "led") {
        QLineEdit * md_le_var = new QLineEdit(this);
        md_le_var->setMinimumSize(150, md_le_var->sizeHint().height());
        if (inputtype.at(0) == "led") { md_le_var->setEnabled(false); }
        if (inputtype.count() > 1) { md_le_var->setInputMask(inputtype.at(1)); }
        md_le_var->setText(value);
        return md_le_var;
    } else if (inputtype.at(0) == "chb") {
        QCheckBox * md_chb_var = new QCheckBox(name, this);
        md_chb_var->setChecked(value.toInt());
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
    } else if (inputtype.at(0) == "cb" || inputtype.at(0) == "cbd") {
        QComboBox * md_cb_var = new QComboBox(this); int n = -1;
        if (inputtype.at(0) == "cbd") { md_cb_var->setEnabled(false); }
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
        md_dte_var->setDisplayFormat("yyyy.MM.dd-hh:mm");
        md_dte_var->setDateTime(value.isEmpty() ? QDateTime::currentDateTime() : QDateTime::fromString(value, "yyyy.MM.dd-hh:mm"));
        return md_dte_var;
    } else if (inputtype.at(0) == "de") {
        QDateEdit * md_de_var = new QDateEdit(this);
        md_de_var->setDisplayFormat("yyyy.MM.dd");
        md_de_var->setDate(value.isEmpty() ? QDate::currentDate() : QDate::fromString(value, "yyyy.MM.dd"));
        return md_de_var;
    } else if (inputtype.at(0) == "pteh" || inputtype.at(0) == "ptehd") {
        QPlainTextEdit * md_pte_var = new QPlainTextEdit(this);
        if (inputtype.at(0) == "ptehd") { md_pte_var->setEnabled(false); }
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

QVariant ModifyDialogue::getInputFromWidget(QWidget * input_widget, const QStringList & inputtype, const QString & key)
{
    QVariant value(QVariant::String);
    if (inputtype.at(0) == "chb") {
        value = ((QCheckBox *)input_widget)->isChecked() ? 1 : 0;
    } else if (inputtype.at(0) == "le" || inputtype.at(0) == "led") {
        value = ((QLineEdit *)input_widget)->text();
        if (key == "id") {
            if (value.toString().isEmpty()) {
                QMessageBox::information(this, tr("Save changes"), tr("Invalid ID."));
                return QVariant(QVariant::String);
            }
            if (md_used_ids.contains(value.toString())) {
                QMessageBox::information(this, tr("Save changes"), tr("This ID is not available. Please choose a different ID."));
                return QVariant(QVariant::String);
            }
        }
    } else if (inputtype.at(0) == "spb") {
        value = ((QSpinBox *)input_widget)->value();
    } else if (inputtype.at(0) == "dspb") {
        value = ((QDoubleSpinBox *)input_widget)->value();
    } else if (inputtype.at(0) == "cb" || inputtype.at(0) == "cbd") {
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
            if (md_used_ids.contains(value.toString())) {
                QMessageBox::information(this, tr("Save changes"), tr("This date is not available. Please choose a different date."));
                return QVariant(QVariant::String);
            }
        }
    } else if (inputtype.at(0) == "de") {
        value = ((QDateEdit *)input_widget)->date().toString("yyyy.MM.dd");
    } else {
        value = ((QPlainTextEdit *)input_widget)->toPlainText();
    }
    return value.isNull() ? QVariant(QString("")) : value;
}

void ModifyDialogue::save()
{
    QStringList inputtype;
    QMapIterator<QString, QWidget *> i(md_vars);
    while (i.hasNext()) { i.next();
        inputtype = md_dict_input.value(i.key()).split(";");
        QVariant value = getInputFromWidget(i.value(), inputtype, i.key());
        if (value.isNull()) { return; }
        md_values.insert(i.key().split("/").last(), value);
    }
    md_record.update(md_values, true);
    accept();
}
