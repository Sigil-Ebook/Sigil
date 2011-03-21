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
#include "BookManipulation/FolderKeeper.h"
#include "ResourceObjects/Resource.h"
#include "ResourceObjects/NCXResource.h"
#include "Misc/Utility.h"


const QStringList IMAGE_EXTENSIONS = QStringList() << "jpg"   << "jpeg"  << "png"
                                                   << "gif"   << "tif"   << "tiff"
                                                   << "bm"    << "bmp"   << "svg";

const QStringList FONT_EXTENSIONS         = QStringList() << "ttf"   << "ttc"   << "otf";
const QStringList TEXT_EXTENSIONS         = QStringList() << "xhtml" << "html"  << "htm" << "xml";
static const QStringList STYLE_EXTENSIONS = QStringList() << "css"   << "xpgt";

const QString IMAGE_FOLDER_NAME = "Images";
const QString FONT_FOLDER_NAME  = "Fonts";
const QString TEXT_FOLDER_NAME  = "Text";
const QString STYLE_FOLDER_NAME = "Styles";
const QString MISC_FOLDER_NAME  = "Misc";


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
    m_FullPathToMainFolder( m_TempFolder.GetPath() )
{
    CreateFolderStructure();
    CreateInfrastructureFiles();
}


FolderKeeper::~FolderKeeper()
{
    if ( m_FullPathToMainFolder.isEmpty() )

        return;

    foreach( Resource *resource, m_Resources.values() )
    {
        // We disconnect the Deleted signal, since if we don't
        // the OPF will try to update itself on every resource
        // removal (and there's no point to that, FolderKeeper is dying).
        // Disconnecting this speeds up FolderKeeper destruction.
        disconnect( resource, SIGNAL( Deleted( Resource* ) ), 
                    this,     SLOT( RemoveResource( Resource* ) ) );
        resource->Delete();
    } 
}


Resource& FolderKeeper::AddContentFileToFolder( const QString &fullfilepath, 
                                                int reading_order,
                                                bool update_opf )
{
    if ( !QFileInfo( fullfilepath ).exists() )

        boost_throw( FileDoesNotExist() << errinfo_file_name( fullfilepath.toStdString() ) );

    QString new_file_path;
    Resource *resource = NULL;

    // We need to lock here because otherwise
    // several threads can get the same "unique" name.
    // After we deal with the resource hash, other threads can continue.
    {
        QMutexLocker locker( &m_AccessMutex );

        QString filename  = GetUniqueFilenameVersion( QFileInfo( fullfilepath ).fileName() );
        QString extension = QFileInfo( fullfilepath ).suffix().toLower();
        QString relative_path;

        if ( IMAGE_EXTENSIONS.contains( extension ) )
        {
            new_file_path = m_FullPathToImagesFolder + "/" + filename;
            relative_path = IMAGE_FOLDER_NAME + "/" + filename;

            resource = new ImageResource( new_file_path );
        }

        else if ( FONT_EXTENSIONS.contains( extension ) )
        {
            new_file_path = m_FullPathToFontsFolder + "/" + filename;
            relative_path = FONT_FOLDER_NAME + "/" + filename;

            resource = new FontResource( new_file_path );
        }

        else if ( TEXT_EXTENSIONS.contains( extension ) )
        {
            new_file_path = m_FullPathToTextFolder + "/" + filename;
            relative_path = TEXT_FOLDER_NAME + "/" + filename;

            resource = new HTMLResource( new_file_path, reading_order, m_Resources );
        }

        else if ( STYLE_EXTENSIONS.contains( extension ) )
        {
            new_file_path = m_FullPathToStylesFolder + "/" + filename;
            relative_path = STYLE_FOLDER_NAME + "/" + filename;

            if ( extension == "css" )

                resource = new CSSResource( new_file_path );

            else

                resource = new XPGTResource( new_file_path );
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

    connect( resource, SIGNAL( Deleted( Resource* ) ), 
             this,     SLOT( RemoveResource( Resource* ) ), Qt::DirectConnection );
    connect( resource, SIGNAL( Renamed( Resource*, QString ) ), 
             m_OPF,    SLOT( ResourceRenamed( Resource*, QString ) ), Qt::DirectConnection );

    if ( update_opf )
    
        emit ResourceAdded( *resource );

    return *resource;
}


int FolderKeeper::GetHighestReadingOrder() const
{
    int count_of_html_resources = 0;

    foreach( Resource *resource, m_Resources.values() )
    {
        if ( resource->Type() == Resource::HTMLResource )
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


QStringList FolderKeeper::GetAllFilenames() const
{
    QStringList filelist;

    foreach( Resource* resource, m_Resources.values() )
    {
        filelist.append( resource->Filename() );
    }

    return filelist;
}


void FolderKeeper::RemoveResource( Resource *resource )
{
    Q_ASSERT( resource );

    m_Resources.remove( resource->GetIdentifier() );

    emit ResourceRemoved( *resource );
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
    connect( m_OPF, SIGNAL( Deleted( Resource* ) ), this, SLOT( RemoveResource( Resource* ) ) );
    connect( m_NCX, SIGNAL( Deleted( Resource* ) ), this, SLOT( RemoveResource( Resource* ) ) );

    connect( this, SIGNAL( ResourceAdded( const Resource& ) ),   m_OPF, SLOT( AddResource(    const Resource& ) ) );
    connect( this, SIGNAL( ResourceRemoved( const Resource& ) ), m_OPF, SLOT( RemoveResource( const Resource& ) ) );

    Utility::WriteUnicodeTextFile( CONTAINER_XML, m_FullPathToMetaInfFolder + "/container.xml" );
}






