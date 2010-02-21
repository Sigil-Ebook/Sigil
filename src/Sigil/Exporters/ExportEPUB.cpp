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
#include "ExportSGF.h"
#include "../BookManipulation/FolderKeeper.h"
#include "OPFWriter.h"
#include "NCXWriter.h"
#include <ZipArchive.h>
#include "../BookManipulation/CleanSource.h"
#include "../Misc/Utility.h"
#include "../BookManipulation/XHTMLDoc.h"
#include <QDomDocument>


const QString BODY_START = "<\\s*body[^>]*>";
const QString BODY_END   = "</\\s*body\\s*>";
const QString BREAK_TAG_SEARCH  = "(<div>\\s*)?<hr\\s*class\\s*=\\s*\"[^\"]*sigilChapterBreak[^\"]*\"\\s*/>(\\s*</div>)?";

static const QString ID_AND_NAME_ATTRIBUTE = "<[^>]*(?:id|name)\\s*=\\s*\"([^\"]+)\"[^>]*>";

static const QString OPF_FILE_NAME = "content.opf"; 
static const QString NCX_FILE_NAME = "toc.ncx";
static const QString CONTAINER_XML_FILE_NAME = "container.xml"; 


// Constructor;
// the first parameter is the location where the book 
// should be save to, and the second is the book to be saved
ExportEPUB::ExportEPUB( const QString &fullfilepath, const Book &book ) 
    : m_FullFilePath( fullfilepath ), m_Book( book ), m_Folder( book.mainfolder )
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
    CreatePublication();

    SaveTo( m_FullFilePath );
}


// Creates the publication from the Book
// (creates XHTML, CSS, OPF, NCX files etc.)
void ExportEPUB::CreatePublication()
{
//     QStringList css_files = CreateStyleFiles();
//     QString header        = CreateHeader( css_files );
// 
//     CreateXHTMLFiles( header );

    //UpdateAnchors();

    CreateContainerXML();
    CreateContentOPF();
    CreateTocNCX();
}


// Saves the publication to the specified path;
// the second optional parameter specifies the
// mimetype to write to the special "mimetype" file
void ExportEPUB::SaveTo( const QString &fullfilepath, const QString &mimetype )
{
    QTemporaryFile mimetype_file;

    if ( mimetype_file.open() )
    {
        QTextStream out( &mimetype_file );

        // We ALWAYS output in UTF-8
        out.setCodec( "UTF-8" );

        out << mimetype;

        // Write to disk immediately
        out.flush();
        mimetype_file.flush();		
    }

    CZipArchive zip;

    // FIXME: check for return true on zip.open

#ifdef Q_WS_WIN
    // The location where the epub file will be written to
    zip.Open( fullfilepath.utf16(), CZipArchive::zipCreate );  

    // Add the uncompressed mimetype file as per OPF spec
    zip.AddNewFile( mimetype_file.fileName().utf16(), QString( "mimetype" ).utf16(), 0 );

    // Add all the files and folders in the publication structure
    zip.AddNewFiles( QDir::toNativeSeparators( m_Folder.GetFullPathToMainFolder() ).utf16() );

#else
    // The location where the epub file will be written to
    zip.Open( fullfilepath.toUtf8().data(), CZipArchive::zipCreate );  

    // Add the uncompressed mimetype file as per OPF spec
    zip.AddNewFile( mimetype_file.fileName().toUtf8().data(), QString( "mimetype" ).toUtf8().data(), 0 );

    // Add all the files and folders in the publication structure
    zip.AddNewFiles( QDir::toNativeSeparators( m_Folder.GetFullPathToMainFolder() ).toUtf8().data() );
#endif

    zip.Close();
}


// Creates the publication's container.xml file
void ExportEPUB::CreateContainerXML()
{
    QString xml =	"<?xml version=\"1.0\"?>\n"
                    "<container version=\"1.0\" xmlns=\"urn:oasis:names:tc:opendocument:xmlns:container\">\n"
                    "    <rootfiles>\n"
                    "        <rootfile full-path=\"OEBPS/content.opf\" media-type=\"application/oebps-package+xml\"/>\n"
                    "   </rootfiles>\n"
                    "</container>\n";

    QTemporaryFile file;

    if ( file.open() )
    {
        QTextStream out( &file );

        // We ALWAYS output in UTF-8
        out.setCodec( "UTF-8" );

        out << xml;

        // Write to disk immediately
        out.flush();
        file.flush();

        m_Folder.AddInfraFileToFolder( file.fileName(), CONTAINER_XML_FILE_NAME );
    }

    // FIXME: throw exception if not open
}


// Creates the publication's content.opf file
void ExportEPUB::CreateContentOPF()
{
    QTemporaryFile file;

    if ( file.open() )
    {
        QTextStream out( &file );

        // We ALWAYS output in UTF-8
        out.setCodec( "UTF-8" );

        OPFWriter opf( m_Book, m_Folder );

        out << opf.GetXML();

        // Write to disk immediately
        out.flush();
        file.flush();

        m_Folder.AddInfraFileToFolder( file.fileName(), OPF_FILE_NAME );
    }

    // FIXME: throw exception if not open
}


// Creates the publication's toc.ncx file
void ExportEPUB::CreateTocNCX()
{
    QTemporaryFile file;

    if ( file.open() )
    {
        QTextStream out( &file );

        // We ALWAYS output in UTF-8
        out.setCodec( "UTF-8" );

        NCXWriter ncx( m_Book, m_Folder );

        out << ncx.GetXML();

        // Write to disk immediately
        out.flush();
        file.flush();

        m_Folder.AddInfraFileToFolder( file.fileName(), NCX_FILE_NAME );
    }

    // FIXME: throw exception if not open
}






