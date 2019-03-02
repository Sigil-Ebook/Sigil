/************************************************************************
**
**  Copyright (C) 2019 Kevin B. Hendricks, Stratford, Ontario, Canada
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
#ifndef ALLFILESWIDGET_H
#define ALLFILESWIDGET_H

#include <QtCore/QHash>
#include <QtWidgets/QDialog>
#include <QtGui/QStandardItemModel>
#include <QtCore/QSharedPointer>
#include <QtWidgets/QAction>
#include <QtWidgets/QMenu>

#include "ResourceObjects/Resource.h"
#include "BookManipulation/Book.h"
#include "Dialogs/ReportsWidgets/ReportsWidget.h"

#include "ui_ReportsAllFilesWidget.h"

class QString;
class QStringList;

class AllFilesWidget : public ReportsWidget

{
    Q_OBJECT

public:
    AllFilesWidget();
    ~AllFilesWidget();

    void CreateReport(QSharedPointer<Book> book);

    void SetupTable(int sort_column = 1, Qt::SortOrder sort_order = Qt::AscendingOrder);

signals:
    void CloseDialog();
    void DeleteFilesRequest(QStringList);
    void OpenFileRequest(QString, int);

private slots:
    void Sort(int logicalindex, Qt::SortOrder order);

    void FilterEditTextChangedSlot(const QString &text);

    void DoubleClick();

    void Save();

private:
    QString GetType(Resource *resource);
    void ReadSettings();
    void WriteSettings();

    void connectSignalsSlots();

    QList<Resource *> m_AllResources;

    QSharedPointer<Book> m_Book;

    QStandardItemModel *m_ItemModel;

    QString m_LastDirSaved;
    QString m_LastFileSaved;

    Ui::AllFilesWidget ui;
};

#endif // ALLFILESWIDGET_H
