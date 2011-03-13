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

#ifndef TABBED_MODIFY_DIALOGUE_H
#define TABBED_MODIFY_DIALOGUE_H

#include "modify_dialogue.h"

class QTabWidget;
class QTreeWidget;
class QLineEdit;
class QComboBox;
class QGroupBox;
class QScrollArea;

class ModifyDialogueTab;
class MDLineEdit;
class MDComboBox;
class MTDictionary;

class TabbedModifyDialogue : public ModifyDialogue
{
    Q_OBJECT

public:
    TabbedModifyDialogue(DBRecord *, QWidget * = NULL);
    ~TabbedModifyDialogue();

protected slots:
    void save();

protected:
    void addMainGridLayout(QVBoxLayout *);

    void addTab(ModifyDialogueTab *);

    QTabWidget * main_tabw;
    QList<ModifyDialogueTab *> tabs;
};

class ModifyDialogueTab : public QWidget
{
    Q_OBJECT

public:
    ModifyDialogueTab(QWidget * = NULL);

    void setLayout(QLayout *);

    const QString & name() { return tab_name; }
    virtual void save(const QVariant &) = 0;

    virtual QWidget * widget();
    QScrollArea * createScrollArea();

protected:
    void setName(const QString tab_name) { this->tab_name = tab_name; }

    QString tab_name;
};

#endif // TABBED_MODIFY_DIALOGUE_H
