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

#include <unzip.h>
#ifdef _WIN32
#include <iowin32.h>
#endif

#include <QtCore/QtCore>
#include <QtCore/QFileInfo>
#include <QtCore/QFutureSynchronizer>
#include <QtGui/QMessageBox>
#include <QtGui/QTextDocument>
#include <QtXml/QXmlStreamReader>

#include "BookManipulation/FolderKeeper.h"
#include "Importers/ImportOEBPS.h"
#include "Misc/Utility.h"
#include "ResourceObjects/OPFResource.h"
#include "ResourceObjects/NCXResource.h"
#include "ResourceObjects/Resource.h"
#include "sigil_constants.h"
#include "sigil_exception.h"

#ifndef MAX_PATH
// Set Max length to 256 because that's the max path size on many systems.
#define MAX_PATH 256
#endif
// This is the same read buffer size used by Java and Perl.
#define BUFF_SIZE 8192

using boost::make_tuple;

const QString DUBLIN_CORE_NS             = "http://purl.org/dc/elements/1.1/";
static const QString OEBPS_MIMETYPE      = "application/oebps-package+xml";
static const QString UPDATE_ERROR_STRING = "SG_ERROR";
const QString NCX_MIMETYPE               = "application/x-dtbncx+xml";
static const QString NCX_EXTENSION       = "ncx";

ImportOEBPS::ImportOEBPS( const QString &fullfilepath )
    :
    Importer( fullfilepath ),
    m_ExtractedFolderPath( m_TempFolder.GetPath() ),
    m_HasSpineItems(false),
    m_NCXNotInManifest(false)
{

}


void ImportOEBPS::ExtractContainer()
{
    int res = 0;
#ifdef Q_OS_WIN32
    zlib_filefunc64_def ffunc;
    fill_win32_filefunc64W(&ffunc);
    unzFile zfile = unzOpen2_64(QDir::toNativeSeparators(m_FullFilePath).toStdWString().c_str(), &ffunc);
#else
    unzFile zfile = unzOpen64(QDir::toNativeSeparators(m_FullFilePath).toUtf8().constData());
#endif

    if (zfile == NULL) {
        boost_throw(EPUBLoadParseError() << errinfo_epub_load_parse_errors( QString(QObject::tr("Cannot open EPUB: %1")).arg(QDir::toNativeSeparators(m_FullFilePath)).toStdString() ) );
    }

    res = unzGoToFirstFile(zfile);
    if (res == UNZ_OK) {
        do {
            // Get the name of the file in the archive.
            char file_name[MAX_PATH] = {0};
            unz_file_info64 file_info;
            unzGetCurrentFileInfo64(zfile, &file_info, file_name, MAX_PATH, NULL, 0, NULL, 0);
            QString qfile_name = QString::fromUtf8(file_name);

            // If there is no file name then we can't do anything with it.
            if (!qfile_name.isEmpty()) {
                // We use the dir object to create the path in the temporary directory.
                // Unfortunately, we need a dir ojbect to do this as it's not a static function.
                QDir dir(m_ExtractedFolderPath);
                // Full file path in the temporary directory.
                QString file_path = m_ExtractedFolderPath + "/" + qfile_name;
                QFileInfo qfile_info(file_path);

                // Is this entry a directory?
                if (file_info.uncompressed_size == 0 && qfile_name.endsWith('/')) {
                    dir.mkpath(qfile_name);
                    continue;
                } else {
                    dir.mkpath(qfile_info.path());
                }

                // Open the file entry in the archive for reading.
                if (unzOpenCurrentFile(zfile) != UNZ_OK) {
                    unzClose(zfile);
                    boost_throw(EPUBLoadParseError() << errinfo_epub_load_parse_errors( QString(QObject::tr("Cannot extract file: %1")).arg(qfile_name).toStdString() ) );
                }

                // Open the file on disk to write the entry in the archive to.
                QFile entry(file_path);
                if (!entry.open(QIODevice::WriteOnly|QIODevice::Truncate)) {
                    unzCloseCurrentFile(zfile);
                    unzClose(zfile);
                    boost_throw(EPUBLoadParseError() << errinfo_epub_load_parse_errors( QString(QObject::tr("Cannot extract file: %1")).arg(qfile_name).toStdString() ) );
                }

                // Buffered reading and writing.
                char buff[BUFF_SIZE] = {0};
                int read = 0;
                while ((read = unzReadCurrentFile(zfile, buff, BUFF_SIZE)) > 0) {
                    entry.write(buff, read);
                }
                entry.close();
                // Read errors are marked by a negative read amount.
                if (read < 0) {
                    unzCloseCurrentFile(zfile);
                    unzClose(zfile);
                    boost_throw(EPUBLoadParseError() << errinfo_epub_load_parse_errors( QString(QObject::tr("Cannot extract file: %1")).arg(qfile_name).toStdString() ) );
                }

                // The file was read but the CRC did not match.
                // We don't check the read file size vs the uncompressed file size
                // because if they're different there should be a CRC error.
                if (unzCloseCurrentFile(zfile) == UNZ_CRCERROR) {
                    unzClose(zfile);
                    boost_throw(EPUBLoadParseError() << errinfo_epub_load_parse_errors( QString(QObject::tr("Cannot extract file: %1")).arg(qfile_name).toStdString() ) );
                }
            }
        } while ((res = unzGoToNextFile(zfile)) == UNZ_OK);
    }

    if (res != UNZ_END_OF_LIST_OF_FILE) {
        unzClose(zfile);
        boost_throw(EPUBLoadParseError() << errinfo_epub_load_parse_errors( QString(QObject::tr("Cannot open EPUB: %1")).arg(QDir::toNativeSeparators(m_FullFilePath)).toStdString() ) );
    }

    unzClose(zfile);
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
        const QString error = QString(
            QObject::tr("Unable to parse container.xml file.\nLine: %1 Column %2 - %3"))
                    .arg(container.lineNumber())
                    .arg(container.columnNumber())
                    .arg(container.errorString());
        boost_throw( EPUBLoadParseError() << errinfo_epub_load_parse_errors( error.toStdString() ) );
    }

    if ( m_OPFFilePath.isEmpty() || !QFile::exists(m_OPFFilePath) )
    {
        boost_throw( EPUBLoadParseError() 
            << errinfo_epub_load_parse_errors( QString(QObject::tr("No appropriate OPF file found")).toStdString() ) );
    }
}


