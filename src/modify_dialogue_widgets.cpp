#include "modify_dialogue_widgets.h"

#include "input_widgets.h"

void ModifyDialogueWidgets::addGroupedInputWidgets(const QString & group_name, const QList<MDAbstractInputWidget *> & widgets)
{
    MDGroupedInputWidgets * gw = new MDGroupedInputWidgets(group_name, widget());
    addInputWidget(gw);
    for (int i = 0; i < widgets.count(); ++i) {
        widgets.at(i)->setShowInForm(false);
        addInputWidget(widgets.at(i));
        gw->addWidget(widgets.at(i));
    }
}
