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

#include "FileNameDelegate.h"
#include <QEvent>

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
            event->accept();
            return true;
        }
    }
    return QStyledItemDelegate::eventFilter(object, event);
}
