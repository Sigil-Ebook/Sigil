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
#include "NCXWriter.h"
#include "BookManipulation/Book.h"
#include "Misc/Utility.h"
#include "ResourceObjects/HTMLResource.h"
#include "BookManipulation/XercesCppUse.h"


NCXWriter::NCXWriter( const Book &book, QIODevice &device )
    : 
    XMLWriter( book, device ),
    m_Headings( Headings::MakeHeadingHeirarchy( 
                Headings::GetHeadingList( book.GetFolderKeeper().GetResourceTypeList< HTMLResource >( true ) ) ) )
{

}


void NCXWriter::WriteXML()
{
    m_Writer->writeStartDocument();

    m_Writer->writeDTD( "<!DOCTYPE ncx PUBLIC \"-//NISO//DTD ncx 2005-1//EN\"\n" 
                         "   \"http://www.daisy.org/z3986/2005/ncx-2005-1.dtd\">\n" );

    m_Writer->writeStartElement( "ncx" );

    m_Writer->writeAttribute( "xmlns", "http://www.daisy.org/z3986/2005/ncx/" );
    m_Writer->writeAttribute( "version", "2005-1" );

    WriteHead();
    WriteDocTitle();
    WriteNavMap();

    m_Writer->writeEndElement();
    m_Writer->writeEndDocument();
}


void NCXWriter::WriteHead()
{
    m_Writer->writeStartElement( "head" );

        m_Writer->writeEmptyElement( "meta" );
        m_Writer->writeAttribute( "name", "dtb:uid" );

        m_Writer->writeAttribute( "content", m_Book.GetPublicationIdentifier() );

        // The heading depth should be exactly the same as the depth
        // of the NavMap element (since they are used to write it)
        m_Writer->writeEmptyElement( "meta" );
        m_Writer->writeAttribute( "name", "dtb:depth" );
        m_Writer->writeAttribute( "content", QString::number( GetHeadingsDepth() ) );

        m_Writer->writeEmptyElement( "meta" );
        m_Writer->writeAttribute( "name", "dtb:totalPageCount" );
        m_Writer->writeAttribute( "content", "0" );

        m_Writer->writeEmptyElement( "meta" );
        m_Writer->writeAttribute( "name", "dtb:maxPageNumber" );
        m_Writer->writeAttribute( "content", "0" );

    m_Writer->writeEndElement();
}


void NCXWriter::WriteDocTitle()
{
    QString document_title;
    
    QList< QVariant > titles = m_Book.GetMetadata().value( "Title" );

    if ( titles.isEmpty() )
    
        document_title = "Unknown";

    else // FIXME: handle multiple titles

        document_title = titles.first().toString();

    m_Writer->writeStartElement( "docTitle" );
    m_Writer->writeTextElement( "text", document_title );
    m_Writer->writeEndElement();
}


void NCXWriter::WriteNavMap()
{
    int play_order = 1;

    m_Writer->writeStartElement( "navMap" );

    if ( !m_Headings.isEmpty() )
    {
        // The NavMap is written recursively; 
        // WriteNavPoint is called for each heading in the tree
        foreach( Headings::Heading heading, m_Headings )
        {
            WriteNavPoint( heading, play_order );
        }
    }

    else
    {
        // No headings? Well the spec *demands* an NCX file
        // with a NavMap with at least one NavPoint, so we 
        // write a dummy one.
        WriteFallbackNavPoint();
    }

    m_Writer->writeEndElement();
}


void NCXWriter::WriteFallbackNavPoint()
{
    m_Writer->writeStartElement( "navPoint" );

    m_Writer->writeAttribute( "id", QString( "navPoint-%1" ).arg( 1 ) );
    m_Writer->writeAttribute( "playOrder", QString( "%1" ).arg( 1 ) );

    m_Writer->writeStartElement( "navLabel" );
    m_Writer->writeTextElement( "text", "Start");
    m_Writer->writeEndElement();

    QList< HTMLResource* > html_resources = m_Book.GetFolderKeeper().GetResourceTypeList< HTMLResource >( true ); 
    Q_ASSERT( !html_resources.isEmpty() );

    m_Writer->writeEmptyElement( "content" );
    m_Writer->writeAttribute( "src", Utility::URLEncodePath( html_resources.at( 0 )->GetRelativePathToOEBPS() ) );

    m_Writer->writeEndElement();
}


void NCXWriter::WriteNavPoint( const Headings::Heading &heading, int &play_order )
{
    // Headings that shouldn't be included in the TOC
    // are naturally not written to it
    if ( heading.include_in_toc )
    {
        m_Writer->writeStartElement( "navPoint" );

        m_Writer->writeAttribute( "id", QString( "navPoint-%1" ).arg( play_order ) );
        m_Writer->writeAttribute( "playOrder", QString( "%1" ).arg( play_order ) );

        play_order++;

        m_Writer->writeStartElement( "navLabel" );
        m_Writer->writeTextElement( "text", heading.text );
        m_Writer->writeEndElement();

        QString heading_file = heading.resource_file->GetRelativePathToOEBPS();       

        m_Writer->writeEmptyElement( "content" );

        // If this heading appears right after a chapter break,
        // then it "represents" and links to its file; otherwise,
        // we link to the heading element directly
        if ( heading.at_file_start )
        {
            m_Writer->writeAttribute( "src", Utility::URLEncodePath( heading_file ) );
        }

        else
        { 
            QString path = heading_file + "#" + XtoQ( heading.element->getAttribute( QtoX( "id" ) ) );
            m_Writer->writeAttribute( "src", Utility::URLEncodePath( path ) );
        }
    }

    foreach( Headings::Heading child, heading.children )
    {
        WriteNavPoint( child, play_order );
    }

    if ( heading.include_in_toc )

        m_Writer->writeEndElement();
}


int NCXWriter::GetHeadingsDepth() const
{
    int max_depth = 0;

    // The first heading level can already have 
    // several headings.
    foreach ( Headings::Heading heading, m_Headings )
    {
        int current_depth = 0;

        DepthWalker( heading, current_depth, max_depth );
    }

    return max_depth;
}


void NCXWriter::DepthWalker( const Headings::Heading &heading, int &current_depth, int &max_depth ) const
{
   if ( heading.include_in_toc )
   {
        current_depth++;

        if ( current_depth > max_depth )

            max_depth = current_depth;
    }

    foreach( Headings::Heading child_heading, heading.children )
    {
        int new_current_depth = current_depth;
        
        DepthWalker( child_heading, new_current_depth, max_depth );                            
    }
 }


