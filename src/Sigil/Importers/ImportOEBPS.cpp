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
#include "ImportOEBPS.h"
#include "../Misc/Utility.h"
#include "ResourceObjects/Resource.h"
#include <ZipArchive.h>

static const QString OEBPS_MIMETYPE      = "application/oebps-package+xml";
static const QString UPDATE_ERROR_STRING = "SG_ERROR";


ImportOEBPS::ImportOEBPS( const QString &fullfilepath )
    : Importer( fullfilepath )
{

}


// Destructor
ImportOEBPS::~ImportOEBPS()
{
    if ( !m_ExtractedFolderPath.isEmpty() )

        QtConcurrent::run( Utility::DeleteFolderAndFiles, m_ExtractedFolderPath );
}


// Extracts the EPUB file to a temporary folder;
// the path to this folder is stored in m_ExtractedFolderPath
void ImportOEBPS::ExtractContainer()
{
    QDir folder( Utility::GetNewTempFolderPath() );
    m_ExtractedFolderPath = folder.absolutePath();
    folder.mkpath( m_ExtractedFolderPath );

    CZipArchive zip;

    try
    {
#ifdef Q_WS_WIN
        zip.Open( m_FullFilePath.utf16(), CZipArchive::zipOpenReadOnly );
#else
        zip.Open( m_FullFilePath.toUtf8().data(), CZipArchive::zipOpenReadOnly );
#endif

        int file_count = (int) zip.GetCount();
        QString folder_path = folder.absolutePath();

#ifdef Q_WS_WIN
        const ushort *win_path = folder_path.utf16();
#else
        QByteArray utf8_path( folder_path.toUtf8() );
        char *nix_path = utf8_path.data();
#endif
        for ( int i = 0; i < file_count; ++i )
        {
#ifdef Q_WS_WIN
            zip.ExtractFile( i, win_path );
#else
            zip.ExtractFile( i, nix_path );
#endif
        }

        zip.Close(); 
    }

    // We have to to do this here: if we don't wrap
    // this exception and try to catch "raw" in MainWindow,
    // we get some dumb header name clash from ZipArchive
    catch ( CZipException &exception )
    {
        // The error description is always ASCII
#ifdef Q_WS_WIN
        boost_throw( CZipExceptionWrapper() 
                     << errinfo_zip_info( QString::fromStdWString( exception.GetErrorDescription() ).toStdString() ) );
#else
        boost_throw( CZipExceptionWrapper()
                     << errinfo_zip_info( QString::fromAscii( exception.GetErrorDescription().c_str() ).toStdString() ) );
#endif

    }
}


// Locates the OPF file in the extracted folder;
// the path to the OPF is stored in m_OPFFilePath
void ImportOEBPS::LocateOPF()
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
void ImportOEBPS::ReadOPF()
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

                if ( !href.endsWith( ".ncx" ) )

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
void ImportOEBPS::LoadMetadata()
{
    QHash< QString, QList< QVariant > > metadata;

    foreach( Metadata::MetaElement meta, m_MetaElements )
    {
        Metadata::MetaElement book_meta = Metadata::Instance().MapToBookMetadata( meta, "DublinCore" );

        if ( !book_meta.name.isEmpty() && !book_meta.value.toString().isEmpty() )
        {
            metadata[ book_meta.name ].append( book_meta.value );
        }
    }

    m_Book->SetMetadata( metadata );
}

// Loads the referenced files into the main folder of the book.
// Returns a hash with keys being old references (URLs) to resources,
// and values being the new references to those resources.
QHash< QString, QString > ImportOEBPS::LoadFolderStructure()
{ 
    QList< QString > keys = m_Files.keys();
    int num_files = keys.count();

    QFutureSynchronizer< tuple< QString, QString > > sync;

    for ( int i = 0; i < num_files; ++i )
    {   
        sync.addFuture( QtConcurrent::run( this, &ImportOEBPS::LoadOneFile, keys.at( i ) ) );   
    }

    sync.waitForFinished();

    QList< QFuture< tuple< QString, QString > > > futures = sync.futures();
    int num_futures = futures.count();

    QHash< QString, QString > updates;

    for ( int i = 0; i < num_futures; ++i )
    {
        tuple< QString, QString > result = futures.at( i ).result();
        updates[ result.get< 0 >() ] = result.get< 1 >();
    }

    updates.remove( UPDATE_ERROR_STRING );
    return updates;
}


tuple< QString, QString > ImportOEBPS::LoadOneFile( const QString &key )
{
    QString path         = m_Files.value( key );
    QString fullfilepath = QFileInfo( m_OPFFilePath ).absolutePath() + "/" + path;

    try
    {
        Resource &resource = m_Book->GetFolderKeeper().AddContentFileToFolder( fullfilepath, m_ReadingOrderIds.indexOf( key ) );
        QString newpath = "../" + resource.GetRelativePathToOEBPS(); 

        return make_tuple( path, newpath );
    }
    
    catch ( FileDoesNotExist& )
    {
    	return make_tuple( UPDATE_ERROR_STRING, UPDATE_ERROR_STRING );
    }
}

