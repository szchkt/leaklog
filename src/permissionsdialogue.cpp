/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2017 Matus & Michal Tomlein

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

#include "permissionsdialogue.h"
#include "global.h"
#include "mtdictionary.h"
#include "dbinfo.h"

#include <QGridLayout>
#include <QButtonGroup>
#include <QRadioButton>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QTreeWidget>
#include <QLabel>
#include <QHeaderView>
#include <QSqlError>

using namespace Global;

PermissionsDialogue::PermissionsDialogue(QWidget *parent):
QDialog(parent, Qt::Dialog) {
    setWindowTitle(tr("Configure permissions - Leaklog"));

    QGridLayout *gl = new QGridLayout(this);

    QLabel *information = new QLabel(this);
    information->setWordWrap(true);
    information->setText(tr("Permissions only apply when the database is locked."));
    gl->addWidget(information, 0, 0);

    QTreeWidget *tree = new QTreeWidget(this);
    tree->setColumnCount(4);
    tree->setIndentation(0);
    tree->setHeaderHidden(true);
    tree->setSelectionMode(QAbstractItemView::NoSelection);
    tree->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    tree->setMinimumSize(600, 400);
    gl->addWidget(tree, 1, 0);

    const MTDictionary &permissions = Global::permissions();

    QString nobody_text = isDatabaseRemote() ? tr("Administrator") : tr("Nobody");

    for (int i = 0; i < permissions.count(); ++i) {
        QString permission = DBInfo::valueForKey(permissions.key(i) + "_permitted");

        QTreeWidgetItem *item = new QTreeWidgetItem(tree);
        item->setText(0, permissions.value(i));

        QButtonGroup *group = new QButtonGroup(this);
        permission_groups.insert(permissions.key(i), group);

        QRadioButton *everyone = new QRadioButton(tr("Everyone"), this);
        group->addButton(everyone, 1);
        tree->setItemWidget(item, 1, everyone);

        if (isOwnerPermissionApplicable(permissions.key(i))) {
            QRadioButton *owner = new QRadioButton(tr("Author"), this);
            group->addButton(owner, 2);
            tree->setItemWidget(item, 2, owner);

            if (permission == "owner")
                owner->setChecked(true);
        }

        QRadioButton *nobody = new QRadioButton(nobody_text, this);
        group->addButton(nobody, 0);
        tree->setItemWidget(item, 3, nobody);

        if (permission == "false")
            nobody->setChecked(true);
        else if (permission != "owner")
            everyone->setChecked(true);
    }

    QDialogButtonBox *bb = new QDialogButtonBox(this);
    bb->setStandardButtons(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
    bb->button(QDialogButtonBox::Save)->setText(tr("Save"));
    bb->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));
    QObject::connect(bb, SIGNAL(accepted()), this, SLOT(save()));
    QObject::connect(bb, SIGNAL(rejected()), this, SLOT(reject()));
    gl->addWidget(bb, 2, 0);
}

void PermissionsDialogue::save()
{
    QMapIterator<QString, QButtonGroup *> i(permission_groups);
    while (i.hasNext()) { i.next();
        QString value = "true";
        if (i.value()->checkedId() == 0)
            value = "false";
        else if (i.value()->checkedId() == 2)
            value = "owner";
        DBInfo::setValueForKey(i.key() + "_permitted", value);
    }
    accept();
}
