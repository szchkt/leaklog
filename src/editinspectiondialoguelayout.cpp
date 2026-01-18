/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2026 Matus & Michal Tomlein

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

#include "editinspectiondialoguelayout.h"

#include "inputwidgets.h"
#include "global.h"

#include <QTreeWidget>
#include <QHeaderView>
#include <QPainter>

void UniformRowColourTreeWidget::drawRow(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QColor colour = itemFromIndex(index)->data(0, BackgroundColourRole).value<QColor>();
    if (colour.isValid()) {
        if (Global::textColourForBaseColour(colour) == Qt::white)
            colour.setAlpha(64);
        painter->fillRect(option.rect, colour);
    }
    QTreeWidget::drawRow(painter, option, index);
}

EditInspectionDialogueLayout::EditInspectionDialogueLayout(QList<MDAbstractInputWidget *> *inputwidgets, QMap<QString, QString> *groups, QGridLayout *grid_main)
    : EditDialogueLayout(inputwidgets, grid_main),
      groups(groups)
{
}

void EditInspectionDialogueLayout::layout()
{
    const int num_cols = 2;

    QList<QTreeWidget *> trees;
    QMap<QString, QTreeWidgetItem *> group_items;

    for (int c = 0; c < num_cols; ++c) {
        QTreeWidget *tree = new UniformRowColourTreeWidget;
        tree->setHeaderHidden(true);
        tree->setHeaderLabels(QStringList() << "1" << "2" << "3");
        tree->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
        tree->setSelectionMode(QAbstractItemView::NoSelection);
        tree->setFocusPolicy(Qt::NoFocus);

        trees << tree;
        addWidget(tree, 0, 2 * c, 1, 2);
    }

    QString refr_add_id = Global::createUUIDv5(DBInfo::databaseUUID(), "refr_add");
    QVector<QList<QWidget *> > widgets(num_cols);

    for (int i = 0; i < md_inputwidgets->count(); ++i) {
        MDAbstractInputWidget *widget = md_inputwidgets->at(i);

        if (!widget->rowSpan())
            continue;

        QTreeWidgetItem *item = new QTreeWidgetItem;
        if (!widget->colour().isEmpty())
            item->setData(0, UniformRowColourTreeWidget::BackgroundColourRole, QColor(widget->colour()));

        QString group_id = widget->groupId();
        int tree = !group_id.isEmpty() && widget->groupId() != refr_add_id;

        if (!group_id.isEmpty()) {
            QTreeWidgetItem *group = group_items.value(group_id);
            if (!group) {
                if (widget->hasGroupLabel()) {
                    group = new QTreeWidgetItem(trees.at(tree));
                    trees.at(tree)->setItemWidget(group, 0, widget->groupLabel()->widget());
                } else {
                    group = new QTreeWidgetItem(trees.at(tree), QStringList() << groups->value(group_id));
                }
                group->setExpanded(true);
                group->setFirstColumnSpanned(true);
                group_items.insert(group_id, group);
            }
            group->addChild(item);
        } else {
            trees.at(0)->addTopLevelItem(item);
        }

        trees.at(tree)->setItemWidget(item, 1, widget->label()->widget());
        trees.at(tree)->setItemWidget(item, 2, widget->widget());

        widgets[tree] << widget->widget();
    }

    // Tab order

    QList<QWidget *> all_widgets;

    foreach (const QList<QWidget *> &list, widgets)
        all_widgets << list;

    for (int i = 1; i < all_widgets.count(); ++i)
        QWidget::setTabOrder(all_widgets.at(i - 1), all_widgets.at(i));
}
