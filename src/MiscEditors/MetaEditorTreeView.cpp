/************************************************************************
**
**  Copyright (C) 2025 Kevin B. Hendricks, Stratford Ontario Canada
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

#include <QModelIndex>
#include "MiscEditors/MetaEditorTreeView.h"

MetaEditorTreeView::MetaEditorTreeView(QWidget *parent)
    :
    QTreeView(parent)
{
}

void MetaEditorTreeView::keyPressEvent(QKeyEvent * event) {
      QModelIndex current = currentIndex();
      if (current.isValid()) {
          switch (event->key()) {
            case Qt::Key_Right: {
	      if (isExpanded(current) || (current.parent() != QModelIndex())) {
                  QModelIndex next = current.sibling(current.row(), current.column()+1);
                  if (next.isValid()) {
		      selectionModel()->setCurrentIndex(next, QItemSelectionModel::ClearAndSelect);
		  }
	      } else {
	          expand(current);
	      }
	      event->accept();
	      return;
              break;
            }
            case Qt::Key_Left: {
	      if (!isExpanded(current) || (current.parent() != QModelIndex())) {
                  QModelIndex next = current.sibling(current.row(), current.column()-1);
                  if (next.isValid()) {
		      selectionModel()->setCurrentIndex(next, QItemSelectionModel::ClearAndSelect);
		  }
              } else {
		 collapse(current);
	      }
	      event->accept();
	      return;
	      break;
            }
	  }
      }
      QTreeView::keyPressEvent(event);
}

