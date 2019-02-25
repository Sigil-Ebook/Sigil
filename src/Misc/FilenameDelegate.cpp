/************************************************************************
**
**  Copyright (C) 2011 John Schember <john@nachtimwald.com>
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

#include <QtCore/QEvent>
#include <QKeyEvent>
#include <QtWidgets/QLineEdit>
#include <QApplication>

#include "Misc/FilenameDelegate.h"

FilenameDelegate::FilenameDelegate(QWidget *parent)
    : QStyledItemDelegate(parent)
{
}

bool FilenameDelegate::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::FocusIn) {
        if (QLineEdit *edit = qobject_cast<QLineEdit *>(object)) {
            QString text = edit->text();
            int pos = text.lastIndexOf('.');

            if (pos == -1) {
                pos = text.length();
            }

            edit->setSelection(0, pos);

	    // Due to bug introduced into Qt sometime after version 5.6
	    // the cursor is made not visibile whenever a qlinedit
	    // has a selection when first focused/selected.
	    // No mouse click will cause the cursor to appear, only a key release event 
	    // for some insane reason. So create a fake shift key press/release event that
	    // will not change the cursor position but will make the cursor visible after a
	    // mouse event deselects things.
	    QApplication::postEvent(edit, new QKeyEvent(QEvent::KeyPress, Qt::Key_Shift, Qt::NoModifier));
	    QApplication::postEvent(edit, new QKeyEvent(QEvent::KeyRelease, Qt::Key_Shift, Qt::NoModifier));
	    QApplication::processEvents();			 
            event->accept();
            return true;
        }
    }

    return QStyledItemDelegate::eventFilter(object, event);
}
