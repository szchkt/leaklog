#include "modify_circuit_dialogue.h"

#include "global.h"
#include "records.h"

#include <QSqlQuery>
#include <QHeaderView>

ModifyCircuitDialogue::ModifyCircuitDialogue(DBRecord * record, QWidget * parent)
    : TabbedModifyDialogue(record, parent)
{
    addTab(new ModifyCircuitDialogueUnitsTab);
}

ModifyCircuitDialogueUnitsTab::ModifyCircuitDialogueUnitsTab(QWidget * parent)
    : ModifyDialogueTab(parent)
{
    setName(tr("Circuit units"));

    QGridLayout * grid = new QGridLayout(this);
    tree = new QTreeWidget(this);
    tree->setColumnCount(3);
    QStringList header_labels;
    header_labels << tr("Available unit types");
    header_labels << tr("Location");
    header_labels << tr("Add");
    tree->setHeaderLabels(header_labels);
    tree->setSelectionMode(QAbstractItemView::NoSelection);
    tree->header()->setStretchLastSection(false);
    tree->header()->setResizeMode(0, QHeaderView::Stretch);
    tree->header()->setResizeMode(1, QHeaderView::ResizeToContents);
    tree->header()->setResizeMode(2, QHeaderView::ResizeToContents);
    QObject::connect(tree, SIGNAL(itemExpanded(QTreeWidgetItem*)), this, SLOT(manufacturerItemExpanded(QTreeWidgetItem*)));
    QObject::connect(tree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(itemDoubleClicked(QTreeWidgetItem*,int)));
    QObject::connect(tree, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(itemClicked(QTreeWidgetItem*, int)));
    grid->addWidget(tree, 0, 0);

    QList<ModifyDialogueTableCell *> header;
    header.append(new ModifyDialogueTableCell(tr("Manufacturer"), "manufacturer"));
    header.append(new ModifyDialogueTableCell(tr("Type"), "type"));
    header.append(new ModifyDialogueTableCell(tr("Location"), "location"));
    header.append(new ModifyDialogueTableCell(tr("Serial number"), "sn"));

    table = new ModifyCircuitDialogueTable(tr("Used circuit units"), header, this);
    grid->addWidget(table, 0, 1);

    loadManufacturers();
}

void ModifyCircuitDialogueUnitsTab::save(int)
{
}

void ModifyCircuitDialogueUnitsTab::loadManufacturers()
{
    ModifyCircuitDialogueTreeItem * item;
    QSqlQuery query("SELECT DISTINCT manufacturer FROM circuit_unit_types ORDER BY manufacturer");
    while (query.next()) {
        item = new ModifyCircuitDialogueTreeItem(tree);
        item->setText(0, query.value(0).toString());
        item->setIsType(false);
        item->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
    }
}

void ModifyCircuitDialogueUnitsTab::manufacturerItemExpanded(QTreeWidgetItem * qitem)
{
    ModifyCircuitDialogueTreeItem * parent_item = (ModifyCircuitDialogueTreeItem *) qitem;
    ModifyCircuitDialogueTreeItem * item = NULL;
    if (parent_item->isType() || parent_item->childCount()) return;

    QSqlQuery query(QString("SELECT id, type, location FROM circuit_unit_types WHERE manufacturer = '%1' ORDER BY type")
                    .arg(parent_item->text(0)));

    while (query.next()) {
        item = new ModifyCircuitDialogueTreeItem(parent_item);
        item->setText(0, query.value(1).toString());
        item->setText(1, location(query.value(2).toInt()));
        item->setText(2, tr("Add"));
        item->setIcon(2, QIcon(QString::fromUtf8(":/images/images/add16.png")));
        item->setManufacturer(parent_item->text(0));
        item->setLocation(query.value(2).toInt());
    }
}

void ModifyCircuitDialogueUnitsTab::itemDoubleClicked(QTreeWidgetItem * qitem, int col)
{
    ModifyCircuitDialogueTreeItem * item = (ModifyCircuitDialogueTreeItem *) qitem;
    if (!item->isType() || col == 2) return;

    addToTable(item);
}

void ModifyCircuitDialogueUnitsTab::itemClicked(QTreeWidgetItem * qitem, int col)
{
    ModifyCircuitDialogueTreeItem * item = (ModifyCircuitDialogueTreeItem *) qitem;
    if (!item->isType() || col != 2) return;

    addToTable(item);
}

void ModifyCircuitDialogueUnitsTab::addToTable(ModifyCircuitDialogueTreeItem * item)
{
    QMap<QString, ModifyDialogueTableCell *> cells;
    cells.insert("manufacturer", new ModifyDialogueTableCell(item->manufacturer(), "manufacturer"));
    cells.insert("type", new ModifyDialogueTableCell(item->text(0), "type"));
    cells.insert("location", new ModifyDialogueTableCell(location(item->location()), "location"));
    cells.insert("sn", new ModifyDialogueTableCell(QString(), "sn", Global::String));
    table->addRow(cells);
}

const QString ModifyCircuitDialogueUnitsTab::location(int num)
{
    switch (num) {
    case CircuitUnitType::External:
        return tr("External");
    case CircuitUnitType::Internal:
        return tr("Internal");
    }
}

ModifyCircuitDialogueTable::ModifyCircuitDialogueTable(const QString & name, const QList<ModifyDialogueTableCell *> & header, QWidget * parent)
    : ModifyDialogueTable(name, header, parent)
{
    layout->addStretch();
}
