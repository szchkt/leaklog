/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2012 Matus & Michal Tomlein

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

#include "edit_customer_dialogue.h"

#include "records.h"
#include "edit_dialogue_table.h"
#include "global.h"

#include <QRadioButton>
#include <QButtonGroup>
#include <QApplication>

OperatorInputWidget::OperatorInputWidget(const QVariantMap & attributes, QWidget * parent):
    MDGroupedInputWidgets(QApplication::translate("Customer", "Operator:"), parent)
{
    setSkipSave(false);
    setId("operator_id");

    operator_choice = new QButtonGroup(this);

    QRadioButton * customer = new QRadioButton(QApplication::translate("Customer", "Customer"), this);
    operator_choice->addButton(customer, 0);
    grid->addWidget(customer, grid->rowCount(), 0, 1, 2);
    QRadioButton * service_company = new QRadioButton(QApplication::translate("Customer", "Service company"), this);
    operator_choice->addButton(service_company, 1);
    grid->addWidget(service_company, grid->rowCount(), 0, 1, 2);
    QRadioButton * other = new QRadioButton(QApplication::translate("Customer", "Other"), this);
    operator_choice->addButton(other, 2);
    grid->addWidget(other, grid->rowCount(), 0, 1, 2);

    QObject::connect(operator_choice, SIGNAL(buttonClicked(int)), this, SLOT(operatorChoiceChanged(int)));

    operator_id = new MDLineEdit("operator_id", QApplication::translate("Customer", "ID:"), this, QString(), 99999999);
    operator_id->setShowInForm(false);
    operator_id->setVisible(false);
    addWidget(operator_id);

    input_widgets << new MDLineEdit("operator_company", QApplication::translate("Customer", "Company:"), this, attributes.value("operator_company").toString());
    input_widgets << new MDAddressEdit("operator_address", QApplication::translate("Customer", "Address:"), this, attributes.value("operator_address").toString());
    input_widgets << new MDLineEdit("operator_mail", QApplication::translate("Customer", "E-mail:"), this, attributes.value("operator_mail").toString());
    input_widgets << new MDLineEdit("operator_phone", QApplication::translate("Customer", "Phone:"), this, attributes.value("operator_phone").toString());

    foreach (MDAbstractInputWidget * widget, input_widgets) {
        widget->setShowInForm(false);
        widget->setVisible(false);
        addWidget(widget);
    }

    setVariantValue(attributes.value("operator_id"));
}

QVariant OperatorInputWidget::variantValue() const
{
    switch (operator_choice->checkedId()) {
    case 1:
        return -1;
    case 2:
        return operator_id->variantValue();
    }
    return 0;
}

void OperatorInputWidget::setVariantValue(const QVariant & value)
{
    switch (value.toInt()) {
    case -1:
        operator_choice->button(1)->setChecked(true);
        operatorChoiceChanged(1);
        break;
    case 0:
        operator_choice->button(0)->setChecked(true);
        operatorChoiceChanged(0);
        break;
    default:
        operator_choice->button(2)->setChecked(true);
        operator_id->setVariantValue(value);
        operatorChoiceChanged(2);
        break;
    }
}

void OperatorInputWidget::addToEditDialogue(EditDialogueWidgets & md)
{
    md.addInputWidget(this);

    foreach (MDAbstractInputWidget * widget, input_widgets)
        md.addInputWidget(widget);
}

void OperatorInputWidget::operatorChoiceChanged(int id)
{
    operator_id->setVisible(id == 2);

    foreach (MDAbstractInputWidget * widget, input_widgets)
        widget->setVisible(id == 2);

    QApplication::processEvents();
    static_cast<QWidget *>(this->parent())->resize(0, 0);
}

EditCustomerDialogue::EditCustomerDialogue(Customer * record, QWidget * parent):
    EditDialogue(record, parent),
    original_customer_id(idFieldValue().toString())
{
    QList<EditDialogueTableCell *> cells;
    EditDialogueTableCell * cell = new EditDialogueTableCell(tr("Name"), Global::String);
    cell->setId("name");
    cells.append(cell);
    cell = new EditDialogueTableCell(tr("E-mail"), Global::String);
    cell->setId("mail");
    cells.append(cell);
    cell = new EditDialogueTableCell(tr("Phone"), Global::String);
    cell->setId("phone");
    cells.append(cell);
    persons_table = new EditDialogueBasicTable(tr("Contact persons"), cells, this);
    md_grid_main->addWidget(persons_table, 0, 2, md_grid_main->rowCount(), 1);
    persons_table->setMinimumWidth(500);

    ListOfVariantMaps persons = Person("", original_customer_id).listAll();
    QMap<QString, EditDialogueTableCell *> person_data;

    for (int i = 0; i < persons.count(); ++i) {
        former_ids.append(persons.at(i).value("id").toInt());

        cell = new EditDialogueTableCell(persons.at(i).value("name"), Global::String);
        cell->setId("name");
        person_data.insert("name", cell);
        cell = new EditDialogueTableCell(persons.at(i).value("mail"), Global::String);
        cell->setId("mail");
        person_data.insert("mail", cell);
        cell = new EditDialogueTableCell(persons.at(i).value("phone"), Global::String);
        cell->setId("phone");
        person_data.insert("phone", cell);
        person_data.insert("id", new EditDialogueTableCell(persons.at(i).value("id"), "id"));
        persons_table->addRow(person_data);
    }

    if (!persons.count()) persons_table->addNewRow();
}

void EditCustomerDialogue::save()
{
    if (!EditDialogue::save(false)) return;

    qint64 next_id = -1;

    QList<MTDictionary> all_values = persons_table->allValues();

    Person person;
    for (int i = 0; i < all_values.count(); ++i) {
        if (all_values.at(i).value("name").isEmpty()) continue;
        QVariantMap map;

        if (all_values.at(i).contains("id")) {
            person = Person(all_values.at(i).value("id"));
            if (former_ids.contains(all_values.at(i).value("id").toInt()))
                former_ids.removeAll(all_values.at(i).value("id").toInt());
        } else {
            if (next_id < 0)
                next_id = qMax(Person().max("id") + (qint64)1, (qint64)QDateTime::currentDateTime().toTime_t());
            else
                next_id++;

            person = Person();
            map.insert("id", QString::number(next_id));
        }

        map.insert("company_id", idFieldValue());
        map.insert("name", all_values.at(i).value("name"));
        map.insert("mail", all_values.at(i).value("mail"));
        map.insert("phone", all_values.at(i).value("phone"));
        person.update(map);
    }
    for (int i = 0; i < former_ids.count(); ++i)
        Person(QString::number(former_ids.at(i))).remove();

    accept();
}
