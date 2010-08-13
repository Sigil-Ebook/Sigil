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

#include <stdafx.h>
#include "../BookManipulation/Book.h"
#include "../BookManipulation/XHTMLDoc.h"
#include "../Misc/Utility.h"
#include "ResourceObjects/HTMLResource.h"
#include "../BookManipulation/CleanSource.h"
#include "../SourceUpdates/AnchorUpdates.h"
#include "../SourceUpdates/PerformHTMLUpdates.h"
#include "../SourceUpdates/AnchorUpdates.h"
#include "../SourceUpdates/UniversalUpdates.h"
#include <QDomDocument>

static const QString FIRST_CSS_NAME   = "Style0001.css";
static const QString PLACEHOLDER_TEXT = "PLACEHOLDER";
static const QString EMPTY_HTML_FILE  = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
                                        "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.1//EN\"\n"
                                        "    \"http://www.w3.org/TR/xhtml1/DTD/xhtml11.dtd\">\n\n"							
                                        "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n"
                                        "<head>\n"
                                        "<title></title>\n"
                                        "</head>\n"
                                        "<body>\n"

                                        // The "nbsp" is here so that the user starts writing
                                        // inside the <p> element; if it's not here, webkit
                                        // inserts text _outside_ the <p> element
                                        "<p>&nbsp;</p>\n"
                                        "</body>\n"
                                        "</html>";


Book::Book()
    : 
    m_PublicationIdentifier( Utility::CreateUUID() ),
    m_IsModified( false )
{
   
}


QUrl Book::GetBaseUrl() const
{
    return QUrl::fromLocalFile( m_Mainfolder.GetFullPathToTextFolder() + "/" );
}



FolderKeeper& Book::GetFolderKeeper()
{
    return m_Mainfolder;
}


const FolderKeeper& Book::GetConstFolderKeeper()
{
    return m_Mainfolder;
}


QString Book::GetPublicationIdentifier() const
{
    return m_PublicationIdentifier;
}


QHash< QString, QList< QVariant > > Book::GetMetadata() const
{
    return m_Metadata;
}


void Book::SetMetadata( const QHash< QString, QList< QVariant > > metadata )
{
    m_Metadata = metadata;
    SetModified( true );
}


HTMLResource& Book::CreateNewHTMLFile()
{
    QString folderpath = Utility::GetNewTempFolderPath();
    QDir dir( folderpath );
    dir.mkpath( folderpath );

    QString fullfilepath = folderpath + "/" + m_Mainfolder.GetUniqueFilenameVersion( FIRST_CHAPTER_NAME );
    int reading_order = m_Mainfolder.GetHighestReadingOrder() + 1;

    Utility::WriteUnicodeTextFile( PLACEHOLDER_TEXT, fullfilepath );

    HTMLResource &html_resource = *qobject_cast< HTMLResource* >( 
                                        &m_Mainfolder.AddContentFileToFolder( fullfilepath, reading_order ) );

    QtConcurrent::run( Utility::DeleteFolderAndFiles, dir.absolutePath() );

    SetModified( true );
    return html_resource;
}


void Book::CreateEmptyHTMLFile()
{
    QDomDocument document;
    XHTMLDoc::LoadTextIntoDocument( EMPTY_HTML_FILE, document );

    CreateNewHTMLFile().SetDomDocument( document );
    SetModified( true );
}


void Book::CreateEmptyCSSFile()
{
    QString folderpath = Utility::GetNewTempFolderPath();
    QDir dir( folderpath );
    dir.mkpath( folderpath );

    QString fullfilepath = folderpath + "/" + m_Mainfolder.GetUniqueFilenameVersion( FIRST_CSS_NAME );

    Utility::WriteUnicodeTextFile( "", fullfilepath );

    m_Mainfolder.AddContentFileToFolder( fullfilepath );

    QtConcurrent::run( Utility::DeleteFolderAndFiles, dir.absolutePath() );
    SetModified( true );
}


HTMLResource& Book::CreateChapterBreakOriginalResource( const QString &content, HTMLResource& originating_resource )
{
    const QString &originating_filename = originating_resource.Filename();

    originating_resource.RenameTo( m_Mainfolder.GetUniqueFilenameVersion( FIRST_CHAPTER_NAME ) );

    int reading_order = originating_resource.GetReadingOrder();
    Q_ASSERT( reading_order >= 0 );

    QList< HTMLResource* > html_resources = m_Mainfolder.GetResourceTypeList< HTMLResource >( true );

    // We need to "make room" for the reading order of the new resource
    for ( int i = reading_order; i < html_resources.count(); ++i )
    {
        HTMLResource* resource = html_resources[ i ];
        resource->SetReadingOrder( resource->GetReadingOrder() + 1 );
    }

    HTMLResource &html_resource = CreateNewHTMLFile();
    html_resource.RenameTo( originating_filename );

    QDomDocument document;
    XHTMLDoc::LoadTextIntoDocument( CleanSource::Clean( content ), document );
    html_resource.SetDomDocument( document );

    html_resource.SetReadingOrder( reading_order );

    // We can just append this since we don't need
    // them in sorted order for the updates.
    html_resources.append( &html_resource );
    AnchorUpdates::UpdateAllAnchorsWithIDs( html_resources );

    SetModified( true );
    return html_resource;
}

void Book::CreateNewChapters( const QStringList& new_chapters )
{
    CreateNewChapters( new_chapters, QHash< QString, QString >() );
    SetModified( true );
}


