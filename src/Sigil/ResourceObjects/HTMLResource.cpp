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

#include <stdafx.h>
#include "HTMLResource.h"
#include "../Misc/Utility.h"
#include "../BookManipulation/CleanSource.h"

static const QString LOADED_CONTENT_MIMETYPE = "application/xhtml+xml";


HTMLResource::HTMLResource( const QString &fullfilepath, 
                            QHash< QString, Resource* > *hash_owner,
                            int reading_order,
                            QObject *parent )
    : 
    Resource( fullfilepath, hash_owner, parent ),
    m_WebPage( *new QWebPage( this ) ),
    m_ReadingOrder( reading_order ),
    m_InitialLoadFromDiskDone( false )
{

}

Resource::ResourceType HTMLResource::Type() const
{
    return Resource::HTMLResource;
}


QWebPage& HTMLResource::GetWebPage()
{
    return m_WebPage;
}


void HTMLResource::SetHtml( const QString &source )
{
    SetRawHTML( CleanSource::Clean( source ) );
}


QString HTMLResource::GetHtml()
{
    RemoveWebkitClasses();

    return m_WebPage.mainFrame()->toHtml();
}


void HTMLResource::SaveToDisk()
{
    QWriteLocker locker( &m_ReadWriteLock );

    Utility::WriteUnicodeTextFile( m_WebPage.mainFrame()->toHtml(), m_FullFilePath );
}


void HTMLResource::LoadFromDisk()
{
    // When a thread enters this function, the content is either:
    // 1. already loaded          ( m_InitialLoadFromDiskDone = true  )
    // 2. currently being loaded  ( m_InitialLoadFromDiskDone = false )
    // 3. not loaded              ( m_InitialLoadFromDiskDone = false )

    // Check for 1.
    if ( m_InitialLoadFromDiskDone )
    
        return;

    // If this returns false, then it's 2.
    // If it returns true, we have the lock and it's 3.
    if ( !m_ReadWriteLock.tryLockForWrite() )

        return;  

    // We use SetRawHTML since the importing procedure should have
    // cleaned the file already. No need to do it twice.
    SetRawHTML( Utility::ReadUnicodeTextFile( m_FullFilePath ) );

    m_InitialLoadFromDiskDone = true;

    m_ReadWriteLock.unlock();
}


int HTMLResource::GetReadingOrder()
{
    return m_ReadingOrder;
}


void HTMLResource::SetReadingOrder( int reading_order )
{
    m_ReadingOrder = reading_order;
}


// Removes every occurrence of "signing" classes
// with which webkit litters our source code 
void HTMLResource::RemoveWebkitClasses()
{
    // TODO: Use the QWebElement API to perform the same thing

    //m_Source.replace( QRegExp( "(class=\"[^\"]*)Apple-style-span" ), "\\1" );
    //m_Source.replace( QRegExp( "(class=\"[^\"]*)webkit-indent-blockquote" ), "\\1" );
}

void HTMLResource::SetRawHTML( const QString &source )
{
    m_WebPage.mainFrame()->setContent( source.toUtf8(), LOADED_CONTENT_MIMETYPE, GetBaseUrl() ); 
    m_WebPage.setContentEditable( true );

    // TODO: we kill external links; a dialog should be used
    // that asks the user if he wants to open this external link in a browser
    m_WebPage.setLinkDelegationPolicy( QWebPage::DelegateAllLinks );

    QWebSettings &settings = *m_WebPage.settings();
    settings.setAttribute( QWebSettings::LocalContentCanAccessRemoteUrls, false );
    settings.setAttribute( QWebSettings::JavascriptCanAccessClipboard, true );
    settings.setAttribute( QWebSettings::ZoomTextOnly, true );
}


