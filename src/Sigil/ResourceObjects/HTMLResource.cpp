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

#include <stdafx.h>
#include "HTMLResource.h"
#include "BookManipulation/XercesCppUse.h"
#include "Misc/Utility.h"
#include "BookManipulation/CleanSource.h"
#include "BookManipulation/XhtmlDoc.h"
#include "BookManipulation/GuideSemantics.h"

static const QString LOADED_CONTENT_MIMETYPE = "application/xhtml+xml";

const QString XML_TAG = "<?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"no\"?>";

HTMLResource::HTMLResource( const QString &fullfilepath, 
                            const QHash< QString, Resource* > &resources,
                            QObject *parent )
    : 
    Resource( fullfilepath, parent ),
    m_WebPage( NULL ),
    m_TextDocument( NULL ),
    m_WebPageModified( false ),
    m_WebPageIsOld( true ),
    m_TextDocumentIsOld( true ),
    c_jQuery(         Utility::ReadUnicodeTextFile( ":/javascript/jquery-1.4.2.min.js"          ) ),
    c_jQueryScrollTo( Utility::ReadUnicodeTextFile( ":/javascript/jquery.scrollTo-1.4.2-min.js" ) ),
    m_Resources( resources )
{

}


Resource::ResourceType HTMLResource::Type() const
{
    return Resource::HTMLResourceType;
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


void HTMLResource::SetDomDocument( shared_ptr< xc::DOMDocument > document )
{
    QWriteLocker locker( &GetLock() );

    m_DomDocument = document;
    MarkSecondaryCachesAsOld();
}


const xc::DOMDocument& HTMLResource::GetDomDocumentForReading()
{
    return *m_DomDocument;
}


xc::DOMDocument& HTMLResource::GetDomDocumentForWriting()
{
    // We can't just mark the caches as old right here since
    // some consumers need write access but don't end up writing anything.

    return *m_DomDocument;
}


void HTMLResource::MarkSecondaryCachesAsOld()
{
    m_WebPageIsOld      = true;
    m_TextDocumentIsOld = true;
}

void HTMLResource::UpdateDomDocumentFromWebPage()
{
    if ( !WebPageModified() )

        return;

    Q_ASSERT( QThread::currentThread() == QApplication::instance()->thread() );

    m_DomDocument = XhtmlDoc::LoadTextIntoDocument( GetWebPageHTML() );

    m_TextDocumentIsOld = true;
    SetWebPageModified( false );
}


void HTMLResource::UpdateDomDocumentFromTextDocument()
{
    if ( !TextDocumentModified() )

        return;

    Q_ASSERT( QThread::currentThread() == QApplication::instance()->thread() );
    Q_ASSERT( m_TextDocument );

    m_DomDocument = XhtmlDoc::LoadTextIntoDocument( CleanSource::Clean( m_TextDocument->toPlainText() ) );

    m_WebPageIsOld = true;
    SetTextDocumentModified( false );
}


void HTMLResource::UpdateWebPageFromDomDocument()
{
    Q_ASSERT( QThread::currentThread() == QApplication::instance()->thread() );

    if ( !m_WebPageIsOld && !m_RefreshNeeded )

        return;

    if ( m_WebPage == NULL )
    {
        m_WebPage = new QWebPage( this );
        connect( m_WebPage, SIGNAL( contentsChanged() ), this, SLOT( SetWebPageModified() ) );
    }

    SetWebPageHTML( XhtmlDoc::GetDomDocumentAsString( *m_DomDocument ) );

    m_WebPageIsOld = false;
    m_RefreshNeeded = false;
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

    m_TextDocument->setPlainText( CleanSource::PrettyPrint( XhtmlDoc::GetDomDocumentAsString( *m_DomDocument ) ) );

    m_TextDocumentIsOld = false;
}


void HTMLResource::UpdateWebPageFromTextDocument()
{
    Q_ASSERT( QThread::currentThread() == QApplication::instance()->thread() );
    Q_ASSERT( m_TextDocument );
    Q_ASSERT( m_WebPage );

    const QString &source  = m_TextDocument->toPlainText();
    const QString &cleaned = CleanSource::Clean( source );

    // We can't just check for the modified state of the other
    // cache since the other cache could have saved its changes
    // back to the DOM and thus be set as unmodified.

    if ( m_OldSourceCache != cleaned || m_RefreshNeeded )
    {
        SetWebPageHTML( cleaned );

        // We store the original, "uncleaned" source
        // in the source cache since that is what we
        // want to compare against. If we didn't, the user
        // would see the old source in the TextDocument even
        // after we became committed to the cleaned source.
        // See issue #286.
        m_OldSourceCache = source;
        m_RefreshNeeded  = false;
    }

    // We need to transfer the modified state to the other cache
    SetWebPageModified( TextDocumentModified() );
}


void HTMLResource::UpdateTextDocumentFromWebPage()
{
    Q_ASSERT( QThread::currentThread() == QApplication::instance()->thread() );
    Q_ASSERT( m_TextDocument );
    Q_ASSERT( m_WebPage );

    const QString &source = GetWebPageHTML();

    // We can't just check for the modified state of the other
    // cache since the other cache could have saved its changes
    // back to the DOM and thus be set as unmodified.

    if ( m_OldSourceCache != source )
    {
        m_TextDocument->setPlainText( source );
        m_OldSourceCache = source;
    }

    // We need to transfer the modified state to the other cache
    SetTextDocumentModified( WebPageModified() );
}


void HTMLResource::SaveToDisk( bool book_wide_save )
{
    {
        QWriteLocker locker( &GetLock() );

        Utility::WriteUnicodeTextFile( CleanSource::PrettyPrint( XhtmlDoc::GetDomDocumentAsString( *m_DomDocument ) ),
                                       GetFullPath() );
    }

    if ( !book_wide_save )

        emit ResourceUpdatedOnDisk();
}


void HTMLResource::RemoveWebkitCruft()
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

    QWebElement body_tag =  m_WebPage->mainFrame()->findFirstElement( "body" );

    // Removing junk webkit styles
    body_tag.setStyleProperty( "word-wrap", "" );
    body_tag.setStyleProperty( "-webkit-nbsp-mode", "" );
    body_tag.setStyleProperty( "-webkit-line-break", "" );

    // Banish the irritating <body style=""> tags
    if( body_tag.attribute( "style", "none" ) == "" )
    {
        body_tag.removeAttribute( "style" );
    }
}


