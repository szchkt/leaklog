/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2026 Matus & Michal Tomlein

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

#include <QDateTime>
#include <QEvent>
#include <QTimer>

static const int task_timer_interval = 10000;

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

    task_timer = new QTimer(this);
    task_timer->setSingleShot(true);
    QObject::connect(task_timer, SIGNAL(timeout()), this, SLOT(taskTimerTimeout()));
}

bool ActivityEventFilter::eventFilter(QObject *obj, QEvent *event)
{
    switch (event->type()) {
        case QEvent::MouseButtonPress:
        case QEvent::KeyPress: {
            qint64 time = QDateTime::currentMSecsSinceEpoch();
            if (time - last_activity_time > task_timer_interval) {
                task_timer->start(0);
            }
            last_activity_time = time;
            timer->start();
            break;
        }

        default:
            break;
    }

    return QObject::eventFilter(obj, event);
}

void ActivityEventFilter::taskTimerTimeout()
{
    int time_since_activity = (int)(QDateTime::currentMSecsSinceEpoch() - last_activity_time);

    emit performPeriodicTasks();

    task_timer->start(qMax(task_timer_interval, time_since_activity));
}
