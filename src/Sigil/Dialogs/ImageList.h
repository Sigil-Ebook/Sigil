/************************************************************************
**
**  Copyright (C) 2011 John Schember <john@nachtimwald.com>
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
#ifndef IMAGELIST_H
#define IMAGELIST_H

#include <QDialog>
#include "ui_ImageList.h"

class QString;
class QStringList;

class ImageList : public QDialog
{
    Q_OBJECT

public:
    ImageList(QWidget *parent=0);

    /**
     * Set the base path for all images.
     *
     * We only want to display the file name for each image not the full
     * path. We store the path so we can reconstruct the full path for preview.
     * This only works due to the normalization of the EPUB file structure
     * that takes place when opening a document. In the future if normalization
     * does not take place we will need to store the full path for each item.
     *
     * @param basepath The base path all images are located at.
     */
    void setBasepath(const QString &basepath);

    /**
     * A list of images in the file.
     *
     * The list should be full image paths.
     *
     * @param images The image paths.
     */
    void setImages(QStringList images);

    /**
     * The image selected in the dialog.
     *
     * @return The filename of the selected image.
     */
    QString selectedImage();

private slots:
    /**
     * Displays a given image in the list in the preview area.
     */
    void loadPreview(QListWidgetItem *current, QListWidgetItem *previous);

private:
    void connectSignalsSlots();

    QString m_basepath;
    Ui::ImageList ui;
};

#endif // IMAGELIST_H
