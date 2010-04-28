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
#include "../BookManipulation/Book.h"
#include "../Misc/Utility.h"
#include "ResourceObjects/HTMLResource.h"
#include "../BookManipulation/CleanSource.h"
#include "../SourceUpdates/AnchorUpdates.h"
#include "../SourceUpdates/PerformHTMLUpdates.h"
#include "../SourceUpdates/AnchorUpdates.h"
#include <QDomDocument>

static const QString FIRST_CSS_NAME   = "Style0001.css";
static const QString PLACEHOLDER_TEXT = "PLACEHOLDER";
static const QString EMPTY_HTML_FILE  = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
                                        "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\"\n"
                                        "    \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n\n"							
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
    m_PublicationIdentifier( Utility::CreateUUID() )
{
   
}


Book::Book( const Book& other )
{
    m_Metadata = other.m_Metadata;
    m_PublicationIdentifier = other.m_PublicationIdentifier;
    m_Mainfolder = other.m_Mainfolder;
}


Book& Book::operator = ( const Book& other )
{
    // Protect against invalid self-assignment
    if ( this != &other ) 
    {
        m_Metadata = other.m_Metadata;
        m_PublicationIdentifier = other.m_PublicationIdentifier;
        m_Mainfolder = other.m_Mainfolder;
    }

    // By convention, always return *this
    return *this;
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


QString Book::GetPublicationIdentifier()
{
    return m_PublicationIdentifier;
}


QHash< QString, QList< QVariant > > Book::GetMetadata()
{
    return m_Metadata;
}


void Book::SetMetadata( const QHash< QString, QList< QVariant > > metadata )
{
    m_Metadata = metadata;
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

    return html_resource;
}


void Book::CreateEmptyHTMLFile()
{
    QDomDocument document;
    document.setContent( EMPTY_HTML_FILE );

    CreateNewHTMLFile().SetDomDocument( document );
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
}


HTMLResource& Book::CreateChapterBreakOriginalResource( const QString &content, HTMLResource& originating_resource )
{
    const QString &originating_filename = originating_resource.Filename();

    originating_resource.RenameTo( m_Mainfolder.GetUniqueFilenameVersion( FIRST_CHAPTER_NAME ) );

    int reading_order = originating_resource.GetReadingOrder();
    Q_ASSERT( reading_order >= 0 );

    QList< HTMLResource* > html_resources = m_Mainfolder.GetSortedHTMLResources();

    // We need to "make room" for the reading order of the new resource
    for ( int i = reading_order; i < html_resources.count(); ++i )
    {
        HTMLResource* resource = html_resources[ i ];
        resource->SetReadingOrder( resource->GetReadingOrder() + 1 );
    }

    HTMLResource &html_resource = CreateNewHTMLFile();
    html_resource.RenameTo( originating_filename );

    QDomDocument document;
    document.setContent( CleanSource::Clean( content ) );
    html_resource.SetDomDocument( document );

    html_resource.SetReadingOrder( reading_order );

    // We can just append this since we don't need
    // them in sorted order for the updates.
    html_resources.append( &html_resource );
    AnchorUpdates::UpdateAllAnchorsWithIDs( html_resources );

    return html_resource;
}

void Book::CreateNewChapters( const QStringList& new_chapters )
{
    CreateNewChapters( new_chapters, QHash< QString, QString >() );
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

    AnchorUpdates::UpdateAllAnchorsWithIDs( m_Mainfolder.GetSortedHTMLResources() );
}


void Book::SaveAllResourcesToDisk()
{
    QList< Resource* > resources =  m_Mainfolder.GetResourceList();
    QtConcurrent::blockingMap( resources, SaveOneResourceToDisk );
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
        document.setContent( CleanSource::Clean( source ) );
        html_resource->SetDomDocument( document );
    }

    else
    {
        html_resource->SetDomDocument( 
            PerformHTMLUpdates( CleanSource::Clean( source ), html_updates, QHash< QString, QString >() )() );
    }    
}
