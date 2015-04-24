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
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QPushButton>

#include "sigil_exception.h"
#include "BookManipulation/FolderKeeper.h"
#include "Dialogs/ReportsWidgets/ImageFilesWidget.h"
#include "Misc/NumericItem.h"
#include "Misc/SettingsStore.h"
#include "Misc/Utility.h"
#include "ResourceObjects/ImageResource.h"
#include "ResourceObjects/SVGResource.h"

static const int THUMBNAIL_SIZE = 100;
static const int THUMBNAIL_SIZE_INCREMENT = 50;

static const QString SETTINGS_GROUP = "reports";
static const QString DEFAULT_REPORT_FILE = "ImageFilesReport.csv";

ImageFilesWidget::ImageFilesWidget()
    :
    m_ItemModel(new QStandardItemModel),
    m_ThumbnailSize(THUMBNAIL_SIZE),
    m_ContextMenu(new QMenu(this)),
    m_LastDirSaved(QString()),
    m_LastFileSaved(QString())
{
    ui.setupUi(this);
    ui.fileTree->setContextMenuPolicy(Qt::CustomContextMenu);
    CreateContextMenuActions();
    connectSignalsSlots();
    ReadSettings();
}

void ImageFilesWidget::CreateReport(QSharedPointer<Book> book)
{
    m_Book = book;
    m_AllImageResources.clear();
    QList<ImageResource *> image_resources = m_Book->GetFolderKeeper()->GetResourceTypeList<ImageResource>(false);
    QList<SVGResource *> svg_resources = m_Book->GetFolderKeeper()->GetResourceTypeList<SVGResource>(false);
    // Images actually consist of 2 resource types ImageResource and SVGResource
    foreach(ImageResource * image_resource, image_resources) {
        m_AllImageResources.append(image_resource);
    }
    foreach(SVGResource * svg_resource, svg_resources) {
        m_AllImageResources.append(svg_resource);
    }
    SetupTable();
}

void ImageFilesWidget::SetupTable(int sort_column, Qt::SortOrder sort_order)
{
    m_ItemModel->clear();
    QStringList header;
    header.append(tr("Name"));
    header.append(tr("File Size (KB)"));
    header.append(tr("Times Used"));
    header.append(tr("Width"));
    header.append(tr("Height"));
    header.append(tr("Pixels"));
    header.append(tr("Color"));

    if (m_ThumbnailSize) {
        header.append(tr("Image"));
    }

    m_ItemModel->setHorizontalHeaderLabels(header);
    ui.fileTree->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui.fileTree->setModel(m_ItemModel);
    ui.fileTree->header()->setSortIndicatorShown(true);
    QSize icon_size(m_ThumbnailSize, m_ThumbnailSize);
    ui.fileTree->setIconSize(icon_size);
    double total_size = 0;
    int total_links = 0;
    QHash<QString, QStringList> image_html_files_hash = m_Book->GetHTMLFilesUsingImages();
    foreach(Resource * resource, m_AllImageResources) {
        QString filepath = "../" + resource->GetRelativePathToOEBPS();
        QString path = resource->GetFullPath();
        QImage image(path);
        QList<QStandardItem *> rowItems;
        // Filename
        QStandardItem *name_item = new QStandardItem();
        name_item->setText(resource->Filename());
        name_item->setToolTip(filepath);
        name_item->setData(filepath);
        rowItems << name_item;
        // File Size
        double ffsize = QFile(path).size() / 1024.0;
        total_size += ffsize;
        QString fsize = QString::number(ffsize, 'f', 2);
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
        height_item->setText(QString::number(image.height()));
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
    disconnect(ui.fileTree->header(), SIGNAL(sortIndicatorChanged(int, Qt::SortOrder)), this, SLOT(Sort(int, Qt::SortOrder)));
    ui.fileTree->header()->setSortIndicator(sort_column, sort_order);
    connect(ui.fileTree->header(), SIGNAL(sortIndicatorChanged(int, Qt::SortOrder)), this, SLOT(Sort(int, Qt::SortOrder)));
    // Totals
    NumericItem *nitem;
    QList<QStandardItem *> rowItems;
    // Files
    nitem = new NumericItem();
    nitem->setText(QString::number(m_AllImageResources.count()) % tr(" files"));
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

    for (int i = 0; i < ui.fileTree->header()->count(); i++) {
        ui.fileTree->resizeColumnToContents(i);
    }
}

void ImageFilesWidget::IncreaseThumbnailSize()
{
    m_ThumbnailSize += THUMBNAIL_SIZE_INCREMENT;
    ui.ThumbnailDecrease->setEnabled(true);
    SetupTable();
    WriteSettings();
}

void ImageFilesWidget::DecreaseThumbnailSize()
{
    m_ThumbnailSize -= THUMBNAIL_SIZE_INCREMENT;

    if (m_ThumbnailSize <= 0) {
        m_ThumbnailSize = 0;
        ui.ThumbnailDecrease->setEnabled(false);
    }

    SetupTable();
    WriteSettings();
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
            ui.fileTree->setRowHidden(row, parent_index, false);

            if (first_visible_row == -1) {
                first_visible_row = row;
            }
        } else {
            ui.fileTree->setRowHidden(row, parent_index, true);
        }
    }

    if (!text.isEmpty() && first_visible_row != -1) {
        // Select the first non-hidden row
        ui.fileTree->setCurrentIndex(root_item->child(first_visible_row, 0)->index());
    } else {
        // Clear current and selection, which clears preview image
        ui.fileTree->setCurrentIndex(QModelIndex());
    }
}

void ImageFilesWidget::Sort(int logicalindex, Qt::SortOrder order)
{
    SetupTable(logicalindex, order);
}

