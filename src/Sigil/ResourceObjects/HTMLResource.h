/************************************************************************
**
**  Copyright (C) 2009, 2010  Strahinja Markovic
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
#include <QHash>
#include "Resource.h"
#include "../BookManipulation/GuideSemantics.h"

class QWebPage;
class QTextDocument;
class QString;


class HTMLResource : public Resource 
{
    Q_OBJECT

public:
    
    /**
     * Constructor.
     *
     * @param fullfilepath The full path to the file that this
     *                     resource is representing.
     * @param hash_owner The hash object that is the "owner" of this resource.
     *                   Needed so that the resource can remove itself from the
     *                   hash when it is deleted.
     * @param reading_order The reading order of the HTML resource.
     * @param semantic_information The guide element information and other
     *                             semantic info in key-value pairs.
     * @param parent The object's parent.
     */
    HTMLResource( const QString &fullfilepath, 
                  QHash< QString, Resource* > *hash_owner,
                  int reading_order,
                  QHash< QString, QString > semantic_information,
                  QObject *parent = NULL );

    /**
     * The less-than operator overload. Overridden because HTMLResources
     * are compared by their reading order, not by filename.
     *
     * @param other The other HTMLResource object we're comparing with.
     */
    virtual bool operator< ( const HTMLResource& other );

    /**
     * Returns the guide element semantic type.
     * 
     * @return The guide semantic type.
     */
    GuideSemantics::GuideSemanticType GetGuideSemanticType() const;

    /**
     * Returns the guide element semantic title. 
     *
     * @return The guide element semantic title. 
     */
    QString GetGuideSemanticTitle() const;

    /**
     * Sets the guide semantic type information.
     *
     * @param type The new semantic type.
     */ 
    void SetGuideSemanticType( GuideSemantics::GuideSemanticType type );

    // inherited
    virtual ResourceType Type() const;

    /**
     * The QWebPage instance for rendering this HTML resource.
     *
     * @return A reference to the QWebPage instance.
     */
    QWebPage& GetWebPage();

    /**
     * The text document for editing the HTML source code.
     * 
     * @return An reference to the QTextDocument instance.
     */
    QTextDocument& GetTextDocument();

    /**
     * Sets the DOM document that is *the* representaion of the 
     * resource's content.
     *
     * @param document The new QDom instance.
     */
    void SetDomDocument( const QDomDocument &document );

    /**
     * Returns a const reference to the DOM document that can be read
     * in consumers. If you need write access, use GetDomDocumentForWriting().
     * @warning Make sure to get a read lock externally before calling this function!
     *
     * @return A const reference to the QDom.
     */
    const QDomDocument& GetDomDocumentForReading();

    /**
     * Returns a reference to the DOM document that can be read and written to
     * in consumers. If you need just read access, use GetDomDocumentForReading().
     * @warning Make sure to get a write lock externally before calling this function!
     * @warning Call MarkSecondaryCachesAsOld() if you updated the document!
     *
     * @return A const reference to the QDom.
     */
    QDomDocument& GetDomDocumentForWriting();

    /**
     * Marks the QTextDocument and QWebPage as needing a update/refresh.
     */
    void MarkSecondaryCachesAsOld();

    // All of these Update* functions may look silly,
    // but updating only the parts that are needed
    // improves performance *considerably*.
    // They are always called only from the GUI thread.

    /**
     * Updates the DOM document from the content of the QWebPage.
     * @warning Only ever call this from the GUI thread.
     */ 
    void UpdateDomDocumentFromWebPage();

    /**
     * Updates the DOM document from the content of the QTextDocument.
     */ 
    void UpdateDomDocumentFromTextDocument();

    /**
     * Updates the QWebPage from the content of the DOM.
     * @warning Only ever call this from the GUI thread.
     */ 
    void UpdateWebPageFromDomDocument();

    /**
     * Updates the QTextDocument from the content of the DOM.
     */
    void UpdateTextDocumentFromDomDocument();

    /**
     * Updates the QWebPage from the content of the QTextDocument.
     */
    void UpdateWebPageFromTextDocument();

    /**
     * Updates the QWebPage from the content of the DOM.
     */
    void UpdateTextDocumentFromWebPage();

    // inherited
    void SaveToDisk( bool book_wide_save = false );

    /**
     * Returns the reading roder of the resource.
     *
     * @return The reading order.
     */
    int GetReadingOrder() const;

    /**
     * Sets the reading order of the resource.
     *
     * @param reading_order The new reading order.
     */
    void SetReadingOrder( int reading_order );
    
    /**
     * Removes all the cruft with which WebKit litters our source code.
     * The cruft is removed from the QWebPage cache, and includes
     * superfluous CSS styles and classes. 
     */
    void RemoveWebkitCruft();

    /**
     * Splits the content of the resource into multiple chapters.
     * The SGF chapter markers are used as the break points.
     * The first chapter is set as the content of the resource,
     * and the others are returned.
     *
     * @return The content of all the chapters except the first. 
     */
    QStringList SplitOnSGFChapterMarkers();


