/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2011 Matus & Michal Tomlein

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

EditCustomerDialogue::EditCustomerDialogue(Customer * record, QWidget * parent):
        EditDialogue(record, parent)
{
    Person persons_record("", idFieldValue().toString());

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

    ListOfVariantMaps persons = persons_record.listAll();
    QMap<QString, EditDialogueTableCell *> person_data;

    for (int i = 0; i < persons.count(); ++i) {
        cell = new EditDialogueTableCell(persons.at(i).value("name"), Global::String);
        cell->setId("name");
        person_data.insert("name", cell);
        cell = new EditDialogueTableCell(persons.at(i).value("mail"), Global::String);
        cell->setId("mail");
        person_data.insert("mail", cell);
        cell = new EditDialogueTableCell(persons.at(i).value("phone"), Global::String);
        cell->setId("phone");
        person_data.insert("phone", cell);
        persons_table->addRow(person_data);
    }

    if (!persons.count()) persons_table->addNewRow();
}

void EditCustomerDialogue::save()
{
    if (!EditDialogue::save(false)) return;

    Person persons_record("", idFieldValue().toString());
    persons_record.remove();

    Person person;
    int next_id = person.list("MAX(id) AS max").value("max").toInt() + 1;

    QList<MTDictionary> all_values = persons_table->allValues();
    QVariantMap person_values;
    person_values.insert("company_id", idFieldValue());

    for (int i = 0; i < all_values.count(); ++i) {
        if (all_values.at(i).value("name").isEmpty()) continue;

        person_values.insert("name", all_values.at(i).value("name"));
        person_values.insert("mail", all_values.at(i).value("mail"));
        person_values.insert("phone", all_values.at(i).value("phone"));

        person.setId(QString::number(next_id));
        person.update(person_values);

        next_id++;
    }

    accept();
}
