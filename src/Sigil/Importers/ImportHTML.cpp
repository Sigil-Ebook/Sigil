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
#include "ImportHTML.h"
#include "Misc/Utility.h"
#include "Misc/TempFolder.h"
#include "Misc/HTMLEncodingResolver.h"
#include "BookManipulation/Metadata.h"
#include "BookManipulation/CleanSource.h"
#include "BookManipulation/XercesCppUse.h"
#include "ResourceObjects/HTMLResource.h"
#include "ResourceObjects/CSSResource.h"
#include "SourceUpdates/PerformHTMLUpdates.h"
#include "SourceUpdates/UniversalUpdates.h"
#include "BookManipulation/XhtmlDoc.h"


// Constructor;
// The parameter is the file to be imported
ImportHTML::ImportHTML( const QString &fullfilepath )
    : 
    Importer( fullfilepath ),
    m_IgnoreDuplicates( false )
{

}


void ImportHTML::SetBook( QSharedPointer< Book > book, bool ignore_duplicates )
{
    m_Book = book;
    m_IgnoreDuplicates = ignore_duplicates;
}


// Reads and parses the file 
// and returns the created Book
QSharedPointer< Book > ImportHTML::GetBook()
{
    if ( !Utility::IsFileReadable( m_FullFilePath ) )

        boost_throw( CannotReadFile() << errinfo_file_fullpath( m_FullFilePath.toStdString() ) );

    shared_ptr< xc::DOMDocument > document = XhtmlDoc::LoadTextIntoDocument( LoadSource() );

    StripFilesFromAnchors( *document );
    LoadMetadata( *document );

    UpdateFiles( CreateHTMLResource(), *document, LoadFolderStructure( *document ) );

    return m_Book;
}


// Loads the source code into the Book
QString ImportHTML::LoadSource()
{
    return 
        CleanSource::Clean( 
            XhtmlDoc::ResolveCustomEntities( 
                HTMLEncodingResolver::ReadHTMLFile( m_FullFilePath ) ) );
}


// Strips the file specifier on all the href attributes 
// of anchor tags with filesystem links with fragment identifiers;
// thus something like <a href="chapter01.html#firstheading" />
// becomes just <a href="#firstheading" />
void ImportHTML::StripFilesFromAnchors( xc::DOMDocument &document )
{
    QList< xc::DOMElement* > anchors = XhtmlDoc::GetTagMatchingDescendants( document, "a" );

    for ( int i = 0; i < anchors.count(); ++i )
    {
        xc::DOMElement &element = *anchors.at( i );
        Q_ASSERT( &element );

        // We strip the file specifier on all
        // the filesystem links with fragment identifiers
        if ( element.hasAttribute( QtoX( "href" ) ) &&
             QUrl( XtoQ( element.getAttribute( QtoX( "href" ) ) ) ).isRelative() &&
             XtoQ( element.getAttribute( QtoX( "href" ) ) ).contains( "#" )
           )
        {
            QString value = ( "#" + XtoQ( element.getAttribute( QtoX( "href" ) ) ).split( "#" )[ 1 ] );
            element.setAttribute( QtoX( "href" ), QtoX( value ) );            
        } 
    }     
}

// Searches for meta information in the HTML file
// and tries to convert it to Dublin Core
void ImportHTML::LoadMetadata( const xc::DOMDocument &document )
{
    QList< xc::DOMElement* > metatags = XhtmlDoc::GetTagMatchingDescendants( document, "meta" );

    QHash< QString, QList< QVariant > > metadata;

    for ( int i = 0; i < metatags.count(); ++i )
    {
        xc::DOMElement &element = *metatags.at( i );
       
        Metadata::MetaElement book_meta = Metadata::Instance().MapToBookMetadata( element );

        if ( !book_meta.name.isEmpty() && !book_meta.value.toString().isEmpty() )
        {
            metadata[ book_meta.name ].append( book_meta.value );
        }        
    }

    m_Book->SetMetadata( metadata );
}


HTMLResource& ImportHTML::CreateHTMLResource()
{
    TempFolder tempfolder;

    QString fullfilepath = tempfolder.GetPath() + "/" + QFileInfo( m_FullFilePath ).fileName();
    Utility::WriteUnicodeTextFile( "TEMP_SOURCE", fullfilepath );

    int reading_order = m_Book->GetConstFolderKeeper().GetHighestReadingOrder() + 1;

    HTMLResource &resource = *qobject_cast< HTMLResource* >(
                                &m_Book->GetFolderKeeper().AddContentFileToFolder( fullfilepath, reading_order ) );

    return resource;
}


