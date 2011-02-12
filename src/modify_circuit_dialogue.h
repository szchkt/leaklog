#ifndef MODIFY_CIRCUIT_DIALOGUE_H
#define MODIFY_CIRCUIT_DIALOGUE_H

#include "tabbed_modify_dialogue.h"
#include "modify_dialogue_table.h"

#include <QTreeWidgetItem>

class ModifyCircuitDialogueTreeItem;
class ModifyCircuitDialogueTable;

class ModifyCircuitDialogue : public TabbedModifyDialogue
{
public:
    ModifyCircuitDialogue(DBRecord *, QWidget * = NULL);
};

class ModifyCircuitDialogueUnitsTab : public ModifyDialogueTab
{
    Q_OBJECT

public:
    ModifyCircuitDialogueUnitsTab(const QString &, const QString &, QWidget * = NULL);

    void save(int);

private slots:
    void manufacturerItemExpanded(QTreeWidgetItem *);
    void itemDoubleClicked(QTreeWidgetItem *, int);
    void itemClicked(QTreeWidgetItem*, int);

private:
    void loadRows(const QString &, const QString &);
    void loadManufacturers();
    void addToTable(ModifyCircuitDialogueTreeItem *);

    ModifyCircuitDialogueTable * table;
    QTreeWidget * tree;
    QString customer_id;
};

class ModifyCircuitDialogueTable : public ModifyDialogueTable
{
    Q_OBJECT

public:
    ModifyCircuitDialogueTable(const QString &, const QList<ModifyDialogueTableCell *> &, QWidget *);

private slots:
    void activateRow() {}

private:
    void addHiddenRow(ModifyDialogueTableRow *) {}
    QList<ModifyDialogueTableCell *> hiddenAttributes() { return QList<ModifyDialogueTableCell *>(); }
};

class ModifyCircuitDialogueTreeItem : public QTreeWidgetItem
{
public:
    ModifyCircuitDialogueTreeItem(QTreeWidget * parent) : QTreeWidgetItem(parent) { is_type = true; }
    ModifyCircuitDialogueTreeItem(QTreeWidgetItem * parent) : QTreeWidgetItem(parent) { is_type = true; }

    void setManufacturer(const QString & manufacturer) { this->unit_manufacturer = manufacturer; }
    const QString & manufacturer() { return unit_manufacturer; }
    void setUnitType(const QString & type) { this->unit_type = type; }
    const QString & unitType() { return unit_type; }
    void setIsType(bool is_type) { this->is_type = is_type; }
    bool isType() { return is_type; }
    void setLocation(int loc) { this->unit_location = loc; }
    int location() { return unit_location; }

private:
    QString unit_manufacturer;
    QString unit_type;
    bool is_type;
    int unit_location;
};

#endif // MODIFY_CIRCUIT_DIALOGUE_H
