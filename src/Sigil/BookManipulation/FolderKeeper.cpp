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

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QString>
#include <QtCore/QThread>
#include <QtCore/QTime>
#include <QtGui/QApplication>

#include "BookManipulation/FolderKeeper.h"
#include "sigil_constants.h"
#include "sigil_exception.h"
#include "ResourceObjects/NCXResource.h"
#include "ResourceObjects/Resource.h"
#include "Misc/Utility.h"
#include "Misc/OpenExternally.h"

const QStringList IMAGE_EXTENSIONS = QStringList() << "jpg"   << "jpeg"  << "png"
                                                   << "gif"   << "tif"   << "tiff"
                                                   << "bm"    << "bmp";
const QStringList SVG_EXTENSIONS = QStringList() << "svg";

const QStringList JPG_EXTENSIONS = QStringList()   << "jpg"   << "jpeg";
const QStringList TIFF_EXTENSIONS = QStringList()  << "tif"  << "tiff";

// Exception for non-standard Apple files in META-INF.
// container.xml and encryption.xml will be rewritten
// on export. Other files in this directory are passed
// through untouched.
const QRegExp FILE_EXCEPTIONS ( "META-INF|page-map" );

const QStringList FONT_EXTENSIONS         = QStringList() << "ttf"   << "ttc"   << "otf";
const QStringList TEXT_EXTENSIONS         = QStringList() << "xhtml" << "html"  << "htm" << "xml";
static const QStringList STYLE_EXTENSIONS = QStringList() << "css";

const QString IMAGE_FOLDER_NAME = "Images";
const QString FONT_FOLDER_NAME  = "Fonts";
const QString TEXT_FOLDER_NAME  = "Text";
const QString STYLE_FOLDER_NAME = "Styles";
const QString MISC_FOLDER_NAME  = "Misc";

const QStringList IMAGE_MIMEYPES = QStringList() << "image/gif" << "image/jpeg"
                                                 << "image/png";
const QStringList SVG_MIMETYPES = QStringList() << "image/svg+xml";
const QStringList TEXT_MIMETYPES = QStringList() << "application/xhtml+xml"
                                                 << "application/x-dtbook+xml"
                                                 << "application/xml";
const QStringList STYLE_MIMETYPES = QStringList() << "text/css";

static const QString CONTAINER_XML = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                                     "<container version=\"1.0\" xmlns=\"urn:oasis:names:tc:opendocument:xmlns:container\">\n"
                                     "    <rootfiles>\n"
                                     "        <rootfile full-path=\"OEBPS/content.opf\" media-type=\"application/oebps-package+xml\"/>\n"
                                     "   </rootfiles>\n"
                                     "</container>\n";


FolderKeeper::FolderKeeper( QObject *parent )
    : 
    QObject( parent ),
    m_OPF( NULL ),
    m_NCX( NULL ),
    m_FSWatcher( new QFileSystemWatcher() ),
    m_FullPathToMainFolder( m_TempFolder.GetPath() )
{
    CreateFolderStructure();
    CreateInfrastructureFiles();
}


FolderKeeper::~FolderKeeper()
{
    if ( m_FullPathToMainFolder.isEmpty() )

        return;

    delete m_FSWatcher;
    m_FSWatcher = 0;

    foreach( Resource *resource, m_Resources.values() )
    {
        // We disconnect the Deleted signal, since if we don't
        // the OPF will try to update itself on every resource
        // removal (and there's no point to that, FolderKeeper is dying).
        // Disconnecting this speeds up FolderKeeper destruction.
        disconnect( resource, SIGNAL( Deleted( const Resource& ) ), 
                    this,     SLOT( RemoveResource( const Resource& ) ) );
        resource->Delete();
    } 
}


