/************************************************************************
**
**  Copyright (C) 2020-2021 Kevin B. Hendricks, Stratford, ON, Canada
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
#include <QUrl>
#include <QApplication>
#include <QWidgetList>
#include <QDebug>

#include "BookManipulation/Book.h"
#include "MainUI/MainWindow.h"
#include "BookManipulation/FolderKeeper.h"
#include "Misc/URLInterceptor.h"

#define DBG if(0)

URLInterceptor::URLInterceptor(QObject *parent)
    : QWebEngineUrlRequestInterceptor(parent)
{
}

void URLInterceptor::interceptRequest(QWebEngineUrlRequestInfo &info)
{
    // Debug:  output all requests
    DBG qDebug() << "-----";
    DBG qDebug() << "method: " << info.requestMethod();
    DBG qDebug() << "party: " << info.firstPartyUrl();
    DBG qDebug() << "request" << info.requestUrl();
    DBG qDebug() << "navtype: " << info.navigationType();
    DBG qDebug() << "restype: " << info.resourceType();
    DBG qDebug() << "ActiveWindow: " <<  qApp->activeWindow();

    if (info.requestMethod() != "GET") {
        info.block(true);
        qDebug() << "Warning: URLInterceptor Blocking POST request from " << info.firstPartyUrl();
        return;
    }
    
    QUrl destination(info.requestUrl());
    QUrl sourceurl(info.firstPartyUrl());

    // note: need to handle our "sigil" scheme but toLocalFile will NOT work on a "sigil" scheme url
    // so temporarily remap them to local file scheme for the purposes of this routine
    if (destination.scheme() == "sigil") {
        destination.setScheme("file");
        destination.setQuery(QString());
    }
    if (sourceurl.scheme() == "sigil") {
        sourceurl.setScheme("file");
        sourceurl.setQuery(QString());
    }
 
    // Finally let the navigation type determine what to verify against:
    // Use firstPartyURL when NavigationTypeLink or NavigationTypeOther (ie. a true source url)
    // Otherwise if we Typed it in it is from our own PreviewUpdate page
    if (info.navigationType() == QWebEngineUrlRequestInfo::NavigationTypeTyped) {
        sourceurl = destination;
    }

    // verify all url file and sigil schemes before allowing
    // if ((destination.scheme() == "file") || (destination.scheme() == "sigil")) {
    if (destination.scheme() == "file") {
        // find the relavent MainWindow
        QString bookfolder;
        QString mathjaxfolder;
        QString usercssfolder = Utility::DefinePrefsDir() + "/";
        QString sourcefolder = sourceurl.toLocalFile();
        DBG qDebug() << "sourcefolder: " << sourcefolder;
        const QWidgetList topwidgets = qApp->topLevelWidgets();
        foreach(QWidget* widget, topwidgets) {
            MainWindow * mw = qobject_cast<MainWindow *>(widget);
            if (mw) {
                QSharedPointer<Book> book = mw->GetCurrentBook();
                QString path_to_book = book->GetFolderKeeper()->GetFullPathToMainFolder() + "/";
                DBG qDebug() << "path_to_book: " << path_to_book;
                QString path_to_mathjax = mw->GetMathJaxFolder();
                if (sourcefolder.startsWith(path_to_book)) {
                    bookfolder = path_to_book;
                    mathjaxfolder = path_to_mathjax;
                    DBG qDebug() << "mainwin: " <<  mw;
                    DBG qDebug() << "book: " << bookfolder;
                    DBG qDebug() << "mathjax: " << mathjaxfolder;
                    DBG qDebug() << "usercss: " << usercssfolder;
                    DBG qDebug() << "party: " << info.firstPartyUrl();
                    DBG qDebug() << "source: " << sourcefolder;
                    break;
                }
            }
        }
        // if can not determine book folder block it
        if (bookfolder.isEmpty()) {
            info.block(true);
            qDebug() << "Error: URLInterceptor can not determine book folder so all file requests blocked";
            return;
        }
        // path must be inside of bookfolder, Note it is legal for it not to exist
        QString destpath = destination.toLocalFile();
        if (destpath.startsWith(bookfolder)) {
            info.block(false);
            return;
        }
        // or path must be inside the Sigil user's preferences directory
        if (destpath.startsWith(usercssfolder)) {
            info.block(false);
            return;
        }
        // or the path must be inside the Sigil's MathJax folder 
        if (destpath.startsWith(mathjaxfolder)) {
            info.block(false);
            return;
        }

        // otherwise block it to prevent access to any outside Sigil user file path
        info.block(true);
        qDebug() << "Warning: URLInterceptor blocking access to url " << destination;
        qDebug() << "    from " << info.firstPartyUrl();
        return;
    }

    // allow others to proceed
    info.block(false);
    return;
}
