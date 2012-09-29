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

#include <QHeaderView>

#include "MiscEditors/SearchEditorTreeView.h"

SearchEditorTreeView::SearchEditorTreeView(QWidget* parent)
 : QTreeView(parent)
{
    setAcceptDrops(true);
    setDropIndicatorShown(true);
    setDragDropMode(QAbstractItemView::InternalMove);
    setAutoScroll(true);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSortingEnabled(false);
    setTabKeyNavigation(true);
    setAutoExpandDelay(200);
}

SearchEditorTreeView::~SearchEditorTreeView()
{
}
 
void SearchEditorTreeView::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->source() == this) {
        event->setDropAction(Qt::MoveAction);
        event->accept();
    }
    else {
        event->ignore();
    }
}

void SearchEditorTreeView::dragMoveEvent(QDragMoveEvent *event)
{
    // Allow scrolling to work
    QTreeView::dragMoveEvent(event);

    event->accept();
}

QModelIndex SearchEditorTreeView::moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
{
    if (cursorAction == QAbstractItemView::MoveNext) {
        QModelIndex index = currentIndex();

        // Only the first column of a group is editable
        if (!model()->data(index.sibling(index.row(), 0), Qt::UserRole + 1).toBool()) {
            // Move to next column in row if there is one
            if (index.column() < (header()->count() - 1)) {
                return model()->index(index.row(), index.column() + 1, index.parent());
            }
        }

        // Reset to first column in row so that default moveCursor moves it down one
        if (indexBelow(index).isValid()) {
            setCurrentIndex(model()->index(index.row(), 0, index.parent()));
        }
    }
    else if (cursorAction == QAbstractItemView::MovePrevious) {
        QModelIndex index = currentIndex();

        // Only the first column of a group is editable
        if (!model()->data(index.sibling(index.row(), 0), Qt::UserRole + 1).toBool()) {
            // Move to previous column in row if there is one
            if (index.column() > 0) {
                return model()->index(index.row(), index.column() - 1, index.parent());
            }
        }

        if (indexAbove(index).isValid()) {
            // If row above is a group always reset to first column otherwise last column
            if (model()->data(indexAbove(index).sibling(indexAbove(index).row(), 0), Qt::UserRole + 1).toBool()) {
                setCurrentIndex(model()->index(index.row(), 0, index.parent()));
            }
            else {
                setCurrentIndex(model()->index(index.row(), header()->count() - 1, index.parent()));
            }
        }
    }
 
    return QTreeView::moveCursor(cursorAction, modifiers);
}
