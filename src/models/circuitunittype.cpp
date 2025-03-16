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

#include "circuitunittype.h"

#include "inputwidgets.h"
#include "editdialoguewidgets.h"
#include "global.h"

#include <QApplication>

using namespace Global;

CircuitUnitType::CircuitUnitType(const QString &uuid):
    DBRecord(tableName(), uuid)
{}

void CircuitUnitType::initEditDialogue(EditDialogueWidgets *md)
{
    QString currency = DBInfo::valueForKey("currency", "EUR");

    md->setWindowTitle(tr("Circuit Unit Type"));
    MTDictionary refrigerants(listRefrigerants());
    MTDictionary locations;
    locations.insert(QString::number(CircuitUnitType::External), tr("External"));
    locations.insert(QString::number(CircuitUnitType::Internal), tr("Internal"));
    MTDictionary output_units;
    output_units.insert("kW", "kW");
    output_units.insert("m3", "m3");

    md->addInputWidget(new MDLineEdit("manufacturer", tr("Manufacturer:"), md->widget(), manufacturer()));
    md->addInputWidget(new MDLineEdit("type", tr("Type:"), md->widget(), type()));
    md->addInputWidget(new MDComboBox("refrigerant", tr("Refrigerant:"), md->widget(), refrigerant(), refrigerants));
    md->addInputWidget(new MDDoubleSpinBox("refrigerant_amount", tr("Refrigerant amount:"), md->widget(), 0.0, 999999.9, refrigerantAmount(), QApplication::translate("Units", "kg")));
    md->addInputWidget(new MDComboBox("oil", tr("Oil:"), md->widget(), stringValue("oil", "poe"), oils()));
    md->addInputWidget(new MDDoubleSpinBox("oil_amount", tr("Oil amount:"), md->widget(), 0.0, 999999.9, oilAmount(), QApplication::translate("Units", "kg")));
    md->addInputWidget(new MDDoubleSpinBox("acquisition_price", tr("Acquisition price:"), md->widget(), 0.0, 999999999.9, acquisitionPrice(), currency));
    md->addInputWidget(new MDDoubleSpinBox("list_price", tr("List price:"), md->widget(), 0.0, 999999999.9, listPrice(), currency));
    md->addInputWidget(new MDDoubleSpinBox("discount", tr("Discount:"), md->widget(), 0.0, 100.0, discount(), "%"));
    md->addInputWidget(new MDComboBox("location", tr("Location:"), md->widget(), stringValue("location", QString::number(CircuitUnitType::External)), locations));
    md->addInputWidget(new MDLineEdit("unit", tr("Unit of measure:"), md->widget(), unit()));
    QList<MDAbstractInputWidget *> gw_list;
    gw_list.append(new MDDoubleSpinBox("output", tr("Value:"), md->widget(), 0.0, 999999.9, output()));
    gw_list.append(new MDComboBox("output_unit", tr("Unit:"), md->widget(), stringValue("output_unit", "kW"), output_units));
    gw_list.append(new MDDoubleSpinBox("output_t0_tc", tr("At t0/tc:"), md->widget(), 0.0, 999999.9, outputT0Tc()));
    md->addGroupedInputWidgets(tr("Output:"), gw_list);
    md->addInputWidget(new MDPlainTextEdit("notes", tr("Notes:"), md->widget(), notes()));
}

bool CircuitUnitType::checkValues(QWidget *parent)
{
    if (refrigerant().isEmpty())
        return showErrorMessage(parent, tr("Add circuit unit type - Leaklog"), QApplication::translate("Circuit", "Select a refrigerant."));

    return true;
}

QString CircuitUnitType::tableName()
{
    return "circuit_unit_types";
}

class CircuitUnitTypeColumns
{
public:
    CircuitUnitTypeColumns() {
        columns << Column("uuid", "UUID PRIMARY KEY");
        columns << Column("manufacturer", "TEXT");
        columns << Column("type", "TEXT");
        columns << Column("refrigerant", "TEXT");
        columns << Column("refrigerant_amount", "NUMERIC");
        columns << Column("acquisition_price", "NUMERIC");
        columns << Column("list_price", "NUMERIC");
        columns << Column("location", "INTEGER");
        columns << Column("unit", "TEXT");
        columns << Column("oil", "TEXT");
        columns << Column("oil_amount", "NUMERIC");
        columns << Column("output", "NUMERIC");
        columns << Column("output_unit", "TEXT");
        columns << Column("output_t0_tc", "NUMERIC");
        columns << Column("notes", "TEXT");
        columns << Column("discount", "NUMERIC");
        columns << Column("date_updated", "TEXT");
        columns << Column("updated_by", "TEXT");
    }

    ColumnList columns;
};

const ColumnList &CircuitUnitType::columns()
{
    static CircuitUnitTypeColumns columns;
    return columns.columns;
}

class CircuitUnitTypeAttributes
{
public:
    CircuitUnitTypeAttributes() {
        dict.insert("uuid", QApplication::translate("CircuitUnitType", "ID"));
        dict.insert("manufacturer", QApplication::translate("CircuitUnitType", "Manufacturer"));
        dict.insert("type", QApplication::translate("CircuitUnitType", "Type"));
        dict.insert("refrigerant", QApplication::translate("CircuitUnitType", "Refrigerant"));
        dict.insert("refrigerant_amount", QApplication::translate("CircuitUnitType", "Refrigerant amount"));
        dict.insert("oil", QApplication::translate("CircuitUnitType", "Oil"));
        dict.insert("oil_amount", QApplication::translate("CircuitUnitType", "Oil amount"));
        dict.insert("acquisition_price", QApplication::translate("CircuitUnitType", "Acquisition price"));
        dict.insert("list_price", QApplication::translate("CircuitUnitType", "List price"));
        dict.insert("discount", QApplication::translate("CircuitUnitType", "Discount"));
        dict.insert("location", QApplication::translate("CircuitUnitType", "Location"));
        dict.insert("unit", QApplication::translate("CircuitUnitType", "Unit of measure"));
        dict.insert("output", QApplication::translate("CircuitUnitType", "Output"));
        dict.insert("output_t0_tc", QApplication::translate("CircuitUnitType", "Output at t0/tc"));
        dict.insert("notes", QApplication::translate("CircuitUnitType", "Notes"));
    }

    MTDictionary dict;
};

const MTDictionary &CircuitUnitType::attributes()
{
    static CircuitUnitTypeAttributes dict;
    return dict.dict;
}

const QString CircuitUnitType::locationToString(int id)
{
    switch (id) {
        case CircuitUnitType::External:
            return tr("External");
        case CircuitUnitType::Internal:
            return tr("Internal");
    }
    return QString();
}
