/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2023 Matus & Michal Tomlein

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

#include "mttabwidget.h"

#include <QTabBar>

void MTTabWidget::setTabBar(QTabBar *tb)
{
    QTabWidget::setTabBar(tb);
}

QTabBar *MTTabWidget::tabBar() const
{
    return QTabWidget::tabBar();
}

void MTTabWidget::setTabBarVisible(bool visible)
{
    tabBar()->setVisible(visible);
}

void MTTabWidget::tabInserted(int index)
{
    QTabWidget::tabInserted(index);
    tabBar()->setVisible(count() > 0);
}

void MTTabWidget::tabRemoved(int index)
{
    QTabWidget::tabRemoved(index);
    tabBar()->setVisible(count() > 0);
}
