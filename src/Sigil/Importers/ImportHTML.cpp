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
#include "../BookManipulation/CleanSource.h"
#include <QDomDocument>
#include "../BookManipulation/XHTMLDoc.h"

static const QString ENTITY_SEARCH = "<!ENTITY\\s+(\\w+)\\s+\"([^\"]+)\">";

const QString HEAD_END = "</\\s*head\\s*>";


// Constructor;
// The parameter is the file to be imported
ImportHTML::ImportHTML( const QString &fullfilepath )
    : ImportTXT( fullfilepath )
{

}


// Reads and parses the file 
// and returns the created Book
Book ImportHTML::GetBook()
{
    if ( !Utility::IsFileReadable( m_FullFilePath ) )

        return Book();

    LoadSource(); 

    StripFilesFromAnchors();
    UpdateReferences( LoadFolderStructure() );

    m_Book.source = CleanSource::Clean( m_Book.source );

    return m_Book;
}


// Returns a style tag created 
// from the provided path to a CSS file
QString ImportHTML::CreateStyleTag( const QString &fullfilepath ) const
{
    QString source    = Utility::ReadUnicodeTextFile( fullfilepath ); 
    QString style_tag = "";

    if ( QFileInfo( fullfilepath ).suffix() == "css" )
    {
        style_tag = "<style type=\"text/css\">\n" + source + "\n</style>\n";
    }

    else // XPGT stylesheet
    {
        style_tag = "<style type=\"application/vnd.adobe-page-template+xml\">\n" + source + "\n</style>\n";
    }

    return style_tag;
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
void ImportHTML::StripFilesFromAnchors()
{
    QDomDocument document;
 
    // We need to clean the source first because
    // QDomDocument only accepts valid XML
    document.setContent( CleanSource::Clean( m_Book.source ) );

    QDomNodeList anchors = document.elementsByTagName( "a" );

    for ( int i = 0; i < anchors.count(); i++ )
    {
        QDomElement element = anchors.at( i ).toElement();

        // We strip the file specifier on all
        // the filesystem links with fragment identifiers
        if (    element.hasAttribute( "href" ) &&
                QUrl( element.attribute( "href" ) ).isRelative() &&
                element.attribute( "href" ).contains( "#" )
           )
        {
            element.setAttribute( "href", "#" + element.attribute( "href" ).split( "#" )[ 1 ] );            
        } 
    }

    m_Book.source = XHTMLDoc::GetQDomNodeAsString( document );      
}


// Accepts a hash with keys being old references (URLs) to resources,
// and values being the new references to those resources.
// The book XHTML source is updated accordingly.
void ImportHTML::UpdateReferences( const QHash< QString, QString > updates )
{
    QHash< QString, QString > html_updates = updates;
    QHash< QString, QString > css_updates;

    foreach( QString old_path, html_updates.keys() )
    {
        QString extension = QFileInfo( old_path ).suffix().toLower();

        // Font file updates are CSS updates, not HTML updates
        if ( extension == "ttf" || extension == "otf" )
        {
            css_updates[ old_path ] = html_updates[ old_path ];
            html_updates.remove( old_path );
        }
    }

    UpdateHTMLReferences( html_updates );
    UpdateCSSReferences( css_updates );
}


// Updates the resource references in the HTML.
// Accepts a hash with keys being old references (URLs) to resources,
// and values being the new references to those resources.
void ImportHTML::UpdateHTMLReferences( const QHash< QString, QString > updates )
{
    QDomDocument document;
    document.setContent( m_Book.source );

    UpdateReferenceInNode( document.documentElement(), updates );

    m_Book.source = XHTMLDoc::GetQDomNodeAsString( document );
}


// Updates the resource references in the attributes 
// of the one specified node in the HTML.
// Accepts a hash with keys being old references (URLs) to resources,
// and values being the new references to those resources.
void ImportHTML::UpdateReferenceInNode( QDomNode node, const QHash< QString, QString > updates ) const
{
    QDomNamedNodeMap attributes = node.attributes();

    for ( int i = 0; i < attributes.count(); i++ )
    {
        QDomAttr attribute = attributes.item( i ).toAttr();

        if ( !attribute.isNull() )
        {
            foreach ( QString old_path, updates.keys() )
            {
                QString filename = QFileInfo( old_path ).fileName();

                QRegExp file_match( ".*/" + QRegExp::escape( filename ) + "|" + QRegExp::escape( filename ) );

                if ( file_match.exactMatch( attribute.value() ) )

                    attribute.setValue( updates[ old_path ] );
            }            
        }
    }

    QDomNodeList children = node.childNodes();

    for ( int i = 0; i < children.count(); i++ )
    {
        UpdateReferenceInNode( children.at( i ), updates );
    }
}


// Updates the resource references in the CSS.
// Accepts a hash with keys being old references (URLs) to resources,
// and values being the new references to those resources.
void ImportHTML::UpdateCSSReferences( const QHash< QString, QString > updates )
{
    foreach( QString old_path, updates.keys() )
    {
        QString filename  = QFileInfo( old_path ).fileName();

        QRegExp reference = QRegExp( "src:\\s*\\w+\\([\"']*([^\\)]*/" + QRegExp::escape( filename ) + "|"
                                        + QRegExp::escape( filename ) + ")[\"']*\\)" );

        int index = -1;

        while ( true )
        {
            int newindex = m_Book.source.indexOf( reference );

            // We need to make sure we don't end up
            // replacing the same thing over and over again
            if ( ( index == newindex ) || ( newindex == -1 ) )

                break;

            m_Book.source.replace( reference.cap( 1 ), updates[ old_path ] );

            index = newindex;
        }
    }  
}


// Loads the source code into the Book
void ImportHTML::LoadSource()
{
    QFile file( m_FullFilePath );

    // Check if we can open the file
    if ( !file.open( QFile::ReadOnly | QFile::Text ) )
    {
        QMessageBox::warning(	0,
            QObject::tr( "Sigil" ),
            QObject::tr( "Cannot read file %1:\n%2." )
            .arg( m_FullFilePath )
            .arg( file.errorString() ) 
            );
        return;
    }

    QByteArray data = file.readAll();

    m_Book.source = GetCodecForHTML( data )->toUnicode( data );
    m_Book.source = ResolveCustomEntities( m_Book.source );
}


// Accepts an HTML stream and tries to determine its encoding;
// if no encoding is detected, the default codec for this locale is returned.
// We use this function because Qt's QTextCodec::codecForHtml() function
// leaves a *lot* to be desired.
const QTextCodec* ImportHTML::GetCodecForHTML( const QByteArray &raw_text ) const
{
    // Qt docs say Qt will take care of deleting
    // any QTextCodec objects on application exit

    // This is a workaround for a bug in QTextCodec which
    // expects the 'charset' attribute to always come after
    // the 'http-equiv' attribute
    QString ascii_data = raw_text;
    ascii_data.replace( QRegExp( "<\\s*meta([^>]*)http-equiv=\"Content-Type\"([^>]*)>" ),
                           "<meta http-equiv=\"Content-Type\" \\1 \\2>" );

    QTextCodec *locale_codec   = QTextCodec::codecForLocale();
    QTextCodec *detected_codec = QTextCodec::codecForHtml( ascii_data.toAscii(), QTextCodec::codecForLocale() ); 

    // If Qt's function was unable to detect an encoding, 
    // we look for one ourselves.
    if ( detected_codec->name() == locale_codec->name() )
    {
        int head_end = ascii_data.indexOf( QRegExp( HEAD_END ) );

        if ( head_end != -1 )
        {
            QString head = Utility::Substring( 0, head_end, ascii_data );

            QRegExp charset( "charset=([^\"]+)\"" );
            head.indexOf( charset );

            QTextCodec *real_codec = QTextCodec::codecForName( charset.cap( 1 ).toAscii() );

            if ( real_codec != 0 )

                return real_codec; 

            else

                return locale_codec;
        }
    }

    return detected_codec;
}



// Loads the referenced files into the main folder of the book;
// as the files get a new name, the references are updated
QHash< QString, QString > ImportHTML::LoadFolderStructure()
{
    QHash< QString, QString > updates;
    
    updates = LoadImages();
    LoadStyleFiles();

    return updates;    
}


// Loads the images into the book
QHash< QString, QString > ImportHTML::LoadImages()
{
    // "Normal" HTML image elements
    QList< QDomNode > image_nodes = XHTMLDoc::GetTagsInDocument( m_Book.source, "img" );

    // SVG image elements
    image_nodes.append( XHTMLDoc::GetTagsInDocument( m_Book.source, "image" ) );

    QStringList image_links;

    // Get a list of all images referenced
    foreach( QDomNode node, image_nodes )
    {
        QDomElement element = node.toElement();
        QString url_reference;

        if ( element.hasAttribute( "src" ) )

            url_reference = element.attribute( "src" );

        else // This covers the SVG "image" tags

            url_reference = element.attribute( "xlink:href" );
        
        if ( !url_reference.isEmpty() )

            image_links << url_reference;
    }

    // Remove duplicate references
    image_links.removeDuplicates();

    QHash< QString, QString > updates;

    // Load the images into the book and
    // update all references with new urls
    foreach( QString image_link, image_links )
    {
        QDir folder( QFileInfo( m_FullFilePath ).absoluteDir() );

        QString fullfilepath = QFileInfo( folder, QUrl( image_link ).toString() ).absoluteFilePath();
        QString newpath      = "../" + m_Book.mainfolder.AddContentFileToFolder( fullfilepath );

        updates[ image_link ] = newpath;
    }

    return updates;
}


// Loads CSS files from link tags to style tags
void ImportHTML::LoadStyleFiles()
{
    QDomDocument document;
    document.setContent( m_Book.source );

    QDomNodeList link_nodes = document.elementsByTagName( "link" );

    QStringList style_tags;

    // Get all the style files references in link tags
    // and convert them into style tags
    for ( int i = 0; i < link_nodes.count(); i++ )
    {
        QDir folder( QFileInfo( m_FullFilePath ).absoluteDir() );

        QDomElement element = link_nodes.at( i ).toElement();

        QFileInfo file_info( folder, QUrl( element.attribute( "href" ) ).toString() );

        if (    file_info.suffix().toLower() == "css" ||
                file_info.suffix().toLower() == "xpgt"
           )
        {
            style_tags << CreateStyleTag( file_info.absoluteFilePath() );
        }
    }

    QDomNode head = document.elementsByTagName( "head" ).at( 0 );

    // Remove the link tags
    while ( !link_nodes.isEmpty() )
    {
        head.removeChild( link_nodes.at( 0 ) );        
    }

    QString new_source = XHTMLDoc::GetQDomNodeAsString( document );

    // Paste the new style tags into the head section
    foreach( QString style, style_tags )
    {
        new_source.replace( QRegExp( "(</\\s*(?:head|HEAD)[^>]*>)" ), style + "\n\\1" );
    }  

    m_Book.source = new_source;
}


