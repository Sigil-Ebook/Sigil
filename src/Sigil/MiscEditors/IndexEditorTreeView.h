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
#ifndef INDEXEDITORTREEVIEW_H
#define INDEXEDITORTREEVIEW_H

#include <QtGui/QTreeView>
#include <QDragEnterEvent>
#include <QDragMoveEvent>

class IndexEditorTreeView : public QTreeView
{
//     Q_OBJECT

public:
     IndexEditorTreeView(QWidget* parent = 0);
     ~IndexEditorTreeView();

protected:
    QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers);

};

#endif // INDEXEDITORTREEVIEW_H
