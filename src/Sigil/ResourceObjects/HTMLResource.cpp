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
#include "../BookManipulation/XHTMLDoc.h"

static const QString LOADED_CONTENT_MIMETYPE = "application/xhtml+xml";


HTMLResource::HTMLResource( const QString &fullfilepath, 
                            QHash< QString, Resource* > *hash_owner,
                            int reading_order,
                            QObject *parent )
    : 
    Resource( fullfilepath, hash_owner, parent ),
    m_WebPage( NULL ),
    m_WebPageIsOld( true ),
    m_ReadingOrder( reading_order )
{

}

Resource::ResourceType HTMLResource::Type() const
{
    return Resource::HTMLResource;
}


QWebPage& HTMLResource::GetWebPage()
{
    return *m_WebPage;
}


// only ever call this from the GUI thread
void HTMLResource::SetHtml( const QString &source )
{
    Q_ASSERT( QThread::currentThread() == QApplication::instance()->thread() );

    QString new_source = CleanSource::Clean( source );
    
    m_Document.setContent( new_source );

    if ( m_WebPage == NULL )

        m_WebPage = new QWebPage( this );

    SetRawHTML( new_source );

    m_WebPageIsOld = false;
}


QString HTMLResource::GetHtml()
{
    // TODO: use a special Tidy pretty-printer
    return XHTMLDoc::GetQDomNodeAsString( m_Document );
}

void HTMLResource::SetDocument( const QDomDocument &document )
{
    QWriteLocker locker( &m_ReadWriteLock );

    m_Document = document;
    m_WebPageIsOld = true;
}

// Make sure to get a read lock externally before calling this function!
const QDomDocument& HTMLResource::GetDocumentForReading()
{
    // We can't check with tryLockForRead because that
    // can still legitimately succeed.
    Q_ASSERT( m_ReadWriteLock.tryLockForWrite() == false );

    return m_Document;
}


// Make sure to get a read lock externally before calling this function!
QDomDocument& HTMLResource::GetDocumentForWriting()
{
    Q_ASSERT( m_ReadWriteLock.tryLockForWrite() == false );

    m_WebPageIsOld = true;
    
    return m_Document;
}


// only ever call this from the GUI thread
void HTMLResource::UpdateDocumentFromWebPage()
{
    Q_ASSERT( QThread::currentThread() == QApplication::instance()->thread() );

    RemoveWebkitClasses();

    m_Document.setContent( m_WebPage->mainFrame()->toHtml() );
}

// only ever call this from the GUI thread
void HTMLResource::UpdateWebPageFromDocument()
{
    Q_ASSERT( QThread::currentThread() == QApplication::instance()->thread() );

    if ( m_WebPageIsOld == false )

        return;

    if ( m_WebPage == NULL )

        m_WebPage = new QWebPage( this );

    SetRawHTML( XHTMLDoc::GetQDomNodeAsString( m_Document ) );

    m_WebPageIsOld = false;
}


void HTMLResource::SaveToDisk()
{
    QWriteLocker locker( &m_ReadWriteLock );

    Utility::WriteUnicodeTextFile( XHTMLDoc::GetQDomNodeAsString( m_Document ), m_FullFilePath );
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
    m_WebPage->mainFrame()->setContent( source.toUtf8(), LOADED_CONTENT_MIMETYPE, GetBaseUrl() ); 
    m_WebPage->setContentEditable( true );

    // TODO: we kill external links; a dialog should be used
    // that asks the user if he wants to open this external link in a browser
    m_WebPage->setLinkDelegationPolicy( QWebPage::DelegateAllLinks );

    QWebSettings &settings = *m_WebPage->settings();
    settings.setAttribute( QWebSettings::LocalContentCanAccessRemoteUrls, false );
    settings.setAttribute( QWebSettings::JavascriptCanAccessClipboard, true );
    settings.setAttribute( QWebSettings::ZoomTextOnly, true );
}

