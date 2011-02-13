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
#include "ImportOEBPS.h"
#include "Misc/Utility.h"
#include "ResourceObjects/Resource.h"
#include "ResourceObjects/OPFResource.h"
#include "ResourceObjects/NCXResource.h"
#include "BookManipulation/FolderKeeper.h"
#include <ZipArchive.h>

static const QString DUBLIN_CORE_NS      = "http://purl.org/dc/elements/1.1/";
static const QString OEBPS_MIMETYPE      = "application/oebps-package+xml";
static const QString UPDATE_ERROR_STRING = "SG_ERROR";
const QString NCX_MIMETYPE               = "application/x-dtbncx+xml";


ImportOEBPS::ImportOEBPS( const QString &fullfilepath )
    : Importer( fullfilepath )
{

}


ImportOEBPS::~ImportOEBPS()
{
    if ( !m_ExtractedFolderPath.isEmpty() )

        QtConcurrent::run( Utility::DeleteFolderAndFiles, m_ExtractedFolderPath );
}


// TODO: create a wrapper lib for this CZipArchive POS
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
            bool success = zip.ExtractFile( i, win_path );
#else
            bool success = zip.ExtractFile( i, nix_path );
#endif
            if ( !success )
            {
                CZipFileHeader* file_header = zip.GetFileInfo( i );
                #ifdef Q_WS_WIN
                std::string filename = QString::fromStdWString( file_header->GetFileName() ).toStdString();
                #else
                std::string filename = QString::fromAscii( file_header->GetFileName().c_str() ).toStdString();
                #endif

                zip.Close(); 

                boost_throw( CannotExtractFile() << errinfo_file_fullpath( filename ) );
            }
        }

        zip.Close(); 
    }

    // We have to to do this here: if we don't wrap
    // this exception and try to catch it "raw" in MainWindow,
    // we get a header name clash from ZipArchive. 
    // The headers are Windows-specific so we can't just rename them.
    catch ( CZipException &exception )
    {
        zip.Close( CZipArchive::afAfterException ); 

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


void ImportOEBPS::LocateOPF()
{
    QString fullpath = m_ExtractedFolderPath + "/META-INF/container.xml";
    QXmlStreamReader container( Utility::ReadUnicodeTextFile( fullpath ) );

    while ( !container.atEnd() ) 
    {
        container.readNext();

        if ( container.isStartElement() && 
             container.name() == "rootfile"
           ) 
        {
            if ( container.attributes().hasAttribute( "media-type" ) &&
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
        boost_throw( ErrorParsingContentXml() 
                     << errinfo_XML_parsing_error_string(  container.errorString().toStdString() )
                     << errinfo_XML_parsing_line_number(   container.lineNumber() )
                     << errinfo_XML_parsing_column_number( container.columnNumber() )
                   );
    }

    if ( m_OPFFilePath.isEmpty() )
    {
        boost_throw( NoAppropriateOPFFileFound() );    
    }
}


void ImportOEBPS::ReadOPF()
{
    QString opf_text = Utility::ReadUnicodeTextFile( m_OPFFilePath );

    // MASSIVE hack for XML 1.1 "support";
    // this is only for people who specify
    // XML 1.1 when they actually only use XML 1.0 
    QString source = opf_text.replace(  QRegExp( "<\\?xml\\s+version=\"1.1\"\\s*\\?>" ),
                                                 "<?xml version=\"1.0\"?>"
                                     );

    QXmlStreamReader opf_reader( source );

    while ( !opf_reader.atEnd() ) 
    {
        opf_reader.readNext();

        if ( !opf_reader.isStartElement() ) 

            continue;

        if ( opf_reader.name() == "package" )

            m_UniqueIdentifierId = opf_reader.attributes().value( "", "unique-identifier" ).toString();
        
        // Parse and store Dublin Core metadata elements
        else if ( opf_reader.namespaceUri() == DUBLIN_CORE_NS )
        
            ReadDublinCoreElement( opf_reader );

        else if ( opf_reader.name() == "meta" )

            ReadRegularMetaElement( opf_reader );

        // Get the list of content files that
        // make up the publication
        else if ( opf_reader.name() == "item" )

            ReadManifestItemElement( opf_reader );

        // We read this just to get the NCX id
        else if ( opf_reader.name() == "spine" )

            ReadSpineElement( opf_reader );

        // Get the list of XHTML files that
        // represent the reading order
        else if ( opf_reader.name() == "itemref" )

            ReadSpineItemRefElement( opf_reader );

        // Get the <guide> semantic information 
        else if ( opf_reader.name() == "reference" )
        
            ReadGuideReferenceElement( opf_reader );        
    }

    if ( opf_reader.hasError() )
    {
        boost_throw( ErrorParsingOpf() 
                     << errinfo_XML_parsing_error_string( opf_reader.errorString().toStdString() )
                     << errinfo_XML_parsing_line_number( opf_reader.lineNumber() )
                     << errinfo_XML_parsing_column_number( opf_reader.columnNumber() )
                   );
    }
}


void ImportOEBPS::ReadDublinCoreElement( QXmlStreamReader &opf_reader )
{
    Metadata::MetaElement meta;                

    // We create a copy of the attributes because
    // the QXmlStreamAttributes die out after we 
    // move away from the token.
    foreach( QXmlStreamAttribute attribute, opf_reader.attributes() )
    {
        meta.attributes[ attribute.name().toString() ] = attribute.value().toString();
    }

    meta.name = opf_reader.name().toString();

    QString element_text = opf_reader.readElementText();
    meta.value = element_text;

    // Empty metadata entries
    if ( !element_text.isEmpty() )

        m_MetaElements.append( meta );
}


void ImportOEBPS::ReadRegularMetaElement( QXmlStreamReader &opf_reader )
{
    QString name    = opf_reader.attributes().value( "", "name"    ).toString(); 
    QString content = opf_reader.attributes().value( "", "content" ).toString();

    // For now, we only recognize the special iPad
    // cover meta. It is in the form of name=cover
    // and content=imageID, where the ID is from the manifest.
    if ( name == "cover" )
    {
        QHash< QString, QString > semantics;
        semantics[ name ] = name;
        m_SemanticInformation[ content ] = semantics;
    }
}


void ImportOEBPS::ReadManifestItemElement( QXmlStreamReader &opf_reader )
{
    QString id   = opf_reader.attributes().value( "", "id"         ).toString(); 
    QString href = opf_reader.attributes().value( "", "href"       ).toString();
    QString type = opf_reader.attributes().value( "", "media-type" ).toString();

    // Paths are percent encoded in the OPF, we use "normal" paths internally.
    href = Utility::URLDecodePath( href );

    if ( type != NCX_MIMETYPE )         
    {                    
        if ( !m_MainfestFilePaths.contains( href ) )
        {
            m_Files[ id ] = href;
            m_MainfestFilePaths << href;
        }
    }

    else
    {
        m_NcxCandidates[ id ] = href;
    }
}


void ImportOEBPS::ReadSpineElement( QXmlStreamReader &opf_reader )
{
    QString ncx_id = opf_reader.attributes().value( "", "toc" ).toString();

    m_NCXFilePath = QFileInfo( m_OPFFilePath ).absolutePath() + "/" + m_NcxCandidates[ ncx_id ];
}


void ImportOEBPS::ReadSpineItemRefElement( QXmlStreamReader &opf_reader )
{
    m_ReadingOrderIds.append( opf_reader.attributes().value( "", "idref" ).toString() );
}


void ImportOEBPS::ReadGuideReferenceElement( QXmlStreamReader &opf_reader )
{
    QString type  = opf_reader.attributes().value( "", "type"  ).toString(); 
    QString title = opf_reader.attributes().value( "", "title" ).toString();
    QString href  = opf_reader.attributes().value( "", "href"  ).toString();

    // Paths are percent encoded in the OPF, we use "normal" paths internally.
    href = Utility::URLDecodePath( href );

    // We remove the fragment identifier if there is one.
    href = !href.contains( "#" ) ? href : href.left( href.indexOf( "#" ) );

    foreach( QString id, m_Files.keys() )
    {
        if ( m_Files[ id ] == href )
        {
            QHash< QString, QString > semantics;
            semantics[ type ] = title;
            m_SemanticInformation[ id ] = semantics;

            break;
        }
    }
}


void ImportOEBPS::LoadInfrastructureFiles()
{
    m_Book->GetOPF().SetText( Utility::ReadUnicodeTextFile( m_OPFFilePath ) );
    m_Book->GetNCX().SetText( Utility::ReadUnicodeTextFile( m_NCXFilePath ) );
}


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


QHash< QString, QString > ImportOEBPS::LoadFolderStructure()
{ 
    QList< QString > keys = m_Files.keys();
    int num_files = keys.count();

    QFutureSynchronizer< tuple< QString, QString > > sync;

    for ( int i = 0; i < num_files; ++i )
    {   
        QString id = keys.at( i );
        sync.addFuture( QtConcurrent::run( 
                this, 
                &ImportOEBPS::LoadOneFile, 
                m_Files.value( id ),
                m_ReadingOrderIds.indexOf( id ),
                m_SemanticInformation.value( id ) ) );   
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


tuple< QString, QString > ImportOEBPS::LoadOneFile( const QString &path,
                                                    int reading_order,
                                                    const QHash< QString, QString > &semantic_info )
{
    QString fullfilepath = QFileInfo( m_OPFFilePath ).absolutePath() + "/" + path;

    try
    {
        Resource &resource = m_Book->GetFolderKeeper().AddContentFileToFolder( fullfilepath, 
                                reading_order, semantic_info );
        QString newpath = "../" + resource.GetRelativePathToOEBPS(); 

        return make_tuple( fullfilepath, newpath );
    }
    
    catch ( FileDoesNotExist& )
    {
    	return make_tuple( UPDATE_ERROR_STRING, UPDATE_ERROR_STRING );
    }
}