void ImportHTML::UpdateFiles( HTMLResource &html_resource, 
                              xc::DOMDocument &document,
                              const QHash< QString, QString > &updates )
{
    Q_ASSERT( &html_resource != NULL );

    QHash< QString, QString > html_updates;
    QHash< QString, QString > css_updates;
    tie( html_updates, css_updates, boost::tuples::ignore ) = 
        UniversalUpdates::SeparateHtmlCssXmlUpdates( updates );

    QList< Resource* > all_files = m_Book->GetFolderKeeper().GetResourceList();
    int num_files = all_files.count();

    QList< CSSResource* > css_resources;

    for ( int i = 0; i < num_files; ++i )
    {
        Resource *resource = all_files.at( i );

        if ( resource->Type() == Resource::CSSResource )   
        
            css_resources.append( qobject_cast< CSSResource* >( resource ) );          
    }

    QFutureSynchronizer<void> sync;
    sync.addFuture( QtConcurrent::map( css_resources, 
        boost::bind( UniversalUpdates::LoadAndUpdateOneCSSFile, _1, css_updates ) ) );

    html_resource.SetDomDocument( PerformHTMLUpdates( document, html_updates, css_updates )() );

    sync.waitForFinished();
}


// Loads the referenced files into the main folder of the book;
// as the files get a new name, the references are updated
QHash< QString, QString > ImportHTML::LoadFolderStructure( const xc::DOMDocument &document )
{
    QFutureSynchronizer< QHash< QString, QString > > sync;

    sync.addFuture( QtConcurrent::run( this, &ImportHTML::LoadImages,     &document ) );
    sync.addFuture( QtConcurrent::run( this, &ImportHTML::LoadStyleFiles, &document ) );
    
    sync.waitForFinished();

    QList< QFuture< QHash< QString, QString > > > futures = sync.futures();
    int num_futures = futures.count();

    QHash< QString, QString > updates;

    for ( int i = 0; i < num_futures; ++i )
    {
        updates.unite( futures.at( i ).result() );
    }   

    return updates;
}


// Loads the images into the book
QHash< QString, QString > ImportHTML::LoadImages( const xc::DOMDocument *document )
{
    QStringList image_paths = XhtmlDoc::GetImagePathsFromImageChildren( *document );
    QHash< QString, QString > updates;
    QDir folder( QFileInfo( m_FullFilePath ).absoluteDir() );

    QStringList current_filenames = m_Book->GetConstFolderKeeper().GetAllFilenames();

    // Load the images into the book and
    // update all references with new urls
    foreach( QString image_path, image_paths )
    {
        try
        {
            QString filename = QFileInfo( image_path ).fileName();

            if ( m_IgnoreDuplicates && current_filenames.contains( filename ) )

                continue;

            QString fullfilepath  = QFileInfo( folder, image_path ).absoluteFilePath();
            QString newpath       = "../" + m_Book->GetFolderKeeper()
                                        .AddContentFileToFolder( fullfilepath ).GetRelativePathToOEBPS();
            updates[ fullfilepath ] = newpath;
        }
        
        catch ( FileDoesNotExist& )
        {
            // Do nothing. If the referenced file does not exist,
            // well then we don't load it.
        	// TODO: log this.
        }
    }

    return updates;
}


QHash< QString, QString > ImportHTML::LoadStyleFiles( const xc::DOMDocument *document )
{
    QList< xc::DOMElement* > link_nodes = XhtmlDoc::GetTagMatchingDescendants( *document, "link" );
    QHash< QString, QString > updates;

    QStringList current_filenames = m_Book->GetConstFolderKeeper().GetAllFilenames();

    for ( int i = 0; i < link_nodes.count(); ++i )
    {
        xc::DOMElement &element = *link_nodes.at( i );
        Q_ASSERT( &element );

        QDir folder( QFileInfo( m_FullFilePath ).absoluteDir() );
        QString relative_path = Utility::URLDecodePath( XtoQ( element.getAttribute( QtoX( "href" ) ) ) );

        QFileInfo file_info( folder, relative_path );

        if ( file_info.suffix().toLower() == "css" ||
             file_info.suffix().toLower() == "xpgt"
           )
        {
            try
            {
                if ( m_IgnoreDuplicates && current_filenames.contains( file_info.fileName() ) )

                    continue;

                QString newpath = "../" + m_Book->GetFolderKeeper().AddContentFileToFolder( 
                                                file_info.absoluteFilePath() ).GetRelativePathToOEBPS();

                updates[ relative_path ] = newpath;
            }

            catch ( FileDoesNotExist& )
            {
                // Do nothing. If the referenced file does not exist,
                // well then we don't load it.
                // TODO: log this.
            }
        }
    }

    return updates;
}



