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
#include "ImportEPUB.h"
#include "../BookManipulation/CleanSource.h"
#include "../Misc/Utility.h"
#include <ZipArchive.h>
#include "../BookManipulation/XHTMLDoc.h"
#include <QDomDocument>

static const QString OEBPS_MIMETYPE = "application/oebps-package+xml";

// Constructor;
// The parameter is the file to be imported
ImportEPUB::ImportEPUB( const QString &fullfilepath )
    : ImportHTML( fullfilepath )
{

}


// Destructor
ImportEPUB::~ImportEPUB()
{
    if ( !m_ExtractedFolderPath.isEmpty() )

        Utility::DeleteFolderAndFiles( m_ExtractedFolderPath );
}


// Reads and parses the file 
// and returns the created Book
Book ImportEPUB::GetBook()
{
    if ( !Utility::IsFileReadable( m_FullFilePath ) )

        return Book();

    // These read the EPUB file
    ExtractContainer();
    LocateOPF();
    ReadOPF();

    // These mutate the m_Book object
    LoadMetadata();
    LoadSource();
    AddHeaderToSource();
    StripFilesFromAnchors();
    UpdateReferences( LoadFolderStructure() );

    m_Book.source = CleanSource::Clean( m_Book.source );

    return m_Book;
}


// Extracts the EPUB file to a temporary folder;
// the path to this folder is stored in m_ExtractedFolderPath
void ImportEPUB::ExtractContainer()
{
    QDir folder( Utility::GetNewTempFolderPath() );

    m_ExtractedFolderPath = folder.absolutePath();

    folder.mkpath( m_ExtractedFolderPath );

    CZipArchive zip;

#ifdef Q_WS_WIN
    zip.Open( m_FullFilePath.utf16(), CZipArchive::zipOpenReadOnly );
#else
    zip.Open( m_FullFilePath.toUtf8().data(), CZipArchive::zipOpenReadOnly );
#endif

    int file_count = (int) zip.GetCount();

    for ( int i = 0; i < file_count; ++i )
    {
        #ifdef Q_WS_WIN
        zip.ExtractFile( i, folder.absolutePath().utf16() );
        #else
        zip.ExtractFile( i, folder.absolutePath().toUtf8().data() );
        #endif
    }

    zip.Close(); 
}


// Locates the OPF file in the extracted folder;
// the path to the OPF is stored in m_OPFFilePath
void ImportEPUB::LocateOPF()
{
    QDir folder( m_ExtractedFolderPath );

    folder.cd( "META-INF" );

    QString fullpath = folder.absoluteFilePath( "container.xml" );

    QDomDocument document;
    document.setContent( Utility::ReadUnicodeTextFile( fullpath ) );

    // Each <rootfile> element specifies the rootfile of 
    // a single rendition of the contained publication.
    // There is *usually* just one, and it is *usually* the OPF doc.
    QDomNodeList root_files = document.elementsByTagName( "rootfile" );

    for ( int i = 0; i < root_files.count(); ++i )
    {
        QDomElement element = root_files.at( i ).toElement();

        if (  element.hasAttribute( "media-type" ) &&
              element.attribute( "media-type" ) == OEBPS_MIMETYPE 
           )
        {
            m_OPFFilePath = m_ExtractedFolderPath + "/" + element.attribute( "full-path", "" );

            // As per OCF spec, the first rootfile element
            // with the OEBPS mimetype is considered the "main" one.
            break;
        }
    }

    // TODO: throw exception if no appropriate OEBPS root file was found
}


