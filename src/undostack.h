/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2013 Matus & Michal Tomlein

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

#ifndef UNDOSTACK_H
#define UNDOSTACK_H

#include <QObject>

class QAction;
class QMenu;

class UndoStack : public QObject
{
    Q_OBJECT

public:
    UndoStack(QAction *undo_action, QWidget *parent);

    int savepoint();
    bool rollbackToSavepoint(int savepoint);

    void clear();

signals:
    void undoTriggered();

private slots:
    void hovered(QAction *action = NULL);
    void undo(QAction *action);

private:
    QAction *m_undo_action;
    QMenu *m_undo_menu;
};

class UndoCommand : QObject
{
    Q_OBJECT

public:
    UndoCommand(UndoStack *undo_stack, const QString &description);

    inline QString description() const { return m_description; }

private:
    QString m_description;
};

#endif // UNDOSTACK_H