void ImageFilesWidget::Save()
{
    QString report_info;
    QString row_text;

    // Get headings
    for (int col = 0; col < ui.fileTree->header()->count(); col++) {
        QStandardItem *item = m_ItemModel->horizontalHeaderItem(col);
        QString text = "";
        if (item) {
            text = item->text();
        }
        if (col == 0) {
            row_text.append(text);
        } else {
            row_text.append("," % text);
        }
    }

    report_info.append(row_text % "\n");

    // Get data from table
    for (int row = 0; row < m_ItemModel->rowCount(); row++) {
        row_text = "";

        for (int col = 0; col < ui.fileTree->header()->count(); col++) {
            QStandardItem *item = m_ItemModel->item(row, col);
            QString text = "";
            if (item) {
                text = item->text();
            }
            if (col == 0) {
                row_text.append(text);
            } else {
                row_text.append("," % text);
            }
        }

        report_info.append(row_text % "\n");
    }

    // Check if another report changed the default directory
    ReadSettings();
    // Save the file
    QString filter_string = "*.csv;;*.txt;;*.*";
    QString default_filter = "";
    QString save_path = m_LastDirSaved + "/" + m_LastFileSaved;
    QString destination = QFileDialog::getSaveFileName(this,
                          tr("Save Report As Comma Separated File"),
                          save_path,
                          filter_string,
                          &default_filter
                                                      );

    if (destination.isEmpty()) {
        return;
    }

    try {
        Utility::WriteUnicodeTextFile(report_info, destination);
    } catch (CannotOpenFile) {
        QMessageBox::warning(this, tr("Sigil"), tr("Cannot save report file."));
    }

    m_LastDirSaved = QFileInfo(destination).absolutePath();
    m_LastFileSaved = QFileInfo(destination).fileName();
    WriteSettings();
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

    // Last file open
    m_LastDirSaved = settings.value("last_dir_saved").toString();
    m_LastFileSaved = settings.value("last_file_saved_image_files").toString();

    if (m_LastFileSaved.isEmpty()) {
        m_LastFileSaved = DEFAULT_REPORT_FILE;
    }

    settings.endGroup();
}

void ImageFilesWidget::WriteSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    // The thumbnail size
    settings.setValue("thumbnail_size", m_ThumbnailSize);
    // Last file open
    settings.setValue("last_dir_saved", m_LastDirSaved);
    settings.setValue("last_file_saved_image_files", m_LastFileSaved);
    settings.endGroup();
}

void ImageFilesWidget::DoubleClick()
{
    QModelIndex index = ui.fileTree->selectionModel()->selectedRows(0).first();

    if (index.row() != m_ItemModel->rowCount() - 1) {
        QString filename = m_ItemModel->itemFromIndex(index)->data().toString();
        emit FindTextInTags(filename);
    }
}

void ImageFilesWidget::Delete()
{
    QStringList files_to_delete;

    if (ui.fileTree->selectionModel()->hasSelection()) {
        foreach(QModelIndex index, ui.fileTree->selectionModel()->selectedRows(0)) {
            files_to_delete.append(m_ItemModel->itemFromIndex(index)->text());
        }
    }

    emit DeleteFilesRequest(files_to_delete);
}


void ImageFilesWidget::CreateContextMenuActions()
{
    m_Delete    = new QAction(tr("Delete From Book") + "...",     this);
    m_Delete->setShortcut(QKeySequence::Delete);
    // Has to be added to the dialog itself for the keyboard shortcut to work.
    addAction(m_Delete);
}

void ImageFilesWidget::OpenContextMenu(const QPoint &point)
{
    SetupContextMenu(point);
    m_ContextMenu->exec(ui.fileTree->viewport()->mapToGlobal(point));
    m_ContextMenu->clear();
}

void ImageFilesWidget::SetupContextMenu(const QPoint &point)
{
    m_ContextMenu->addAction(m_Delete);
    // We do not enable the delete option if no rows selected or the totals row is selected.
    m_Delete->setEnabled(ui.fileTree->selectionModel()->selectedRows().count() > 0);
    int last_row = ui.fileTree->model()->rowCount() - 1;

    if (ui.fileTree->selectionModel()->isRowSelected(last_row, QModelIndex())) {
        m_Delete->setEnabled(false);
    }
}

void ImageFilesWidget::connectSignalsSlots()
{
    connect(ui.leFilter,             SIGNAL(textChanged(QString)),
            this,                    SLOT(FilterEditTextChangedSlot(QString)));
    connect(ui.fileTree, SIGNAL(doubleClicked(const QModelIndex &)),
            this,         SLOT(DoubleClick()));
    connect(ui.ThumbnailIncrease,    SIGNAL(clicked()),
            this,                    SLOT(IncreaseThumbnailSize()));
    connect(ui.ThumbnailDecrease,    SIGNAL(clicked()),
            this,                    SLOT(DecreaseThumbnailSize()));
    connect(ui.fileTree->header(), SIGNAL(sortIndicatorChanged(int, Qt::SortOrder)),
            this,                    SLOT(Sort(int, Qt::SortOrder)));
    connect(ui.fileTree,  SIGNAL(customContextMenuRequested(const QPoint &)),
            this,         SLOT(OpenContextMenu(const QPoint &)));
    connect(m_Delete,     SIGNAL(triggered()), this, SLOT(Delete()));
    connect(ui.buttonBox->button(QDialogButtonBox::Close), SIGNAL(clicked()), this, SIGNAL(CloseDialog()));
    connect(ui.buttonBox->button(QDialogButtonBox::Save), SIGNAL(clicked()), this, SLOT(Save()));
}