QStringList HTMLResource::SplitOnSGFChapterMarkers()
{
    QStringList chapters = XhtmlDoc::GetSGFChapterSplits( XhtmlDoc::GetDomDocumentAsString( *m_DomDocument ) );

    m_DomDocument = XhtmlDoc::LoadTextIntoDocument( CleanSource::Clean( chapters.takeFirst() ) );
    MarkSecondaryCachesAsOld();

    return chapters;
}


void HTMLResource::LinkedResourceUpdated()
{
    m_RefreshNeeded = true;

    QWebSettings::clearMemoryCaches();
}


// We need to load the jQuery libs here since
// we sometimes need to use them outside of
// BookViewEditor.
void HTMLResource::WebPageJavascriptOnLoad()
{
    Q_ASSERT( m_WebPage );

    m_WebPage->mainFrame()->evaluateJavaScript( c_jQuery         );
    m_WebPage->mainFrame()->evaluateJavaScript( c_jQueryScrollTo );
}


void HTMLResource::SetWebPageModified( bool modified )
{
    m_WebPageModified = modified;
}


// QWebPage::isModified() only looks at form data,
// which makes it absolutely useless.
bool HTMLResource::WebPageModified()
{
    return m_WebPageModified;
}


// We have a "modified" getter and setter for
// the TextDocument even when it has its own
// because QWebPage does not and we want a unified interface.
void HTMLResource::SetTextDocumentModified( bool modified )
{
    m_TextDocument->setModified( modified );
}


bool HTMLResource::TextDocumentModified()
{
    return m_TextDocument->isModified();
}


QString HTMLResource::GetWebPageHTML()
{
    Q_ASSERT( m_WebPage );

    RemoveWebkitCruft();

    // Set the xml tag here rather than let Tidy do it.
    // This prevents false mismatches with the cache later on.
    QString html_from_Qt = m_WebPage->mainFrame()->toHtml();
    return ConvertToEntities( CleanSource::PrettyPrint( CleanSource::Clean( XML_TAG % html_from_Qt ) ) );
}


