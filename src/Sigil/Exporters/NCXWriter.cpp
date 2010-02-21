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
#include "NCXWriter.h"
#include "../BookManipulation/Book.h"
#include "../Misc/Utility.h"


// Constructor;
// The first parameter is the book being exported,
// and the second is the FolderKeeper object representing
// the folder where the book will be exported
NCXWriter::NCXWriter( QSharedPointer< Book > book, const FolderKeeper &fkeeper )
    : 
    XMLWriter( book, fkeeper ), 
    m_HeadingIDsPerFile( GetHeadingIDsPerFile() ),
    m_HeadingSourcesPerFile( GetHeadingSourcesPerFile() ),
    m_Headings( Headings::MakeHeadingHeirarchy( Headings::GetHeadingList( 
                                                m_Book->GetFolderKeeper().GetSortedHTMLResources() ) ) )
{

}


// Returns the created XML file
QString NCXWriter::GetXML()
{
    m_Writer->writeStartDocument();

    m_Writer->setAutoFormatting( true );

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

    return m_Source;
}


// Writes the <head> element
void NCXWriter::WriteHead()
{
    // The heading depth should be exactly the same as the depth
    // of the NavMap element (since they are used to write it)
    QString depth = QString::number( GetHeadingsDepth() );

    m_Writer->writeStartElement( "head" );

        m_Writer->writeEmptyElement( "meta" );
        m_Writer->writeAttribute( "name", "dtb:uid" );

        // TODO: We should use the ISBN if it's provided
        m_Writer->writeAttribute( "content", m_Book->GetPublicationIdentifier() );

        m_Writer->writeEmptyElement( "meta" );
        m_Writer->writeAttribute( "name", "dtb:depth" );
        m_Writer->writeAttribute( "content", depth );

        m_Writer->writeEmptyElement( "meta" );
        m_Writer->writeAttribute( "name", "dtb:totalPageCount" );
        m_Writer->writeAttribute( "content", "0" );

        m_Writer->writeEmptyElement( "meta" );
        m_Writer->writeAttribute( "name", "dtb:maxPageNumber" );
        m_Writer->writeAttribute( "content", "0" );

    m_Writer->writeEndElement();
}


// Writes the <docTitle> element
void NCXWriter::WriteDocTitle()
{
    QString document_title;
    
    QList< QVariant > titles = m_Book->GetMetadata().value( "Title" );

    if ( titles.isEmpty() )
    
        document_title = "Unknown";

    else // FIXME: handle multiple titles

        document_title = titles.first().toString();

    m_Writer->writeStartElement( "docTitle" );
    m_Writer->writeTextElement( "text", document_title );
    m_Writer->writeEndElement();
}


// Writes the <navMap> element
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


// Writes a fallback NavPoint for when the book has no headings
void NCXWriter::WriteFallbackNavPoint()
{
    m_Writer->writeStartElement( "navPoint" );

    m_Writer->writeAttribute( "id", QString( "navPoint-%1" ).arg( 1 ) );
    m_Writer->writeAttribute( "playOrder", QString( "%1" ).arg( 1 ) );

    m_Writer->writeStartElement( "navLabel" );
    m_Writer->writeTextElement( "text", "Start");
    m_Writer->writeEndElement();

    QString first_text_file = "";

    foreach( QString relfilepath, m_Files )
    {
        // We skip all the files that are not in the
        // text subdirectory
        if ( !relfilepath.contains( TEXT_FOLDER_NAME + "/" ) )

            continue;

        first_text_file = relfilepath;
    }

    Q_ASSERT( !first_text_file.isEmpty() );

    m_Writer->writeEmptyElement( "content" );
    m_Writer->writeAttribute( "src", first_text_file );

    m_Writer->writeEndElement();
}


// Called recursively to write the TOC tree;
// the first parameter is the heading being written,
// the second is a reference to the NavPoints playorder
void NCXWriter::WriteNavPoint( const Headings::Heading &heading, int &play_order )
{
    // Headings that shouldn't be included in the TOC
    // are naturally not written to it
    if ( heading.include_in_toc == true )
    {
        m_Writer->writeStartElement( "navPoint" );

        m_Writer->writeAttribute( "id", QString( "navPoint-%1" ).arg( play_order ) );
        m_Writer->writeAttribute( "playOrder", QString( "%1" ).arg( play_order ) );

        play_order++;

        m_Writer->writeStartElement( "navLabel" );
        m_Writer->writeTextElement( "text", heading.element.text() );
        m_Writer->writeEndElement();

        QString myfile = GetHeadingFile( heading );        

        m_Writer->writeEmptyElement( "content" );

        // If this heading appears right after a chapter break,
        // then it "represents" and links to its file; otherwise,
        // we link to the heading element directly
        if ( heading.at_file_start == true )
        
            m_Writer->writeAttribute( "src", myfile );

        else
            
            m_Writer->writeAttribute( "src", myfile + "#" + heading.element.attribute( "id" ) );
    }

    foreach( Headings::Heading child, heading.children )
    {
        WriteNavPoint( child, play_order );
    }

    if ( heading.include_in_toc == true )

        m_Writer->writeEndElement();
}


