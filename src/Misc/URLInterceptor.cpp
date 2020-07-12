/************************************************************************
**
**  Copyright (C) 2020  Kevin B. Hendricks, Stratford, ON, Canada
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
#include <QDebug>

#include "BookManipulation/Book.h"
#include "Misc/Utility.h"
#include "MainUI/MainWindow.h"
#include "BookManipulation/FolderKeeper.h"
#include "URLInterceptor.h"

#define INTERCEPTDEBUG 1

URLInterceptor::URLInterceptor(QObject *parent)
    : QWebEngineUrlRequestInterceptor(parent)
{
}

void URLInterceptor::interceptRequest(QWebEngineUrlRequestInfo &info)
{

#if INTERCEPTDEBUG
    // Debug:  output all requests
    qDebug() << "-----";
    qDebug() << "method: " << info.requestMethod();
    qDebug() << "party: " << info.firstPartyUrl();
    qDebug() << "request" << info.requestUrl();
    qDebug() << "navtype: " << info.navigationType();
    qDebug() << "restype: " << info.resourceType();
    qDebug() << "ActiveWindow: " <<  qApp->activeWindow();
#endif

    if (info.requestMethod() != "GET") {
        info.block(true);
        qDebug() << "Warning: URLInterceptor Blocking POST request from " << info.firstPartyUrl();
        return;
    }
    
    QUrl destination(info.requestUrl());

    // verify all url file schemes before allowing
    if (destination.scheme() == "file") {
        QString bookfolder;
        QString mathjaxfolder;
        QString usercssfolder;
        MainWindow * mainwin = qobject_cast<MainWindow*>(Utility::GetMainWindow());
#if INTERCEPTDEBUG
        qDebug() << "mainwin: " <<  mainwin;
#endif
        if (mainwin) {
            QSharedPointer<Book> book = mainwin->GetCurrentBook();
            bookfolder = book->GetFolderKeeper()->GetFullPathToMainFolder() + "/";
            mathjaxfolder = mainwin->GetMathJaxFolder();
            usercssfolder = Utility::DefinePrefsDir() + "/";
#if INTERCEPTDEBUG
            qDebug() << "book: " << bookfolder;
            qDebug() << "mathjax: " << mathjaxfolder;
            qDebug() << "usercss: " << usercssfolder;
#endif
        }
        // if can not determine book folder block it
        if (bookfolder.isEmpty()) {
            info.block(true);
            qDebug() << "Error: URLInterceptor can not determine book folder so all file: requests blocked";
            return;
        }
        // path must be inside of bookfolder, Nore it is legal for it not to exist
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
        // otherwise block it to prevent access to any user file path
        info.block(true);
        qDebug() << "Warning: URLInterceptor blocking access to url " << destination;
        qDebug() << "    from " << info.firstPartyUrl();
        return;
    }

    // allow others to proceed
    info.block(false);
    return;
}
