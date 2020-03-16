/************************************************************************
**
**  Copyright (C) 2019-2020 Kevin B. Hendricks
**  Copyright (C) 2013      Dave Heiland
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

#include <QString>
#include <QImage>
#include <QFileInfo>
#include <QApplication>
#include <QGuiApplication>
#include <QtWidgets/QLayout>
#include <QtWebEngineWidgets/QWebEngineView>
#include <QtWebEngineWidgets/QWebEngineSettings>
#include <QtWebEngineWidgets/QWebEngineProfile>

#include "MainUI/MainWindow.h"
#include "ResourceObjects/ImageResource.h"
#include "ViewEditors/SimplePage.h"
#include "Misc/Utility.h"
#include "sigil_constants.h"
#include "Dialogs/ViewImage.h"

static QString SETTINGS_GROUP = "view_image";

static const QString IMAGE_HTML_BASE =
    "<html>"
    "<head>"
    "<style type=\"text/css\">"
    "body { -webkit-user-select: none; }"
    "img { display: block; margin-left: auto; margin-right: auto; border-style: solid; border-width: 1px; }"
    "hr { width: 75%; }"
    "div { text-align: center; }"
    "</style>"
    "</head>"
    "<body>"
    "<p><img src=\"%1\" /></p>"
    "<hr />"
    "<div>%2&times;%3px | %4 KB | %5%6</div>"
    "</body>"
    "</html>";


ViewImage::ViewImage(QWidget *parent)
    :
    QDialog(parent)
{
    ui.setupUi(this);
    ui.webView->setPage(new SimplePage(ui.webView));
    ui.webView->setContextMenuPolicy(Qt::NoContextMenu);
    ui.webView->setFocusPolicy(Qt::NoFocus);
    ui.webView->setAcceptDrops(false);
    ReadSettings();
}

ViewImage::~ViewImage()
{
    WriteSettings();
}

void ViewImage::ShowImage(QString path)
{
    ui.webView->page()->profile()->clearHttpCache();
    const QFileInfo fileInfo = QFileInfo(path);
    const double ffsize = fileInfo.size() / 1024.0;
    const QString fsize = QLocale().toString(ffsize, 'f', 2);
    const QImage img(path);
    const QUrl imgUrl = QUrl::fromLocalFile(path);
    QString colors_shades = img.isGrayscale() ? tr("shades") : tr("colors");
    QString grayscale_color = img.isGrayscale() ? tr("Grayscale") : tr("Color");
    QString colorsInfo = "";
    if (img.depth() == 32) {
        colorsInfo = QString(" %1bpp").arg(img.bitPlaneCount());
    } else if (img.depth() > 0) {
        colorsInfo = QString(" %1bpp (%2 %3)").arg(img.bitPlaneCount()).arg(img.colorCount()).arg(colors_shades);
    }
    QString html = IMAGE_HTML_BASE.arg(imgUrl.toString())
	                          .arg(img.width())
                                  .arg(img.height())
                                  .arg(fsize)
                                  .arg(grayscale_color)
                                  .arg(colorsInfo);
    if (Utility::IsDarkMode()) {
	html = Utility::AddDarkCSS(html);
    }
    ui.webView->page()->setBackgroundColor(Utility::WebViewBackgroundColor());
    ui.webView->setHtml(html, imgUrl);
    QApplication::processEvents();
}

void ViewImage::ReadSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
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
    settings.setValue("geometry", saveGeometry());
    settings.endGroup();
}

