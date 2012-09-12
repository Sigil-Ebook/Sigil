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

#include <QtCore/QFile>
#include <QtGui/QFont>

#include "Misc/NumericItem.h"
#include "Misc/SettingsStore.h"
#include "ImageFilesWidget.h"

static const int THUMBNAIL_SIZE = 100;
static const int THUMBNAIL_SIZE_INCREMENT = 50;

static QString SETTINGS_GROUP = "reports_image_files";

ImageFilesWidget::ImageFilesWidget(QList<Resource*> image_resources, QSharedPointer<Book> book)
    :
    m_ImageResources(image_resources),
    m_Book(book),
    m_ItemModel(new QStandardItemModel),
    m_ThumbnailSize(THUMBNAIL_SIZE)
{
    ui.setupUi(this);
    connectSignalsSlots();

    ReadSettings();

    SetupTable();
}

void ImageFilesWidget::SetupTable(int sort_column, Qt::SortOrder sort_order)
{
    m_ItemModel->clear();

    QStringList header;

    header.append(tr("Name" ));
    header.append(tr("File Size (KB)" ));
    header.append(tr("Times Used" ));
    header.append(tr("Width" ));
    header.append(tr("Height" ));
    header.append(tr("Pixels" ));
    header.append(tr("Color" ));
    if (m_ThumbnailSize) {
        header.append(tr("Image" ));
    }

    m_ItemModel->setHorizontalHeaderLabels(header);

    ui.imageTree->setSelectionBehavior(QAbstractItemView::SelectRows);

    ui.imageTree->setModel(m_ItemModel);

    ui.imageTree->header()->setSortIndicatorShown(true);

    QSize icon_size(m_ThumbnailSize, m_ThumbnailSize);
    ui.imageTree->setIconSize(icon_size);

    double total_size = 0;
    int total_links = 0;
    QHash<QString, QStringList> image_html_files_hash = m_Book->GetHTMLFilesUsingImages();

    foreach (Resource *resource, m_ImageResources) {
            QString filepath = "../" + resource->GetRelativePathToOEBPS();
            QString path = resource->GetFullPath();
            QImage image(path);

            QList<QStandardItem *> rowItems;

            // Filename
            QStandardItem *name_item = new QStandardItem();
            name_item->setText(resource->Filename());
            name_item->setToolTip(filepath);
            rowItems << name_item;

            // File Size
            double ffsize = QFile(path).size() / 1024.0;
            total_size += ffsize;
            QString fsize = QLocale().toString(ffsize, 'f', 2);
            NumericItem *size_item = new NumericItem();
            size_item->setText(fsize);
            rowItems << size_item;

            // Times Used
            QStringList image_html_files = image_html_files_hash[filepath];
            total_links += image_html_files.count();
            NumericItem *link_item = new NumericItem();
            link_item->setText(QString::number(image_html_files.count()));
            if (!image_html_files.isEmpty()) {
                link_item->setToolTip(image_html_files.join("\n"));
            }
            rowItems << link_item;

            // Width
            NumericItem *width_item = new NumericItem();
            width_item->setText(QString::number(image.width()));
            rowItems << width_item;

            // Height
            NumericItem *height_item = new NumericItem();
            height_item->setText(QString::number(image.width()));
            rowItems << height_item;

            // Pixels
            NumericItem *pixel_item = new NumericItem();
            pixel_item->setText(QString::number(image.width() * image.height()));
            rowItems << pixel_item;

            // Color
            QStandardItem *color_item = new QStandardItem();
            color_item->setText(image.allGray() ? "Grayscale" : "Color");
            rowItems << color_item;

            // Thumbnail
            if (m_ThumbnailSize) {
                QPixmap pixmap(resource->GetFullPath());
                if (pixmap.height() > m_ThumbnailSize || pixmap.width() > m_ThumbnailSize) {
                    pixmap = pixmap.scaled(QSize(m_ThumbnailSize, m_ThumbnailSize), Qt::KeepAspectRatio);
                }
                QStandardItem *icon_item = new QStandardItem();
                icon_item->setIcon(QIcon(pixmap));
                rowItems << icon_item;
            }

            for (int i = 0; i < rowItems.count(); i++) {
                rowItems[i]->setEditable(false);
            }
            m_ItemModel->appendRow(rowItems);
    }

    // Sort before adding the totals row
    // Since sortIndicator calls this routine, must disconnect/reconnect while resorting
    disconnect (ui.imageTree->header(), SIGNAL(sortIndicatorChanged(int, Qt::SortOrder)), this, SLOT(Sort(int, Qt::SortOrder)));
    ui.imageTree->header()->setSortIndicator(sort_column, sort_order);
    connect (ui.imageTree->header(), SIGNAL(sortIndicatorChanged(int, Qt::SortOrder)), this, SLOT(Sort(int, Qt::SortOrder)));

    // Totals
    NumericItem *nitem;
    QList<QStandardItem *> rowItems;

    // Files
    nitem = new NumericItem();
    nitem->setText(QString::number(m_ImageResources.count()) % tr(" files"));
    rowItems << nitem;

    // File size
    nitem = new NumericItem();
    nitem->setText(QLocale().toString(total_size, 'f', 2) % tr("KB"));
    rowItems << nitem;

    // Links
    nitem = new NumericItem();
    nitem->setText(QString::number(total_links));
    rowItems << nitem;

    // Width placeholder
    nitem = new NumericItem();
    rowItems << nitem;

    // Height placeholder
    nitem = new NumericItem();
    rowItems << nitem;

    // Pixels placeholder 
    nitem = new NumericItem();
    rowItems << nitem;

    // Color placeholder 
    nitem = new NumericItem();
    rowItems << nitem;

    if (m_ThumbnailSize) {
        // Icon placeholder 
        nitem = new NumericItem();
        rowItems << nitem;
    }

    // Add the row in bold
    QFont font = *new QFont();
    font.setWeight(QFont::Bold);
    for (int i = 0; i < rowItems.count(); i++) {
        rowItems[i]->setEditable(false);
        rowItems[i]->setFont(font);
    }
    m_ItemModel->appendRow(rowItems);

    for (int i = 0; i < ui.imageTree->header()->count(); i++) {
        ui.imageTree->resizeColumnToContents(i);
    }
}

