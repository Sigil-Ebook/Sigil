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

#include "Dialogs/ImageList.h"
#include "Misc/Utility.h"

ImageList::ImageList(QWidget *parent) :
    QDialog(parent)
{
    ui.setupUi(this);
    connectSignalsSlots();
}

void ImageList::setBasepath(const QString &basepath)
{
    m_basepath = basepath;
    if (!m_basepath.endsWith("/")) {
        m_basepath.append("/");
    }
}

void ImageList::setImages(QStringList images)
{
    images.sort();
    foreach (QString image_path, images) {
        ui.imageList->addItem(Utility::ReplaceFirst(m_basepath, "", image_path));
    }
    ui.imageList->setCurrentRow(0);
}

QString ImageList::selectedImage()
{
    QList<QListWidgetItem *> selected_images = ui.imageList->selectedItems();
    if (selected_images.isEmpty()) {
        return QString();
    }
    return selected_images.at(0)->text();
}

void ImageList::loadPreview(QListWidgetItem *current, QListWidgetItem *previous)
{
    Q_UNUSED(previous);

    ui.preview->setPixmap(QPixmap(m_basepath + current->text()));
}

void ImageList::connectSignalsSlots()
{
    connect(ui.imageList, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), this, SLOT(loadPreview(QListWidgetItem*, QListWidgetItem*)));
}
