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
    m_TextDocument( NULL ),
    m_WebPageIsOld( true ),
    m_TextDocumentIsOld( true ),
    m_ReadingOrder( reading_order )
{

}


Resource::ResourceType HTMLResource::Type() const
{
    return Resource::HTMLResource;
}


QWebPage& HTMLResource::GetWebPage()
{
    Q_ASSERT( m_WebPage );

    return *m_WebPage;
}


QTextDocument& HTMLResource::GetTextDocument()
{
    Q_ASSERT( m_TextDocument );

    return *m_TextDocument;
}


void HTMLResource::SetDomDocument( const QDomDocument &document )
{
    QWriteLocker locker( &m_ReadWriteLock );

    m_DomDocument = document;

    m_WebPageIsOld      = true;
    m_TextDocumentIsOld = true;
}


// Make sure to get a read lock externally before calling this function!
const QDomDocument& HTMLResource::GetDomDocumentForReading()
{
    // We can't check with tryLockForRead because that
    // can still legitimately succeed.
    Q_ASSERT( m_ReadWriteLock.tryLockForWrite() == false );

    return m_DomDocument;
}


// Make sure to get a write lock externally before calling this function!
// Call MarkSecondaryCachesAsOld() if you updated the document.
QDomDocument& HTMLResource::GetDomDocumentForWriting()
{
    Q_ASSERT( m_ReadWriteLock.tryLockForWrite() == false );

    // We can't just mark the caches as old right here since
    // some consumers need write access but don't end up writing anything.

    return m_DomDocument;
}


void HTMLResource::MarkSecondaryCachesAsOld()
{
    m_WebPageIsOld      = true;
    m_TextDocumentIsOld = true;
}


// only ever call this from the GUI thread
void HTMLResource::UpdateDomDocumentFromWebPage()
{
    Q_ASSERT( QThread::currentThread() == QApplication::instance()->thread() );

    m_DomDocument.setContent( GetWebPageHTML() );
}


void HTMLResource::UpdateDomDocumentFromTextDocument()
{
    Q_ASSERT( QThread::currentThread() == QApplication::instance()->thread() );
    Q_ASSERT( m_TextDocument );

    m_DomDocument.setContent( CleanSource::Clean( m_TextDocument->toPlainText() ) );
}


// only ever call this from the GUI thread
void HTMLResource::UpdateWebPageFromDomDocument()
{
    Q_ASSERT( QThread::currentThread() == QApplication::instance()->thread() );

    if ( !m_WebPageIsOld )

        return;

    if ( m_WebPage == NULL )

        m_WebPage = new QWebPage( this );

    SetWebPageHTML( XHTMLDoc::GetQDomNodeAsString( m_DomDocument ) );

    m_WebPageIsOld = false;
}


void HTMLResource::UpdateTextDocumentFromDomDocument()
{
    Q_ASSERT( QThread::currentThread() == QApplication::instance()->thread() );

    if ( !m_TextDocumentIsOld )

        return;

    if ( m_TextDocument == NULL )
    {
        m_TextDocument = new QTextDocument( this );
        m_TextDocument->setDocumentLayout( new QPlainTextDocumentLayout( m_TextDocument ) );
    }

    m_TextDocument->setPlainText( CleanSource::PrettyPrint( XHTMLDoc::GetQDomNodeAsString( m_DomDocument ) ) );

    m_TextDocumentIsOld = false;
}


void HTMLResource::UpdateWebPageFromTextDocument()
{
    Q_ASSERT( QThread::currentThread() == QApplication::instance()->thread() );
    Q_ASSERT( m_TextDocument );
    Q_ASSERT( m_WebPage );

    QString source = CleanSource::Clean( m_TextDocument->toPlainText() );

    if ( m_OldSourceCache != source )
    {
        SetWebPageHTML( source );
        m_OldSourceCache = source;
    }
}


void HTMLResource::UpdateTextDocumentFromWebPage()
{
    Q_ASSERT( QThread::currentThread() == QApplication::instance()->thread() );
    Q_ASSERT( m_TextDocument );
    Q_ASSERT( m_WebPage );

    QString source = GetWebPageHTML();

    if ( m_OldSourceCache != source )
    {
        m_TextDocument->setPlainText( source );
        m_OldSourceCache = source;
    }    
}


void HTMLResource::SaveToDisk()
{
    QWriteLocker locker( &m_ReadWriteLock );

    Utility::WriteUnicodeTextFile( CleanSource::PrettyPrint( XHTMLDoc::GetQDomNodeAsString( m_DomDocument ) ),
                                   m_FullFilePath );
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
    Q_ASSERT( m_WebPage );

    QWebElementCollection collection = m_WebPage->mainFrame()->findAllElements( ".Apple-style-span" );

    foreach( QWebElement element, collection )
    {
        element.toggleClass( "Apple-style-span" );
    }

    collection = m_WebPage->mainFrame()->findAllElements( ".webkit-indent-blockquote" );

    foreach( QWebElement element, collection )
    {
        element.toggleClass( "webkit-indent-blockquote" );
    }
}


bool HTMLResource::LessThan( HTMLResource* res_1, HTMLResource* res_2 )
{
    return res_1->GetReadingOrder() < res_2->GetReadingOrder();
}


QString HTMLResource::GetWebPageHTML()
{
    Q_ASSERT( m_WebPage );

    RemoveWebkitClasses();

    return CleanSource::Clean( m_WebPage->mainFrame()->toHtml() );
}


void HTMLResource::SetWebPageHTML( const QString &source )
{
    Q_ASSERT( m_WebPage );

    qDebug() << GetBaseUrl();

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