void HTMLResource::SetWebPageHTML( const QString &source )
{
    Q_ASSERT( m_WebPage );

    connect( m_WebPage, SIGNAL( loadFinished( bool ) ), this, SLOT( WebPageJavascriptOnLoad() ) );

    // NOTE: content loading is asynchronous, and attempts to read the content back out immediately
    // with toHtml() *will* fail if the page contains external links, particularly if those cannot
    // be resolved and it ends up waiting for a timeout. The web page content is only readable
    // once it issues the loadFinished() signal.
    m_WebPage->mainFrame()->setContent( source.toUtf8(), LOADED_CONTENT_MIMETYPE, GetBaseUrl() );

    m_WebPage->setContentEditable( true );

    // TODO: we kill external links; a dialog should be used
    // that asks the user if he wants to open this external link in a browser
    m_WebPage->setLinkDelegationPolicy( QWebPage::DelegateAllLinks );

    TrackNewResources( GetPathsToLinkedResources() );

    QWebSettings &settings = *m_WebPage->settings();
    settings.setAttribute( QWebSettings::LocalContentCanAccessRemoteUrls, false );
    settings.setAttribute( QWebSettings::JavascriptCanAccessClipboard, true );
    settings.setAttribute( QWebSettings::ZoomTextOnly, true );
}


QStringList HTMLResource::GetPathsToLinkedResources()
{
    Q_ASSERT( m_WebPage );

    QWebElementCollection elements = m_WebPage->mainFrame()->findAllElements( "link, img" );

    QStringList linked_resources;

    foreach( QWebElement element, elements )
    {
        // We skip the link elements that are not stylesheets
        if ( element.tagName().toLower() == "link" && 
             element.attribute( "rel" ).toLower() != "stylesheet" )
        {
            continue;
        }

        if ( element.hasAttribute( "href" ) )
        
            linked_resources.append( element.attribute( "href" ) );        

        else if ( element.hasAttribute( "src" ) )
        
            linked_resources.append( element.attribute( "src" ) );
    }

    return linked_resources;
}


void HTMLResource::TrackNewResources( const QStringList &filepaths )
{    
    foreach( QString resource_id, m_LinkedResourceIDs )
    {
        Resource *resource = m_Resources.value( resource_id );

        if ( resource )
        {
            disconnect( resource, SIGNAL( ResourceUpdatedOnDisk() ),    this, SLOT( LinkedResourceUpdated() ) );
            disconnect( resource, SIGNAL( Deleted( const Resource& ) ), this, SLOT( LinkedResourceUpdated() ) );
        }
    }

    m_LinkedResourceIDs.clear();
    QStringList filenames;

    foreach( QString filepath, filepaths )
    {
        filenames.append( QFileInfo( filepath ).fileName() );
    }

    foreach( Resource *resource, m_Resources.values() )
    {
        if ( filenames.contains( resource->Filename() ) )
            
            m_LinkedResourceIDs.append( resource->GetIdentifier() );
    }

    foreach( QString resource_id, m_LinkedResourceIDs )
    {
        Resource *resource = m_Resources.value( resource_id );

        if ( resource )
        {
            connect( resource, SIGNAL( ResourceUpdatedOnDisk() ),    this, SLOT( LinkedResourceUpdated() ) );
            connect( resource, SIGNAL( Deleted( const Resource& ) ), this, SLOT( LinkedResourceUpdated() ) );
        }
    }
}


QString HTMLResource::ConvertToEntities( const QString &source )
{
    QString newsource = source;

    QChar shy[] = { 0x00ad };
    newsource = newsource.replace( QString::fromRawData( shy, 1 ), "&shy;" );
    QChar mdash[] = { 0x2014 };
    newsource = newsource.replace( QString::fromRawData( mdash, 1 ), "&mdash;" );
    QChar ndash[] = { 0x213 };
    newsource = newsource.replace( QString::fromRawData( ndash, 1 ), "&ndash;" );

    return newsource;
}
