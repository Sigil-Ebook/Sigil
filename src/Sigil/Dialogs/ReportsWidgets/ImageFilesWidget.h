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
#ifndef IMAGEFILESWIDGET_H
#define IMAGEFILESWIDGET_H

#include <QtCore/QSharedPointer>
#include <QtGui/QAction>
#include <QtGui/QMenu>

#include "ResourceObjects/Resource.h"
#include <QtCore/QHash>
#include <QtGui/QDialog>
#include <QtGui/QStandardItemModel>
#include "BookManipulation/Book.h"
#include "ReportsWidget.h"

#include "ui_ReportsImageFilesWidget.h"

class QString;
class QStringList;

class ImageFilesWidget : public ReportsWidget
{
    Q_OBJECT

public:
    ImageFilesWidget(QList<Resource*> image_resources, QSharedPointer<Book> book);

    void SetupTable(int sort_column = 1, Qt::SortOrder sort_order = Qt::AscendingOrder);

    QString SelectedFile();

    ReportsWidget::Results saveSettings();

signals:
    void Done();

private slots:
    void OpenContextMenu(const QPoint &point);

    void Sort(int logicalindex, Qt::SortOrder order);

    /**
     * Filters the list of displayed images
     */
    void FilterEditTextChangedSlot(const QString &text);

    void IncreaseThumbnailSize();
    void DecreaseThumbnailSize();

    void Delete();

private:
    void CreateContextMenuActions();
    void SetupContextMenu(const QPoint &point);

    void ReadSettings();
    void connectSignalsSlots();

    QList<Resource*> m_ImageResources;

    QSharedPointer<Book> m_Book;

    QStandardItemModel *m_ItemModel;

    int m_ThumbnailSize;

    QMenu *m_ContextMenu;

    QAction *m_Delete;

    bool m_DeleteFiles;

    Ui::ImageFilesWidget ui;
};

#endif // IMAGEFILESWIDGET_H
