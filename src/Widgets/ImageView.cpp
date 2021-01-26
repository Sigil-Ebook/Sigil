/************************************************************************
**
**  Copyright (C) 2019-2021 Kevin B. Hendricks
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
#include <QVBoxLayout>
#include <QApplication>
#include <QGuiApplication>
#include <QPrinter>
#include <QtWidgets/QLayout>
#include <QtWebEngineWidgets/QWebEngineView>
#include <QtWebEngineWidgets/QWebEngineSettings>
#include <QtWebEngineWidgets/QWebEngineProfile>

#include "ViewEditors/SimplePage.h"
#include "Misc/Utility.h"
#include "Widgets/ImageView.h"

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


ImageView::ImageView(QWidget *parent)
    :
    QWidget(parent),
    m_WebView(new QWebEngineView(this)),
    m_layout(new QVBoxLayout(this))
{
    m_WebView->setPage(new SimplePage(m_WebView));
    m_WebView->setContextMenuPolicy(Qt::NoContextMenu);
    m_WebView->setFocusPolicy(Qt::NoFocus);
    m_WebView->setAcceptDrops(false);
    m_WebView->setUrl(QUrl("about:blank"));
    m_layout->addWidget(m_WebView);
}

ImageView::~ImageView()
{
}

void ImageView::ShowImage(QString path)
{
    m_path = path;
    m_WebView->page()->profile()->clearHttpCache();
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
    m_WebView->page()->setBackgroundColor(Utility::WebViewBackgroundColor());
    m_WebView->setHtml(html, imgUrl);
}

void ImageView::ReloadViewer()
{
    QString path = m_path;
    ShowImage(path);
}