private slots:

    /**
     * Called whenever a linked resource is updated.
     * A linked resource is for example a CSS file.
     */
    void LinkedResourceUpdated();

    /**
     * Loads the required JavaScript on web page loads.
     */
    void WebPageJavascriptOnLoad();

private:

    /**
     * Returns the HTML content of the QWebPage.
     *
     * @return The HTML content.
     */
    QString GetWebPageHTML();

    /**
     * Sets the HTML content of the QWebPage
     * 
     * @param source The new HTML source.
     */
    void SetWebPageHTML( const QString &source );

    /**
     * Returns the paths to all the linked resources
     * like images and stylesheets.
     *
     * @return The paths to the linked resources.
     */
    QStringList GetPathsToLinkedResources();

    /**
     * Makes sure the given paths are watched for updates.
     * 
     * @param filepaths The paths to resources to watch.
     */
    void TrackNewResources( const QStringList &filepaths );

    /**
     * IDs of all the resources that are linked in this 
     * HTML resource.
     * 
     * We use resource IDs and not resource pointers
     * to avoid problems with dangling pointers.
     */
    QStringList m_LinkedResourceIDs;

    /**
     * This is the actual HTML resource backing store.
     * The final arbiter of the content in the HTMLResource.
     * AKA the main, primary cache.
     */
    QDomDocument m_DomDocument;

    /**
     * This is here only for convenience so that
     * the FlowTabs don't need to create and populate
     * it every time, but just once. Improves performance.
     * AKA the auxiliary, secondary cache of the rendered page.
     */
    QWebPage *m_WebPage;

    /**
     * This is here only for convenience so that
     * the FlowTabs don't need to create and populate
     * it every time, but just once. Improves performance.
     * AKA the auxiliary, secondary cache of the colored source.
     */
    QTextDocument *m_TextDocument;
    
    /**
     * This holds the state of the content of the secondary
     * caches the last time they were updated. It's used
     * to prevent syncing between them when no new changes
     * have been made.
     */
    QString m_OldSourceCache;

    /**
     * True when the QWebPage cache needs to be updated.
     */
    bool m_WebPageIsOld;
    
    /**
     * True when the QTextDocument cache needs to be updated.
     */
    bool m_TextDocumentIsOld;

    /**
     * True when the QWebPage cache is good, but a page refresh is necessary.
     * This happens when for instance a linked stylesheet is updated on disk.
     */
    bool m_RefreshNeeded;

    /**
     * Stores the guide semantic type of this resource. 
     * This type will be used in the guide element of the OPF.
     */
    GuideSemantics::GuideSemanticType m_GuideSemanticType;

    /**
     * Stores the guide semantic title of this resource. 
     * This type will be used in the guide element of the OPF.
     */
    QString m_GuideSemanticTitle;

    /**
     * The reading order of the HTML resource. Resources with a lower
     * reading order are shown before those with a higher one.
     * Starts at 0, not 1.
     */
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
