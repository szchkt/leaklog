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

#ifndef MODIFY_CIRCUIT_DIALOGUE_COMPRESSORS_TAB_H
#define MODIFY_CIRCUIT_DIALOGUE_COMPRESSORS_TAB_H

#include "tabbed_modify_dialogue.h"
#include <QLineEdit>

class QTreeWidgetItem;
class CompressorsTableRow;

class ModifyCircuitDialogueCompressorsTab : public ModifyDialogueTab
{
    Q_OBJECT

public:
    ModifyCircuitDialogueCompressorsTab(const QString &, const QString &, QWidget * = NULL);

    void save(const QVariant &);
    QWidget * widget() { return this; }

private slots:
    CompressorsTableRow * addRow(int = -1);
    void rowWantsToBeRemoved(CompressorsTableRow *);

private:
    void load(const QString &);

    QTreeWidget * tree;
    QList<CompressorsTableRow *> rows;
    QList<int> deleted_ids;
    QString customer_id;
};

class CompressorsTableRow : public QWidget
{
    Q_OBJECT

public:
    CompressorsTableRow(QTreeWidget *, int = -1, QWidget * = NULL);
    ~CompressorsTableRow();

    void setName(const QString & name) { name_le->setText(name); }
    QString name() { return name_le->text(); }

    void setManufacturer(const QString & manufacturer) { manufacturer_le->setText(manufacturer); }
    QString manufacturer() { return manufacturer_le->text(); }

    void setType(const QString & type) { type_le->setText(type); }
    QString type() { return type_le->text(); }

    void setSn(const QString & sn) { sn_le->setText(sn); }
    QString sn() { return sn_le->text(); }

    int id() { return m_id; }
    bool hasId() { return m_id >= 0; }

    bool isEmpty() { return name_le->text().isEmpty() &&
                manufacturer_le->text().isEmpty() &&
                type_le->text().isEmpty() &&
                sn_le->text().isEmpty(); }

    QTreeWidgetItem * treeItem() { return tree_item; }

private slots:
    void remove();

signals:
    void wantsToBeRemoved(CompressorsTableRow *);

private:
    int m_id;
    QLineEdit * name_le;
    QLineEdit * manufacturer_le;
    QLineEdit * type_le;
    QLineEdit * sn_le;
    QPushButton * remove_btn;
    QTreeWidgetItem * tree_item;
};

#endif // MODIFY_CIRCUIT_DIALOGUE_COMPRESSORS_TAB_H
