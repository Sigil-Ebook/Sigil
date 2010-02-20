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
#include "ImportHTML.h"
#include "../Misc/Utility.h"
#include "../Misc/HTMLEncodingResolver.h"
#include "../BookManipulation/Metadata.h"
#include "../BookManipulation/CleanSource.h"
#include "ResourceObjects/HTMLResource.h"
#include "ResourceObjects/CSSResource.h"
#include "../SourceUpdates/PerformHTMLUpdates.h"
#include "../SourceUpdates/PerformCSSUpdates.h"
#include "../BookManipulation/XHTMLDoc.h"
#include <QDomDocument>

static const QString ENTITY_SEARCH = "<!ENTITY\\s+(\\w+)\\s+\"([^\"]+)\">";
static const QStringList IMAGE_TAGS = QStringList() << "img" << "image"; 


// Constructor;
// The parameter is the file to be imported
ImportHTML::ImportHTML( const QString &fullfilepath )
    : Importer( fullfilepath )
{

}


void ImportHTML::SetBook( QSharedPointer< Book > book )
{
    m_Book = book;
}


// Reads and parses the file 
// and returns the created Book
QSharedPointer< Book > ImportHTML::GetBook()
{
    if ( !Utility::IsFileReadable( m_FullFilePath ) )

        boost_throw( CannotReadFile() << errinfo_file_read( m_FullFilePath.toStdString() ) );

    QDomDocument document;
    document.setContent( LoadSource() );

    StripFilesFromAnchors( document );
    LoadMetadata( document );

    UpdateFiles( CreateHTMLResource(), document );

    return m_Book;
}


// Loads the source code into the Book
QString ImportHTML::LoadSource()
{
    QString source = CleanSource::Clean( HTMLEncodingResolver::ReadHTMLFile( m_FullFilePath ) );
    return ResolveCustomEntities( source );
}


// Resolves custom ENTITY declarations
QString ImportHTML::ResolveCustomEntities( const QString &html_source ) const
{
    QString source = html_source;
    QRegExp entity_search( ENTITY_SEARCH );

    QHash< QString, QString > entities;

    int main_index = 0;

    // Catch all custom entity declarations...
    while ( true )
    {
        main_index = source.indexOf( entity_search, main_index );

        if ( main_index == -1 )

            break;

        entities[ "&" + entity_search.cap( 1 ) + ";" ] = entity_search.cap( 2 );

        // Erase the entity declaration
        source.replace( entity_search.cap( 0 ), "" );
    }

    // ...and now replace all occurrences
    foreach( QString key, entities.keys() )
    {
        source.replace( key, entities[ key ] );
    }

    // Clean up what's left of the custom entity declaration field
    source.replace( QRegExp( "\\[\\s*\\]>" ), "" );

    return source;
}


// Strips the file specifier on all the href attributes 
// of anchor tags with filesystem links with fragment identifiers;
// thus something like <a href="chapter01.html#firstheading" />
// becomes just <a href="#firstheading" />
void ImportHTML::StripFilesFromAnchors( QDomDocument &document )
{
    QDomNodeList anchors = document.elementsByTagName( "a" );

    for ( int i = 0; i < anchors.count(); ++i )
    {
        QDomElement element = anchors.at( i ).toElement();

        Q_ASSERT( !element.isNull() );

        // We strip the file specifier on all
        // the filesystem links with fragment identifiers
        if ( element.hasAttribute( "href" ) &&
             QUrl( element.attribute( "href" ) ).isRelative() &&
             element.attribute( "href" ).contains( "#" )
           )
        {
            element.setAttribute( "href", "#" + element.attribute( "href" ).split( "#" )[ 1 ] );            
        } 
    }     
}

// Searches for meta information in the HTML file
// and tries to convert it to Dublin Core
void ImportHTML::LoadMetadata( const QDomDocument &document )
{
    QDomNodeList metatags = document.elementsByTagName( "meta" );

    for ( int i = 0; i < metatags.count(); ++i )
    {
        QDomElement element = metatags.at( i ).toElement();

        Metadata::MetaElement meta;
        meta.name  = element.attribute( "name" );
        meta.value = element.attribute( "content" );
        meta.attributes[ "scheme" ] = element.attribute( "scheme" );

        if ( ( !meta.name.isEmpty() ) && ( !meta.value.toString().isEmpty() ) ) 
        { 
            Metadata::MetaElement book_meta = Metadata::Instance().MapToBookMetadata( meta , "HTML" );

            if ( !book_meta.name.isEmpty() && !book_meta.value.toString().isEmpty() )
            {
                m_Book->metadata[ book_meta.name ].append( book_meta.value );
            }
        }
    }    
}


