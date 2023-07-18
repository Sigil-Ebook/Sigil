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
#include <QPrinter>
#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QPainter>
#include <QFileInfo>
#include <QDir>
#include <QUrl>
#include <QStandardPaths>
#include <QDebug>

#include "Misc/SettingsStore.h"
#include "ViewEditors/ViewPreview.h"
#include "Parsers/GumboInterface.h"

static const QStringList DARKCSSLINKS = QStringList() << "qrc:///dark/mac_dark_scrollbar.css"
                                                      << "qrc:///dark/win_dark_scrollbar.css"
                                                      << "qrc:///dark/lin_dark_scrollbar.css";

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
    emit printStarted();
    // destroy previous QWebEngineView to avoid leaking
    if (m_viewprev){
        delete m_viewprev;
        m_viewprev = nullptr;
        DBG qDebug() << "Cleaning up old WebEngineView";
    }
    m_skipPreview = skipPrev;
    m_inPrintPreview = false;

    // Create new (undisplayed) ViewPreview widget to which the EPUB's
    // page url and text is loaded without any potential darkmode injections
    m_viewprev = new ViewPreview(nullptr, false);
    connect(m_viewprev, &ViewPreview::loadFinished, this, &WebViewPrinter::loadFinished);
    m_viewprev->CustomSetDocument(filepath, GetHtmlWithNoDarkMode(text));
}

QString WebViewPrinter::getPrintToFilePath(QFileInfo &fi) {
    QString filename = fi.baseName() + ".pdf";
    QString path = QDir(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).filePath(filename);
    return path;
}

void WebViewPrinter::print()
{
    DBG qDebug() << "Skipping Print Preview.";
    SettingsStore ss;
    QPrinter printer(QPrinter::HighResolution);
    printer.setResolution(ss.printDPI());
    DBG qDebug() << "Print DPI = " << printer.resolution();
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
        emit printEnded();
        return;
    printDocument(&printer);
    emit printEnded();
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

    SettingsStore ss;
    QPrinter printer(QPrinter::HighResolution);
    printer.setResolution(ss.printDPI());
    DBG qDebug() << "Print Preview DPI = " << printer.resolution();
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
    emit printEnded();
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


QString WebViewPrinter::GetHtmlWithNoDarkMode(const QString xhtmltext) 
{
    QString text = xhtmltext;

    // now remove any leftovers and make sure it is well formed
    GumboInterface gi = GumboInterface(text, "any_version");

    QList<GumboNode*> nodes;
    QList<GumboTag> tags;

    // remove any added AddDarkCSS (style node has id="Sigil_Injected")
    tags = QList<GumboTag>() << GUMBO_TAG_STYLE;
    nodes = gi.get_all_nodes_with_tags(tags);
    foreach(GumboNode * node, nodes) {
        GumboAttribute* attr = gumbo_get_attribute(&node->v.element.attributes, "id");
        if (attr && QString::fromUtf8(attr->value) == "Sigil_Injected") {
            // qDebug() << "removing Sigil_Injected dark style";
            gumbo_remove_from_parent(node);
            gumbo_destroy_node(node);
            break;
        }
    }
    // then the associated scrollbar stylesheet link
    tags = QList<GumboTag>() << GUMBO_TAG_LINK;
    nodes = gi.get_all_nodes_with_tags(tags);
    foreach(GumboNode * node, nodes) {
        GumboAttribute* attr = gumbo_get_attribute(&node->v.element.attributes, "href");
        if (attr) {
            QString attrval = QString::fromUtf8(attr->value);
            if (DARKCSSLINKS.contains(attrval) ) {
                // qDebug() << "removing dark css links";
                gumbo_remove_from_parent(node);
                gumbo_destroy_node(node);
                break;
            }
        }
    }

    text = gi.getxhtml();
    return text;
}
