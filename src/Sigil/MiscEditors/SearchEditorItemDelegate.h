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
#ifndef SEARCHEDITORITEMDELEGATE_H
#define SEARCHEDITORITEMDELEGATE_H

#include <QtGui/QStyledItemDelegate>

class SearchEditorItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
 
public:
    SearchEditorItemDelegate(QObject *parent = 0);
    ~SearchEditorItemDelegate();
 
    virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    virtual void setEditorData (QWidget *editor, const QModelIndex &index) const;
    virtual void setModelData (QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
 
};
 
#endif // SEARCHEDITORITEMDELEGATE_H
