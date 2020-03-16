/************************************************************************
 **
 **  Copyright (C) 2020 Kevin B. Hendricks
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
#include <QToolButton>
#include <QFileInfo>
#include <QApplication>
#include <QGuiApplication>
#include <QUrl>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QtWebEngineWidgets/QWebEngineProfile>
#include <QtWebEngineWidgets/QWebEngineView>
#include <QGuiApplication>
#include <QApplication>
#include "ViewEditors/SimplePage.h"
#include "Misc/Utility.h"
#include "Misc/SettingsStore.h"
#include "Dialogs/ViewAV.h"

static QString SETTINGS_GROUP = "view_av";

static const QString AUDIO_HTML_BASE =
    "<html>"
    "<head>"
    "<style type=\"text/css\">"
    "body { -webkit-user-select: none; }"
    "audio { display: block; margin-left: auto; margin-right: auto; }"
    "</style>"
    "</head>"
    "<body>"
    "<p><audio controls=\"controls\" src=\"%1\"></audio></p>"
    "</body>"
    "</html>";

static const QString VIDEO_HTML_BASE =
    "<html>"
    "<head>"
    "<style type=\"text/css\">"
    "body { -webkit-user-select: none; }"
    "video { display: block; margin-left: auto; margin-right: auto; }"
    "</style>"
    "</head>"
    "<body>"
    "<p><video controls=\"controls\" width=\"560\" src=\"%1\"></video></p>"
    "</body>"
    "</html>";

static const QStringList AUDIO_EXTENSIONS = QStringList() << "aac" << "m4a" << "mp3" << 
                                                             "mpeg" << "mpg" << "oga" << "ogg";

static const QStringList VIDEO_EXTENSIONS = QStringList() << "m4v"   << "mp4"  << "mov" << 
                                                             "ogv"  << "webm";

ViewAV::ViewAV(QWidget *parent)
    : QDialog(parent),
      m_WebView(new QWebEngineView(this)),
      m_bp(new QToolButton(this)),
      m_layout(new QVBoxLayout(this))
{
    m_WebView->setPage(new SimplePage(m_WebView));
    m_WebView->setContextMenuPolicy(Qt::NoContextMenu);
    m_WebView->setFocusPolicy(Qt::NoFocus);
    m_WebView->setAcceptDrops(false);
    m_WebView->setUrl(QUrl("about:blank"));
    m_layout->addWidget(m_WebView);
    m_bp->setToolTip(tr("Close this window"));
    m_bp->setText(tr("Done"));
    m_bp->setToolButtonStyle(Qt::ToolButtonTextOnly);
    QHBoxLayout* hl = new QHBoxLayout();
    hl->addStretch(0);
    hl->addWidget(m_bp);
    m_layout->addLayout(hl);
    ReadSettings();
    ConnectSignalsToSlots();
}

ViewAV::~ViewAV()
{
    WriteSettings();
}


QSize ViewAV::sizeHint()
{
    return QSize(450,250);
}

void ViewAV::ShowAV(QString path)
{
    m_path = path;
    m_WebView->page()->profile()->clearHttpCache();
    const QUrl avurl = QUrl::fromLocalFile(path);
    QFileInfo fi(path);
    QString ext = fi.suffix().toLower();
    QString html;
    if (AUDIO_EXTENSIONS.contains(ext)) {
        html = AUDIO_HTML_BASE.arg(avurl.toString());
    } else {
        html = VIDEO_HTML_BASE.arg(avurl.toString());
    }
    if (Utility::IsDarkMode()) {
        html = Utility::AddDarkCSS(html);
    }
    m_WebView->page()->setBackgroundColor(Utility::WebViewBackgroundColor());
    m_WebView->setHtml(html, avurl);
    QApplication::processEvents();
}

void ViewAV::ReadSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    QByteArray geometry = settings.value("geometry").toByteArray();
    if (!geometry.isNull()) {
        restoreGeometry(geometry);
    } else {
	resize(sizeHint());
    }
    settings.endGroup();
}

void ViewAV::WriteSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    settings.setValue("geometry", saveGeometry());
    settings.endGroup();
}

void ViewAV::ConnectSignalsToSlots()
{
    connect(m_bp, SIGNAL(clicked()), this, SLOT(accept()));
}
