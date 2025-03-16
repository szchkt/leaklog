/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2025 Matus & Michal Tomlein

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

#ifndef MTTABWIDGET_H
#define MTTABWIDGET_H

#include <QTabWidget>

class MTTabWidget : public QTabWidget
{
    Q_OBJECT

public:
    MTTabWidget(QWidget *parent = NULL): QTabWidget(parent) {}

    void setTabBar(QTabBar *tb);
    QTabBar *tabBar() const;

    void setTabBarVisible(bool visible);

protected:
    void tabInserted(int index);
    void tabRemoved(int index);
};

#endif // MTTABWIDGET_H
