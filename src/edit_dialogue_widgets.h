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

#ifndef EDIT_DIALOGUE_WIDGETS_H
#define EDIT_DIALOGUE_WIDGETS_H

#include <QStringList>

class MDAbstractInputWidget;

class EditDialogueWidgets
{
public:
    void addInputWidget(MDAbstractInputWidget * iw) { md_inputwidgets << iw; }
    void addGroupedInputWidgets(const QString &, const QList<MDAbstractInputWidget *> &);

    void setUsedIds(const QStringList & ids) { md_used_ids = ids; }

    virtual void setWindowTitle(const QString &) = 0;

    virtual QWidget * widget() = 0;

protected:
    QStringList md_used_ids;
    QList<MDAbstractInputWidget *> md_inputwidgets;
};

#endif // EDIT_DIALOGUE_WIDGETS_H
