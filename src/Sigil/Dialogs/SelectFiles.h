/************************************************************************
**
**
**  Copyright (C) 2012,2013 John Schember <john@nachtimwald.com>
**  Copyright (C) 2012,2013 Dave Heiland
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
#ifndef SELECTFILES_H
#define SELECTFILES_H

#include <QtWidgets/QDialog>
#include <QtGui/QStandardItemModel>

#include "ResourceObjects/Resource.h"

#include "ui_SelectFiles.h"

class QString;
class QStringList;
class QWebView;

class SelectFiles : public QDialog
{
    Q_OBJECT

public:
    SelectFiles(QList<Resource *> image_resources, QString default_selected_image, QWidget *parent = 0);
    ~SelectFiles();

    /**
     * The image(s) selected in the dialog.
     *
     * @return The filename of the selected image.
     */
    QStringList SelectedImages();

    bool IsInsertFromDisk();

public slots:
    /**
     * Set the list of image resources to display.
     */
    void SetImages();
protected:

    void resizeEvent(QResizeEvent *event);

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

    void InsertFromDisk();

    void SplitterMoved(int pos, int index);

private:
    void ReadSettings();
    void connectSignalsSlots();

    void SetPreviewImage();

    QList<Resource *> m_MediaResources;

    QStandardItemModel *m_SelectFilesModel;

    QStandardItem *GetLastSelectedImageItem();
    QString GetLastSelectedImageName();

    bool m_PreviewLoaded;

    QString m_DefaultSelectedImage;

    int m_ThumbnailSize;

    bool m_IsInsertFromDisk;

    QListWidgetItem *m_AllItem;
    QListWidgetItem *m_ImageItem;
    QListWidgetItem *m_VideoItem;
    QListWidgetItem *m_AudioItem;

    QWebView *m_WebView;

    Ui::SelectFiles ui;
};

#endif // SELECTFILES_H
