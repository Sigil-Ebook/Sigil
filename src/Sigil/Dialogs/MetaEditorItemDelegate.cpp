/************************************************************************
**
**  Copyright (C) 2011, 2012  John Schember <john@nachtimwald.com>
**  Copyright (C) 2012 Dave Heiland
**  Copyright (C) 2009, 2010, 2011  Strahinja Markovic  <strahinja.markovic@gmail.com>
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

#include <QtCore/QFile>

#include "Dialogs/MetaEditorItemDelegate.h"
#include "BookManipulation/Metadata.h"

#include <QComboBox>

static const int COL_COMBOBOX = 3;

MetaEditorItemDelegate::MetaEditorItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}


MetaEditorItemDelegate::~MetaEditorItemDelegate()
{
}


QWidget *MetaEditorItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // ComboBox ony in designated column
    if (index.column() != COL_COMBOBOX) {
        return QStyledItemDelegate::createEditor(parent, option, index);
    }

    // Create the combobox and populate it
    QComboBox *cb = new QComboBox(parent);
    // Add name - all translations done in Metadata for consistency
    cb->addItem(Metadata::Instance().GetText("creator"));
    cb->addItem(Metadata::Instance().GetText("contributor"));
    return cb;
}


void MetaEditorItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if (QComboBox *cb = qobject_cast<QComboBox *>(editor)) {
        // get the index of the text in the combobox that matches the current value of the item
        QString currentText = index.data(Qt::EditRole).toString();
        int cbIndex = cb->findText(currentText);

        // if it is valid, adjust the combobox
        if (cbIndex >= 0) {
            cb->setCurrentIndex(cbIndex);
        }
    } else {
        QStyledItemDelegate::setEditorData(editor, index);
    }
}


void MetaEditorItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    if (QComboBox *cb = qobject_cast<QComboBox *>(editor)) {
        // save the current text of the combo box as the current value of the item
        model->setData(index, cb->currentText(), Qt::EditRole);
    } else {
        QStyledItemDelegate::setModelData(editor, model, index);
    }
}
