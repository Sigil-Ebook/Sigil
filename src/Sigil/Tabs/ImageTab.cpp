/************************************************************************
**
**  Copyright (C) 2009, 2010, 2011  Strahinja Markovic  <strahinja.markovic@gmail.com>
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
#include <QtCore/QLocale>
#include <QtCore/QString>
#include <QtCore/QUrl>
#include <QtGui/QLayout>
#include <QtWebKit/QWebView>

#include "Misc/SettingsStore.h"
#include "ResourceObjects/ImageResource.h"
#include "Tabs/ImageTab.h"

const QString IMAGE_HTML_BASE =
        "<html>"
        "<head>"
        "<style type=\"text/css\">"
        "img { display: block; margin-left: auto; margin-right: auto; border-style: solid; border-width: 1px; }"
        "hr { width: 75%; }"
        "div { text-align: center; }"
        "</style>"
        "<body>"
        "<p><img src=\"%1\" /></p>"
        "<hr />"
        "<div>%2x%3px | %4 KB | %5</div>"
        "</body>"
        "</html>";

ImageTab::ImageTab( ImageResource& resource, QWidget *parent )
    :
    ContentTab( resource, parent ),
    m_ImageResource( resource ),
    m_WebView(*new QWebView(this))
{
    QImage img(resource.GetFullPath());
    QString path = resource.GetFullPath();
    double ffsize = QFile(path).size() / 1024.0;
    QString fsize = QLocale().toString(ffsize, 'f', 2);

    QString html = IMAGE_HTML_BASE.arg(path).arg(img.width()).arg(img.height()).arg(fsize).arg(img.allGray() ? "BW" : "Color");
    m_WebView.setHtml(html, QUrl::fromLocalFile(path));

    m_WebView.setContextMenuPolicy(Qt::NoContextMenu);
    
    m_Layout.addWidget( &m_WebView);

    // Set the Zoom factor but be sure no signals are set because of this.
    SettingsStore settings;
    m_CurrentZoomFactor = settings.zoomImage();
    Zoom();

    ConnectSignalsToSlots();
}

float ImageTab::GetZoomFactor() const
{
    return m_CurrentZoomFactor;
}

void ImageTab::SetZoomFactor( float new_zoom_factor )
{
    // Save the zoom for this type.
    SettingsStore settings;
    settings.setZoomImage(new_zoom_factor);
    m_CurrentZoomFactor = new_zoom_factor;

    Zoom();
    emit ZoomFactorChanged( m_CurrentZoomFactor );
}


void ImageTab::UpdateDisplay()
{
    SettingsStore settings;
    float stored_factor = settings.zoomImage();
    if ( stored_factor != m_CurrentZoomFactor )
    {
        m_CurrentZoomFactor = stored_factor;
        Zoom();
    }
}

void ImageTab::ConnectSignalsToSlots()
{
    connect(&m_Resource, SIGNAL(Modified()), &m_WebView, SLOT(reload()));
    connect(&m_Resource, SIGNAL(Deleted(Resource)), this, SLOT(Close()));
}

void ImageTab::Zoom()
{
    m_WebView.setZoomFactor(m_CurrentZoomFactor);
}
