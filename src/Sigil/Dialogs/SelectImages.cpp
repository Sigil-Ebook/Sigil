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

#include "Dialogs/SelectImages.h"
#include "Misc/SettingsStore.h"

static const int COL_NAME = 0;
static const int COL_IMAGE = 1;

static const int THUMBNAIL_SIZE = 100;
static const int THUMBNAIL_SIZE_INCREMENT = 50;

static QString SETTINGS_GROUP = "select_images";

SelectImages::SelectImages(QString basepath, QList<Resource *> image_resources, QString default_selected_image, QWidget *parent) :
    QDialog(parent),
    m_Basepath(basepath),
    m_ImageResources(image_resources),
    m_SelectImagesModel(new QStandardItemModel),
    m_PreviewLoaded(false),
    m_DefaultSelectedImage(default_selected_image),
    m_ThumbnailSize(THUMBNAIL_SIZE)
{
    ui.setupUi(this);

    ReadSettings();

    SetImages();

    connectSignalsSlots();
}

QStringList SelectImages::SelectedImages()
{
    QList<QString> selected_images;

    // Shift-click order is top to bottom regardless of starting position
    // Ctrl-click order is first clicked to last clicked (included shift-clicks stay ordered as is)

    if (ui.imageTree->selectionModel()->hasSelection()) {
        QModelIndexList selected_indexes = ui.imageTree->selectionModel()->selectedRows(0);

        foreach (QModelIndex index, selected_indexes) {
            selected_images.append(m_SelectImagesModel->itemFromIndex(index)->text());
        }
    }

    return selected_images;
}

void SelectImages::SetImages()
{
    m_SelectImagesModel->clear();

    QStringList header;

    header.append(tr("Name" ));
    if (m_ThumbnailSize) {
        header.append(tr("Images"));
    }
    m_SelectImagesModel->setHorizontalHeaderLabels(header);

    ui.imageTree->setSelectionBehavior(QAbstractItemView::SelectRows);

    ui.imageTree->setModel(m_SelectImagesModel);

    QSize icon_size(m_ThumbnailSize, m_ThumbnailSize);
    ui.imageTree->setIconSize(icon_size);

    int row = 0;
    foreach (Resource *resource, m_ImageResources) {
            QString filepath = "../" + resource->GetRelativePathToOEBPS();

            QList<QStandardItem *> rowItems;

            QStandardItem *name_item = new QStandardItem();
            name_item->setText(resource->Filename());
            name_item->setToolTip(filepath);
            name_item->setEditable(false);
            rowItems << name_item;

            if (m_ThumbnailSize) {
                QPixmap pixmap(resource->GetFullPath());
                if (pixmap.height() > m_ThumbnailSize || pixmap.width() > m_ThumbnailSize) {
                    pixmap = pixmap.scaled(QSize(m_ThumbnailSize, m_ThumbnailSize), Qt::KeepAspectRatio);
                }

                QStandardItem *icon_item = new QStandardItem();
                icon_item->setIcon(QIcon(pixmap));
                icon_item->setEditable(false);
                rowItems << icon_item;
            }

            m_SelectImagesModel->appendRow(rowItems);
            row++;
    }

    ui.imageTree->header()->setStretchLastSection(true);
    for (int i = 0; i < ui.imageTree->header()->count(); i++) {
        ui.imageTree->resizeColumnToContents(i);
    }

    FilterEditTextChangedSlot(ui.Filter->text());
    SelectDefaultImage();
}

void SelectImages::SelectDefaultImage()
{
    QStandardItem *root_item = m_SelectImagesModel->invisibleRootItem();
    QModelIndex parent_index;

    for (int row = 0; row < root_item->rowCount(); row++) {
        if (root_item->child(row, COL_NAME)->text() == m_DefaultSelectedImage) {
            ui.imageTree->selectionModel()->select(m_SelectImagesModel->index(row, 0, parent_index), QItemSelectionModel::Select | QItemSelectionModel::Rows);
            ui.imageTree->setCurrentIndex(root_item->child(row, 0)->index());
            SetPreviewImage();
            break;
        }
    }
}
void SelectImages::IncreaseThumbnailSize()
{
    m_ThumbnailSize += THUMBNAIL_SIZE_INCREMENT;
    ui.ThumbnailDecrease->setEnabled(true);
    m_DefaultSelectedImage = GetLastSelectedImageName();
    SetImages();
}

void SelectImages::DecreaseThumbnailSize()
{
    m_ThumbnailSize -= THUMBNAIL_SIZE_INCREMENT;
    if (m_ThumbnailSize <= 0) {
        m_ThumbnailSize = 0;
        ui.ThumbnailDecrease->setEnabled(false);
    }
    m_DefaultSelectedImage = GetLastSelectedImageName();
    SetImages();
}

