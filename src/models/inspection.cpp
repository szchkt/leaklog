/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2018 Matus & Michal Tomlein

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

#include "inspection.h"

#include "inputwidgets.h"
#include "editdialoguewidgets.h"
#include "global.h"
#include "customer.h"
#include "circuit.h"
#include "inspectionfile.h"
#include "variables.h"

#include <QApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>

using namespace Global;

QString Inspection::titleForInspectionType(Type type)
{
    switch (type) {
        case RegularInspection:
            return tr("Regular Inspection");
        case NominalInspection:
            return tr("Nominal Inspection");
        case Repair:
            return tr("Repair");
        case InspectionAfterRepair:
            return tr("Inspection After Repair");
        case StrengthTightnessTest:
            return tr("Strength and Tightness Test");
        case VacuumTest:
            return tr("Vacuum Test");
        case CircuitMoved:
            return tr("Circuit Moved");
        case SkippedInspection:
            return tr("Inspection Skipped");
    }

    return tr("Unknown Inspection Type");
}

bool Inspection::showDescriptionForInspectionType(Inspection::Type type)
{
    return type < 0 || type >= StrengthTightnessTest;
}

QString Inspection::descriptionForInspectionType(Inspection::Type type, const QString &type_data)
{
    QStringList data = type_data.split(UNIT_SEPARATOR);

    switch (type) {
        case CircuitMoved: {
            QString from_id = data.value(0);
            QVariantMap from_values = from_id.isEmpty() ? QVariantMap() : Customer(from_id).list("id, company");
            QString from = from_values.value("company").toString();
            if (from.isEmpty())
                from = data.value(2);

            if (!from_values.value("id").toString().isEmpty())
                from.append(QString(" (%1)").arg(from_values.value("id").toString()));

            QString to_id = data.value(1);
            QVariantMap to_values = to_id.isEmpty() ? QVariantMap() : Customer(to_id).list("id, company");
            QString to = to_values.value("company").toString();
            if (to.isEmpty())
                to = data.value(3);

            if (!to_values.value("id").toString().isEmpty())
                to.append(QString(" (%1)").arg(to_values.value("id").toString()));

            return tr("Circuit moved from customer %1 to %2.").arg(from).arg(to);
        }

        case SkippedInspection:
            return data.value(0);

        default:
            break;
    }

    return titleForInspectionType(type);
}

QString Inspection::tableName()
{
    return "inspections";
}

MTQuery Inspection::queryByInspector(const QString &inspector_uuid)
{
    return MTQuery("inspections LEFT JOIN customers ON inspections.customer_uuid = customers.uuid"
                   " LEFT JOIN circuits ON inspections.circuit_uuid = circuits.uuid",
                   {{"inspector_uuid", inspector_uuid}});
}

class InspectionColumns
{
public:
    InspectionColumns() {
        columns << Column("uuid", "UUID PRIMARY KEY");
        columns << Column("customer_uuid", "UUID");
        columns << Column("circuit_uuid", "UUID");
        columns << Column("date", "TEXT");
        columns << Column("inspection_type", "INTEGER NOT NULL DEFAULT 0");
        columns << Column("inspection_type_data", "TEXT");
        columns << Column("outside_interval", "INTEGER");
        columns << Column("inspection_data", "TEXT");
        columns << Column("date_updated", "TEXT");
        columns << Column("updated_by", "TEXT");
    }

    ColumnList columns;
};

const ColumnList &Inspection::columns()
{
    static InspectionColumns columns;
    return columns.columns;
}

Inspection::Inspection(const QString &uuid, const QVariantMap &savedValues):
    DBRecord(tableName(), uuid, savedValues),
    m_scope(Variable::Inspection)
{}

