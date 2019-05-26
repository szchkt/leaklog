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

#include "editdialoguewidgets.h"

#include "inputwidgets.h"

void EditDialogueWidgets::addGroupedInputWidgets(const QString &group_name, const QList<MDAbstractInputWidget *> &widgets)
{
    MDGroupedInputWidgets *gw = new MDGroupedInputWidgets(group_name, widget());
    addInputWidget(gw);
    for (int i = 0; i < widgets.count(); ++i) {
        widgets.at(i)->setRowSpan(0);
        addInputWidget(widgets.at(i));
        gw->addWidget(widgets.at(i));
    }
}

MDAbstractInputWidget *EditDialogueWidgets::inputWidget(const QString &id) const
{
    for (int i = 0; i < md_inputwidgets.count(); ++i) {
        if (md_inputwidgets.at(i)->id() == id) {
            return md_inputwidgets.at(i);
        }
    }
    return NULL;
}
