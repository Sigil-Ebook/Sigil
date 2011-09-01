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
const QString XML_NAMESPACE_CRUFT = "xmlns=\"http://www.w3.org/1999/xhtml\"";
const QString REPLACE_SPANS = "<span class=\"SigilReplace_\\d*\"( id=\"SigilReplace_\\d*\")*>";

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
    c_jQuery(         Utility::ReadUnicodeTextFile( ":/javascript/jquery-1.6.2.min.js"          ) ),
    c_jQueryScrollTo( Utility::ReadUnicodeTextFile( ":/javascript/jquery.scrollTo-1.4.2-min.js" ) ),
    c_jQueryWrapSelection( Utility::ReadUnicodeTextFile( ":/javascript/jquery.wrapSelection.js" ) ),
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

    m_DomDocument = XhtmlDoc::LoadTextIntoDocument( ConvertToEntities( CleanSource::Clean( m_TextDocument->toPlainText() ) ) );

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

    SetWebPageModified( false );
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

    m_TextDocument->setPlainText( ConvertToEntities( CleanSource::PrettyPrint( XhtmlDoc::GetDomDocumentAsString( *m_DomDocument ) ) ) );

    m_TextDocument->setModified( false );
    m_TextDocumentIsOld = false;
}


void HTMLResource::SaveToDisk( bool book_wide_save )
{
    {
        QWriteLocker locker( &GetLock() );

        Utility::WriteUnicodeTextFile( ConvertToEntities( CleanSource::PrettyPrint( XhtmlDoc::GetDomDocumentAsString( *m_DomDocument ) ) ),
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

QString HTMLResource::RemoveBookViewReplaceSpans( const QString &source )
{
    QRegExp replace_spans( REPLACE_SPANS );
    replace_spans.setMinimal( true );
    QRegExp span_open_or_close( "<\\s*(/)*\\s*span\\s*>");
    span_open_or_close.setMinimal( true );

    QString newsource = "";
    int left_pos = 0;
    int index = source.indexOf( replace_spans );
    while( index != -1 )
    {
        // Append the text between the last capture and this one.
        newsource.append( source.mid( left_pos, index - left_pos ) );

        // Advance past the captured opening tag.
        index += replace_spans.cap(0).length();
        left_pos = index;

        // Check for nested spans.
        int nest_count = 1; // set to 1 as we already have an open span
        int next_span_tag = index;
        do 
        {
            next_span_tag = source.indexOf( span_open_or_close, index );
            if( next_span_tag == -1 )
            {
                // Content is not well-formed, which should never happen here.
                boost_throw( ErrorParsingXml()
                             << errinfo_XML_parsing_error_string( "GetWebPageHTML() has returned invalid xhtml" ) );
            }

            if( !span_open_or_close.cap(0).contains( "/" ) )
            {
                // Opening tag, so increment the counter.
                nest_count++;
            }
            else
            {
                // Closing tag, so decrement the counter
                nest_count--;
            }
        } while( nest_count > 0 );

        // next_span_tag now points to the start of the closing tag of the span we're removing.
        // Append the source from the end of the span tag to the start of the closing tag
        newsource.append( source.mid( index, next_span_tag - index ) );

        // Move left_pos past the closing tag and search for another span to remove.
        left_pos = next_span_tag + span_open_or_close.cap(0).length(); 
        index = source.indexOf( replace_spans, left_pos );
    }

    // Append the rest of the source after all the spans have been removed.
    newsource.append( source.mid( left_pos ) );

    // It's possible that we might have replace spans nested within each other,
    // so go back to the start and check again, and recurse if found.
    if( newsource.indexOf( replace_spans ) != -1 )
    {
        newsource = RemoveBookViewReplaceSpans( newsource );
    }

    return newsource;
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
    m_WebPage->mainFrame()->evaluateJavaScript( c_jQueryWrapSelection );
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

    html_from_Qt = RemoveBookViewReplaceSpans( html_from_Qt );
    html_from_Qt = html_from_Qt.remove( XML_NAMESPACE_CRUFT );
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

    // Track resources whose change will necessitate an update of the WebView.
    // At present this only applies to css files and images.
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

    newsource = newsource.replace( QString::fromUtf8( "\u00ad" ), "&shy;" );
    newsource = newsource.replace( QString::fromUtf8( "\u2014" ), "&mdash;" );
    newsource = newsource.replace( QString::fromUtf8( "\u2013" ), "&ndash;" );

    return newsource;
}
