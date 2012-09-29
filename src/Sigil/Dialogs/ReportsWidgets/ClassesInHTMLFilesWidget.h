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
#ifndef CLASSESINHTMLFILESWIDGET_H
#define CLASSESINHTMLFILESWIDGET_H

#include <QtCore/QHash>
#include <QtGui/QDialog>
#include <QtGui/QStandardItemModel>
#include <QtCore/QSharedPointer>

#include "ResourceObjects/Resource.h"
#include "BookManipulation/Book.h"
#include "BookManipulation/BookReports.h"
#include "ReportsWidget.h"

#include "ui_ReportsClassesInHTMLFilesWidget.h"

class QString;
class QStringList;

class ClassesInHTMLFilesWidget : public ReportsWidget
{
    Q_OBJECT

public:
    ClassesInHTMLFilesWidget(QList<Resource *> html_resources, QList<Resource *> css_resources, QSharedPointer<Book> book);

    ReportsWidget::Results saveSettings();

signals:
    void Done();

private slots:
    void FilterEditTextChangedSlot(const QString &text);

private:
    void connectSignalsSlots();

    void SetupTable();
    void AddTableData(QList<BookReports::StyleData *> html_classes_usage);

    QList<Resource *> m_HTMLResources;
    QList<Resource *> m_CSSResources;

    QSharedPointer< Book > m_Book;

    QStandardItemModel *m_ItemModel;

    Ui::ClassesInHTMLFilesWidget ui;
};

#endif // CLASSESINHTMLFILESWIDGET_H
