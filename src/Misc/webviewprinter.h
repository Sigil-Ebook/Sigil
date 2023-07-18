
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


#pragma once
#ifndef WEBVIEWPRINTER_H
#define WEBVIEWPRINTER_H

#include <QObject>
#include <QUrl>
#include <QFileInfo>

class QPrinter;
class ViewPreview;

class WebViewPrinter: public QObject
{
    Q_OBJECT

public:
    WebViewPrinter(QObject *parent = nullptr);
    ~WebViewPrinter();

    /**
    ** Initialize new hidden ViewPreview and
    ** and load the content to be printed into it
    **/
    void setContent(QString filepath, QString text, bool skipPrev);
    
signals:
    void printStarted();
    void printEnded();
                   
private slots:
    /**
    ** Called when QWebEngineView::loadFinished signal fires
    **/
    void loadFinished(bool ok);

private:
    /**
    ** set default print-to-file path on linux to keep CUPS dialog from defaulting
    ** to (and showing) the ebook's location in Sigil's temporary scratch area
    **/
    QString getPrintToFilePath(QFileInfo &fi);

    /**
    ** Go directly to Printer Dialog
    **/
    void print();

    /**
    ** Launch a QPrintPreview Dialog before printing
    **/
    void printPreview();

    /**
    ** QWebEnginePage::print statement and callback
    **/
    void printDocument(QPrinter *printer);

    /**
    ** Remove any Sigil Injected dark mode code from the xhtml
    **/
    QString GetHtmlWithNoDarkMode(const QString xhtmltext);
    
    /**
    ** New hidden ViewPreview
    **/
    ViewPreview *m_viewprev = nullptr;

    bool m_inPrintPreview = false;

    /**
    ** holds preference setting to bypass Print Preview
    **/
    bool m_skipPreview = false;
};

#endif // WEBVIEWPRINTER_H