// Parses the OPF file and stores the parsed information
// inside m_MetaElements, m_Files and m_ReadingOrderIds
void ImportEPUB::ReadOPF()
{
    QString opf_text = Utility::ReadUnicodeTextFile( m_OPFFilePath );

    // MASSIVE hack for XML 1.1 "support";
    // this is only for people who specify
    // XML 1.1 when they actually only use XML 1.0 
    QString source = opf_text.replace(  QRegExp( "<\\?xml\\s+version=\"1.1\"\\s*\\?>" ),
                                                 "<?xml version=\"1.0\"?>"
                                     );

    QXmlStreamReader opf( source );

    while ( !opf.atEnd() ) 
    {
        // Get the next token from the stream
        QXmlStreamReader::TokenType type = opf.readNext();

        if ( type == QXmlStreamReader::StartElement ) 
        {
            // Parse and store Dublin Core metadata elements
            if ( opf.qualifiedName().toString().startsWith( "dc:" ) == true )
            {
	        Metadata::MetaElement meta;                
                
                // We create a copy of the attributes because
                // the QXmlStreamAttributes die out after we 
                // move away from the token
                foreach( QXmlStreamAttribute attribute, opf.attributes() )
                {
                    meta.attributes[ attribute.name().toString() ] = attribute.value().toString();
                }

                meta.name = opf.name().toString();

                QString element_text = opf.readElementText();
                meta.value = element_text;

                // Empty metadata entries
                if ( !element_text.isEmpty() )

                    m_MetaElements.append( meta );
            }

            // Get the list of content files that
            // make up the publication
            else if ( opf.name() == "item" )           
            {
                QString id      = opf.attributes().value( "", "id" ).toString(); 
                QString href    = opf.attributes().value( "", "href" ).toString();

                if ( !href.contains( ".ncx" ) )
                     
                    m_Files[ id ] = href;
            }

            // Get the list of XHTML files that
            // represent the reading order
            else if ( opf.name() == "itemref" )           
            {
                m_ReadingOrderIds.append( opf.attributes().value( "", "idref" ).toString() );
            }
        }
    }

    if ( opf.hasError() )
    {
        // TODO: error handling
    }
    
}

// Loads the metadata from the m_MetaElements list
// (filled by reading the OPF) into the book
void ImportEPUB::LoadMetadata()
{

  foreach( Metadata::MetaElement meta, m_MetaElements )
    {

        Metadata::MetaElement book_meta = Metadata::Instance().MapToBookMetadata( meta, "DublinCore" );

	if ( !book_meta.name.isEmpty() && !book_meta.value.isEmpty() )
	{
            m_Book.metadata[ book_meta.name ].append( book_meta.value );
	}
    }    
}


// Loads the source code into the Book
void ImportEPUB::LoadSource()
{
    bool is_first_text_file = true;

    foreach( QString id, m_ReadingOrderIds )
    {
        QString fullpath = QFileInfo( m_OPFFilePath ).absolutePath() + "/" + m_Files[ id ];
        QString text     = ResolveCustomEntities( Utility::ReadUnicodeTextFile( fullpath ) );

        // We extract the content of the files
        // that is within the <body> tag
        QRegExp body_start_tag( BODY_START );
        QRegExp body_end_tag( BODY_END );

        int body_begin	= text.indexOf( body_start_tag, 0 ) + body_start_tag.matchedLength();
        int body_end	= text.indexOf( body_end_tag, 0 );

        QString content = Utility::Substring( body_begin, body_end, text );

        // We don't add our chapter break tag
        // for the first text file
        if ( is_first_text_file == false )
        {            
            m_Book.source += BREAK_TAG_INSERT + "\n" + content;
        }

        else
        {
            m_Book.source += content;

            is_first_text_file = false;
        }
    }  

    m_Book.source += "</body> </html>";
}


// Adds the header to the Book source code
void ImportEPUB::AddHeaderToSource()
{
    QString header =    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                        "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\"\n"
                        "    \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n\n"							
                        "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n"
                        "<head>\n";    

    // Creates <style> tags from CSS files
    foreach( QString path, m_Files.values() )
    {
        if ( path.contains( ".css" ) || path.contains( ".xpgt" )  )
        {
            QString style_tag = CreateStyleTag( QFileInfo( m_OPFFilePath ).absolutePath() + "/" + path );

            header += style_tag;
        }
    }

    header += "</head>\n<body>\n";

    m_Book.source = header + m_Book.source;
}


// Loads the referenced files into the main folder of the book.
// Returns a hash with keys being old references (URLs) to resources,
// and values being the new references to those resources.
QHash< QString, QString > ImportEPUB::LoadFolderStructure()
{
    QHash< QString, QString > updates;

    foreach( QString key, m_Files.keys() )
    {
        QString path = m_Files[ key ];

        // We skip over the book text and style files
        if (    ( !m_ReadingOrderIds.contains( key ) ) &&
                ( !path.contains( ".css" ) )           &&
                ( !path.contains( ".xpgt" ) )                   
            )
        {
            QString fullfilepath = QFileInfo( m_OPFFilePath ).absolutePath() + "/" + path;

            QString newpath = m_Book.mainfolder.AddContentFileToFolder( fullfilepath );
            newpath = "../" + newpath;  

            updates[ path ] = newpath;
        }        
    }

    return updates;
}



