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

#include "mainwindow.h"
#include "global.h"

#include <QSysInfo>

#import <AppKit/AppKit.h>

void MainWindow::macInitUI()
{
    NSView *view = (NSView *)winId();
    view.window.collectionBehavior = NSWindowCollectionBehaviorFullScreenPrimary;
}

int Global::macVersion()
{
    static int version = 0;
    if (!version) {
        NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
        NSDictionary *systemVersion = [NSDictionary dictionaryWithContentsOfFile:@"/System/Library/CoreServices/SystemVersion.plist"];
        NSArray<NSString *> *productVersion = [systemVersion[@"ProductVersion"] componentsSeparatedByString:@"."];
        int majorVersion = productVersion.firstObject.intValue;
        if (majorVersion == 10) {
            version = productVersion[1].intValue + 2;
        } else {
            version = majorVersion + 7;
        }
        [pool release];
    }
    return version;
}
