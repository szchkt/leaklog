/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2019 Matus & Michal Tomlein

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

#include "undostack.h"
#include "mtsqlquery.h"

#include <QMenu>

UndoStack::UndoStack(QAction *undo_action, QWidget *parent):
    QObject(parent), m_undo_action(undo_action)
{
    m_undo_menu = new QMenu(parent);
    QObject::connect(m_undo_menu, SIGNAL(aboutToHide()), this, SLOT(hovered()));
    QObject::connect(m_undo_menu, SIGNAL(hovered(QAction *)), this, SLOT(hovered(QAction *)));
    QObject::connect(m_undo_menu, SIGNAL(triggered(QAction *)), this, SLOT(undo(QAction *)));
    m_undo_action->setMenu(m_undo_menu);
    m_undo_action->setEnabled(false);
}

int UndoStack::savepoint()
{
    QList<QAction *> actions(m_undo_menu->actions());
    int savepoint = -1;
    MTSqlQuery query;

    if (query.exec(QString("SAVEPOINT savepoint_%1").arg(actions.count()))) {
        savepoint = actions.count();

        QString description = tr("Unknown operation");
        QList<UndoCommand *> commands = findChildren<UndoCommand *>();
        if (commands.count())
            description = commands.last()->description();

        m_undo_menu->insertAction(actions.value(0), new QAction(description, this));

        m_undo_action->setEnabled(true);
    }

    return savepoint;
}

bool UndoStack::rollbackToSavepoint(int savepoint)
{
    if (savepoint >= 0 && savepoint < m_undo_menu->actions().count()) {
        MTSqlQuery query;

        if (query.exec(QString("ROLLBACK TO SAVEPOINT savepoint_%1").arg(savepoint))) {
            while (savepoint < m_undo_menu->actions().count()) {
                QAction *action = m_undo_menu->actions().first();
                m_undo_menu->removeAction(action);
                delete action;
            }

            m_undo_action->setEnabled(m_undo_menu->actions().count());

            emit undoTriggered();

            return true;
        }
    }

    return false;
}

void UndoStack::clear()
{
    foreach (QAction *a, m_undo_menu->actions())
        delete a;
    m_undo_action->setEnabled(false);
}

void UndoStack::hovered(QAction *action)
{
    bool checked = action != NULL;

    foreach (QAction *a, m_undo_menu->actions()) {
        a->setCheckable(checked);
        a->setChecked(checked);

        if (a == action)
            checked = false;
    }
}

void UndoStack::undo(QAction *action)
{
    QList<QAction *> actions(m_undo_menu->actions());
    int savepoint = actions.count() - actions.indexOf(action) - 1;

    if (savepoint >= 0 && savepoint < actions.count())
        rollbackToSavepoint(savepoint);
}

UndoCommand::UndoCommand(UndoStack *undo_stack, const QString &description):
    QObject(undo_stack),
    m_description(description)
{
}
