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

#include "mttabwidget.h"

#include <QTabBar>
#if defined(Q_OS_MACOS) && QT_VERSION >= QT_VERSION_CHECK(6, 12, 0)
#include <QApplication>
#include <QEvent>
#include <QScopedValueRollback>
#include <QStyleOptionTab>

namespace {
class MTSpacedTabBar : public QTabBar
{
public:
    explicit MTSpacedTabBar(QWidget *parent): QTabBar(parent) {}

protected:
    void tabInserted(int index) override
    {
        QTabBar::tabInserted(index);
        if (QWidget *button = tabButton(index, LeftSide))
            button->installEventFilter(this);
        positionCloseButtons();
    }

    void tabLayoutChange() override
    {
        QTabBar::tabLayoutChange();
        positionCloseButtons();
    }

    void paintEvent(QPaintEvent *event) override
    {
        // Scrolling can move tab buttons without calling tabLayoutChange().
        // During a drag, keep Qt's animation positions via the move filter.
        if (!(QApplication::mouseButtons() & Qt::LeftButton))
            positionCloseButtons();
        QTabBar::paintEvent(event);
    }

    bool eventFilter(QObject *object, QEvent *event) override
    {
        if (event->type() == QEvent::Move && !m_positioning) {
            QScopedValueRollback<bool> guard(m_positioning, true);
            auto *button = static_cast<QWidget *>(object);
            button->move(button->pos() + QPoint(isRightToLeft() ? -6 : 6, 0));
        }
        return QTabBar::eventFilter(object, event);
    }

private:
    void positionCloseButtons()
    {
        if (m_positioning)
            return;
        QScopedValueRollback<bool> guard(m_positioning, true);
        for (int i = 0; i < count(); ++i) {
            if (QWidget *button = tabButton(i, LeftSide)) {
                QStyleOptionTab option;
                initStyleOption(&option, i);
                const QRect rect = style()->subElementRect(QStyle::SE_TabBarTabLeftButton, &option, this);
                button->move(rect.topLeft() + QPoint(isRightToLeft() ? -6 : 6, 0));
            }
        }
    }

    bool m_positioning = false;
};
}
#endif

MTTabWidget::MTTabWidget(QWidget *parent): QTabWidget(parent)
{
#if defined(Q_OS_MACOS) && QT_VERSION >= QT_VERSION_CHECK(6, 12, 0)
    setTabBar(new MTSpacedTabBar(this));
#endif
}

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
