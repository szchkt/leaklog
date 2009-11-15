/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2009 Matus & Michal Tomlein

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

#ifndef LEAKLOG_MAIN_H
#define LEAKLOG_MAIN_H

#include <QApplication>
#include <QPointer>

#include "main_window.h"

class MTApplication : public QApplication
{
    Q_OBJECT

public:
    MTApplication(int & argc, char ** argv):
    QApplication(argc, argv) {}
    MTApplication(int & argc, char ** argv, bool GUIenabled):
    QApplication(argc, argv, GUIenabled) {}
    MTApplication(int & argc, char ** argv, Type type):
    QApplication(argc, argv, type) {}
#ifdef Q_WS_X11
    MTApplication(Display * display, Qt::HANDLE visual = 0, Qt::HANDLE colormap = 0):
    QApplication(display, visual, colormap) {}
    MTApplication(Display * display, int & argc, char ** argv, Qt::HANDLE visual = 0, Qt::HANDLE colormap = 0):
    QApplication(display, argc, argv, visual, colormap) {}
#endif
    void setAppMainWindow(MainWindow * w) { app_main_window = w; }

protected:
    bool event(QEvent * event) {
        switch (event->type()) {
            case QEvent::FileOpen:
                if (app_main_window) {
                    app_main_window->openFile(static_cast<QFileOpenEvent *>(event)->file());
                }
                return true;
            default:
                return this->QApplication::event(event);
        }
    }

private:
    QPointer<MainWindow> app_main_window;
};

#endif // LEAKLOG_MAIN_H
