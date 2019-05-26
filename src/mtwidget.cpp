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

#include "mtwidget.h"
#include "mainwindow.h"

MainWindow *MTWidget::parentWindow()
{
    if (!parent_window) {
        QWidget *parent_widget = parentWidget();
        MainWindow *main_window;

        while (parent_widget) {
            main_window = qobject_cast<MainWindow *>(parent_widget);
            if (main_window) {
                parent_window = main_window;
                break;
            }
            parent_widget = parent_widget->parentWidget();
        }
    }

    return parent_window;
}

MainWindow *MTWidget::parentWindow() const
{
    QWidget *parent_widget = parentWidget();
    MainWindow *main_window = NULL;

    while (parent_widget) {
        main_window = qobject_cast<MainWindow *>(parent_widget);
        if (main_window)
            break;
        parent_widget = parent_widget->parentWidget();
    }

    return main_window;
}
