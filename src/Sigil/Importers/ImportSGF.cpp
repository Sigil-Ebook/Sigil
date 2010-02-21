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
#include "ImportSGF.h"
#include "../Misc/Utility.h"
#include "../BookManipulation/CleanSource.h"
#include "ResourceObjects/HTMLResource.h"
#include "ResourceObjects/TextResource.h"
#include "../SourceUpdates/PerformHTMLUpdates.h"


// Constructor;
// The parameter is the file to be imported
ImportSGF::ImportSGF( const QString &fullfilepath )
    : ImportOEBPS( fullfilepath )
{

}


// Reads and parses the file 
// and returns the created Book;
// Overrides;
QSharedPointer< Book > ImportSGF::GetBook()
{
    if ( !Utility::IsFileReadable( m_FullFilePath ) )

        boost_throw( CannotReadFile() << errinfo_file_fullpath( m_FullFilePath.toStdString() ) );

    // These read the EPUB file
    ExtractContainer();
    LocateOPF();
    ReadOPF();

    // These mutate the m_Book object
    LoadMetadata();

    QString source = LoadSource();
    QString header = CreateHeader( CreateStyleResources( source ) );

    // We remove the first and only XHTML resource
    // since we don't want to load that directly.
    // We will chop it up and create new XHTMLs in CreateXHTMLFiles.
    m_Files.remove( m_ReadingOrderIds.at( 0 ) );

    CreateXHTMLFiles( source, header, LoadFolderStructure() );
    UpdateAnchors( GetIDLocations() );

    return m_Book;
}


// Loads the source code into the Book
QString ImportSGF::LoadSource()
{
    QString fullpath = QFileInfo( m_OPFFilePath ).absolutePath() + "/" + m_Files.values().first();
    return CleanSource::ToValidXHTML( Utility::ReadUnicodeTextFile( fullpath ) ); 
}


// Creates style files from the style tags in the source
// and returns a list of their file paths relative 
// to the OEBPS folder in the FolderKeeper
QList< Resource* > ImportSGF::CreateStyleResources( const QString &source )
{    
    QList< XHTMLDoc::XMLElement > style_tag_nodes = XHTMLDoc::GetTagsInHead( source, "style" );

    QString folderpath = Utility::GetNewTempFolderPath();
    QDir dir( folderpath );
    dir.mkpath( folderpath );

    QFutureSynchronizer< Resource* > sync;

    for ( int i = 0; i < style_tag_nodes.count(); ++i ) 
    {
        sync.addFuture( QtConcurrent::run( 
            this, &ImportSGF::CreateOneStyleFile, style_tag_nodes.at( i ), folderpath, i ) );        
    }

    sync.waitForFinished();

    QtConcurrent::run( Utility::DeleteFolderAndFiles, folderpath );

    QList< QFuture< Resource* > > futures = sync.futures();
    QList< Resource* > style_resources;

    for ( int i = 0; i < futures.count(); ++i )
    {
        style_resources.append( futures.at( i ).result() );
    }    

    return style_resources;
}


Resource* ImportSGF::CreateOneStyleFile( const XHTMLDoc::XMLElement &element, 
                                         const QString &folderpath, 
                                         int index )
{
    QString style_text = element.text;
    style_text = RemoveSigilStyles( style_text );
    style_text = StripCDATA( style_text );
    style_text = style_text.trimmed();  

    if ( style_text.isEmpty() )

        return NULL;
    
    QString extension    = element.attributes.value( "type" ) == "text/css" ? "css" : "xpgt";
    QString filename     = QString( "style" ) + QString( "%1" ).arg( index + 1, 3, 10, QChar( '0' ) ) + "." + extension;
    QString fullfilepath = folderpath + "/" + filename;

    Utility::WriteUnicodeTextFile( style_text, fullfilepath );

    TextResource *text_resource = qobject_cast< TextResource* >( 
                                     &m_Book->mainfolder.AddContentFileToFolder( fullfilepath ) );

    Q_ASSERT( text_resource );
    text_resource->SetText( style_text );

    return text_resource;
}


// Strips CDATA declarations from the provided source
QString ImportSGF::StripCDATA( const QString &style_source )
{
    QString newsource = style_source;

    newsource.replace( "/*<![CDATA[*/", "" );
    newsource.replace( "/*]]>*/", "" );
    newsource.replace( "/**/", "" );

    newsource.replace( "<![CDATA[", "" );
    newsource.replace( "]]>", "" );

    return newsource;
}


// Removes Sigil styles from the provided source
QString ImportSGF::RemoveSigilStyles( const QString &style_source )
{
    QString newsource = style_source;

    QRegExp chapter_break_style( "hr\\.sigilChapterBreak[^\\}]+\\}" );
    QRegExp sigil_comment( "/\\*SG.*SG\\*/" );

    sigil_comment.setMinimal( true );

    newsource.remove( chapter_break_style );
    newsource.remove( sigil_comment );

    return newsource;
}


// Takes a list of style sheet file names 
// and returns the header for XHTML files
QString ImportSGF::CreateHeader( const QList< Resource* > &style_resources )
{
    QString header = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                     "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\"\n"
                     "    \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n\n"							
                     "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n"
                     "<head>\n";

    foreach( Resource* resource, style_resources )
    {
        if ( resource )
        {
            header += "    <link href=\"../" + resource->GetRelativePathToOEBPS() + 
                      "\" rel=\"stylesheet\" type=\"text/css\" />\n";
        }
    }

    header += "</head>\n";

    return header;
}


