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
#include "OPFWriter.h"
#include "../BookManipulation/Metadata.h"
#include "../BookManipulation/Book.h"
#include "../Misc/Utility.h"

static const int FLOW_SIZE_THRESHOLD = 1000;


// Constructor;
// The first parameter is the book being exported,
// and the second is the list of files
// in the folder that will become the exported book
OPFWriter::OPFWriter( const Book &book, const FolderKeeper &fkeeper )
    : XMLWriter( book, fkeeper )
{
    CreateMimetypes();
}


// Returns the created XML file
QString OPFWriter::GetXML()
{
    m_Writer->writeStartDocument();

    m_Writer->setAutoFormatting( true );

    m_Writer->writeStartElement( "package" );

    m_Writer->writeAttribute( "xmlns", "http://www.idpf.org/2007/opf" );
    m_Writer->writeAttribute( "unique-identifier", "BookID" );
    m_Writer->writeAttribute( "version", "2.0" );

    WriteMetadata();
    WriteManifest();
    WriteSpine();
    WriteGuide();

    m_Writer->writeEndElement();
    m_Writer->writeEndDocument();
    
    return m_Source;
}


// Writes the <metadata> element
void OPFWriter::WriteMetadata()
{
    m_Writer->writeStartElement( "metadata" );

    m_Writer->writeAttribute( "xmlns:dc", "http://purl.org/dc/elements/1.1/" );
    m_Writer->writeAttribute( "xmlns:opf", "http://www.idpf.org/2007/opf" );

        foreach ( QString name, m_Book.metadata.keys() )
        {
            foreach ( QVariant single_value, m_Book.metadata[ name ] )
            {
                MetadataDispatcher( name, single_value );
            }
        }
        
        m_Writer->writeStartElement( "dc:identifier" );

        m_Writer->writeAttribute( "id", "BookID" );

        if ( m_Book.metadata.contains( "CustomID" ) )
        {
            m_Writer->writeAttribute( "opf:scheme", "CustomID" );
            m_Writer->writeCharacters( m_Book.metadata[ "CustomID" ][ 0 ].toString() );
        }

        else
        {
            m_Writer->writeAttribute( "opf:scheme", "UUID" );
            m_Writer->writeCharacters( m_Book.PublicationIdentifier );
        }       

        m_Writer->writeEndElement();

        m_Writer->writeEmptyElement( "meta" );
        m_Writer->writeAttribute( "name", "Sigil version" );
        m_Writer->writeAttribute( "content", SIGIL_FULL_VERSION );

    m_Writer->writeEndElement();
}


// Dispatches each metadata entry based on its type;
// the specialized Write* functions write the elements
void OPFWriter::MetadataDispatcher( const QString &metaname, const QVariant &metavalue )
{
    // There is a relator for the publisher, but there is
    // also a special publisher element that we would rather use
    if (  Metadata::Instance().GetRelatorMap().contains( metaname ) &&
          metaname != QObject::tr( "Publisher" ) 
       )
    {
        WriteCreatorsAndContributors( metaname, metavalue.toString() );
    }

    else if ( metaname == QObject::tr( "Language" ) )
    {
        WriteSimpleMetadata(	metaname.toLower(), 
                                Metadata::Instance().GetLanguageMap()[ metavalue.toString() ] 
                            );
    }

    else if (	( metaname == QObject::tr( "ISBN" ) ) || 
                ( metaname == QObject::tr( "ISSN" ) ) ||
                ( metaname == QObject::tr( "DOI" ) )
            )
    {
        WriteIdentifier( metaname, metavalue.toString() );
    }

    else if ( metaname == QObject::tr( "CustomID" ) )
    {
        // Don't write the CustomID, it is used as the
        // main identifier if present
    }

    else if ( metaname.contains( QObject::tr( "Date" ) ) )
    {
        WriteDate( metaname, metavalue );		
    }
    
    // Everything else should be simple
    else
    {
        WriteSimpleMetadata( metaname.toLower(), metavalue.toString() );
    }
}


// Write <creator> and <contributor> metadata elements
void OPFWriter::WriteCreatorsAndContributors( const QString &metaname, const QString &metavalue )
{
    // Authors get written as creators, all other relators
    // are written as contributors
    if ( metaname == QObject::tr( "Author" ) )

        m_Writer->writeStartElement( "dc:creator" );

    else

        m_Writer->writeStartElement( "dc:contributor" );

    m_Writer->writeAttribute( "opf:role", Metadata::Instance().GetRelatorMap()[ metaname ].relator_code );

    // if the name is written in standard form 
    // ("John Doe"), just write it out
    if ( GetNormalName( metavalue ).isEmpty() )
    {
        m_Writer->writeCharacters( metavalue );
    }

    // Otherwise it is written in reversed form
    // ("Doe, John") and we write the reversed form
    // to the "file-as" attribute and the normal form as the value
    else
    {
        m_Writer->writeAttribute( "opf:file-as", metavalue );
        m_Writer->writeCharacters( GetNormalName( metavalue ) );	
    }	

    m_Writer->writeEndElement();
}


// Writes simple metadata; the metaname will be the element name
// and the metavalue will be written as the value
void OPFWriter::WriteSimpleMetadata( const QString &metaname, const QString &metavalue )
{
    m_Writer->writeTextElement( "dc:" + metaname, metavalue );
}


