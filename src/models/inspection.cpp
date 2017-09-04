/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2017 Matus & Michal Tomlein

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
#include <QMessageBox>

using namespace Global;

QString Inspection::descriptionForInspectionType(Inspection::Type type, const QString &type_data)
{
    QStringList data = type_data.split(UNIT_SEPARATOR);

    switch (type) {
        case Inspection::CircuitMovedType: {
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

        case Inspection::InspectionSkippedType:
            return data.value(0);

        default:
            break;
    }

    return QString();
}

QString Inspection::tableName()
{
    return "inspections";
}

MTQuery Inspection::queryByInspector(const QString &inspector_uuid)
{
    return MTQuery("inspections LEFT JOIN customers ON inspections.customer_uuid = customers.uuid"
                   " LEFT JOIN circuits ON inspections.circuit_uuid = circuits.uuid",
                   {"inspector_uuid", inspector_uuid});
}

class InspectionColumns
{
public:
    InspectionColumns() {
        columns << Column("uuid", "UUID PRIMARY KEY");
        columns << Column("customer_uuid", "UUID");
        columns << Column("circuit_uuid", "UUID");
        columns << Column("date", "TEXT");
        columns << Column("nominal", "INTEGER");
        columns << Column("repair", "INTEGER");
        columns << Column("outside_interval", "INTEGER");
        columns << Column("inspection_type", "INTEGER NOT NULL DEFAULT 0");
        columns << Column("inspection_type_data", "TEXT");
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

    bool nominal_found = false;
    QStringList used_ids; MTSqlQuery query_used_ids;
    query_used_ids.setForwardOnly(true);
    query_used_ids.prepare("SELECT date, nominal FROM inspections WHERE circuit_uuid = :circuit_uuid" + QString(uuid().isEmpty() ? "" : " AND date <> :date"));
    query_used_ids.bindValue(":circuit_uuid", circuit_record.uuid());
    if (!uuid().isEmpty()) { query_used_ids.bindValue(":date", date()); }
    if (query_used_ids.exec()) {
        while (query_used_ids.next()) {
            used_ids << query_used_ids.value(0).toString();
            if (!nominal_found && query_used_ids.value(1).toInt())
                nominal_found = true;
        }
    }
    md->setUsedIds(used_ids);

    MDDateTimeEdit *date_edit = new MDDateTimeEdit("date", tr("Date:"), md->widget(), date());
    if (DBInfo::isDatabaseLocked()) {
        date_edit->setMinimumDate(QDate::fromString(DBInfo::lockDate(), DATE_FORMAT));
    }
    md->addInputWidget(date_edit);
    MTCheckBoxGroup *chbgrp_i_type = new MTCheckBoxGroup(md->widget());
    MDCheckBox *chb_nominal = new MDCheckBox("nominal", tr("Nominal inspection"), md->widget(), isNominal(), true);
    if (nominal_found)
        QObject::connect(chb_nominal, SIGNAL(toggled(MTCheckBox *, bool)), this, SLOT(showSecondNominalInspectionWarning(MTCheckBox *, bool)));

    md->addInputWidget(chb_nominal);
    chbgrp_i_type->addCheckBox((MTCheckBox *)chb_nominal->widget());
    MDCheckBox *chb_repair = new MDCheckBox("repair", tr("Repair"), md->widget(), isRepair(), true);
    md->addInputWidget(chb_repair);
    chbgrp_i_type->addCheckBox((MTCheckBox *)chb_repair->widget());
    md->addInputWidget(new MDCheckBox("outside_interval", tr("Outside the inspection interval"), md->widget(), isOutsideInterval()));

    if (!uuid().isEmpty() && !compressors().exists()) {
        m_scope |= Variable::Compressor;
        md->setMaximumRowCount(14);
    }

    Variables query(QSqlDatabase(), m_scope);
    query.initEditDialogueWidgets(md, savedValues(), this, date_edit->variantValue().toDateTime(), chb_repair, chb_nominal);
}

Customer Inspection::customer()
{
    return customerUUID();
}

Circuit Inspection::circuit()
{
    return circuitUUID();
}

MTRecordQuery<InspectionCompressor> Inspection::compressors() const
{
    return InspectionCompressor::query({"inspection_uuid", uuid()});
}

MTRecordQuery<InspectionFile> Inspection::files() const
{
    return InspectionFile::query({"inspection_uuid", uuid()});
}

bool Inspection::remove() const
{
    compressors().removeAll();
    files().removeAll();
    return MTRecord::remove();
}

void Inspection::showSecondNominalInspectionWarning(MTCheckBox *checkbox, bool state)
{
    if (state) {
        QMessageBox message(checkbox->parentWidget());
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
                QMetaObject::invokeMethod(checkbox, "toggle", Qt::QueuedConnection);
                break;
        }
    }
}
