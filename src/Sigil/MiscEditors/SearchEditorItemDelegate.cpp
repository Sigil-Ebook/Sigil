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

#include <QtCore/QFile>
#include <QComboBox>

#include "MiscEditors/SearchEditorItemDelegate.h"
#include "Misc/FindFields.h"

static const int COL_SEARCH_MODE = 4;
static const int COL_LOOK_WHERE = 5;
static const int COL_SEARCH_DIRECTION = 6;

SearchEditorItemDelegate::SearchEditorItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}
 
SearchEditorItemDelegate::~SearchEditorItemDelegate()
{
}
 
QWidget* SearchEditorItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QComboBox *cb = new QComboBox(parent);

    if (index.column() == COL_SEARCH_MODE) {
        QList<FindFields::SearchModePair> mode_pairs = FindFields::instance()->GetSearchModes();
        foreach(FindFields::SearchModePair mode_pair, mode_pairs) {
            cb->addItem(mode_pair.first);
        }
    }
    else if (index.column() == COL_LOOK_WHERE) {
        QList<FindFields::LookWherePair> look_pairs = FindFields::instance()->GetLookWheres();
        foreach(FindFields::LookWherePair look_pair, look_pairs) {
            cb->addItem(look_pair.first);
        }
    }
    else if (index.column() == COL_SEARCH_DIRECTION) {
        QList<FindFields::SearchDirectionPair> direction_pairs = FindFields::instance()->GetSearchDirections();
        foreach(FindFields::SearchDirectionPair direction_pair, direction_pairs) {
            cb->addItem(direction_pair.first);
        }
    }

    return cb;
}
 
void SearchEditorItemDelegate::setEditorData (QWidget *editor, const QModelIndex &index) const
{
    if (QComboBox *cb = qobject_cast<QComboBox *>(editor)) {
        // Get the index of the text in the combobox that matches the current value of the itenm
        QString currentText = index.data().toString();
        int cbIndex = cb->findText(currentText);
        // If it's valid, adjust the combobox
        if (cbIndex >= 0)
            cb->setCurrentIndex(cbIndex);
    } 
    else {
        QStyledItemDelegate::setEditorData(editor, index);
    }
}
 
void SearchEditorItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    if (QComboBox *cb = qobject_cast<QComboBox *>(editor)) {
        model->setData(index, cb->itemData(cb->currentIndex(), Qt::EditRole));
    }
    else {
        QStyledItemDelegate::setModelData(editor, model, index);
    }
}
