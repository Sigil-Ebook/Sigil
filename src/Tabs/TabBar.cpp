/************************************************************************
**
**  Copyright (C) 2015-2024 Kevin B. Hendricks, Stratford, Ontario, Canada
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
#include <QAction>
#include <QtWidgets/QMenu>
#include <QPointer>

#include "Misc/Utility.h"
#include "Tabs/TabBar.h"

TabBar::TabBar(QWidget *parent)
    : QTabBar(parent),
      m_TabIndex(-1)
{
#if defined(Q_OS_MAC)
    // Qt MacOSX missing tab close icon - https://bugreports.qt.io/browse/QTBUG-61092
    // and prevent the silly show only when cursor is near it that came after
    // having a gui control that only appears if cursor is near is sheer stupidity
    const QString FORCE_TAB_CLOSE_BUTTON = 
        "QTabBar::close-button { "
            "background-image: url(:/dark/closedock-macstyle.svg);"
        "}";
    setStyleSheet(FORCE_TAB_CLOSE_BUTTON);
#endif
    setFocusPolicy(Qt::StrongFocus);
}

void TabBar::mouseDoubleClickEvent(QMouseEvent *event)
{
    emit TabBarDoubleClicked();
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
        emit TabBarClicked();
    }

    QTabBar::mousePressEvent(event);
}

void TabBar::ShowContextMenu(QMouseEvent *event, int tab_index)
{
    QPointer<QMenu> menu = new QMenu();
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
    if (!menu.isNull()) {
        delete menu.data();
    }
}

void TabBar::EmitCloseOtherTabs()
{
    emit CloseOtherTabsRequest(m_TabIndex);
}
