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

#ifndef EDIT_DIALOGUE_WIDGETS_H
#define EDIT_DIALOGUE_WIDGETS_H

#include <QStringList>
#include <QMap>

class MDAbstractInputWidget;
class QWidget;

class EditDialogueWidgets
{
public:
    EditDialogueWidgets(): md_rows_in_column(14) {}

    void addInputWidget(MDAbstractInputWidget *iw) { md_inputwidgets << iw; }
    int inputWidgetCount() const { return md_inputwidgets.count(); }
    MDAbstractInputWidget *inputWidget(const QString &) const;

    void addInputWidgetGroup(const QString &group_id, const QString &group_name) {
        md_groups.insert(group_id, group_name);
    }
    void addGroupedInputWidgets(const QString &, const QList<MDAbstractInputWidget *> &);

    virtual void setWindowTitle(const QString &) = 0;

    virtual QWidget *widget() = 0;

    void setMaximumRowCount(int rows_in_column) { md_rows_in_column = rows_in_column; }

protected:
    QList<MDAbstractInputWidget *> md_inputwidgets;
    QMap<QString, QString> md_groups;
    int md_rows_in_column;
};

#endif // EDIT_DIALOGUE_WIDGETS_H
