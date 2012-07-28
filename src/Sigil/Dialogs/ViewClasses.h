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
#ifndef VIEWCLASSES_H
#define VIEWCLASSES_H

#include <QtCore/QHash>
#include <QtGui/QDialog>
#include <QtGui/QStandardItemModel>
#include <QtCore/QSharedPointer>

#include "ResourceObjects/Resource.h"
#include "BookManipulation/Book.h"

#include "ui_ViewClasses.h"

class QString;
class QStringList;

class ViewClasses : public QDialog
{
    Q_OBJECT

public:
    ViewClasses(QSharedPointer<Book> book, QWidget *parent = 0);

    void SetFiles(int sort_column = 1, Qt::SortOrder sort_order = Qt::AscendingOrder);

    QString SelectedFile();

private slots:
    void Sort(int logicalindex, Qt::SortOrder order);

    void FilterEditTextChangedSlot(const QString &text);

    void WriteSettings();

private:
    void SetSelectedFile();

    void ReadSettings();
    void connectSignalsSlots();

    QSharedPointer< Book > m_Book;

    QStandardItemModel *m_ViewClassesModel;

    QString m_SelectedFile;

    Ui::ViewClasses ui;
};

#endif // VIEWCLASSES_H