// Writes the <identifier> elements;
// the metaname will be used for the scheme
// and the metavalue for the value
void OPFWriter::WriteIdentifier( const QString &metaname, const QString &metavalue )
{
    m_Writer->writeStartElement( "dc:identifier" );

    m_Writer->writeAttribute( "opf:scheme", metaname );
    m_Writer->writeCharacters( metavalue );

    m_Writer->writeEndElement();
}


// Writes the <date> elements;
// the metaname will be used for the event
// and the metavalue for the value 
void OPFWriter::WriteDate( const QString &metaname, const QVariant &metavalue )
{
    m_Writer->writeStartElement( "dc:date" );

    QString date		= metavalue.toDate().toString( "yyyy-MM-dd" );
    QString event_type	= metaname.split( " " )[ 2 ].toLower();

    m_Writer->writeAttribute( "opf:event", event_type );
    m_Writer->writeCharacters( date );

    m_Writer->writeEndElement();
}


// Takes the reversed form of a name ("Doe, John")
// and returns the normal form ("John Doe"); if the
// provided name is already normal, returns an empty string
QString OPFWriter::GetNormalName( const QString &name ) const
{
    if ( !name.contains( "," ) )

        return QString();

    // FIXME: add check and error throw for multiple commas

    QStringList splits = name.split( "," );

    return splits[ 1 ].trimmed() + " " + splits[ 0 ].trimmed();
}


// Writes the <manifest> element
void OPFWriter::WriteManifest()
{
    m_Writer->writeStartElement( "manifest" );

    m_Writer->writeEmptyElement( "item" );
    m_Writer->writeAttribute( "id", "ncx" );
    m_Writer->writeAttribute( "href", "toc.ncx" );
    m_Writer->writeAttribute( "media-type", m_Mimetypes[ "ncx" ] );

    foreach( QString file, m_Files )
    {
        QString name		= QFileInfo( file ).baseName();
        QString extension	= QFileInfo( file ).suffix();

        m_Writer->writeEmptyElement( "item" );
        m_Writer->writeAttribute( "id", name + "." + extension );
        m_Writer->writeAttribute( "href", file );
        m_Writer->writeAttribute( "media-type", m_Mimetypes[ extension ] );
    }

    m_Writer->writeEndElement();	
}


// Writes the <spine> element
void OPFWriter::WriteSpine()
{
    m_Writer->writeStartElement( "spine" );
    m_Writer->writeAttribute( "toc", "ncx" );

    foreach( QString file, m_Files )
    {
        // We skip all the files that are not in the
        // text subdirectory
        if ( !file.contains( "text/" ) )

            continue;

        QString name		= QFileInfo( file ).baseName();
        QString extension	= QFileInfo( file ).suffix();

        m_Writer->writeEmptyElement( "itemref" );
        m_Writer->writeAttribute( "idref", name + "." + extension );
    }

    m_Writer->writeEndElement();
}


// Writes the <guide> element
void OPFWriter::WriteGuide()
{
    QString relative_path = m_Files.filter( "text/" )[ 0 ];
    
    // We write the cover page (and the guide in general)
    // only if the first OPS document (flow) is smaller
    // than our threshold
    if ( IsFlowUnderThreshold( relative_path, FLOW_SIZE_THRESHOLD ) )
    {
        m_Writer->writeStartElement( "guide" );

        m_Writer->writeEmptyElement( "reference" );
        m_Writer->writeAttribute( "type", "cover" );
        m_Writer->writeAttribute( "title", "Cover Page" );
        m_Writer->writeAttribute( "href", relative_path );

        m_Writer->writeEndElement();	
    }
}


// Returns true if the content of the file specified
// has fewer characters than 'threshold' number;
// by the path specified is relative to the OEBPS folder 
bool OPFWriter::IsFlowUnderThreshold( const QString &relative_path, int threshold ) const
{
    QString fullfilepath = m_Folder.GetFullPathToOEBPSFolder() + "/" + relative_path;

    QRegExp content( BODY_START + "(.*)" + BODY_END );

    Utility::ReadUnicodeTextFile( fullfilepath ).contains( content );

    return content.cap( 1 ).count() < threshold;
}


// Initializes m_Mimetypes
void OPFWriter::CreateMimetypes()
{
    m_Mimetypes[ "jpg"   ] = "image/jpeg"; 
    m_Mimetypes[ "jpeg"  ] = "image/jpeg"; 
    m_Mimetypes[ "png"   ] = "image/png";
    m_Mimetypes[ "gif"   ] = "image/gif";
    m_Mimetypes[ "tif"   ] = "image/tiff";
    m_Mimetypes[ "tiff"  ] = "image/tiff";
    m_Mimetypes[ "bm"    ] = "image/bmp";
    m_Mimetypes[ "bmp"   ] = "image/bmp";
    m_Mimetypes[ "svg"   ] = "image/svg+xml";	

    m_Mimetypes[ "ncx"   ] = "application/x-dtbncx+xml"; 
    m_Mimetypes[ "xhtml" ] = "application/xhtml+xml"; 
    m_Mimetypes[ "html"  ] = "text/html"; 
    m_Mimetypes[ "htm"   ] = "text/html"; 
    m_Mimetypes[ "css"   ] = "text/css"; 

    // Hopefully we won't get a lot of these
    m_Mimetypes[ "xpgt"  ] = "application/vnd.adobe-page-template+xml"; 

    // Until the standards gods grace us with font mimetypes,
    // these will have to do
    m_Mimetypes[ "otf"   ] = "application/x-font-opentype"; 
    m_Mimetypes[ "ttf"   ] = "application/x-font-truetype";
}









