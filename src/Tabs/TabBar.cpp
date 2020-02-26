/************************************************************************
**
**  Copyright (C) 2015-2020 Kevin B. Hendricks, Stratford, Ontario, Canada
**  Copyright (C) 2020      Doug Massay
**  Copyright (C) 2012      John Schember <john@nachtimwald.com>
**  Copyright (C) 2012      Dave Heiland
**
**  This file is part of Sigil.
**
**  Sigil is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  Sigil is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with Sigil.  If not, see <http://www.gnu.org/licenses/>.
**
*************************************************************************/

#include <QtGui/QContextMenuEvent>
#include <QtWidgets/QAction>
#include <QtWidgets/QMenu>
#include <QTimer>
#include <QDebug>

#include "Misc/Utility.h"
#include "Tabs/TabBar.h"

TabBar::TabBar(QWidget *parent)
    : QTabBar(parent),
      m_TabIndex(-1),
      is_ok_to_move(false)
{
#if defined(Q_OS_MAC)
    // work around Qt MacOSX bug missing tab close icons
    // see:  https://bugreports.qt.io/browse/QTBUG-61092
    // still broken in document mode in Qt.5.12.2 !!!!
    const QString FORCE_TAB_CLOSE_BUTTON = 
        "QTabBar::close-button { "
            "background-image: url(:/qt-project.org/styles/commonstyle/images/standardbutton-closetab-16.png);"
        "}"
        "QTabBar::close-button:hover { "
            "background-image: url(:/qt-project.org/styles/commonstyle/images/standardbutton-closetab-hover-16.png);"
        "}";
    setStyleSheet(FORCE_TAB_CLOSE_BUTTON);
#endif
    m_MoveDelay = new QTimer(this);
    m_MoveDelay->setInterval(250);
    m_MoveDelay->setSingleShot(true);
    connect(m_MoveDelay, SIGNAL(timeout()), this, SLOT(processDelayTimer()));
}

void TabBar::mouseDoubleClickEvent(QMouseEvent *event)
{
    emit TabBarDoubleClicked();
}

void TabBar::mouseMoveEvent(QMouseEvent *event)
{
    // If timer hasn't expired, block the mouse move (with button down) event.
    // This is being done to prevent the "dancing tab" problem when clicking
    // on an inactive tab without the mouse being absolutely still.
    // event->button() reports Qt::NoButton when moving with mouse button down
    qDebug() << "Mouse button " << event->button();
    if (!is_ok_to_move) {
        qDebug() << "Timer left: " << m_MoveDelay->remainingTime();
        return;
    }
    QTabBar::mouseMoveEvent(event);
}

void TabBar::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        int tabCount = count();

        if (tabCount <= 1) {
            return;
        }

        for (int i = 0; i < tabCount; i++) {
            if (tabRect(i).contains(event->pos())) {
                m_TabIndex = i;
                ShowContextMenu(event, i);
                break;
            }
        }
    } else if (event->button() == Qt::LeftButton) {
        // Set the ok_to_move flag to false and start .25 second delay timer
        qDebug() << "Qt::LeftButton pressed, delay timer started";
        is_ok_to_move = false;
        m_MoveDelay->start();
        emit TabBarClicked();
    }

    QTabBar::mousePressEvent(event);
}

void TabBar::ShowContextMenu(QMouseEvent *event, int tab_index)
{
    QMenu *menu = new QMenu();
    QAction *closeOtherTabsAction = new QAction(tr("Close Other Tabs"), menu);
    menu->addAction(closeOtherTabsAction);
    connect(closeOtherTabsAction, SIGNAL(triggered()), this, SLOT(EmitCloseOtherTabs()));
    QPoint p;
    p = mapToGlobal(event->pos());
#ifdef Q_OS_WIN32
    // Relocate the context menu slightly down and right to prevent "automatic" action 
    // highlight on Windows, which then closes all other tabs when the mouse is released.
    p.setX(p.x() + 2);
    p.setY(p.y() + 4);
#endif
    menu->exec(p);
    delete menu;
}

void TabBar::EmitCloseOtherTabs()
{
    emit CloseOtherTabsRequest(m_TabIndex);
}

void TabBar::processDelayTimer()
{
    // Allow the mouse movement (with button down) to happen normally
    qDebug() << "Timer elapsed. Mouse movement allowed.";
    is_ok_to_move = true;
}