HTMLResource& ImportHTML::CreateHTMLResource()
{
    QDir dir( Utility::GetNewTempFolderPath() );
    dir.mkpath( dir.absolutePath() );

    QString fullfilepath = dir.absolutePath() + "/" + FIRST_CHAPTER_NAME;
    Utility::WriteUnicodeTextFile( "TEMP_SOURCE", fullfilepath );

    HTMLResource &resource = *qobject_cast< HTMLResource* >(
                                &m_Book->mainfolder.AddContentFileToFolder( fullfilepath, 0 ) );

    return resource;
}


void ImportHTML::UpdateFiles( HTMLResource &html_resource, QDomDocument &document )
{
    Q_ASSERT( &html_resource != NULL );

    QHash< QString, QString > html_updates;
    QHash< QString, QString > css_updates;
    tie( html_updates, css_updates ) = PerformHTMLUpdates::SeparateHTMLAndCSSUpdates( LoadFolderStructure( document ) );

    QList< Resource* > all_files = m_Book->mainfolder.GetResourceList();
    int num_files = all_files.count();

    QList< CSSResource* > css_resources;

    for ( int i = 0; i < num_files; ++i )
    {
        Resource *resource = all_files.at( i );

        if ( resource->Type() == Resource::CSSResource )   
        
            css_resources.append( qobject_cast< CSSResource* >( resource ) );          
    }

    QFutureSynchronizer<void> sync;
    sync.addFuture( QtConcurrent::map( css_resources, boost::bind( UpdateOneCSSFile, _1, css_updates ) ) );

    html_resource.SetDomDocument( PerformHTMLUpdates( document, html_updates, css_updates )() );

    sync.waitForFinished();
}


void ImportHTML::UpdateOneCSSFile( CSSResource* css_resource, const QHash< QString, QString > &css_updates )
{
    QString source = Utility::ReadUnicodeTextFile( css_resource->GetFullPath() );
    source = PerformCSSUpdates( source, css_updates )();
    css_resource->SetText( source );
}


// Loads the referenced files into the main folder of the book;
// as the files get a new name, the references are updated
QHash< QString, QString > ImportHTML::LoadFolderStructure( const QDomDocument &document )
{
    QFutureSynchronizer< QHash< QString, QString > > sync;

    sync.addFuture( QtConcurrent::run( this, &ImportHTML::LoadImages,     document ) );
    sync.addFuture( QtConcurrent::run( this, &ImportHTML::LoadStyleFiles, document ) );
    
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
QHash< QString, QString > ImportHTML::LoadImages( const QDomDocument &document )
{
    // "Normal" HTML image elements
    QList< QDomNode > image_nodes = XHTMLDoc::GetTagMatchingChildren( document, IMAGE_TAGS );

    QStringList image_links;

    // Get a list of all images referenced
    foreach( QDomNode node, image_nodes )
    {
        QDomElement element = node.toElement();

        Q_ASSERT( !element.isNull() );

        QString url_reference;

        if ( element.hasAttribute( "src" ) )

            url_reference = QUrl::fromPercentEncoding( element.attribute( "src" ).toUtf8() );

        else // This covers the SVG "image" tags

            url_reference = QUrl::fromPercentEncoding( element.attribute( "xlink:href" ).toUtf8() );
        
        if ( !url_reference.isEmpty() )

            image_links << url_reference;
    }

    // Remove duplicate references
    image_links.removeDuplicates();

    QHash< QString, QString > updates;

    QDir folder( QFileInfo( m_FullFilePath ).absoluteDir() );

    // Load the images into the book and
    // update all references with new urls
    foreach( QString image_link, image_links )
    {
        QString fullfilepath = QFileInfo( folder, image_link ).absoluteFilePath();
        QString newpath      = "../" + m_Book->mainfolder.AddContentFileToFolder( fullfilepath ).GetRelativePathToOEBPS();

        updates[ image_link ] = newpath;
    }

    return updates;
}


// Loads CSS files from link tags to style tags
QHash< QString, QString > ImportHTML::LoadStyleFiles( const QDomDocument &document )
{
    QDomNodeList link_nodes = document.elementsByTagName( "link" );

    QHash< QString, QString > updates;

    // Get all the style files references in link tags
    // and convert them into style tags
    for ( int i = 0; i < link_nodes.count(); ++i )
    {
        QDomElement element = link_nodes.at( i ).toElement();

        Q_ASSERT( !element.isNull() );

        QDir folder( QFileInfo( m_FullFilePath ).absoluteDir() );

        QString relative_path = QUrl::fromPercentEncoding( element.attribute( "href" ).toUtf8() );

        QFileInfo file_info( folder, relative_path );

        if (  file_info.suffix().toLower() == "css" ||
              file_info.suffix().toLower() == "xpgt"
           )
        {
            QString newpath = "../" + m_Book->mainfolder.AddContentFileToFolder( 
                                            file_info.absoluteFilePath() ).GetRelativePathToOEBPS();

            updates[ relative_path ] = newpath;
        }
    }

    return updates;
}



