#ifndef MODIFY_CUSTOMER_DIALOGUE_H
#define MODIFY_CUSTOMER_DIALOGUE_H

#include "modify_dialogue.h"

class Customer;
class ModifyDialogueBasicTable;

class ModifyCustomerDialogue : public ModifyDialogue
{
    Q_OBJECT

public:
    ModifyCustomerDialogue(Customer *, QWidget * = NULL);

protected slots:
    void save();

private:
    ModifyDialogueBasicTable * persons_table;
};

#endif // MODIFY_CUSTOMER_DIALOGUE_H