// TODO: This code really needs to be rewritten so that the file-type is determined from the mimetype
// given in the OPF file, rather than relying on the file's extension.
Resource& FolderKeeper::AddContentFileToFolder( const QString &fullfilepath, 
                                                bool update_opf,
                                                const QString &mimetype )
{
    if ( !QFileInfo( fullfilepath ).exists() )

        boost_throw( FileDoesNotExist() << errinfo_file_name( fullfilepath.toStdString() ) );

    QString new_file_path;
    QString normalised_file_path = fullfilepath;
    Resource *resource = NULL;

    // Rename files that start with a '.'
    // These merely introduce needless difficulties
    QFileInfo fileInformation( normalised_file_path );
    QString fileName = fileInformation.fileName();
    if( fileName.left(1) == "." )
    {
        normalised_file_path = fileInformation.canonicalPath() % "/" % fileName.right( fileName.size() - 1 );
    }

    // We need to lock here because otherwise
    // several threads can get the same "unique" name.
    // After we deal with the resource hash, other threads can continue.
    {
        QMutexLocker locker( &m_AccessMutex );

        QString filename  = GetUniqueFilenameVersion( QFileInfo( normalised_file_path ).fileName() );
        QString extension = QFileInfo( normalised_file_path ).suffix().toLower();
        QString relative_path;

        if ( fullfilepath.contains( FILE_EXCEPTIONS ) )
        {
            if( filename == "page-map.xml" )
            {
                new_file_path = m_FullPathToMiscFolder + "/" + filename;

                resource = new XMLResource( new_file_path );
            }
            else 
            {
                // This is a big hack that assumes the new and old filepaths use root paths
                // of the same length. I can't see how to fix this without refactoring
                // a lot of the code to provide a more generalised interface.
                relative_path = fullfilepath.right( fullfilepath.size() - m_FullPathToMainFolder.size() );
                new_file_path = m_FullPathToMainFolder % relative_path;

                resource = new Resource( new_file_path );
            }
        }

        else if ( IMAGE_EXTENSIONS.contains( extension ) || IMAGE_MIMEYPES.contains( mimetype ) )
        {
            new_file_path = m_FullPathToImagesFolder + "/" + filename;
            relative_path = IMAGE_FOLDER_NAME + "/" + filename;

            resource = new ImageResource( new_file_path );
        }

        else if ( SVG_EXTENSIONS.contains( extension ) || SVG_MIMETYPES.contains( mimetype ) )
        {
            new_file_path = m_FullPathToTextFolder + "/" + filename;
            relative_path = IMAGE_FOLDER_NAME + "/" + filename;

            resource = new SVGResource( new_file_path );
        }

        else if ( FONT_EXTENSIONS.contains( extension ) )
        {
            new_file_path = m_FullPathToFontsFolder + "/" + filename;
            relative_path = FONT_FOLDER_NAME + "/" + filename;

            resource = new FontResource( new_file_path );
        }

        else if ( TEXT_EXTENSIONS.contains( extension ) || TEXT_MIMETYPES.contains( mimetype ) )
        {
            new_file_path = m_FullPathToTextFolder + "/" + filename;
            relative_path = TEXT_FOLDER_NAME + "/" + filename;

            resource = new HTMLResource( new_file_path, m_Resources );
        }

        else if ( STYLE_EXTENSIONS.contains( extension ) || STYLE_MIMETYPES.contains( mimetype ) )
        {
            new_file_path = m_FullPathToStylesFolder + "/" + filename;
            relative_path = STYLE_FOLDER_NAME + "/" + filename;

            if ( extension == "css" )

                resource = new CSSResource( new_file_path );
        }

        else
        {
            // Fallback mechanism
            new_file_path = m_FullPathToMiscFolder + "/" + filename;
            relative_path = MISC_FOLDER_NAME + "/" + filename;

            resource = new Resource( new_file_path );
        }    

        m_Resources[ resource->GetIdentifier() ] = resource;
    }

    QFile::copy( fullfilepath, new_file_path );

    if ( QThread::currentThread() != QApplication::instance()->thread() )
    
        resource->moveToThread( QApplication::instance()->thread() );

    connect( resource, SIGNAL( Deleted( const Resource& ) ), 
             this,     SLOT( RemoveResource( const Resource& ) ), Qt::DirectConnection );
    connect( resource, SIGNAL( Renamed( const Resource&, QString ) ), 
             this,     SLOT( ResourceRenamed( const Resource&, QString ) ), Qt::DirectConnection );

    if ( update_opf )
    
        emit ResourceAdded( *resource );

    return *resource;
}


int FolderKeeper::GetHighestReadingOrder() const
{
    int count_of_html_resources = 0;

    foreach( Resource *resource, m_Resources.values() )
    {
        if ( resource->Type() == Resource::HTMLResourceType )
        {
            ++count_of_html_resources;            
        }
    }

    return count_of_html_resources - 1;
}


