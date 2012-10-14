/************************************************************************
**
**  Copyright (C) 2012 John Schember <john@nachtimwald.com>
**  Copyright (C) 2012 Dave Heiland
**  Copyright (C) 2012 Grant Drake
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
#ifndef FINDREPLACEQLINEEDIT_H
#define FINDREPLACEQLINEEDIT_H

#include <QtGui/QLineEdit>
#include <QtGui/QStandardItem>

class QContextMenuEvent;
class QSignalMapper;

class FindReplaceQLineEdit : public QLineEdit
{
 
public:
    FindReplaceQLineEdit(QWidget *parent = 0);
    ~FindReplaceQLineEdit();
 
    virtual void contextMenuEvent(QContextMenuEvent *event);

    bool isTokeniseEnabled();
    void setTokeniseEnabled(bool value);

protected:
    bool event(QEvent *e);

private:
    bool CreateMenuEntries(QMenu *parent_menu, QAction *topAction, QStandardItem *entry);

    QWidget *m_FindReplace;
    QSignalMapper *m_searchMapper;
    bool m_tokeniseEnabled;
};

#endif // FINDREPLACEQLINEEDIT_H