void ImageFilesWidget::IncreaseThumbnailSize()
{
    m_ThumbnailSize += THUMBNAIL_SIZE_INCREMENT;
    ui.ThumbnailDecrease->setEnabled(true);
    SetupTable();
}

void ImageFilesWidget::DecreaseThumbnailSize()
{
    m_ThumbnailSize -= THUMBNAIL_SIZE_INCREMENT;
    if (m_ThumbnailSize <= 0) {
        m_ThumbnailSize = 0;
        ui.ThumbnailDecrease->setEnabled(false);
    }
    SetupTable();
}


void ImageFilesWidget::FilterEditTextChangedSlot(const QString &text)
{
    const QString lowercaseText = text.toLower();

    QStandardItem *root_item = m_ItemModel->invisibleRootItem();
    QModelIndex parent_index;

    // Hide rows that don't contain the filter text
    int first_visible_row = -1;
    for (int row = 0; row < root_item->rowCount(); row++) {
        if (text.isEmpty() || root_item->child(row, 0)->text().toLower().contains(lowercaseText)) {
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

void ImageFilesWidget::Sort(int logicalindex, Qt::SortOrder order)
{
    SetupTable(logicalindex, order);
}

void ImageFilesWidget::ReadSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);

    // The thumbnail size
    if (!settings.value("thumbnail_size").toString().isEmpty()) {
        m_ThumbnailSize = settings.value("thumbnail_size").toInt();
        if (m_ThumbnailSize <= 0) {
            ui.ThumbnailDecrease->setEnabled(false);
        }
    }

    settings.endGroup();
}

ReportsWidget::Results ImageFilesWidget::saveSettings()
{
    ReportsWidget::Results results;

    results.filename = "";
    results.line = -1;

    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);

    // The thumbnail size
    settings.setValue("thumbnail_size", m_ThumbnailSize);

    settings.endGroup();

    QString selected_file;

    if (ui.imageTree->selectionModel()->hasSelection()) {
        QModelIndex index = ui.imageTree->selectionModel()->selectedRows(0).first();
        if (index.row() != m_ItemModel->rowCount() - 1) {
            results.filename = m_ItemModel->itemFromIndex(index)->text();
        }
    }

    return results;
}

void ImageFilesWidget::connectSignalsSlots()
{
    connect(ui.leFilter,             SIGNAL(textChanged(QString)),
            this,                    SLOT(FilterEditTextChangedSlot(QString)));
    connect(ui.ThumbnailIncrease,    SIGNAL(clicked()),
            this,                    SLOT(IncreaseThumbnailSize()));
    connect(ui.ThumbnailDecrease,    SIGNAL(clicked()),
            this,                    SLOT(DecreaseThumbnailSize()));
    connect (ui.imageTree->header(), SIGNAL(sortIndicatorChanged(int, Qt::SortOrder)), 
            this,                    SLOT(Sort(int, Qt::SortOrder)));
    connect (ui.imageTree,           SIGNAL(doubleClicked(const QModelIndex &)),
            this,                    SIGNAL(DoubleClick()));
}
