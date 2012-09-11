/************************************************************************
**
**  Copyright (C) 2012 Dave Heiland
**  Copyright (C) 2012 John Schember <john@nachtimwald.com>
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
#ifndef CSSFILESWIDGET_H
#define CSSFILESWIDGET_H

#include <QtCore/QSharedPointer>

#include "ResourceObjects/Resource.h"
#include <QtCore/QHash>
#include <QtGui/QDialog>
#include <QtGui/QStandardItemModel>
#include "BookManipulation/Book.h"
#include "ReportsWidget.h"

#include "ui_ReportsCSSFilesWidget.h"

class QString;
class QStringList;

class CSSFilesWidget : public ReportsWidget
{
    Q_OBJECT

public:
    CSSFilesWidget(QList<Resource*> html_resources, QList<Resource*> css_resources, QSharedPointer<Book> book);

    void SetupTable(int sort_column = 1, Qt::SortOrder sort_order = Qt::AscendingOrder);

    QString SelectedFile();

    QString saveSettings();

private slots:
    void Sort(int logicalindex, Qt::SortOrder order);

    void FilterEditTextChangedSlot(const QString &text);

private:
    void connectSignalsSlots();

    QList<Resource*> m_HTMLResources;
    QList<Resource*> m_CSSResources;

    QSharedPointer<Book> m_Book;

    QStandardItemModel *m_ItemModel;

    Ui::CSSFilesWidget ui;
};

#endif // CSSFILESWIDGET_H
