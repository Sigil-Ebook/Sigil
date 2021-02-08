/************************************************************************
**
**  Copyright (C) 2021      Kevin B. Hendricks, Stratford Ontario Canada 
**  Copyright (C) 2011-2012 John Schember <john@nachtimwald.com>
**  Copyright (C) 2012      Dave Heiland
**  Copyright (C) 2009-2011 Strahinja Markovic  <strahinja.markovic@gmail.com>
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


#include "Dialogs/MetaEditorItemDelegate.h"
#include <QChar>
#include <QString>
#include <QComboBox>
#include <QTextEdit>

static const QString _GS = QString(QChar(29));

static const int COL_COMBOBOX = 1;

MetaEditorItemDelegate::MetaEditorItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}


MetaEditorItemDelegate::~MetaEditorItemDelegate()
{
}


QWidget *MetaEditorItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // ComboBox only in designated column
    if (index.column() != COL_COMBOBOX) {
        return QStyledItemDelegate::createEditor(parent, option, index);
    }

    // Combox only for selected fields with specific choice sets
    QModelIndex nindex = index.model()->index(index.row(), 0, index.parent());
    if (!nindex.isValid() || !m_Choices.contains(nindex.data(Qt::EditRole).toString())) {
        return QStyledItemDelegate::createEditor(parent, option, index);
    }
    QStringList choices = m_Choices[nindex.data(Qt::EditRole).toString()];
    if (!choices.isEmpty()) {
        // Create the combobox and populate it
        QComboBox *cb = new QComboBox(parent);
        foreach(QString opt, choices) {
            QStringList parts = opt.split(_GS);
            cb->addItem(parts.at(0), QVariant(parts.at(1)));
        }
        return cb;
    }
    QTextEdit *te = new QTextEdit(parent);
    return te;
}

void MetaEditorItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (QTextEdit *te = qobject_cast<QTextEdit *>(editor)) {
        te->setGeometry(option.rect);
    } else {
        QStyledItemDelegate::updateEditorGeometry(editor, option, index);
    }
    
}


QSize MetaEditorItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.column() != COL_COMBOBOX) {
        return QStyledItemDelegate::sizeHint(option, index);
    }

    QModelIndex nindex = index.model()->index(index.row(), 0, index.parent());
    if (!nindex.isValid() || !m_Choices.contains(nindex.data(Qt::EditRole).toString())) {
        return QStyledItemDelegate::sizeHint(option, index);
    }
    
    QStringList choices = m_Choices[nindex.data(Qt::EditRole).toString()];
    if (!choices.isEmpty()) {
        return QStyledItemDelegate::sizeHint(option, index);
    }
    return QSize(200, 50);
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
    } else if (QTextEdit *te = qobject_cast<QTextEdit *>(editor)) {
        QString currentText = index.data(Qt::EditRole).toString();
        te->insertPlainText(currentText);
    } else {
        QStyledItemDelegate::setEditorData(editor, index);
    }
}


void MetaEditorItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    if (QComboBox *cb = qobject_cast<QComboBox *>(editor)) {
        // save the current text of the combo box as the current value of the item
        model->setData(index, cb->currentText(), Qt::EditRole);
        model->setData(index, cb->currentData(), Qt::ToolTipRole);
    } else if (QTextEdit *te = qobject_cast<QTextEdit *>(editor)) {
        QString atext = te->toPlainText();
        model->setData(index, atext, Qt::EditRole);
        model->setData(index, QVariant(""), Qt::ToolTipRole);
    } else {
        QStyledItemDelegate::setModelData(editor, model, index);
    }
}