QString FolderKeeper::GetUniqueFilenameVersion( const QString &filename ) const
{
    const QStringList &filenames = GetAllFilenames();

    if ( !filenames.contains( filename ) )

        return filename;

    // name_prefix is part of the name without the number suffix.
    // So for "Section0001.xhtml", it is "Section"
    QString name_prefix = QFileInfo( filename ).baseName().remove( QRegExp( "\\d+$" ) );
    QString extension   = QFileInfo( filename ).completeSuffix();
    
    // Used to search for the filename number suffixes.
    QString search_string = QRegExp::escape( name_prefix ).prepend( "^" ) + 
                            "(\\d*)" +
                            ( !extension.isEmpty() ? ( "\\." + QRegExp::escape( extension ) ) : QString() ) + 
                            "$";

    QRegExp filename_search( search_string );

    int max_num_length = -1;
    int max_num = -1;

    foreach( QString existing_file, filenames )
    {
        if ( existing_file.indexOf( filename_search ) == -1 )

            continue;
        
        bool conversion_successful = false; 
        int number_suffix = filename_search.cap( 1 ).toInt( &conversion_successful );

        if ( conversion_successful && number_suffix > max_num )
        {
            max_num = number_suffix;
            max_num_length = filename_search.cap( 1 ).length();
        }
    }

    if ( max_num == -1 )
    {
        max_num = 0;
        max_num_length = 4;
    }

    const int conversion_base = 10;

    QString new_name = name_prefix + QString( "%1" ).arg( max_num + 1, 
                                                          max_num_length, 
                                                          conversion_base, 
                                                          QChar( '0' ) );

    return new_name + ( !extension.isEmpty() ? ( "." + extension ) : QString() );
}


QStringList FolderKeeper::GetSortedContentFilesList() const
{
    QStringList filelist;

    foreach( Resource* resource, m_Resources.values() )
    {
        filelist.append( resource->GetRelativePathToOEBPS() );
    }
    
    filelist.sort();
    return filelist;
}


QList< Resource* > FolderKeeper::GetResourceList() const
{
    return m_Resources.values();
}


Resource& FolderKeeper::GetResourceByIdentifier( const QString &identifier ) const
{
    return *m_Resources[ identifier ];
}


Resource& FolderKeeper::GetResourceByFilename( const QString &filename ) const
{
    foreach( Resource *resource, m_Resources.values() )
    {
        if ( resource->Filename() == filename )

            return *resource;
    }

    boost_throw( ResourceDoesNotExist() << errinfo_resource_name( filename.toStdString() ) );
}


OPFResource& FolderKeeper::GetOPF() const
{
    return *m_OPF;
}


NCXResource& FolderKeeper::GetNCX() const
{
    return *m_NCX;
}


QString FolderKeeper::GetFullPathToMainFolder() const
{
    return m_FullPathToMainFolder;
}


QString FolderKeeper::GetFullPathToOEBPSFolder() const
{
    return m_FullPathToOEBPSFolder;
}


QString FolderKeeper::GetFullPathToTextFolder() const
{
    return m_FullPathToTextFolder;
}


QString FolderKeeper::GetFullPathToImageFolder() const
{
    return m_FullPathToImagesFolder;
}


QStringList FolderKeeper::GetAllFilenames() const
{
    QStringList filelist;

    foreach( Resource* resource, m_Resources.values() )
    {
        filelist.append( resource->Filename() );
    }

    return filelist;
}


void FolderKeeper::RemoveResource( const Resource& resource )
{
    m_Resources.remove( resource.GetIdentifier() );

    if (m_FSWatcher->files().contains( resource.GetFullPath() ) ) {
        m_FSWatcher->removePath( resource.GetFullPath() );
    }

    emit ResourceRemoved( resource );
}

void FolderKeeper::ResourceRenamed( const Resource& resource, const QString& old_full_path )
{
    m_OPF->ResourceRenamed( resource, old_full_path );
}

void FolderKeeper::ResourceFileChanged( const QString &path ) const
{
    // The file may have been deleted prior to writing a new version - give it a chance to write.
    QTime wake_time = QTime::currentTime().addMSecs(1000);
    while( !QFile::exists(path) && QTime::currentTime() < wake_time ) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    }

    // The signal is also received after resource files are removed / renamed,
    // but it can be safely ignored because QFileSystemWatcher automatically stops watching them.
    if ( QFile::exists(path) )
    {
        // Some editors write the updated contents to a temporary file
        // and then atomically move it over the watched file.
        // In this case QFileSystemWatcher loses track of the file, so we have to add it again.
        if ( !m_FSWatcher->files().contains(path) ) {
            m_FSWatcher->addPath( path );
        }

        foreach( Resource *resource, m_Resources.values() ) {
            if ( resource->GetFullPath() == path ) {
                resource->FileChangedOnDisk();
                return;
            }
        }
    }
}

