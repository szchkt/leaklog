#include "tabbed_modify_dialogue.h"

#include "records.h"

#include <QTabWidget>

TabbedModifyDialogue::TabbedModifyDialogue(DBRecord * record, QWidget * parent)
    : ModifyDialogue(parent)
{
    main_tabw = new QTabWidget;

    init(record);

    md_record->initModifyDialogue(this);

    layoutInputWidgets();
}

void TabbedModifyDialogue::addMainGridLayout(QVBoxLayout * md_vlayout_main)
{
    QWidget * main_form_tab = new QWidget(main_tabw);
    main_form_tab->setLayout(md_grid_main);

    main_tabw->addTab(main_form_tab, QIcon(), tr("Settings"));
    md_vlayout_main->addWidget(main_tabw);
}
