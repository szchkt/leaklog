#include "modify_customer_dialogue.h"

#include "records.h"
#include "modifydialoguetable.h"

ModifyCustomerDialogue::ModifyCustomerDialogue(Customer * record, QWidget * parent):
        ModifyDialogue(record, parent)
{
    Person persons_record("", idFieldValue().toString());

    MTDictionary table_header;
    table_header.insert("name", tr("Name"));
    table_header.insert("mail", tr("E-mail"));
    table_header.insert("phone", tr("Phone"));
    persons_table = new ModifyDialogueBasicTable(tr("Contact persons"), table_header, this);
    md_grid_main->addWidget(persons_table, 0, 2, md_grid_main->rowCount(), 1);

    ListOfVariantMaps persons = persons_record.listAll();
    QMap<QString, QVariant> person_data;

    for (int i = 0; i < persons.count(); ++i) {
        person_data.insert("name", persons.at(i).value("name"));
        person_data.insert("mail", persons.at(i).value("mail"));
        person_data.insert("phone", persons.at(i).value("phone"));
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
