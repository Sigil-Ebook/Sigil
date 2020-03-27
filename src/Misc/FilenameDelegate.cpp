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

void FilenameDelegate::setEditorData(QWidget *editor,
                                    const QModelIndex &index) const
{
    QString value = index.model()->data(index, Qt::EditRole).toString();
    QLineEdit *edit = static_cast<QLineEdit *>(editor);
    QString text = index.model()->data(index, Qt::EditRole).toString();
    text = text.split('/').last();
    edit->setText(text);
    // This nonsense is necessary because QStyledItemDelegate always
    // calls selectAll() last for QLineEdit. Thus canceling any "normal"
    // attempt to setSelection in setEditorData. Also Works around needing
    // to fake a keystroke to get the cursor to appear.
    QObject src;
    // The lambda function is executed using a queued connection
    connect(&src, &QObject::destroyed, edit, [edit, text](){
        //set default selection in the line edit
        int pos = text.lastIndexOf('.');
        if (pos == -1) {
            pos = text.length();
        }
        edit->setSelection(0, pos);
    }, Qt::QueuedConnection);
}

void FilenameDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                   const QModelIndex &index) const
{
    QLineEdit *edit = static_cast<QLineEdit*>(editor);
    if (edit->isModified()) {
        QStyledItemDelegate::setModelData(editor, model, index);
    } else {
        emit edit->editingFinished();
        return;
    }
}

void FilenameDelegate::updateEditorGeometry(QWidget *editor,
                                           const QStyleOptionViewItem &option,
                                           const QModelIndex &) const
{
    editor->setGeometry(option.rect);
}