void ImportOEBPS::ReadOPF()
{
    QString opf_text = PrepareOPFForReading( Utility::ReadUnicodeTextFile( m_OPFFilePath ) );
    QXmlStreamReader opf_reader( opf_text );

    QString ncx_id_on_spine;
    while ( !opf_reader.atEnd() )
    {
        opf_reader.readNext();

        if ( !opf_reader.isStartElement() ) {
            continue;
        }

        if ( opf_reader.name() == "package" ) {
            m_UniqueIdentifierId = opf_reader.attributes().value( "", "unique-identifier" ).toString();
        }
        else if ( opf_reader.name() == "identifier" ) {
            ReadIdentifierElement( opf_reader );
        }
        // Get the list of content files that
        // make up the publication
        else if ( opf_reader.name() == "item" ) {
            ReadManifestItemElement( opf_reader );
        }
        // We read this just to get the NCX id
        else if ( opf_reader.name() == "spine" ) {
            ncx_id_on_spine = opf_reader.attributes().value( "", "toc" ).toString();
        }
        else if ( opf_reader.name() == "itemref" ) {
            m_HasSpineItems = true;
        }
    }

    if ( opf_reader.hasError() ) {
        const QString error = QString(QObject::tr("Unable to read OPF file.\nLine: %1 Column %2 - %3"))
                                    .arg(opf_reader.lineNumber())
                                    .arg(opf_reader.columnNumber())
                                    .arg(opf_reader.errorString());
        boost_throw( EPUBLoadParseError() << errinfo_epub_load_parse_errors( error.toStdString() ) );
    }

    // Ensure we have an NCX available
    LocateOrCreateNCX(ncx_id_on_spine);
}


void ImportOEBPS::ReadIdentifierElement( QXmlStreamReader &opf_reader )
{
    QString id     = opf_reader.attributes().value( "", "id"     ).toString(); 
    QString scheme = opf_reader.attributes().value( "", "scheme" ).toString();
    QString value  = opf_reader.readElementText();

    if ( id == m_UniqueIdentifierId )

        m_UniqueIdentifierValue = value;

    if ( m_UuidIdentifierValue.isEmpty() &&
         ( value.contains( "urn:uuid:" ) || scheme.toLower() == "uuid" ) )
    {
        m_UuidIdentifierValue = value;
    }            
}


void ImportOEBPS::ReadManifestItemElement( QXmlStreamReader &opf_reader )
{
    QString id   = opf_reader.attributes().value( "", "id"         ).toString(); 
    QString href = opf_reader.attributes().value( "", "href"       ).toString();
    QString type = opf_reader.attributes().value( "", "media-type" ).toString();

    // Paths are percent encoded in the OPF, we use "normal" paths internally.
    href = Utility::URLDecodePath( href );
    QString extension = QFileInfo( href ).suffix().toLower();

    if ( type != NCX_MIMETYPE && extension != NCX_EXTENSION )         
    {                    
        if ( !m_MainfestFilePaths.contains( href ) )
        {
            m_Files[ id ] = href;
            m_FileMimetypes[ id ] = type;
            m_MainfestFilePaths << href;
        }
    }

    else
    {
        m_NcxCandidates[ id ] = href;
    }
}


