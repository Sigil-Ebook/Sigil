/************************************************************************
**
**  Copyright (C) 2012 John Schember <john@nachtimwald.com>
**  Copyright (C) 2012 Dave Heiland
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
#include <QtGui/QAction>
#include <QtGui/QMenu>

#include "Tabs/TabBar.h"

TabBar::TabBar(QWidget *parent)
    : QTabBar(parent),
    m_TabManager(parent),
    m_TabIndex(-1)
{
}
 
TabBar::~TabBar()
{
}

void TabBar::mousePressEvent(QMouseEvent *event)
{
    if (event->type() == QEvent::MouseButtonDblClick) {
        emit TabBarClicked();
    }
    else if (event->button() == Qt::RightButton) {
        int tabCount = count();
        if (tabCount <= 1) {
            return;
        }
        for( int i = 0; i < tabCount; i++ ) {
            if (tabRect(i).contains(event->pos())) {
                m_TabIndex = i;
                ShowContextMenu(event, i);
                break;
            }
        }
    }
 
    QTabBar::mousePressEvent(event);
}

void TabBar::ShowContextMenu(QMouseEvent *event, int tab_index)
{
    QMenu *menu = new QMenu();

    QAction *closeOtherTabsAction = new QAction(tr("Close Other Tabs"), menu);
    menu->addAction(closeOtherTabsAction);
    connect(closeOtherTabsAction, SIGNAL(triggered()), this, SLOT(EmitCloseOtherTabs()));

    menu->exec(mapToGlobal(event->pos()));

    delete menu;
}

void TabBar::EmitCloseOtherTabs()
{
    emit CloseOtherTabsRequest(m_TabIndex);
}
