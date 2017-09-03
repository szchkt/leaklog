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

#include "activityeventfilter.h"

#include <QEvent>
#include <QTimer>

ActivityEventFilter::ActivityEventFilter(QObject *parent)
    : QObject(parent)
{
    timer = new QTimer(this);
#ifdef QT_DEBUG
    timer->setInterval(10000);
#else
    timer->setInterval(600000);
#endif
    timer->setSingleShot(true);
    QObject::connect(timer, SIGNAL(timeout()), this, SIGNAL(timeout()));
}

bool ActivityEventFilter::eventFilter(QObject *obj, QEvent *event)
{
    switch (event->type()) {
        case QEvent::MouseButtonPress:
        case QEvent::KeyPress:
            timer->start();
            break;

        default:
            break;
    }

    return QObject::eventFilter(obj, event);
}