void SelectImages::ReloadPreview()
{
    // Make sure we don't load when initial painting is resizing
    if (m_PreviewLoaded) {
        SetPreviewImage();
    }
}

void SelectImages::SelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    SetPreviewImage();
}

void SelectImages::SetPreviewImage()
{
    QPixmap(pixmap);

    QStandardItem *item = GetLastSelectedImageItem();

    if (item && !item->text().isEmpty()) {

        pixmap = QPixmap(m_Basepath + item->text());

        // Set size to match window, with a small border
        int width = ui.preview->width() - 10;
        int height = ui.preview->height() - 10;

        // Resize images before saving - only shrink not enlarge
        if (pixmap.height() > height || pixmap.width() > width) {
            pixmap = pixmap.scaled(QSize(width, height), Qt::KeepAspectRatio);
        }
    }

    ui.preview->setPixmap(pixmap);

    m_PreviewLoaded = true;
}

void SelectImages::FilterEditTextChangedSlot(const QString &text)
{
    const QString lowercaseText = text.toLower();

    QStandardItem *root_item = m_SelectImagesModel->invisibleRootItem();
    QModelIndex parent_index;

    // Hide rows that don't contain the filter text
    int first_visible_row = -1;
    for (int row = 0; row < root_item->rowCount(); row++) {
        if (text.isEmpty() || root_item->child(row, COL_NAME)->text().toLower().contains(lowercaseText)) {
            ui.imageTree->setRowHidden(row, parent_index, false);
            if (first_visible_row == -1) {
                first_visible_row = row;
            }
        }
        else {
            ui.imageTree->setRowHidden(row, parent_index, true);
        }
    }

    if (!text.isEmpty() && first_visible_row != -1) {
        // Select the first non-hidden row
        ui.imageTree->setCurrentIndex(root_item->child(first_visible_row, 0)->index());
    }
    else {
        // Clear current and selection, which clears preview image
        ui.imageTree->setCurrentIndex(QModelIndex());
    }
}

QStandardItem* SelectImages::GetLastSelectedImageItem()
{
    QStandardItem *item = NULL;

    if (ui.imageTree->selectionModel()->hasSelection()) {
        QModelIndexList selected_indexes = ui.imageTree->selectionModel()->selectedRows(0);
        if (!selected_indexes.isEmpty()) {
            item = m_SelectImagesModel->itemFromIndex(selected_indexes.last());
        }
    }

    return item;
}

QString SelectImages::GetLastSelectedImageName()
{
    QString selected_entry = "";

    QStandardItem *item = GetLastSelectedImageItem();

    if (item) {
        selected_entry = item->text();
    }

    return selected_entry;
}

void SelectImages::ReadSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);

    // The size of the window and it's full screen status
    QByteArray geometry = settings.value("geometry").toByteArray();

    if (!geometry.isNull()) {
        restoreGeometry(geometry);
    }

    // The position of the splitter handle
    QByteArray splitter_position = settings.value("splitter").toByteArray();

    if (!splitter_position.isNull()) {
        ui.splitter->restoreState(splitter_position);
    }

    // The thumbnail size
    m_ThumbnailSize = settings.value("thumbnail_size").toInt();
    if (m_ThumbnailSize <= 0) {
        ui.ThumbnailDecrease->setEnabled(false);
    }

    settings.endGroup();
}

void SelectImages::WriteSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);

    // The size of the window and it's full screen status
    settings.setValue("geometry", saveGeometry());

    // The position of the splitter handle
    settings.setValue("splitter", ui.splitter->saveState());

    // The thumbnail size
    settings.setValue("thumbnail_size", m_ThumbnailSize);

    settings.endGroup();
}

void SelectImages::connectSignalsSlots()
{
    QItemSelectionModel* selectionModel = ui.imageTree->selectionModel();

    connect(selectionModel,     SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), 
            this,               SLOT(SelectionChanged(const QItemSelection&, const QItemSelection&)));
    connect(ui.imageTree,       SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(accept()));
    connect(this,               SIGNAL(accepted()), this, SLOT(WriteSettings()));
    connect(ui.Filter,          SIGNAL(textChanged(QString)), 
            this,               SLOT(FilterEditTextChangedSlot(QString)));
    connect(ui.ThumbnailIncrease, SIGNAL(clicked()), this, SLOT(IncreaseThumbnailSize()));
    connect(ui.ThumbnailDecrease, SIGNAL(clicked()), this, SLOT(DecreaseThumbnailSize()));
    connect(ui.preview,         SIGNAL(Resized()), this, SLOT(ReloadPreview()));

}
