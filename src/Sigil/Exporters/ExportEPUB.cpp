/************************************************************************
**
**  Copyright (C) 2009, 2010  Strahinja Markovic
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
#include "ExportEPUB.h"
#include "BookManipulation/FolderKeeper.h"
#include "OPFWriter.h"
#include "EncryptionXmlWriter.h"
#include "NCXWriter.h"
#include <ZipArchive.h>
#include "BookManipulation/CleanSource.h"
#include "Misc/Utility.h"
#include "Misc/FontObfuscation.h"
#include "BookManipulation/XhtmlDoc.h"
#include "ResourceObjects/FontResource.h"

const QString BODY_START = "<\\s*body[^>]*>";
const QString BODY_END   = "</\\s*body\\s*>";

const QString OPF_FILE_NAME            = "content.opf"; 
const QString NCX_FILE_NAME            = "toc.ncx";
const QString CONTAINER_XML_FILE_NAME  = "container.xml";
const QString ENCRYPTION_XML_FILE_NAME = "encryption.xml";

static const QString METAINF_FOLDER_SUFFIX = "/META-INF";
static const QString OEBPS_FOLDER_SUFFIX   = "/OEBPS";

static const QString EPUB_MIME_TYPE = "application/epub+zip";


// Constructor;
// the first parameter is the location where the book 
// should be save to, and the second is the book to be saved
ExportEPUB::ExportEPUB( const QString &fullfilepath, QSharedPointer< Book > book ) 
    : 
    m_FullFilePath( fullfilepath ), 
    m_Book( book ) 
{
    
}


// Destructor
ExportEPUB::~ExportEPUB()
{

}


// Writes the book to the path 
// specified in the constructor
void ExportEPUB::WriteBook()
{
    m_Book->SaveAllResourcesToDisk();

    // TODO: wrap all occurrences of this idiom into an object
    // that deletes the temp folder in the destructor
    QString folderpath = Utility::GetNewTempFolderPath();
    QDir dir( folderpath );
    dir.mkpath( dir.absolutePath() );

    CreatePublication( folderpath );

    if ( m_Book->HasObfuscatedFonts() )

        ObfuscateFonts( folderpath );

    SaveFolderAsEpubToLocation( folderpath, m_FullFilePath );

    QtConcurrent::run( Utility::DeleteFolderAndFiles, folderpath );
}


// Creates the publication from the Book
// (creates XHTML, CSS, OPF, NCX files etc.)
void ExportEPUB::CreatePublication( const QString &fullfolderpath )
{
    Utility::CopyFiles( m_Book->GetFolderKeeper().GetFullPathToMainFolder(), fullfolderpath );

    CreateContainerXML( fullfolderpath + METAINF_FOLDER_SUFFIX );    
    CreateContentOPF( fullfolderpath + OEBPS_FOLDER_SUFFIX );
    CreateTocNCX( fullfolderpath + OEBPS_FOLDER_SUFFIX );

    if ( m_Book->HasObfuscatedFonts() )
        
        CreateEncryptionXML( fullfolderpath + METAINF_FOLDER_SUFFIX );
}


void ExportEPUB::SaveFolderAsEpubToLocation( const QString &fullfolderpath, const QString &fullfilepath )
{
    QTemporaryFile mimetype_file;

    if ( mimetype_file.open() )
    {
        QTextStream out( &mimetype_file );

        // We ALWAYS output in UTF-8
        out.setCodec( "UTF-8" );

        out << EPUB_MIME_TYPE;

        // Write to disk immediately
        out.flush();
        mimetype_file.flush();		
    }

    CZipArchive zip;

    try
    {
    #ifdef Q_WS_WIN
        // The location where the epub file will be written to
        zip.Open( fullfilepath.utf16(), CZipArchive::zipCreate );  

        // Add the uncompressed mimetype file as per OPF spec
        zip.AddNewFile( mimetype_file.fileName().utf16(), QString( "mimetype" ).utf16(), 0 );

        // Add all the files and folders in the publication structure
        zip.AddNewFiles( QDir::toNativeSeparators( fullfolderpath ).utf16(), QString( "*" ).utf16() );

    #else
        // The location where the epub file will be written to
        zip.Open( fullfilepath.toUtf8().data(), CZipArchive::zipCreate );  

        // Add the uncompressed mimetype file as per OPF spec
        zip.AddNewFile( mimetype_file.fileName().toUtf8().data(), QString( "mimetype" ).toUtf8().data(), 0 );

        // Add all the files and folders in the publication structure
        zip.AddNewFiles( QDir::toNativeSeparators( fullfolderpath ).toUtf8().data(), QString( "*" ).toUtf8().data() );
    #endif

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


// Creates the publication's container.xml file
void ExportEPUB::CreateContainerXML( const QString &fullfolderpath )
{
    QString xml =	"<?xml version=\"1.0\"?>\n"
                    "<container version=\"1.0\" xmlns=\"urn:oasis:names:tc:opendocument:xmlns:container\">\n"
                    "    <rootfiles>\n"
                    "        <rootfile full-path=\"OEBPS/content.opf\" media-type=\"application/oebps-package+xml\"/>\n"
                    "   </rootfiles>\n"
                    "</container>\n";

    QTemporaryFile file;

    if ( !file.open() )
    {
        boost_throw( CannotOpenFile() 
                     << errinfo_file_fullpath( file.fileName().toStdString() )
                     << errinfo_file_errorstring( file.errorString().toStdString() ) 
                   );
    }

    QTextStream out( &file );

    // We ALWAYS output in UTF-8
    out.setCodec( "UTF-8" );

    out << xml;

    // Write to disk immediately
    out.flush();
    file.flush();

    QFile::copy( file.fileName(), fullfolderpath + "/" + CONTAINER_XML_FILE_NAME );  
}


// Creates the publication's content.opf file
void ExportEPUB::CreateContentOPF( const QString &fullfolderpath )
{
    QTemporaryFile file;

    if ( !file.open() )
    {
        boost_throw( CannotOpenFile() 
                     << errinfo_file_fullpath( file.fileName().toStdString() )
                     << errinfo_file_errorstring( file.errorString().toStdString() ) 
                   );
    }
    
    OPFWriter opf( m_Book, file );
    opf.WriteXML();

    // Write to disk immediately
    file.flush();

    QFile::copy( file.fileName(), fullfolderpath + "/" + OPF_FILE_NAME ); 
}


// Creates the publication's toc.ncx file
void ExportEPUB::CreateTocNCX( const QString &fullfolderpath )
{
    QTemporaryFile file;

    if ( !file.open() )
    {
        boost_throw( CannotOpenFile() 
                     << errinfo_file_fullpath( file.fileName().toStdString() )
                     << errinfo_file_errorstring( file.errorString().toStdString() ) 
                   );
    }

    NCXWriter ncx( m_Book, file );
    ncx.WriteXML();

    // Write to disk immediately
    file.flush();

    QFile::copy( file.fileName(), fullfolderpath + "/" + NCX_FILE_NAME ); 
}


void ExportEPUB::CreateEncryptionXML( const QString &fullfolderpath )
{
    QTemporaryFile file;

    if ( !file.open() )
    {
        boost_throw( CannotOpenFile() 
                     << errinfo_file_fullpath( file.fileName().toStdString() )
                     << errinfo_file_errorstring( file.errorString().toStdString() ) 
                   );
    }
    
    EncryptionXmlWriter enc( m_Book, file );
    enc.WriteXML();

    // Write to disk immediately
    file.flush();

    QFile::copy( file.fileName(), fullfolderpath + "/" + ENCRYPTION_XML_FILE_NAME ); 
}


void ExportEPUB::ObfuscateFonts( const QString &fullfolderpath )
{
    QString uuid_id = "urn:uuid:" + m_Book->GetPublicationIdentifier();

    QHash< QString, QList< QVariant > > metadata = m_Book->GetMetadata();    

    // Also see OPFWriter::WriteMetadata
    QString main_id = metadata.contains( "CustomID" )        ?
                      metadata[ "CustomID" ][ 0 ].toString() :
                      uuid_id;

    QList< FontResource* > font_resources = m_Book->GetFolderKeeper().GetResourceTypeList< FontResource >();

    foreach( FontResource *font_resource, font_resources )
    {
        QString algorithm = font_resource->GetObfuscationAlgorithm();

        if ( algorithm.isEmpty() )

            continue;

        QString font_path = fullfolderpath + "/" + font_resource->GetRelativePathToRoot();

        if ( algorithm == ADOBE_FONT_ALGO_ID )

            FontObfuscation::ObfuscateFile( font_path, algorithm, uuid_id );

        else 

            FontObfuscation::ObfuscateFile( font_path, algorithm, main_id );
    }    
}