void Inspection::initEditDialogue(EditDialogueWidgets *md)
{
    Customer customer_record(customer());
    Circuit circuit_record(circuit());

    md->setWindowTitle(tr("Customer: %2 %1 Circuit: %3 %1 Inspection").arg(rightTriangle())
                       .arg(customer_record.companyName().isEmpty() ? customer_record.companyID() : customer_record.companyName())
                       .arg(circuit_record.circuitName().isEmpty() ? circuit_record.circuitID() : circuit_record.circuitName()));
    md->setMaximumRowCount(10);

    bool has_inspection = false;
    bool nominal_found = false;
    MTSqlQuery inspections_query;
    inspections_query.prepare("SELECT inspection_type FROM inspections WHERE circuit_uuid = :circuit_uuid" + QString(uuid().isEmpty() ? "" : " AND uuid <> :uuid"));
    inspections_query.bindValue(":circuit_uuid", circuit_record.uuid());
    if (!uuid().isEmpty())
        inspections_query.bindValue(":uuid", uuid());
    if (inspections_query.exec()) {
        while (inspections_query.next()) {
            has_inspection = true;
            if (inspections_query.value(0).toInt() == Inspection::NominalInspection) {
                nominal_found = true;
                break;
            }
        }
    }

    MDDateTimeEdit *date_edit = new MDDateTimeEdit("date", tr("Date:"), md->widget(), date());
    if (DBInfo::isDatabaseLocked()) {
        date_edit->setMinimumDate(QDate::fromString(DBInfo::lockDate(), DATE_FORMAT));
    }
    md->addInputWidget(date_edit);

    QString inspection_type = isNominal() || (uuid().isEmpty() && !has_inspection) ? QString::number(NominalInspection) : QString::number(type());
    MTDictionary types;
    types.insert(QString::number(NominalInspection), tr("Nominal Inspection"));
    types.insert(QString::number(RegularInspection), tr("Regular Inspection"));
    types.insert(QString::number(Repair), tr("Repair"));
    types.insert(QString::number(InspectionAfterRepair), tr("Inspection After Repair"));

    MDComboBox *cb_repair = new MDComboBox("inspection_type", tr("Type:"), md->widget(), inspection_type, types);
    if (nominal_found)
        QObject::connect(cb_repair, SIGNAL(currentIndexChanged(MDComboBox *, int)), this, SLOT(showSecondNominalInspectionWarning(MDComboBox *, int)));
    md->addInputWidget(cb_repair);

    md->addInputWidget(new MDCheckBox("outside_interval", tr("Outside the inspection interval"), md->widget(), isOutsideInterval()));

    if (!uuid().isEmpty() && !compressors().exists()) {
        m_scope |= Variable::Compressor;
        md->setMaximumRowCount(14);
    }

    Variables query(QSqlDatabase(), m_scope);
    query.initEditDialogueWidgets(md, savedValues(), this, date_edit->variantValue().toDateTime(), cb_repair);
}

Customer Inspection::customer()
{
    return customerUUID();
}

Circuit Inspection::circuit()
{
    return circuitUUID();
}

QJsonObject Inspection::data()
{
    return QJsonDocument::fromJson(stringValue("inspection_data").toUtf8()).object();
}

void Inspection::setData(const QJsonObject &value)
{
    setValue("inspection_data", QJsonDocument(value).toJson(QJsonDocument::Compact));
}

MTRecordQuery<InspectionCompressor> Inspection::compressors() const
{
    return InspectionCompressor::query({{"inspection_uuid", uuid()}});
}

MTRecordQuery<InspectionFile> Inspection::files() const
{
    return InspectionFile::query({{"inspection_uuid", uuid()}});
}

bool Inspection::remove() const
{
    compressors().removeAll();
    files().removeAll();
    return MTRecord::remove();
}

void Inspection::showSecondNominalInspectionWarning(MDComboBox *combobox, int index)
{
    if (index == 0) {
        QMessageBox message(combobox->parentWidget());
        message.setWindowTitle(tr("Nominal inspection already exists - Leaklog"));
        message.setWindowModality(Qt::WindowModal);
        message.setWindowFlags(message.windowFlags() | Qt::Sheet);
        message.setIcon(QMessageBox::Information);
        message.setText(tr("This circuit already has a nominal inspection."));
        message.setInformativeText(tr("Are you sure you want to add another?"));
        message.addButton(tr("&Add Another Nominal Inspection"), QMessageBox::AcceptRole);
        message.setDefaultButton(message.addButton(tr("Cancel"), QMessageBox::RejectRole));
        switch (message.exec()) {
            case 0: // Add
                break;
            case 1: // Cancel
                QMetaObject::invokeMethod(combobox, "setCurrentIndex", Qt::QueuedConnection, Q_ARG(int, 1));
                break;
        }
    }
}