void FolderKeeper::WatchResourceFile( const Resource& resource, bool file_renamed )
{
    if ( OpenExternally::mayOpen( resource.Type() ) )
    {
        if (!m_FSWatcher->files().contains( resource.GetFullPath() ) ) {
            m_FSWatcher->addPath( resource.GetFullPath() );
        }

        // when the file is changed externally, mark the owning Book as modified
        // parent() is the Book object
        connect( &resource,  SIGNAL( ResourceUpdatedFromDisk() ),
                 parent(),   SLOT( SetModified() ), Qt::UniqueConnection );
    }
}

// The required folder structure is this:
//	 META-INF
//	 OEBPS
//	    Images
//	    Fonts
//	    Text
//      Styles
//      Misc
void FolderKeeper::CreateFolderStructure()
{
    QDir folder( m_FullPathToMainFolder );

    folder.mkdir( "META-INF" );
    folder.mkdir( "OEBPS"    );

    folder.mkpath( "OEBPS/" + IMAGE_FOLDER_NAME );
    folder.mkpath( "OEBPS/" + FONT_FOLDER_NAME  );
    folder.mkpath( "OEBPS/" + TEXT_FOLDER_NAME  );
    folder.mkpath( "OEBPS/" + STYLE_FOLDER_NAME );
    folder.mkpath( "OEBPS/" + MISC_FOLDER_NAME  );

    m_FullPathToMetaInfFolder = m_FullPathToMainFolder + "/META-INF";
    m_FullPathToOEBPSFolder   = m_FullPathToMainFolder + "/OEBPS";

    m_FullPathToImagesFolder  = m_FullPathToOEBPSFolder + "/" + IMAGE_FOLDER_NAME;
    m_FullPathToFontsFolder   = m_FullPathToOEBPSFolder + "/" + FONT_FOLDER_NAME;
    m_FullPathToTextFolder    = m_FullPathToOEBPSFolder + "/" + TEXT_FOLDER_NAME;
    m_FullPathToStylesFolder  = m_FullPathToOEBPSFolder + "/" + STYLE_FOLDER_NAME;
    m_FullPathToMiscFolder    = m_FullPathToOEBPSFolder + "/" + MISC_FOLDER_NAME;
}


void FolderKeeper::CreateInfrastructureFiles()
{
    m_OPF = new OPFResource( m_FullPathToOEBPSFolder + "/" + OPF_FILE_NAME, this );
    m_NCX = new NCXResource( m_FullPathToOEBPSFolder + "/" + NCX_FILE_NAME, this );

    m_NCX->SetMainID( m_OPF->GetMainIdentifierValue() );

    m_Resources[ m_OPF->GetIdentifier() ] = m_OPF;
    m_Resources[ m_NCX->GetIdentifier() ] = m_NCX;

    // TODO: change from Resource* to const Resource&
    connect( m_OPF, SIGNAL( Deleted( const Resource& ) ), this, SLOT( RemoveResource( const Resource& ) ) );
    connect( m_NCX, SIGNAL( Deleted( const Resource& ) ), this, SLOT( RemoveResource( const Resource& ) ) );

    // For ResourceAdded, the connection has to be DirectConnection,
    // otherwise the default of AutoConnection screws us when 
    // AddContentFileToFolder is called from multiple threads.
    connect( this,  SIGNAL( ResourceAdded( const Resource& ) ),   
             m_OPF, SLOT( AddResource(     const Resource& ) ), Qt::DirectConnection );
    connect( this,  SIGNAL( ResourceRemoved( const Resource& ) ), 
             m_OPF, SLOT( RemoveResource(    const Resource& ) ) );

    connect( m_FSWatcher, SIGNAL( fileChanged( const QString& ) ),
             this,        SLOT( ResourceFileChanged( const QString& )), Qt::DirectConnection );

    Utility::WriteUnicodeTextFile( CONTAINER_XML, m_FullPathToMetaInfFolder + "/container.xml" );
}