void ImportOEBPS::LocateOrCreateNCX( const QString &ncx_id_on_spine )
{
    QString load_warning;
    QString ncx_href = "not_found";
    m_NCXId = ncx_id_on_spine;

    // Handle various failure conditions, such as:
    // - ncx not specified in the spine (search for a matching manifest item by extension)
    // - ncx specified in spine, but no matching manifest item entry (create a new one)
    // - ncx file not physically present (create a new one)
    // - ncx not in spine or manifest item (create a new one)
    if ( !m_NCXId.isEmpty() ) {
        ncx_href = m_NcxCandidates[ m_NCXId ];
    }
    else {
        // Search for the ncx in the manifest by looking for files with
        // a .ncx extension.
        QHashIterator< QString, QString > ncxSearch( m_NcxCandidates );
        while( ncxSearch.hasNext() )
        {
            ncxSearch.next();
            if( QFileInfo( ncxSearch.value() ).suffix().toLower() == NCX_EXTENSION )
            {
                m_NCXId = ncxSearch.key();

                load_warning = "<p>" % QObject::tr("The OPF file did not identify the NCX file correctly.") % "</p>" %
                               "<p>- " % QObject::tr("Sigil has used the following file as the NCX:") % 
                               QString(" <b>%1</b></p>").arg(m_NcxCandidates[ m_NCXId ]);

                ncx_href = m_NcxCandidates[ m_NCXId ];
                break;
            }
        }
    }

    m_NCXFilePath = QFileInfo( m_OPFFilePath ).absolutePath() % "/" % ncx_href;

    if (ncx_href.isEmpty() || !QFile::exists(m_NCXFilePath)) {
        m_NCXNotInManifest = m_NCXId.isEmpty() || ncx_href.isEmpty();
        m_NCXId.clear();

        // Things are really bad and no .ncx file was found in the manifest or
        // the file does not physically exist.  We need to create a new one.
        m_NCXFilePath = QFileInfo( m_OPFFilePath ).absolutePath() % "/" % NCX_FILE_NAME;

        // Create a new file for the NCX.
        NCXResource ncx_resource( m_NCXFilePath, &m_Book->GetFolderKeeper() );
        // We are relying on an identifier being set from the metadata. 
        // It might not have one if the book does not have the urn:uuid: format.
        if (!m_UuidIdentifierValue.isEmpty()) {
            ncx_resource.SetMainID( m_UuidIdentifierValue );
        }
        ncx_resource.SaveToDisk();
        
        if (ncx_href.isEmpty()) {
            load_warning = "<p>" % QObject::tr("The OPF file does not contain an NCX file.") % "</p>" %
                           "<p>- " % QObject::tr("Sigil has created a new one for you.") % "</p>";
        }
        else {
            load_warning = "<p>" % QObject::tr("The NCX file is not present in this EPUB.") % "</p>" %
                           "<p>- " % QObject::tr("Sigil has created a new one for you.") % "</p>";
        }
    }

    if (!load_warning.isEmpty()) {
        AddLoadWarning(load_warning);
    }
}


void ImportOEBPS::LoadInfrastructureFiles()
{
    m_Book->GetOPF().SetText( PrepareOPFForReading( Utility::ReadUnicodeTextFile( m_OPFFilePath ) ) );
    m_Book->GetNCX().SetText( Utility::ReadUnicodeTextFile( m_NCXFilePath ) );
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
                m_FileMimetypes.value( id ) ) );
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
                                                    const QString &mimetype )
{
    QString fullfilepath = QFileInfo( m_OPFFilePath ).absolutePath() + "/" + path;

    try
    {
        Resource &resource = m_Book->GetFolderKeeper().AddContentFileToFolder( fullfilepath, false, mimetype );
        QString newpath = "../" + resource.GetRelativePathToOEBPS(); 

        return make_tuple( fullfilepath, newpath );
    }
    
    catch ( FileDoesNotExist& )
    {
    	return make_tuple( UPDATE_ERROR_STRING, UPDATE_ERROR_STRING );
    }
}


QString ImportOEBPS::PrepareOPFForReading( const QString &source )
{
    QString source_copy( source );
    QString prefix = source_copy.left( XML_DECLARATION_SEARCH_PREFIX_SIZE );
    QRegExp version( VERSION_ATTRIBUTE );
    prefix.indexOf( version );

    // MASSIVE hack for XML 1.1 "support";
    // this is only for people who specify
    // XML 1.1 when they actually only use XML 1.0
    source_copy.replace( version.pos(), version.matchedLength(), "version=\"1.0\"" );

    return source_copy;
}


