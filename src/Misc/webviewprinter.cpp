/****************************************************************************
**
**  Copyright (C) 2023  Doug Massay
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
****************************************************************************/


#include "Misc/webviewprinter.h"
#include <QEventLoop>
#include <QPrintDialog>
#include <QPrinter>
#include <QPainter>
#include <QFileInfo>
#include <QDir>
#include <QUrl>
#include <QPrintPreviewDialog>
#include <QtWebEngineWidgets>
#include <QtWebEngineCore>
#include <QWebEngineView>
#include <QWebEnginePage>
#include <QStandardPaths>
#include <QDebug>

#define DBG if(0)

WebViewPrinter::WebViewPrinter(QObject *parent)
    : QObject(parent)
{

}

WebViewPrinter::~WebViewPrinter()
{
    if (m_viewprev){
        delete m_viewprev;
        m_viewprev = nullptr;
        m_skipPreview = false;
    }
    DBG qDebug() << "WebViewPrinter destroyed";
}

void WebViewPrinter::setContent(QString filepath, QString text, bool skipPrev)
{
    // destroy previous QWebEngineView to avoid leaking
    if (m_viewprev){
        delete m_viewprev;
        m_viewprev = nullptr;
        DBG qDebug() << "Cleaning up old WebEngineView";
    }
    m_skipPreview = skipPrev;
    m_inPrintPreview = false;

    // Create new (undisplayed) QWebEngineView to which the EPUB's
    // page url is loaded without any potential darkmode injections
    m_viewprev = new ViewPreview(0, false);
    connect(m_viewprev, &ViewPreview::loadFinished, this, &WebViewPrinter::loadFinished);
    m_viewprev->CustomSetDocument(filepath, text);
}

QString WebViewPrinter::getPrintToFilePath(QFileInfo &fi) {
    QString filename = fi.baseName() + ".pdf";
    QString path = QDir(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).filePath(filename);
    return path;
}

void WebViewPrinter::print()
{
    DBG qDebug() << "Skipping Print Preview.";
    QPrinter printer;
#if !defined(Q_OS_WIN32) && !defined(Q_OS_MAC)
    QFileInfo fi = QFileInfo(m_viewprev->page()->url().fileName());
    QString path = getPrintToFilePath(fi);
    DBG qDebug() << "Default Print to PDF file name on Linux: " << path;
    printer.setOutputFileName(path);
#endif
    //m_viewprev->setAttribute(Qt::WA_DontShowOnScreen);
    //m_viewprev->show();
    QPrintDialog dialog(&printer);
    if (dialog.exec() != QDialog::Accepted)
        return;
    printDocument(&printer);

}

void WebViewPrinter::printDocument(QPrinter *printer)
{
    QEventLoop loop;
    bool result;
    auto printCallback = [&](bool success) { result = success; loop.quit(); };
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    m_viewprev->page()->print(printer, std::move(printCallback));
#else
    connect(m_viewprev, &ViewPreview::printFinished, std::move(printCallback)); 
    m_viewprev->print(printer);
#endif    
    loop.exec();
    if (!result) {
        DBG qDebug() << "Could not print document.";
        QPainter painter;
        if (painter.begin(printer)) {
            QFont font = painter.font();
            font.setPixelSize(20);
            painter.setFont(font);
            painter.drawText(QPointF(10,25),
                             QStringLiteral("Could not print document."));

            painter.end();
        }
    }
}

void WebViewPrinter::printPreview()
{
    DBG qDebug() << "Launching Print Preview.";
    if (!m_viewprev->page())
        return;
    if (m_inPrintPreview)
        return;
    m_inPrintPreview = true;
    QPrinter printer;
#if !defined(Q_OS_WIN32) && !defined(Q_OS_MAC)
    QFileInfo fi = QFileInfo(m_viewprev->page()->url().fileName());
    QString path = getPrintToFilePath(fi);
    DBG qDebug() << "Default Print to PDF file name on Linux: " << path;
    printer.setOutputFileName(path);
#endif
    QPrintPreviewDialog preview(&printer, m_viewprev);
    connect(&preview, &QPrintPreviewDialog::paintRequested,
            this, &WebViewPrinter::printDocument);
    preview.exec();
    m_inPrintPreview = false;
}

void WebViewPrinter::loadFinished(bool ok)
{
    if (ok) {
        // load QPrintPreview dialog or go directly to Print dialog
        if (!m_skipPreview) {
            printPreview();
        } else {
            print();
        }
    }
    else {
       DBG  qDebug() << "not successfully loaded.";
    }
}
