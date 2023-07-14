/************************************************************************
 **
 **  Copyright (C) 2023 Kevin B. Hendricks, Stratford, Ontario Canada
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
#include <QtWebEngineWidgets>
#include <QtWebEngineCore>
#include <QWebEngineProfile>
#include <QWebEngineView>
#include <QGuiApplication>
#include <QApplication>
#include "ViewEditors/SimplePage.h"
#include "Misc/Utility.h"
#include "Misc/WebProfileMgr.h"
#include "Widgets/PdfView.h"

PdfView::PdfView(QWidget *parent)
    : QWidget(parent),
      m_WebView(new QWebEngineView(this)),
      m_layout(new QVBoxLayout(this))
{
    QWebEngineProfile* profile = WebProfileMgr::instance()->GetOneTimeProfile();
    m_WebView->setPage(new SimplePage(profile, m_WebView));
    m_WebView->setContextMenuPolicy(Qt::NoContextMenu);
    m_WebView->setFocusPolicy(Qt::NoFocus);
    m_WebView->setAcceptDrops(false);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0) || QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
    m_WebView->setUrl(QUrl("about:blank"));
#endif
    m_layout->addWidget(m_WebView);
}


PdfView::~PdfView()
{
}


void PdfView::ShowPdf(QString path)
{
    m_path = path;
    m_WebView->page()->profile()->clearHttpCache();
    const QUrl pdfurl = QUrl::fromLocalFile(path);
    QFileInfo fi(path);
    m_WebView->page()->setBackgroundColor(Utility::WebViewBackgroundColor());
    m_WebView->setUrl(pdfurl);
}

void PdfView::ReloadViewer()
{
    QString path = m_path;
    ShowPdf(path);
}
