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

#pragma once
#ifndef TABBAR_H
#define TABBAR_H

#include <QtGui/QTabBar>

class QContextMenuEvent;

class TabBar : public QTabBar
{
    Q_OBJECT

public:
    TabBar(QWidget *parent = 0);
    ~TabBar();

signals:
    void TabBarClicked();
    void TabBarDoubleClicked();
    void CloseOtherTabsRequest(int tab_index);

protected:
    void mousePressEvent(QMouseEvent *event);

private slots:
    void EmitCloseOtherTabs();

private:
    void ShowContextMenu(QMouseEvent *event, int tab_index);

    QWidget *m_TabManager;

    int m_TabIndex;
};

#endif // TABBAR_H
