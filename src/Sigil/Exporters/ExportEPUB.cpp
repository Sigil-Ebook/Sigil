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
    QStringList css_files   = CreateStyleFiles();
    QString header          = CreateHeader( css_files );

    CreateXHTMLFiles( header );

    UpdateAnchors();

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


// Creates style files from the style tags in the source
// and returns a list of their file paths relative 
// to the OEBPS folder in the FolderKeeper
QStringList ExportEPUB::CreateStyleFiles()
{
    QStringList style_files;

    QList< XHTMLDoc::XMLElement > style_tag_nodes = XHTMLDoc::GetTagsInHead( m_Book.source, "style" );

    foreach( XHTMLDoc::XMLElement element, style_tag_nodes )
    {
        QString extension = "";

        if ( element.attributes.value( "type" )  == "text/css" )  
    
            extension = "css";

        else // This is an XPGT stylesheet

            extension = "xpgt";

        QString style_text = element.text;
        style_text = RemoveSigilStyles( style_text );
        style_text = StripCDATA( style_text );
        style_text = style_text.trimmed();        

        if ( !style_text.isEmpty() )

            style_files << "../" + CreateOneTextFile( style_text, extension );
    }
    
    return style_files;   
}


// Takes a list of style sheet file names 
// and returns the header for XHTML files
QString ExportEPUB::CreateHeader( const QStringList &cssfiles ) const
{
    QString header =    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                        "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\"\n"
                        "    \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n\n"							
                        "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n"
                        "<head>\n";

    foreach( QString file, cssfiles )
    {
        header += "    <link href=\"" + file + "\" rel=\"stylesheet\" type=\"text/css\" />\n";
    }

    header += "</head>\n";

    return header;
}


// Creates XHTML files from the book source;
// the provided header is used as the header of the created files
void ExportEPUB::CreateXHTMLFiles( const QString &header )
{
    QRegExp body_start_tag( BODY_START );
    QRegExp body_end_tag( BODY_END );

    int body_begin	= m_Book.source.indexOf( body_start_tag, 0 ) + body_start_tag.matchedLength();
    int body_end	= m_Book.source.indexOf( body_end_tag, 0 );

    int main_index = body_begin;

    while ( main_index != body_end )
    {
        QRegExp break_tag( BREAK_TAG_SEARCH );

        // We search for our HR break tag
        int break_index = m_Book.source.indexOf( break_tag, main_index );

        QString body;

        // We break up the remainder of the file on the HR tag index if it's found
        if ( break_index > -1 )
        {
            body = Utility::Substring( main_index, break_index, m_Book.source );

            main_index = break_index + break_tag.matchedLength();
        }

        // Otherwise, we take the rest of the file
        else
        {
            body = Utility::Substring( main_index, body_end, m_Book.source );

            main_index = body_end;
        }

        // FIXME: the <title> tag should be created with chapter name
        // and added to the header

        QString wholefile = header + "<body>\n" + body + "</body> </html>";

        wholefile = CleanSource::Clean( wholefile );

        CreateOneTextFile( wholefile, "xhtml" );		
    }	
}


// Creates one text file from the provided source
// and adds it to the FolderKeeper object with
// the provided extension; returns the file path
// relative to the OEBPS folder
QString ExportEPUB::CreateOneTextFile( const QString &source, const QString &extension )
{
    QTemporaryFile file;

    if ( file.open() )
    {
        QTextStream out( &file );

        // We ALWAYS output in UTF-8
        out.setCodec( "UTF-8" );

        out << source;

        // Write to disk immediately
        out.flush();
        file.flush();

        return "";

        //return m_Folder.AddContentFileToFolder( file.fileName(), extension );
    }

    else
    {
        // FIXME: throw exception

        return "";
    }
}


// Strips CDATA declarations from the provided source
QString ExportEPUB::StripCDATA( const QString &style_source ) const
{
    QString newsource = style_source;

    newsource.replace( "/*<![CDATA[*/", "" );
    newsource.replace( "/*]]>*/", "" );
    newsource.replace( "/**/", "" );

    newsource.replace( "<![CDATA[", "" );
    newsource.replace( "]]>", "" );

    return newsource;
}


// Removes Sigil styles from the provided source
QString ExportEPUB::RemoveSigilStyles( const QString &style_source ) const
{
    // TODO: move this functionality to BookNormalization

    QString newsource = style_source;

    QRegExp chapter_break_style( "hr\\.sigilChapterBreak[^\\}]+\\}" );
    QRegExp sigil_comment( "/\\*SG.*SG\\*/" );

    sigil_comment.setMinimal( true );

    newsource.remove( chapter_break_style );
    newsource.remove( sigil_comment );

    return newsource;
}


// Updates the href attributes of all <a> tags
// to point to the files the ID's referenced are located in
void ExportEPUB::UpdateAnchors()
{
    QHash< QString, QString > id_locations = GetIDFileLocations();

    foreach( QString file, m_Folder.GetContentFilesList() )
    {
        if ( !file.contains( TEXT_FOLDER_NAME + "/" ) )

            continue;

        QString fullfilepath = m_Folder.GetFullPathToOEBPSFolder() + "/" + file;
        QString source = Utility::ReadUnicodeTextFile( fullfilepath );

        QDomDocument document;
        document.setContent( source );

        QDomNodeList anchors = document.elementsByTagName( "a" );

        for ( int i = 0; i < anchors.count(); ++i )
        {
            QDomElement element = anchors.at( i ).toElement();

            if (    element.hasAttribute( "href" ) &&
                    QUrl( element.attribute( "href" ) ).isRelative() &&
                    element.attribute( "href" ).contains( "#" )
               )
            {
                // Remove the '#' character
                QString id = element.attribute( "href" ).remove( 0, 1 );

                // If the ID is in a different file, update the link
                if ( id_locations[ id ] != file.remove( TEXT_FOLDER_NAME + "/" ) )

                    element.setAttribute( "href", id_locations[ id ] + "#" + id );            
            } 
        }

        source = CleanSource::Clean( XHTMLDoc::GetQDomNodeAsString( document ) );

        Utility::WriteUnicodeTextFile( source, fullfilepath );
    }
}


// Returns a hash with keys being ID or NAME attributes
// of XHTML elements and the values being the files in
// which these attribute values are located
QHash< QString, QString > ExportEPUB::GetIDFileLocations() const
{
    QHash< QString, QString > id_locations;

    foreach( QString file, m_Folder.GetContentFilesList() )
    {
        if ( !file.contains( TEXT_FOLDER_NAME + "/" ) )

            continue;

        QString fullfilepath = m_Folder.GetFullPathToOEBPSFolder() + "/" + file;
        QString source = Utility::ReadUnicodeTextFile( fullfilepath );

        QRegExp ids_names( ID_AND_NAME_ATTRIBUTE );
        ids_names.setCaseSensitivity( Qt::CaseInsensitive );

        int main_index = 0;

        while ( true )
        {
            main_index = source.indexOf( ids_names, main_index );

            if ( main_index == -1 )

                break;

            id_locations[ ids_names.cap( 1 ) ] = file.remove( TEXT_FOLDER_NAME + "/" );

            main_index += ids_names.matchedLength();
        }
    }

    return id_locations;
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






