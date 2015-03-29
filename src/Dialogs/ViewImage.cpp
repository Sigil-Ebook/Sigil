/************************************************************************
**
**  Copyright (C) 2013 Dave Heiland
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

#include <QtCore/QFileInfo>
#include <QtWidgets/QLayout>
#include <QWebView>
#include <QWebFrame>

#include "MainUI/MainWindow.h"
#include "Dialogs/ViewImage.h"
#include "ResourceObjects/ImageResource.h"
#include "sigil_constants.h"

static QString SETTINGS_GROUP = "view_image";

ViewImage::ViewImage(QWidget *parent)
    :
    QDialog(parent)
{
    ui.setupUi(this);
    ui.webView->setContextMenuPolicy(Qt::NoContextMenu);
    ui.webView->setFocusPolicy(Qt::NoFocus);
    ui.webView->setAcceptDrops(false);
    ui.webView->page()->mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
    ui.webView->page()->mainFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff);

    ReadSettings();
}

ViewImage::~ViewImage()
{
    WriteSettings();
}

void ViewImage::ShowImage(QString path)
{
    MainWindow::clearMemoryCaches();
    const QUrl resourceUrl = QUrl::fromLocalFile(path);
    QString html = IMAGE_HTML_BASE_PREVIEW.arg(resourceUrl.toString());
    ui.webView->setHtml(html, resourceUrl);
}

void ViewImage::ReadSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    // The size of the window and it's full screen status
    QByteArray geometry = settings.value("geometry").toByteArray();

    if (!geometry.isNull()) {
        restoreGeometry(geometry);
    }

    settings.endGroup();
}

void ViewImage::WriteSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    // The size of the window and it's full screen status
    settings.setValue("geometry", saveGeometry());
    settings.endGroup();
}

