#include "modify_inspector_dialogue.h"

#include "input_widgets.h"

#include <QTabWidget>

ModifyInspectorDialogue::ModifyInspectorDialogue(DBRecord * record, QWidget * parent)
    : TabbedModifyDialogue(record, parent)
{
    main_tabw->setTabText(0, tr("Inspector"));

    QList<MDAbstractInputWidget *> tab_inputwidgets;
    tab_inputwidgets << inputWidget("list_price");
    tab_inputwidgets << inputWidget("acquisition_price");
    tab_inputwidgets << inputWidget("category_id");

    addTab(new ModifyInspectorDialogueTab(tab_inputwidgets, this));
}

ModifyInspectorDialogueTab::ModifyInspectorDialogueTab(QList<MDAbstractInputWidget *> inputwidgets, QWidget * parent)
    : ModifyDialogueTab(parent)
{
    setName(tr("Assembly record"));

    this->inputwidgets = inputwidgets;

    init();
}

void ModifyInspectorDialogueTab::init()
{
    for (int i = 0; i < inputwidgets.count(); ++i) {
        inputwidgets.at(i)->setShowInForm(true);
    }

    QGridLayout * grid = new QGridLayout;
    ModifyDialogueColumnLayout(&inputwidgets, grid).layout();

    setLayout(grid);
}

void ModifyInspectorDialogueTab::save(int)
{
}
