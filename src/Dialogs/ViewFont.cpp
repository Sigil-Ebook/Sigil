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

#include <QToolButton>
#include <QString>
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
#include <QFont>
#include <QRawFont>
#include "ViewEditors/SimplePage.h"
#include "Misc/Utility.h"
#include "Misc/SettingsStore.h"
#include "Dialogs/ViewFont.h"

static QString SETTINGS_GROUP = "view_font";

static const QString FONT_HTML_BASE =
    "<html>"
    "<head>"
    "  <title></title>"
    "  <style>" 
    "    @font-face {"
    "      src: url(%1);"
    "      font-family: \"%2\";"
    "      font-weight: \"%3\";"
    "      font-style: \"%4\";"
    "    }"
    "    body { font-family: \"%2\"; }"
    "  </style>"
    "</head>"
    "<body>"
    "  <h1>%5</h1>"
    "  <p>%6, %7 bytes</p>"
    "  <p>&#160;</p>"
    "  <p>LOWERCASE_LETTERS</p>"
    "  <p>UPPERCASE_LETTERS</p>"
    "  <p>DIGITS_SYMBOLS</p>"
    "  <p>SAMPLE_LINE</p>"
    "  <h6>SAMPLE_LINE</h6>"
    "  <h5>SAMPLE_LINE</h5>"
    "  <h4>SAMPLE_LINE</h4>"
    "  <h3>SAMPLE_LINE</h3>"
    "  <h2>SAMPLE_LINE</h2>"
    "  <h1>SAMPLE_LINE</h1>"
    "</body>"
    "</html>";


ViewFont::ViewFont(QWidget *parent)
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

ViewFont::~ViewFont()
{
    WriteSettings();
}


QSize ViewFont::sizeHint()
{
    return QSize(450,250);
}

void ViewFont::ShowFont(QString path)
{
    m_path = path;
    m_WebView->page()->profile()->clearHttpCache();
    QFileInfo fi(path);
    QString file_name = fi.fileName();
    int file_size = fi.size();
    QString font_name = fi.baseName();
    QRawFont rawfont(path, 16.0);
    QString family_name = rawfont.familyName();
    QString weight_name;
    QString style_name;
    if (rawfont.weight() <  QFont::ExtraLight)      weight_name = "Thin";
    else if (rawfont.weight() <  QFont::Light)      weight_name = "ExtraLight";
    else if (rawfont.weight() <  QFont::Normal)     weight_name = "Light";
    else if (rawfont.weight() <  QFont::Medium)     weight_name = "Normal";
    else if (rawfont.weight() <  QFont::DemiBold)   weight_name = "Medium";
    else if (rawfont.weight() <  QFont::Bold)       weight_name = "DemiBold";
    else if (rawfont.weight() <  QFont::ExtraBold)  weight_name = "Bold";
    else if (rawfont.weight() <  QFont::Black)      weight_name = "ExtraBold";
    else if (rawfont.weight() >= QFont::Black)      weight_name = "Black";
#ifdef Q_OS_WIN32
    if (rawfont.style()      == QFont::StyleItalic)  style_name = "Italic";
    else if (rawfont.style() == QFont::StyleOblique) style_name = "Oblique";
#else
    style_name = " " + rawfont.styleName();
#endif
    QString desc = family_name + " " + style_name;
    const QUrl furl = QUrl::fromLocalFile(path);
    QString html = FONT_HTML_BASE.arg(furl.toString())
                                 .arg(font_name)
	                         .arg(weight_name)
                                 .arg(style_name)
	                         .arg(desc)
	                         .arg(file_name)
                                 .arg(QString::number(file_size));
    // allow translators to control over what the font is displaying
    html = html.replace("LOWERCASE_LETTERS", tr("abcdefghijklmnopqrstuvwxyz"));
    html = html.replace("UPPERCASE_LETTERS", tr("ABCDEFGHIJKLMNOPQRSTUVWXYZ"));
    html = html.replace("DIGITS_SYMBOLS", Utility::EncodeXML(tr("0123456789.:,;(*!?'\\/\")$%^&-+@=_-~><")));
    html = html.replace("SAMPLE_LINE", tr("The quick brown fox jumps over the lazy dog"));
    if (Utility::IsDarkMode()) {
        html = Utility::AddDarkCSS(html);
    }
    m_WebView->page()->setBackgroundColor(Utility::WebViewBackgroundColor());
    m_WebView->setHtml(html, furl);
    QApplication::processEvents();
}

void ViewFont::ReadSettings()
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

void ViewFont::WriteSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    settings.setValue("geometry", saveGeometry());
    settings.endGroup();
}

void ViewFont::ConnectSignalsToSlots()
{
    connect(m_bp, SIGNAL(clicked()), this, SLOT(accept()));
}
