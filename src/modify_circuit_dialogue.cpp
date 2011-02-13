#include "modify_circuit_dialogue.h"

#include "global.h"
#include "records.h"
#include "mtdictionary.h"
#include "input_widgets.h"

#include <QSqlQuery>
#include <QPushButton>
#include <QHeaderView>

ModifyCircuitDialogue::ModifyCircuitDialogue(DBRecord * record, QWidget * parent)
    : TabbedModifyDialogue(record, parent)
{
    main_tabw->setTabText(0, tr("Cooling circuit"));
    ModifyCircuitDialogueUnitsTab * tab = new ModifyCircuitDialogueUnitsTab(md_record->parent("parent"), idFieldValue().toString(), this);
    QObject::connect(tab, SIGNAL(updateCircuit(MTDictionary)), this, SLOT(updateCircuit(MTDictionary)));
    addTab(tab);
}

void ModifyCircuitDialogue::updateCircuit(MTDictionary dict)
{
    MDInputWidget * md;
    for (int i = 0; i < dict.count(); ++i) {
        md = inputWidget(dict.key(i));
        if (md)
            md->setVariantValue(dict.value(i));
    }
}

ModifyCircuitDialogueUnitsTab::ModifyCircuitDialogueUnitsTab(const QString & customer_id, const QString & circuit_id, QWidget * parent)
    : ModifyDialogueTab(parent)
{
    setName(tr("Units"));
    this->customer_id = customer_id;

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
    QObject::connect(table, SIGNAL(updateCircuit(MTDictionary)), this, SIGNAL(updateCircuit(MTDictionary)));
    grid->addWidget(table, 0, 1);

    loadManufacturers();
    loadRows(customer_id, circuit_id);
}

void ModifyCircuitDialogueUnitsTab::loadRows(const QString & customer_id, const QString & circuit_id)
{
    QMap<QString, ModifyDialogueTableCell *> cells;

    enum QUERY_RESULTS
    {
        SN = 0,
        MANUFACTURER = 1,
        TYPE = 2,
        LOCATION = 3,
        UNIT_TYPE_ID = 4
    };
    QSqlQuery query(QString("SELECT circuit_units.sn, circuit_unit_types.manufacturer, circuit_unit_types.type, circuit_unit_types.location, circuit_unit_types.id"
                            " FROM circuit_units"
                            " LEFT JOIN circuit_unit_types ON circuit_units.unit_type_id = circuit_unit_types.id"
                            " WHERE circuit_units.company_id = %1 AND circuit_units.circuit_id = %2")
                    .arg(customer_id.toInt()).arg(circuit_id.toInt()));
    while (query.next()) {
        cells.insert("manufacturer", new ModifyDialogueTableCell(query.value(MANUFACTURER), "manufacturer"));
        cells.insert("type", new ModifyDialogueTableCell(query.value(TYPE), "type"));
        cells.insert("unit_type_id", new ModifyDialogueTableCell(query.value(UNIT_TYPE_ID), "unit_type_id"));
        cells.insert("location", new ModifyDialogueTableCell(CircuitUnitType::locationToString(query.value(LOCATION).toInt()), "location"));
        cells.insert("sn", new ModifyDialogueTableCell(query.value(SN), "sn", Global::String));
        table->addRow(cells);
    }
}

void ModifyCircuitDialogueUnitsTab::save(int circuit_id)
{
    MTDictionary dict;
    dict.insert("company_id", customer_id);
    dict.insert("circuit_id", QString::number(circuit_id));

    CircuitUnit unit(dict);
    unit.remove();

    QList<MTDictionary> values = table->allValues();
    QVariantMap map;

    for (int i = 0; i < values.count(); ++i) {
        map.insert("unit_type_id", values.at(i).value("unit_type_id"));
        map.insert("sn", values.at(i).value("sn"));
        unit.update(map);
    }
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
        item->setText(1, CircuitUnitType::locationToString(query.value(2).toInt()));
        item->setText(2, tr("Add"));
        item->setIcon(2, QIcon(QString::fromUtf8(":/images/images/add16.png")));
        item->setUnitType(query.value(0).toString());
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
    cells.insert("unit_type_id", new ModifyDialogueTableCell(item->unitType(), "unit_type_id"));
    cells.insert("location", new ModifyDialogueTableCell(CircuitUnitType::locationToString(item->location()), "location"));
    cells.insert("sn", new ModifyDialogueTableCell(QString(), "sn", Global::String));
    table->addRow(cells);
}

ModifyCircuitDialogueTable::ModifyCircuitDialogueTable(const QString & name, const QList<ModifyDialogueTableCell *> & header, QWidget * parent)
    : ModifyDialogueTable(name, header, parent)
{
    layout->addStretch();

    QPushButton * update_circuit_btn = new QPushButton(tr("Update circuit"), this);
    QObject::connect(update_circuit_btn, SIGNAL(clicked()), this, SLOT(updateCircuit()));

    QHBoxLayout * hlayout = new QHBoxLayout;
    hlayout->addStretch();
    hlayout->addWidget(update_circuit_btn);
    hlayout->setContentsMargins(0, 0, 0, 0);

    layout->addLayout(hlayout);
}

void ModifyCircuitDialogueTable::updateCircuit()
{
    MTDictionary circuit_vars;
    double refr_amount = 0;

    for (int i = 0; i < rows.count(); ++i) {
        CircuitUnitType unit_type_record(rows.at(i)->value("unit_type_id"));
        QVariantMap unit_type = unit_type_record.list();

        circuit_vars.setValue("refrigerant", unit_type.value("refrigerant").toString());
        refr_amount += unit_type.value("refrigerant_amount").toDouble();

        if (unit_type.value("location").toInt() == CircuitUnitType::External) {
            circuit_vars.setValue("manufacturer", unit_type.value("manufacturer").toString());
            circuit_vars.setValue("type", unit_type.value("type").toString());
        }
    }

    circuit_vars.setValue("refrigerant_amount", QString::number(refr_amount));

    updateCircuit(circuit_vars);
}
