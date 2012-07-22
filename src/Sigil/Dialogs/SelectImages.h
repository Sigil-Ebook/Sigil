/************************************************************************
**
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
#ifndef INSERTIMAGE_H
#define INSERTIMAGE_H

#include <QtGui/QDialog>
#include <QtGui/QStandardItemModel>

#include "ResourceObjects/Resource.h"
#include "Misc/SelectImagePreviewer.h"

#include "ui_SelectImages.h"

class QString;
class QStringList;

class SelectImages : public QDialog
{
    Q_OBJECT

public:
    SelectImages(QString basepath, QList<Resource *> image_resources, QString default_selected_image, QWidget *parent = 0);

    /**
     * Set the list of image resources to display.
     */
    void SetImages();

    /**
     * The image(s) selected in the dialog.
     *
     * @return The filename of the selected image.
     */
    QStringList SelectedImages();

private slots:
    /**
     * Displays a given image in the list in the preview area.
     */

    void IncreaseThumbnailSize();
    void DecreaseThumbnailSize();
    void ReloadPreview();
    void SelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

    void SelectDefaultImage();

    /**
     * Filters the list of displayed images
     */
    void FilterEditTextChangedSlot(const QString &text);

    void WriteSettings();

private:
    void ReadSettings();
    void connectSignalsSlots();

    void SetPreviewImage();

    QString m_Basepath;

    QList<Resource *> m_ImageResources;

    QStandardItemModel *m_SelectImagesModel;

    QStandardItem* GetLastSelectedImageItem();
    QString GetLastSelectedImageName();

    bool m_PreviewLoaded;

    QString m_DefaultSelectedImage;

    int m_ThumbnailSize;

    Ui::SelectImages ui;
};

#endif // INSERTIMAGE_H
