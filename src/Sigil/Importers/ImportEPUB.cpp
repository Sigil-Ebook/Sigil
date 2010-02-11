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
#include "../Misc/HTMLEncodingResolver.h"
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
    //StripFilesFromAnchors();

    QHash< QString, QString > updates = LoadFolderStructure(); 
    CleanHTMLFiles();

    // TODO: reference updating... this will be hard
    //UpdateReferences( updates );

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

    QXmlStreamReader container( Utility::ReadUnicodeTextFile( fullpath ) );

    while ( !container.atEnd() ) 
    {
        // Get the next token from the stream
        QXmlStreamReader::TokenType type = container.readNext();

        if (  type == QXmlStreamReader::StartElement && 
              container.name() == "rootfile"
           ) 
        {
            if (  container.attributes().hasAttribute( "media-type" ) &&
                  container.attributes().value( "", "media-type" ) == OEBPS_MIMETYPE 
               )
            {
                m_OPFFilePath = m_ExtractedFolderPath + "/" + container.attributes().value( "", "full-path" ).toString();

                // As per OCF spec, the first rootfile element
                // with the OEBPS mimetype is considered the "main" one.
                break;
            }
        }
    }

    if ( container.hasError() )
    {
        boost_throw( ErrorParsingContentXML() 
                     << errinfo_XML_parsing_error_string( container.errorString().toStdString() )
                     << errinfo_XML_parsing_line_number( container.lineNumber() )
                     << errinfo_XML_parsing_column_number( container.columnNumber() )
                   );
    }

    if ( m_OPFFilePath.isEmpty() )
    {
        boost_throw( NoAppropriateOPFFileFound() );    
    }
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
                QString id   = opf.attributes().value( "", "id" ).toString(); 
                QString href = opf.attributes().value( "", "href" ).toString();

                href = QUrl::fromPercentEncoding( href.toUtf8() );

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
        boost_throw( ErrorParsingOPF() 
                     << errinfo_XML_parsing_error_string( opf.errorString().toStdString() )
                     << errinfo_XML_parsing_line_number( opf.lineNumber() )
                     << errinfo_XML_parsing_column_number( opf.columnNumber() )
                   );
    }
    
}

// Loads the metadata from the m_MetaElements list
// (filled by reading the OPF) into the book
void ImportEPUB::LoadMetadata()
{
    foreach( Metadata::MetaElement meta, m_MetaElements )
    {
        Metadata::MetaElement book_meta = Metadata::Instance().MapToBookMetadata( meta, "DublinCore" );

        if ( !book_meta.name.isEmpty() && !book_meta.value.toString().isEmpty() )
        {
            m_Book.metadata[ book_meta.name ].append( book_meta.value );
        }
    }    
}


void ImportEPUB::CleanHTMLFiles()
{
    QFutureSynchronizer< void > synchronizer;

    foreach( QString file, m_Book.mainfolder.GetContentFilesList() )
    {
        if ( !file.contains( TEXT_FOLDER_NAME + "/" ) )

            continue;
        
        QString fullfilepath = m_Book.mainfolder.GetFullPathToOEBPSFolder() + "/" + file;
        synchronizer.addFuture( QtConcurrent::run( CleanOneHTMLFile, fullfilepath ) );       
    }

    synchronizer.waitForFinished();
}

void ImportEPUB::CleanOneHTMLFile( QString &fullpath )
{
    // TODO: profile whether this load + clean + qdom.setsource
    // is faster than load + clean + qwebpage load

    QString source = CleanSource::Clean( HTMLEncodingResolver::ReadHTMLFile( fullpath ) );
    Utility::WriteUnicodeTextFile( source, fullpath );
}


// Loads the referenced files into the main folder of the book.
// Returns a hash with keys being old references (URLs) to resources,
// and values being the new references to those resources.
QHash< QString, QString > ImportEPUB::LoadFolderStructure()
{
    QHash< QString, QString > updates;

    foreach( QString key, m_Files.keys() )
    {
        QString path         = m_Files[ key ];
        QString fullfilepath = QFileInfo( m_OPFFilePath ).absolutePath() + "/" + path;

        QString newpath = m_Book.mainfolder.AddContentFileToFolder( fullfilepath, m_ReadingOrderIds.indexOf( key ) );
        newpath = "../" + newpath;  

        updates[ path ] = newpath;       
    }

    return updates;
}




