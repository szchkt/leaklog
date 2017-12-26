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
#include "variables.h"

#include <QApplication>
#include <QMessageBox>

using namespace Global;

QString Inspection::titleForInspection(bool nominal, Repair repair)
{
    if (nominal)
        return QApplication::translate("MainWindow", "%1:").arg(tr("Nominal Inspection"));

    switch (repair) {
        case Inspection::IsNotRepair:
            break;
        case Inspection::IsRepair:
            return QApplication::translate("MainWindow", "%1:").arg(tr("Repair"));
        case Inspection::IsAfterRepair:
            return QApplication::translate("MainWindow", "%1:").arg(tr("Inspection After Repair"));
    }

    return QApplication::translate("MainWindow", "%1:").arg(tr("Regular Inspection"));
}

QString Inspection::descriptionForInspectionType(Inspection::Type type, const QString &type_data)
{
    QStringList data = type_data.split(UNIT_SEPARATOR);

    switch (type) {
        case Inspection::CircuitMovedType: {
            int from_id = data.value(0).toInt();
            QString from = from_id ? Customer(QString::number(from_id)).stringValue("company") : QString();
            if (from.isEmpty())
                from = data.value(2);

            from.append(QString(" (%1)").arg(formatCompanyID(from_id)));

            int to_id = data.value(1).toInt();
            QString to = to_id ? Customer(QString::number(to_id)).stringValue("company") : QString();
            if (to.isEmpty())
                to = data.value(3);

            to.append(QString(" (%1)").arg(formatCompanyID(to_id)));

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

class InspectionColumns
{
public:
    InspectionColumns() {
        columns << Column("customer", "INTEGER");
        columns << Column("circuit", "INTEGER");
        columns << Column("date", "TEXT");
        columns << Column("nominal", "INTEGER");
        columns << Column("repair", "INTEGER");
        columns << Column("outside_interval", "INTEGER");
        columns << Column("inspection_type", "INTEGER DEFAULT 0 NOT NULL");
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

Inspection::Inspection():
    DBRecord(tableName(), "date", "", MTDictionary()),
    m_scope(Variable::Inspection)
{}

Inspection::Inspection(const QString &customer, const QString &circuit, const QString &date):
    DBRecord(tableName(), "date", date, MTDictionary(QStringList() << "customer" << "circuit", QStringList() << customer << circuit)),
    m_scope(Variable::Inspection)
{}

Inspection::Inspection(const QString &table, const QString &id_column, const QString &id, const MTDictionary &parents):
    DBRecord(table, id_column, id, parents),
    m_scope(Variable::Inspection)
{}

void Inspection::initEditDialogue(EditDialogueWidgets *md)
{
    QString customer = Customer(parent("customer")).stringValue("company");
    if (customer.isEmpty())
        customer = formatCompanyID(parent("customer"));
    QString circuit = Circuit(parent("customer"), parent("circuit")).stringValue("name");
    if (circuit.isEmpty())
        circuit = parent("circuit").rightJustified(5, '0');
    md->setWindowTitle(tr("Customer: %2 %1 Circuit: %3 %1 Inspection").arg(rightTriangle()).arg(customer).arg(circuit));
    md->setMaximumRowCount(10);
    QVariantMap attributes;
    if (!id().isEmpty() || !values().isEmpty()) {
        attributes = list();
    }
    bool nominal_found = false;
    QStringList used_ids; MTSqlQuery query_used_ids;
    query_used_ids.setForwardOnly(true);
    query_used_ids.prepare("SELECT date, nominal FROM inspections WHERE customer = :customer AND circuit = :circuit" + QString(id().isEmpty() ? "" : " AND date <> :date"));
    query_used_ids.bindValue(":customer", parent("customer"));
    query_used_ids.bindValue(":circuit", parent("circuit"));
    if (!id().isEmpty()) { query_used_ids.bindValue(":date", id()); }
    if (query_used_ids.exec()) {
        while (query_used_ids.next()) {
            used_ids << query_used_ids.value(0).toString();
            if (!nominal_found && query_used_ids.value(1).toInt())
                nominal_found = true;
        }
    }
    md->setUsedIds(used_ids);
    MDDateTimeEdit *date = new MDDateTimeEdit("date", tr("Date:"), md->widget(), id());
    if (DBInfo::isDatabaseLocked()) {
        date->setMinimumDate(QDate::fromString(DBInfo::lockDate(), DATE_FORMAT));
    }
    md->addInputWidget(date);

    QString type = attributes.value("nominal").toInt() || (id().isEmpty() && used_ids.isEmpty()) ? "-1" : QString::number(attributes.value("repair").toInt());
    MTDictionary types;
    types.insert("-1", tr("Nominal Inspection"));
    types.insert("0", tr("Regular Inspection"));
    types.insert("1", tr("Repair"));
    types.insert("2", tr("Inspection After Repair"));

    MDComboBox *cb_repair = new MDComboBox("repair", tr("Type:"), md->widget(), type, types);
    cb_repair->setSkipSave(true);
    if (nominal_found)
        QObject::connect(cb_repair, SIGNAL(currentIndexChanged(MDComboBox *, int)), this, SLOT(showSecondNominalInspectionWarning(MDComboBox *, int)));
    md->addInputWidget(cb_repair);

    md->addInputWidget(new MDCheckBox("outside_interval", tr("Outside the inspection interval"), md->widget(), attributes.value("outside_interval").toInt()));

    if (!id().isEmpty()) {
        MTDictionary parents("customer_id", parent("customer"));
        parents.insert("circuit_id", parent("circuit"));
        parents.insert("date", id());
        if (!InspectionsCompressor(QString(), parents).exists()) {
            m_scope |= Variable::Compressor;
            md->setMaximumRowCount(14);
        }
    }

    Variables query(QSqlDatabase(), m_scope);
    query.initEditDialogueWidgets(md, attributes, this, date->variantValue().toDateTime(), cb_repair);
}

bool Inspection::checkValues(QVariantMap &values, QWidget *parentWidget)
{
    EditDialogueWidgets *dialogue = dynamic_cast<EditDialogueWidgets *>(parentWidget);
    if (dialogue) {
        MDAbstractInputWidget *cb_repair = dialogue->inputWidget("repair");
        int value = cb_repair->variantValue().toInt();
        if (value < 0) {
            values.insert("nominal", 1);
            values.insert("repair", 0);
        } else {
            values.insert("nominal", 0);
            values.insert("repair", value);
        }
    }
    return true;
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

InspectionByInspector::InspectionByInspector(const QString &inspector_id):
    Inspection("inspections LEFT JOIN customers ON inspections.customer = customers.id"
               " LEFT JOIN circuits ON inspections.circuit = circuits.id"
               " AND circuits.parent = inspections.customer",
               "date", "", MTDictionary("inspector", inspector_id))
{}
