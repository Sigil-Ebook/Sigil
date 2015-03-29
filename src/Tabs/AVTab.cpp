/************************************************************************
**
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

#include <QtCore/QString>
#include <QtCore/QUrl>
#include <QtWidgets/QLayout>
#include <QtWebKitWidgets/QWebView>
#include "MainUI/MainWindow.h"
#include "Tabs/AVTab.h"
#include "sigil_constants.h"

const QString AUDIO_HTML_BASE =
    "<html>"
    "<head>"
    "<style type=\"text/css\">"
    "body { -webkit-user-select: none; }"
    "audio { display: block; margin-left: auto; margin-right: auto; }"
    "</style>"
    "<body>"
    "<p><audio controls=\"controls\" src=\"%1\"></audio></p>"
    "</body>"
    "</html>";

const QString VIDEO_HTML_BASE =
    "<html>"
    "<head>"
    "<style type=\"text/css\">"
    "body { -webkit-user-select: none; }"
    "video { display: block; margin-left: auto; margin-right: auto; }"
    "</style>"
    "<body>"
    "<p><video controls=\"controls\" width=\"560\" src=\"%1\"></video></p>"
    "</body>"
    "</html>";

AVTab::AVTab(Resource &resource, QWidget *parent)
    : ContentTab(resource, parent),
      m_WebView(new QWebView(this))
{
    m_WebView->setContextMenuPolicy(Qt::NoContextMenu);
    m_WebView->setFocusPolicy(Qt::NoFocus);
    m_WebView->setAcceptDrops(false);
    m_Layout.addWidget(m_WebView);
    ConnectSignalsToSlots();
    RefreshContent();
}

void AVTab::RefreshContent()
{
    MainWindow::clearMemoryCaches();
    QString html;
    const QString path = m_Resource.GetFullPath();
    const QUrl resourceUrl = QUrl::fromLocalFile(path);
    if (m_Resource.Type() == Resource::AudioResourceType) {
        html = AUDIO_HTML_BASE.arg(resourceUrl.toString());
    } else {
        html = VIDEO_HTML_BASE.arg(resourceUrl.toString());
    }
    m_WebView->setHtml(html, resourceUrl);
}

void AVTab::ConnectSignalsToSlots()
{
    connect(&m_Resource, SIGNAL(ResourceUpdatedOnDisk()), this, SLOT(RefreshContent()));
    connect(&m_Resource, SIGNAL(Deleted(Resource)), this, SLOT(Close()));
}