void Book::CreateNewChapters( const QStringList& new_chapters,
                              const QHash< QString, QString > &html_updates )
{
    if ( new_chapters.isEmpty() )

        return;

    QDir dir( Utility::GetNewTempFolderPath() );
    dir.mkpath( dir.absolutePath() );
    QString folderpath = dir.absolutePath();

    QFutureSynchronizer< void > sync;

    int next_reading_order = m_Mainfolder.GetHighestReadingOrder() + 1;

    for ( int i = 0; i < new_chapters.count(); ++i )
    {
        int reading_order = next_reading_order + i;

        sync.addFuture( 
            QtConcurrent::run( 
                this, &Book::CreateOneNewChapter, new_chapters.at( i ), reading_order, folderpath, html_updates ) );
    }	

    sync.waitForFinished();

    QtConcurrent::run( Utility::DeleteFolderAndFiles, folderpath );

    AnchorUpdates::UpdateAllAnchorsWithIDs( m_Mainfolder.GetResourceTypeList< HTMLResource >() );
    SetModified( true );
}


void Book::MergeWithPrevious( HTMLResource& html_resource )
{
    int previous_file_reading_order = html_resource.GetReadingOrder() - 1;
    Q_ASSERT( previous_file_reading_order >= 0 );

    QList< HTMLResource* > html_resources = m_Mainfolder.GetResourceTypeList< HTMLResource >( true );
    HTMLResource& previous_html = *html_resources[ previous_file_reading_order ];

    QString html_resource_fullpath = html_resource.GetFullPath();

    {
        QDomDocumentFragment body_children_fragment;

        {
            QWriteLocker source_locker( &html_resource.GetLock() );
            QDomDocument source_dom        = html_resource.GetDomDocumentForReading();
            QDomNodeList source_body_nodes = source_dom.elementsByTagName( "body" );

            if ( source_body_nodes.count() != 1 )

                return;

            QDomNode source_body_node = source_body_nodes.at( 0 );
            body_children_fragment    = XHTMLDoc::ConvertToDocumentFragment( source_body_node.childNodes() ); 
        }  

        QWriteLocker sink_locker( &previous_html.GetLock() );
        QDomDocument sink_dom        = previous_html.GetDomDocumentForWriting();
        QDomNodeList sink_body_nodes = sink_dom.elementsByTagName( "body" );

        if ( sink_body_nodes.count() != 1 )

            return;

        QDomNode sink_body_node = sink_body_nodes.at( 0 );         
        sink_body_node.appendChild( sink_dom.importNode( body_children_fragment, true ) );
        previous_html.MarkSecondaryCachesAsOld();

        html_resource.Delete();
    }

    // The html_resources list is now old after we deleted one,
    // and PerformUniversalUpdates accepts generic Resources
    QList< Resource* > resources = m_Mainfolder.GetResourceTypeAsGenericList< HTMLResource >();
    AnchorUpdates::UpdateAllAnchorsWithIDs( html_resources );

    QHash< QString, QString > updates;
    updates[ html_resource_fullpath ] = "../" + previous_html.GetRelativePathToOEBPS();

    UniversalUpdates::PerformUniversalUpdates( true, resources, updates );
    NormalizeReadingOrders();
    SetModified( true );
}


void Book::SaveAllResourcesToDisk()
{
    QList< Resource* > resources =  m_Mainfolder.GetResourceList();
    QtConcurrent::blockingMap( resources, SaveOneResourceToDisk );
}


void Book::NormalizeReadingOrders()
{
    QList< HTMLResource* > html_resources = m_Mainfolder.GetResourceTypeList< HTMLResource >( true );

    for ( int i = 0; i < html_resources.count(); ++i )
    {
        html_resources[ i ]->SetReadingOrder( i );
    }
}


bool Book::IsModified() const
{
    return m_IsModified;
}


void Book::SetModified( bool modified )
{
    bool old_modified_state = m_IsModified;
    m_IsModified = modified;

    if ( modified != old_modified_state )

        emit ModifiedStateChanged( m_IsModified );
}


void Book::SaveOneResourceToDisk( Resource *resource )
{
    resource->SaveToDisk( true );
}

void Book::CreateOneNewChapter( const QString &source,
                                int reading_order,
                                const QString &temp_folder_path )
{
    CreateOneNewChapter( source, reading_order, temp_folder_path, QHash< QString, QString >() );
}


void Book::CreateOneNewChapter( const QString &source, 
                                int reading_order, 
                                const QString &temp_folder_path,
                                const QHash< QString, QString > &html_updates )
{
    QString filename     = FIRST_CHAPTER_PREFIX + QString( "%1" ).arg( reading_order + 1, 4, 10, QChar( '0' ) ) + ".xhtml";
    QString fullfilepath = temp_folder_path + "/" + filename;

    Utility::WriteUnicodeTextFile( "PLACEHOLDER", fullfilepath );

    HTMLResource *html_resource = qobject_cast< HTMLResource* >( 
        &m_Mainfolder.AddContentFileToFolder( fullfilepath, reading_order ) );

    Q_ASSERT( html_resource );

    if ( html_updates.isEmpty() )
    {
        QDomDocument document;
        XHTMLDoc::LoadTextIntoDocument( CleanSource::Clean( source ), document );
        html_resource->SetDomDocument( document );
    }

    else
    {
        html_resource->SetDomDocument( 
            PerformHTMLUpdates( CleanSource::Clean( source ),
                                html_updates, 
                                QHash< QString, QString >() 
                              )() );
    }    
}