// Returns a hash that lists all the heading ID's
// in a particular file
QHash< QString, QStringList > NCXWriter::GetHeadingIDsPerFile() const
{
    QHash< QString, QStringList > file_headings;

    foreach( QString relfilepath, m_Files )
    {
        // We skip all the files that are not in the
        // text subdirectory
        if ( !relfilepath.contains( TEXT_FOLDER_NAME + "/" ) )

            continue;

        QString fullfilepath = m_Folder.GetFullPathToOEBPSFolder() + "/" + relfilepath;
        QString source       = Utility::ReadUnicodeTextFile( fullfilepath );

        // FIXME: need headings.
        //QList< Headings::Heading > headings = Headings::GetHeadingList( source );

//         foreach( Headings::Heading heading, headings )
//         {
//             if ( !heading.id.isEmpty() )
//             
//                 file_headings[ relfilepath ].append( heading.id );
//         }
    }
    
    return file_headings;
}


// Returns a hash that lists all the heading sources
// in a particular file. "Source" means full element HTML.
QHash< QString, QStringList > NCXWriter::GetHeadingSourcesPerFile() const
{
    QHash< QString, QStringList > file_headings;

    foreach( QString relfilepath, m_Files )
    {
        // We skip all the files that are not in the
        // text subdirectory
        if ( !relfilepath.contains( TEXT_FOLDER_NAME + "/" ) )

            continue;

        QString fullfilepath = m_Folder.GetFullPathToOEBPSFolder() + "/" + relfilepath;
        QString source       = Utility::ReadUnicodeTextFile( fullfilepath );

        // FIXME: need headings.
//         QList< Headings::Heading > headings = Headings::GetHeadingList( source );
// 
//         foreach( Headings::Heading heading, headings )
//         {
//             file_headings[ relfilepath ].append( heading.element_source );
//         }
    }

    return file_headings;
}


// Returns the relative path to the XHTML file
// the provided heading is located in.
QString NCXWriter::GetHeadingFile( const Headings::Heading &heading ) const
{
    // MASSIVE HACK!
    //   Sigil used to (stupidly) expect that a heading ID was unique epub-wide.
    // And it is if the file is created with Sigil, but not necessarily
    // true for imported epubs. This problem will go away in 0.2.0 when we
    // switch to multi-flow editing, but is too deep to be fixed in this architecture.
    // We need to support multi-level TOC across XHTML files, so we can't just scan
    // the individual files and search for headings. We need to look at the heading
    // hierarchy epub-wide.
    //   This hack works in 99% of cases, and work on 0.2.0 has already started
    // so hopefully we won't have to live with this for long.

    QList< QString > files_with_heading_ID;

    // We try to search for the file that has the heading's ID.
    // We *really* want to find just one.
    foreach( QString file, m_HeadingIDsPerFile.keys() )
    {
        if ( m_HeadingIDsPerFile[ file ].contains( heading.element.attribute( "id" ) ) )
        {
            files_with_heading_ID.append( file );
        }
    }

    // If only one file has the heading ID, we return it.
    if ( files_with_heading_ID.count() == 1 )

        return files_with_heading_ID[ 0 ];

    // This is very bad. We now try to look for the exact heading source code,
    // including classes, text etc. Anything that would differentiate it from
    // the other headings that unfortunately have the same ID.
    // We return the first file that has this source, which may not be unique...

    // FIXME
//     foreach( QString file, m_HeadingSourcesPerFile.keys() )
//     {
//         if ( m_HeadingSourcesPerFile[ file ].contains( heading.element_source ) )
//         {
//             return file;
//         }
//     }

    // TODO: throw an exception.
    return "";
}


// Returns the depth of the headings tree
// specified in m_Headings
int NCXWriter::GetHeadingsDepth() const
{
    int max_depth = 0;

    foreach ( Headings::Heading heading, m_Headings )
    {
        int current_depth = 0;

        DepthWalker( heading, current_depth, max_depth );
    }

    return max_depth;
}


// Used to walk through the headings tree and 
// search for the it's maximum depth
void NCXWriter::DepthWalker( const Headings::Heading &heading, int &current_depth, int &max_depth ) const
{
   if ( heading.include_in_toc == true )
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


