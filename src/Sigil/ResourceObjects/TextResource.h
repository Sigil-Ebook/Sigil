/************************************************************************
**
**  Copyright (C) 2009, 2010, 2011  Strahinja Markovic  <strahinja.markovic@gmail.com>
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
#ifndef TEXTRESOURCE_H
#define TEXTRESOURCE_H

#include <QtCore/QMutex>

#include "ResourceObjects/Resource.h"

class QTextDocument;

/**
 * A parent class for textual resources like CSS and XPGT stylesheets.
 * Takes care of loading and caching content etc.
 */
class TextResource : public Resource 
{
    Q_OBJECT

public:
    
    /**
     * Constructor.
     *
     * @param fullfilepath The full path to the file that this
     *                     resource is representing.
     * @param parent The object's parent.
     */
    TextResource( const QString &fullfilepath, QObject *parent = NULL );

    /**
     * Returns the text stored in the resource.
     *
     * @return The resource text.
     */
    QString GetText() const;

    /**
     * Sets the text of the resource, replacing the stored content.
     */
    void SetText( const QString& text );

    /**
     * Returns a reference to the QTextDocument that can be read and written to
     * in consumers. If you need just read access, use GetTextDocumentForReading().
     *
     * @warning Make sure to get a write lock externally before calling this function!
     *
     * @return A reference to the QTextDocument cache.
     */
    QTextDocument& GetTextDocumentForWriting();

    // inherited
    void SaveToDisk( bool book_wide_save = false );

    /**
     * Loads the text content into the QTextDocument cache if
     * nothing has been loaded so far. This is not done automatically
     * because we want to do loading on demand (for performance reasons).
     */
    virtual void InitialLoad();
    
    // inherited
    virtual ResourceType Type() const;

private slots:

    /**
     * Performs the delayed update of m_TextDocument with the text
     * stored in m_Cache.
     */
    void DelayedUpdateToTextDocument();

private:

    /**
     * Actually sets the text to m_TextDocument.
     * 
     * @param text The text to set.
     */
    void SetTextInternal( const QString &text );


    ///////////////////////////////
    // PRIVATE MEMBER VARIABLES
    ///////////////////////////////

    /**
     * If \c true, then the m_Cache var is holding cached text.
     */
    bool m_CacheInUse;

    /**
     * The cached text used when threads are in use. @see SetText() internals.
     */
    QString m_Cache;

    /**
     * The access mutex for the cache.
     */
    mutable QMutex m_CacheAccessMutex;

    /**
     * The syntax colored cache of the TextResource text content.
     */
    QTextDocument *m_TextDocument;
};

#endif // TEXTRESOURCE_H
