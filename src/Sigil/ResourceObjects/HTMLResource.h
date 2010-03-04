/************************************************************************
**
**  Copyright (C) 2009  Strahinja Markovic
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

#pragma once
#ifndef HTMLRESOURCE_H
#define HTMLRESOURCE_H

#include <QDomDocument>
#include "Resource.h"

class QWebPage;
class QTextDocument;
class QString;


class HTMLResource : public Resource 
{
    Q_OBJECT

public:
    
    HTMLResource( const QString &fullfilepath, 
                  QHash< QString, Resource* > *hash_owner,
                  int reading_order,
                  QObject *parent = NULL );

    virtual ResourceType Type() const;

    QWebPage& GetWebPage();

    QTextDocument& GetTextDocument();

    void SetDomDocument( const QDomDocument &document );

    const QDomDocument& GetDomDocumentForReading();

    QDomDocument& GetDomDocumentForWriting();

    void MarkSecondaryCachesAsOld();

    // All of these Update* functions may look silly,
    // but updating only the parts that are needed
    // improves performance *considerably*.
    // They are always called only from the GUI thread.

    void UpdateDomDocumentFromWebPage();

    void UpdateDomDocumentFromTextDocument();

    void UpdateWebPageFromDomDocument();

    void UpdateTextDocumentFromDomDocument();

    void UpdateWebPageFromTextDocument();

    void UpdateTextDocumentFromWebPage();

    void SaveToDisk();

    int GetReadingOrder();

    void SetReadingOrder( int reading_order );
    
    void RemoveWebkitClasses();

    // TODO: turn this into operator<
    static bool LessThan( HTMLResource* res_1, HTMLResource* res_2 );

private slots:

    void LinkedResourceUpdated();

    void WebPageJavascriptOnLoad();

private:

    QString GetWebPageHTML();

    void SetWebPageHTML( const QString &source );

    QStringList GetPathsToLinkedResources();

    void TrackNewResources( const QStringList &filepaths );

    // We use resource IDs and not resource pointers
    // to avoid problems with dangling pointers.
    QStringList m_LinkedResourceIDs;

    // This is the actual HTML resource backing store.
    // The final arbiter of the content in the HTMLResource.
    // AKA the main, primary cache.
    QDomDocument m_DomDocument;

    // These are here only for convenience so that
    // the FlowTabs don't need to create and populate
    // them every time, but just once. Improves performance.
    // AKA the auxiliary, secondary caches.
    QWebPage *m_WebPage;
    QTextDocument *m_TextDocument;

    // This holds the state of the content of the secondary
    // caches the last time they were updated. It's used
    // to prevent syncing between them when no new changes
    // have been made.
    QString m_OldSourceCache;

    bool m_WebPageIsOld;
    bool m_TextDocumentIsOld;

    bool m_RefreshNeeded;

    // Starts at 0, not 1
    int m_ReadingOrder;

    /**
     * The javascript source code of the jQuery library.
     */
    const QString c_jQuery;
    
    /**
     * The javascript source code of the jQuery 
     * ScrollTo extension library.
     */
    const QString c_jQueryScrollTo;
};

#endif // HTMLRESOURCE_H