// Creates XHTML files from the book source;
// the provided header is used as the header of the created files
void ImportSGF::CreateXHTMLFiles( const QString &source, 
                                  const QString &header,
                                  const QHash< QString, QString > &html_updates )
{
    QRegExp body_start_tag( BODY_START );
    QRegExp body_end_tag( BODY_END );

    int body_begin = source.indexOf( body_start_tag, 0 ) + body_start_tag.matchedLength();
    int body_end   = source.indexOf( body_end_tag,   0 );

    int main_index    = body_begin;
    int reading_order = 0;

    QDir dir( Utility::GetNewTempFolderPath() );
    dir.mkpath( dir.absolutePath() );
    QString folderpath = dir.absolutePath();

    QFutureSynchronizer< void > sync;

    while ( main_index != body_end )
    {
        // move up?
        QRegExp break_tag( BREAK_TAG_SEARCH );

        // We search for our HR break tag
        int break_index = source.indexOf( break_tag, main_index );

        QString body;

        // We break up the remainder of the file on the HR tag index if it's found
        if ( break_index > -1 )
        {
            body = Utility::Substring( main_index, break_index, source );
            main_index = break_index + break_tag.matchedLength();
        }

        // Otherwise, we take the rest of the file
        else
        {
            body = Utility::Substring( main_index, body_end, source );
            main_index = body_end;
        }

        QString wholefile = header + "<body>\n" + body + "</body> </html>";

        sync.addFuture( 
            QtConcurrent::run( this, &ImportSGF::CreateOneXHTMLFile, wholefile, reading_order, folderpath, html_updates ) );

        ++reading_order;
    }	

    sync.waitForFinished();

    QtConcurrent::run( Utility::DeleteFolderAndFiles, folderpath );
}


void ImportSGF::CreateOneXHTMLFile( QString source, 
                                    int reading_order, 
                                    const QString &folderpath,
                                    const QHash< QString, QString > &html_updates )
{
    QString filename     = QString( "content" ) + QString( "%1" ).arg( reading_order + 1, 3, 10, QChar( '0' ) ) + ".xhtml";
    QString fullfilepath = folderpath + "/" + filename;

    Utility::WriteUnicodeTextFile( "PLACEHOLDER", fullfilepath );

    HTMLResource *html_resource = qobject_cast< HTMLResource* >( 
                                        &m_Book->mainfolder.AddContentFileToFolder( fullfilepath ) );

    Q_ASSERT( html_resource );

    html_resource->SetDomDocument( 
        PerformHTMLUpdates( CleanSource::Clean( source ), html_updates, QHash< QString, QString >() )() );
}


QHash< QString, QString > ImportSGF::GetIDLocations()
{
    QList< HTMLResource* > html_resources = m_Book->mainfolder.GetSortedHTMLResources();

    QList< tuple< QString, QList< QString > > > IDs_in_files =
        QtConcurrent::blockingMapped( html_resources, GetOneFileIDs );

    QHash< QString, QString > ID_locations;

    for ( int i = 0; i < IDs_in_files.count(); ++i )
    {
        QList< QString > file_element_IDs;
        QString resource_filename;

        tie( resource_filename, file_element_IDs ) = IDs_in_files.at( i );

        for ( int j = 0; j < file_element_IDs.count(); ++j )
        {
            ID_locations[ file_element_IDs.at( j ) ] = resource_filename;
        }
    }

    return ID_locations;
}


tuple< QString, QList< QString > > ImportSGF::GetOneFileIDs( HTMLResource* html_resource )
{
    Q_ASSERT( html_resource );

    QReadLocker locker( &html_resource->GetLock() );

    return make_tuple( html_resource->Filename(),
                       XHTMLDoc::GetAllChildIDs( html_resource->GetDomDocumentForReading().documentElement() ) );
}

void ImportSGF::UpdateAnchors( const QHash< QString, QString > ID_locations )
{
    QList< HTMLResource* > html_resources = m_Book->mainfolder.GetSortedHTMLResources();

    QtConcurrent::blockingMap( html_resources, boost::bind( UpdateAnchorsInOneFile, _1, ID_locations ) );
}


void ImportSGF::UpdateAnchorsInOneFile( HTMLResource *html_resource, 
                                        const QHash< QString, QString > ID_locations )
{
    Q_ASSERT( html_resource );

    QWriteLocker locker( &html_resource->GetLock() );

    QDomDocument document = html_resource->GetDomDocumentForWriting();
    QDomNodeList anchors = document.elementsByTagName( "a" );

    QString resource_filename = html_resource->Filename();

    for ( int i = 0; i < anchors.count(); ++i )
    {
        QDomElement element = anchors.at( i ).toElement();

        Q_ASSERT( !element.isNull() );

        if ( element.hasAttribute( "href" ) &&
             QUrl( element.attribute( "href" ) ).isRelative() &&
             element.attribute( "href" ).contains( "#" )
           )
        {
            // Remove the '#' character
            QString id = element.attribute( "href" ).remove( 0, 1 );

            // If the ID is in a different file, update the link
            if ( ID_locations.value( id ) != resource_filename )

                element.setAttribute( "href", "../" + TEXT_FOLDER_NAME + "/" + ID_locations.value( id ) + "#" + id );            
        } 
    }
}