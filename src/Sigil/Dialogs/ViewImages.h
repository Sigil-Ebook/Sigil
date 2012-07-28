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
#ifndef VIEWIMAGES_H
#define VIEWIMAGES_H

#include <QtCore/QSharedPointer>

#include "ResourceObjects/Resource.h"
#include <QtCore/QHash>
#include <QtGui/QDialog>
#include <QtGui/QStandardItemModel>
#include "BookManipulation/Book.h"

#include "ui_ViewImages.h"

class QString;
class QStringList;

class ViewImages : public QDialog
{
    Q_OBJECT

public:
    ViewImages(QString basepath, QList<Resource*> image_resources, QSharedPointer<Book> book,  QWidget *parent = 0);

    void SetImages(int sort_column = 1, Qt::SortOrder sort_order = Qt::AscendingOrder);

    QString SelectedFile();

private slots:
    void Sort(int logicalindex, Qt::SortOrder order);

    /**
     * Filters the list of displayed images
     */
    void FilterEditTextChangedSlot(const QString &text);

    void WriteSettings();

    void IncreaseThumbnailSize();
    void DecreaseThumbnailSize();

private:
    void SetSelectedFile();

    void ReadSettings();
    void connectSignalsSlots();

    QString m_Basepath;

    QList<Resource*> m_ImageResources;

    QSharedPointer<Book> m_Book;

    QStandardItemModel *m_ViewImagesModel;

    int m_ThumbnailSize;

    QString m_SelectedFile;

    Ui::ViewImages ui;
};

#endif // VIEWIMAGES_H
