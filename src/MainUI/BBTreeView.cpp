/************************************************************************
**
**  Copyright (C) 2026 Kevin B. Hendricks, Stratford Ontario Canada
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
#include "MainUI/BBTreeView.h"

BBTreeView::BBTreeView(QWidget *parent)
    :
    QTreeView(parent)
{
}


void BBTreeView::startDrag(Qt::DropActions supportedActions)
{
    QModelIndexList indexes = selectionModel()->selectedIndexes();
    if (indexes.isEmpty()) return;

    // Sort the selected indexes by their row number
    std::sort(indexes.begin(), indexes.end(), [](const QModelIndex &a, const QModelIndex &b) {
        return a.row() < b.row();
    });

    // Check if the selection is contiguous
    bool isContiguous = true;
    for (int i = 0; i < indexes.size() - 1; ++i) {
        if (indexes[i + 1].row() != indexes[i].row() + 1) {
            isContiguous = false;
            break;
        }
    }

    // Only start the drag if the selection is contiguous
    if (isContiguous) {
        QTreeView::startDrag(supportedActions);
    }
}
