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

#include "modify_customer_dialogue.h"

#include "records.h"
#include "modify_dialogue_table.h"
#include "global.h"

ModifyCustomerDialogue::ModifyCustomerDialogue(Customer * record, QWidget * parent):
        ModifyDialogue(record, parent)
{
    Person persons_record("", idFieldValue().toString());

    QList<ModifyDialogueTableCell *> cells;
    ModifyDialogueTableCell * cell = new ModifyDialogueTableCell(tr("Name"), Global::String);
    cell->setId("name");
    cells.append(cell);
    cell = new ModifyDialogueTableCell(tr("E-mail"), Global::String);
    cell->setId("mail");
    cells.append(cell);
    cell = new ModifyDialogueTableCell(tr("Phone"), Global::String);
    cell->setId("phone");
    cells.append(cell);
    persons_table = new ModifyDialogueBasicTable(tr("Contact persons"), cells, this);
    md_grid_main->addWidget(persons_table, 0, 2, md_grid_main->rowCount(), 1);
    persons_table->setMinimumWidth(500);

    ListOfVariantMaps persons = persons_record.listAll();
    QMap<QString, ModifyDialogueTableCell *> person_data;

    for (int i = 0; i < persons.count(); ++i) {
        cell = new ModifyDialogueTableCell(persons.at(i).value("name"), Global::String);
        cell->setId("name");
        person_data.insert("name", cell);
        cell = new ModifyDialogueTableCell(persons.at(i).value("mail"), Global::String);
        cell->setId("mail");
        person_data.insert("mail", cell);
        cell = new ModifyDialogueTableCell(persons.at(i).value("phone"), Global::String);
        cell->setId("phone");
        person_data.insert("phone", cell);
        persons_table->addRow(person_data);
    }

    if (!persons.count()) persons_table->addNewRow();
}

void ModifyCustomerDialogue::save()
{
    if (!ModifyDialogue::save(false)) return;

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
