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

    int next_id = 0;
    for (int i = 0; i < former_ids.count(); ++i)
        next_id = former_ids.at(i) > next_id ? former_ids.at(i) : next_id;
    next_id++;

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
            person = Person();
            map.insert("id", QString::number(next_id++));
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
