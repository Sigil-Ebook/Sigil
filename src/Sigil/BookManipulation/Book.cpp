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


void Book::SaveAllResourcesToDisk()
{
    QtConcurrent::blockingMap( m_Mainfolder.GetResourceList(), SaveOneResourceToDisk );    
}


void Book::SaveOneResourceToDisk( Resource *resource )
{
    resource->SaveToDisk();        
}

