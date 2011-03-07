#include "tabbed_modify_dialogue.h"

#include "records.h"
#include "input_widgets.h"
#include "global.h"

#include <QTabWidget>
#include <QScrollArea>

TabbedModifyDialogue::TabbedModifyDialogue(DBRecord * record, QWidget * parent)
    : ModifyDialogue(parent)
{
    main_tabw = new QTabWidget;

    init(record);

    md_record->initModifyDialogue(this);

    ModifyDialogueColumnLayout(&md_inputwidgets, md_grid_main).layout();
}

TabbedModifyDialogue::~TabbedModifyDialogue()
{
    delete main_tabw;
}

void TabbedModifyDialogue::addMainGridLayout(QVBoxLayout * md_vlayout_main)
{
    QWidget * main_form_tab = new QWidget(main_tabw);
    main_form_tab->setLayout(md_grid_main);

    main_tabw->addTab(main_form_tab, tr("Basic information"));
    md_vlayout_main->addWidget(main_tabw);
}

void TabbedModifyDialogue::addTab(ModifyDialogueTab * tab)
{
    tabs << tab;

    main_tabw->addTab(tab->widget(), tab->name());
}

void TabbedModifyDialogue::save()
{
    if (!ModifyDialogue::save(false)) return;

    for (int i = 0; i < tabs.count(); ++i) {
        tabs.at(i)->save(idFieldValue());
    }

    accept();
}

ModifyDialogueTab::ModifyDialogueTab(QWidget * parent)
    : QWidget(parent)
{
    setAutoFillBackground(false);
}

void ModifyDialogueTab::setLayout(QLayout * layout)
{
    layout->setContentsMargins(9, 9, 9, 9);
    QWidget::setLayout(layout);
}

QWidget * ModifyDialogueTab::widget()
{
    QScrollArea * scroll_area = createScrollArea();
    scroll_area->setWidget(this);
    return scroll_area;
}

QScrollArea * ModifyDialogueTab::createScrollArea()
{
    QScrollArea * scroll_area = new QScrollArea;
    scroll_area->setWidgetResizable(1);
    scroll_area->setFrameStyle(QFrame::NoFrame);
    scroll_area->setAutoFillBackground(true);
    scroll_area->setBackgroundRole(QPalette::NoRole);
    return scroll_area;
}
