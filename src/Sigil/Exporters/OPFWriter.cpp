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
#include "ResourceObjects/HTMLResource.h"
#include "../BookManipulation/GuideSemantics.h"

static const int FLOW_SIZE_THRESHOLD = 1000;


OPFWriter::OPFWriter( QSharedPointer< Book > book, QIODevice &device )
    : 
    XMLWriter( book, device )
{
    CreateMimetypes();
}


void OPFWriter::WriteXML()
{
    m_Writer->writeStartDocument();

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
}


void OPFWriter::WriteMetadata()
{
    QHash< QString, QList< QVariant > > metadata = m_Book->GetMetadata();

    m_Writer->writeStartElement( "metadata" );

    m_Writer->writeAttribute( "xmlns:dc", "http://purl.org/dc/elements/1.1/" );
    m_Writer->writeAttribute( "xmlns:opf", "http://www.idpf.org/2007/opf" );

        foreach ( QString name, metadata.keys() )
        {
            foreach ( QVariant single_value, metadata[ name ] )
            {
                MetadataDispatcher( name, single_value );
            }
        }
        
        m_Writer->writeStartElement( "dc:identifier" );

        m_Writer->writeAttribute( "id", "BookID" );

        if ( metadata.contains( "CustomID" ) )
        {
            m_Writer->writeAttribute( "opf:scheme", "CustomID" );
            m_Writer->writeCharacters( metadata[ "CustomID" ][ 0 ].toString() );
        }

        else
        {
            m_Writer->writeAttribute( "opf:scheme", "UUID" );
            m_Writer->writeCharacters( m_Book->GetPublicationIdentifier() );
        }       

        m_Writer->writeEndElement();

        m_Writer->writeEmptyElement( "meta" );
        m_Writer->writeAttribute( "name", "Sigil version" );
        m_Writer->writeAttribute( "content", SIGIL_VERSION );

    m_Writer->writeEndElement();
}


