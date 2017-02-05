/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2017 Matus & Michal Tomlein

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

#include "mainwindow.h"
#include "global.h"

#include <QSysInfo>

#import <AppKit/AppKit.h>

void MainWindow::macInitUI()
{
#if MAC_OS_X_VERSION_MAX_ALLOWED >= 1070
    if (QSysInfo::MacintoshVersion > QSysInfo::MV_10_6) {
        NSView *view = (NSView *)winId();
        [[view window] setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];
    }
#endif
}

bool MainWindow::isFullScreen() const
{
#if MAC_OS_X_VERSION_MAX_ALLOWED >= 1070
    if (QSysInfo::MacintoshVersion <= QSysInfo::MV_10_6)
        return QWidget::isFullScreen();
    NSView *view = (NSView *)winId();
    return ([[view window] styleMask] & NSFullScreenWindowMask) == NSFullScreenWindowMask;
#else
    return QWidget::isFullScreen();
#endif
}

void MainWindow::showFullScreen()
{
#if MAC_OS_X_VERSION_MAX_ALLOWED >= 1070
    if (QSysInfo::MacintoshVersion <= QSysInfo::MV_10_6) {
        QWidget::showFullScreen();
        return;
    }
    if (!isVisible())
        show();
    NSView *view = (NSView *)winId();
    [[view window] toggleFullScreen:nil];
#else
    QWidget::showFullScreen();
#endif
}

int Global::macVersion()
{
    static int version = 0;
    if (!version) {
        NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
        NSDictionary *systemVersion = [NSDictionary dictionaryWithContentsOfFile:@"/System/Library/CoreServices/SystemVersion.plist"];
        NSArray *productVersion = [[systemVersion objectForKey:@"ProductVersion"] componentsSeparatedByString:@"."];
        version = [[productVersion objectAtIndex:1] intValue] + QSysInfo::MV_10_0;
        [pool release];
    }
    return version;
}
