/************************************************************************
 **
 **  Copyright (C) 2020-2021 Kevin B. Hendricks
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
#include <QFileInfo>
#include <QApplication>
#include <QGuiApplication>
#include <QUrl>
#include <QVBoxLayout>
#include <QtWebEngineWidgets/QWebEngineProfile>
#include <QtWebEngineWidgets/QWebEngineView>
#include <QGuiApplication>
#include <QApplication>
#include <QFont>
#include <QRawFont>
#include <QDebug>

#include "ViewEditors/SimplePage.h"
#include "Misc/Utility.h"
#include "Widgets/FontView.h"

static const QString FONT_HTML_BASE =
    "<html>"
    "<head>"
    "  <title></title>"
    "  <style>" 
    "    @font-face {"
    "      src: url(\"%1\");"
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


FontView::FontView(QWidget *parent)
    : QWidget(parent),
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

FontView::~FontView()
{
}

void FontView::ShowFont(QString path)
{
    m_path = path;
    m_WebView->page()->profile()->clearHttpCache();
    QFileInfo fi(path);
    QString file_name = fi.fileName();
    int file_size = fi.size();
    QString font_name = fi.baseName();
    QRawFont rawfont(path, 16.0);
    QString family_name = rawfont.familyName();
    QString desc = family_name;
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
    if (!desc.isEmpty()) {
        if (desc.contains(weight_name)) weight_name = "";
        if (!weight_name.isEmpty()) desc = desc + " " + weight_name;
    }

    if (rawfont.style()      == QFont::StyleItalic)  style_name = "Italic";
    else if (rawfont.style() == QFont::StyleOblique) style_name = "Oblique";

    if (!desc.isEmpty()) {
        if (!style_name.isEmpty()) desc = desc + " " + style_name;
    } else desc = tr("No reliable font data");

    const QUrl furl = QUrl::fromLocalFile(path);
    QString html = FONT_HTML_BASE.arg(furl.toEncoded().constData())
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
}

void FontView::ReloadViewer()
{
    QString path = m_path;
    ShowFont(path);
}