void OPFWriter::MetadataDispatcher( const QString &metaname, const QVariant &metavalue )
{
    // We ignore badly formed meta elements.
    if ( metaname.isEmpty() || metavalue.isNull() )

        return;

    // There is a relator for the publisher, but there is
    // also a special publisher element that we would rather use
    if (  Metadata::Instance().GetRelatorMap().contains( metaname ) &&
          metaname != QObject::tr( "Publisher" )
       )
    {
        WriteCreatorOrContributor( metaname, metavalue.toString() );
    }

    else if ( metaname == QObject::tr( "Language" ) )
    {
        WriteSimpleMetadata( metaname.toLower(), 
                             Metadata::Instance().GetLanguageMap()[ metavalue.toString() ] );
    }

    else if ( ( metaname == QObject::tr( "ISBN" ) ) || 
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


void OPFWriter::WriteCreatorOrContributor( const QString &metaname, const QString &metavalue )
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


void OPFWriter::WriteSimpleMetadata( const QString &metaname, const QString &metavalue )
{
    m_Writer->writeTextElement( "dc:" + metaname, metavalue );
}


void OPFWriter::WriteIdentifier( const QString &metaname, const QString &metavalue )
{
    m_Writer->writeStartElement( "dc:identifier" );

    m_Writer->writeAttribute( "opf:scheme", metaname );
    m_Writer->writeCharacters( metavalue );

    m_Writer->writeEndElement();
}


void OPFWriter::WriteDate( const QString &metaname, const QVariant &metavalue )
{
    m_Writer->writeStartElement( "dc:date" );

    QString date = metavalue.toDate().toString( "yyyy-MM-dd" );
    
    // The metaname should be "Date of X", where X is
    // "publication", "creation" etc.
    QStringList metaname_words = metaname.split( " " );
    QString event_type = metaname_words.count() == 3          ? 
                         metaname.split( " " )[ 2 ].toLower() :
                         "publication";

    m_Writer->writeAttribute( "opf:event", event_type );
    m_Writer->writeCharacters( date );

    m_Writer->writeEndElement();
}


QString OPFWriter::GetNormalName( const QString &name )
{
    if ( !name.contains( "," ) )

        return QString();

    QStringList splits = name.split( "," );

    return splits[ 1 ].trimmed() + " " + splits[ 0 ].trimmed();
}


QString OPFWriter::GetValidID( const QString &value )
{
    QString new_value = value.simplified();
    int i = 0;

    // Remove all forbidden characters.
    while ( i < new_value.size() )
    {
        if ( !IsValidIDCharacter( new_value.at( i ) ) )
        
            new_value.remove( i, 1 );
        
        else
        
            ++i;        
    }

    if ( new_value.isEmpty() )

        return Utility::CreateUUID();

    QChar first_char = new_value.at( 0 );

    // IDs cannot start with a number, a dash or a dot
    if ( first_char.isNumber()      ||
         first_char == QChar( '-' ) ||
         first_char == QChar( '.' )
       )
    {
        new_value.prepend( "x" );
    }

    return new_value;
}


// This is probably more rigorous 
// than the XML spec, but it's simpler.
// (spec ref: http://www.w3.org/TR/xml-id/#processing)
bool OPFWriter::IsValidIDCharacter( const QChar &character )
{
    return character.isLetterOrNumber() ||
           character == QChar( '=' )    ||
           character == QChar( '-' )    ||
           character == QChar( '_' )    ||
           character == QChar( '.' )    ||
           character == QChar( ':' )
           ;
}


void OPFWriter::WriteManifest()
{
    m_Writer->writeStartElement( "manifest" );

    m_Writer->writeEmptyElement( "item" );
    m_Writer->writeAttribute( "id", "ncx" );
    m_Writer->writeAttribute( "href", "toc.ncx" );
    m_Writer->writeAttribute( "media-type", m_Mimetypes[ "ncx" ] );

    foreach( QString relative_path, m_Book->GetConstFolderKeeper().GetSortedContentFilesList() )
    {
        QFileInfo info( relative_path );
        QString name      = info.baseName();
        QString extension = info.suffix();

        m_Writer->writeEmptyElement( "item" );
        m_Writer->writeAttribute( "id", GetValidID( name + "." + extension ) );
        m_Writer->writeAttribute( "href", Utility::URLEncodePath( relative_path ) );
        m_Writer->writeAttribute( "media-type", m_Mimetypes[ extension.toLower() ] );
    }

    m_Writer->writeEndElement();	
}


void OPFWriter::WriteSpine()
{
    m_Writer->writeStartElement( "spine" );
    m_Writer->writeAttribute( "toc", "ncx" );

    foreach( HTMLResource *html_resource, m_Book->GetConstFolderKeeper().GetSortedHTMLResources() )
    {
        QFileInfo info( html_resource->Filename() );
        QString name      = info.baseName();
        QString extension = info.suffix();

        m_Writer->writeEmptyElement( "itemref" );
        m_Writer->writeAttribute( "idref", GetValidID( name + "." + extension ) );
    }

    m_Writer->writeEndElement();
}


void OPFWriter::WriteGuide()
{
    if ( !GuideTypesPresent() )

        return;

    m_Writer->writeStartElement( "guide" );

    foreach( HTMLResource *html_resource, m_Book->GetConstFolderKeeper().GetSortedHTMLResources() )
    {
        GuideSemantics::GuideSemanticType semantic_type = html_resource->GetGuideSemanticType();

        if ( semantic_type == GuideSemantics::NoType )

            continue;

        QString type_attribute;
        QString title_attribute;
        tie( type_attribute, title_attribute ) = GuideSemantics::Instance().GetGuideTypeMapping()[ semantic_type ];

        m_Writer->writeEmptyElement( "reference" );
        m_Writer->writeAttribute( "type", type_attribute );
        m_Writer->writeAttribute( "title", title_attribute );
        m_Writer->writeAttribute( "href", Utility::URLEncodePath( html_resource->GetRelativePathToOEBPS() ) );
    }

    m_Writer->writeEndElement();
}


bool OPFWriter::IsFlowUnderThreshold( HTMLResource *resource, int threshold ) const
{
    QReadLocker locker( &resource->GetLock() );

    QDomElement doc_element = resource->GetDomDocumentForReading().documentElement();
    return doc_element.text().count() < threshold;
}


bool OPFWriter::GuideTypesPresent()
{
    foreach( HTMLResource *html_resource, m_Book->GetConstFolderKeeper().GetSortedHTMLResources() )
    {
        if ( html_resource->GetGuideSemanticType() != GuideSemantics::NoType )

            return true;
    }

    return false;
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

    // We convert all HTML document types to XHTML
    m_Mimetypes[ "xml"   ] = "application/xhtml+xml"; 
    m_Mimetypes[ "xhtml" ] = "application/xhtml+xml"; 
    m_Mimetypes[ "html"  ] = "application/xhtml+xml"; 
    m_Mimetypes[ "htm"   ] = "application/xhtml+xml"; 
    m_Mimetypes[ "css"   ] = "text/css"; 

    // Hopefully we won't get a lot of these
    m_Mimetypes[ "xpgt"  ] = "application/vnd.adobe-page-template+xml"; 

    // Until the standards gods grace us with font mimetypes,
    // these will have to do
    m_Mimetypes[ "otf"   ] = "application/x-font-opentype"; 
    m_Mimetypes[ "ttf"   ] = "application/x-font-truetype";
}













