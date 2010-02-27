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
#include "../BookManipulation/FolderKeeper.h"
#include "../ResourceObjects/HTMLResource.h"
#include "../ResourceObjects/ImageResource.h"
#include "../ResourceObjects/CSSResource.h"
#include "../ResourceObjects/XPGTResource.h"
#include "../ResourceObjects/FontResource.h"
#include "../ResourceObjects/Resource.h"
#include "../Misc/Utility.h"

static const QStringList IMAGE_EXTENSIONS = QStringList() << "jpg"   << "jpeg"  << "png"
                                                          << "gif"   << "tif"   << "tiff"
                                                          << "bm"    << "bmp"   << "svg";

static const QStringList FONT_EXTENSIONS  = QStringList() << "ttf"   << "otf";
const QStringList TEXT_EXTENSIONS         = QStringList() << "xhtml" << "html"  << "htm" << "xml";
static const QStringList STYLE_EXTENSIONS = QStringList() << "css"   << "xpgt";

const QString IMAGE_FOLDER_NAME = "Images";
const QString FONT_FOLDER_NAME  = "Fonts";
const QString TEXT_FOLDER_NAME  = "Text";
const QString STYLE_FOLDER_NAME = "Styles";
const QString MISC_FOLDER_NAME  = "Misc";

typedef boost::error_info< struct resource_name, std::string > errinfo_resource_name;


// Constructor
FolderKeeper::FolderKeeper()
{
    Initialize();
}


// Copy constructor
FolderKeeper::FolderKeeper( const FolderKeeper& other )
{
    Initialize();

    CopyFiles( other );
}


// Assignment operator
FolderKeeper& FolderKeeper::operator= ( const FolderKeeper& other )
{
    // Protect against invalid self-assignment
    if ( this != &other ) 
    {
        QString path_to_old = m_FullPathToMainFolder;

        Initialize();

        DeleteAllResources( path_to_old );        

        CopyFiles( other );        
    }

    // By convention, always return *this
    return *this;
}


// Destructor
FolderKeeper::~FolderKeeper()
{
    if ( m_FullPathToMainFolder.isEmpty() == false )

        DeleteAllResources( m_FullPathToMainFolder );           
}


// A dispatcher function that routes the given *infrastructure* file
// to the appropriate specific folder function
void FolderKeeper::AddInfraFileToFolder( const QString &fullfilepath, const QString &newfilename  )
{
    if ( newfilename == CONTAINER_XML_FILE_NAME )
    
        QFile::copy( fullfilepath, m_FullPathToMetaInfFolder + "/" + newfilename );

    else if ( ( newfilename == OPF_FILE_NAME ) || ( newfilename == NCX_FILE_NAME ) )

        QFile::copy( fullfilepath, m_FullPathToOEBPSFolder + "/" + newfilename );

    else
    {
        Q_ASSERT( false );
    }
}


Resource& FolderKeeper::AddContentFileToFolder( const QString &fullfilepath, int reading_order )
{
    // We need to lock at the start of the func
    // because otherwise several threads can get
    // the same "unique" name.
    m_AccessMutex.lock();

    QString filename  = GetUniqueFilenameVersion( QFileInfo( fullfilepath ).fileName() );
    QString extension = QFileInfo( fullfilepath ).suffix().toLower();

    QString new_file_path;
    QString relative_path;

    Resource *resource = NULL;

    if ( IMAGE_EXTENSIONS.contains( extension ) )
    {
        new_file_path = m_FullPathToImagesFolder + "/" + filename;
        relative_path = IMAGE_FOLDER_NAME + "/" + filename;

        resource = new ImageResource( new_file_path, &m_Resources );
    }

    else if ( FONT_EXTENSIONS.contains( extension ) )
    {
        new_file_path = m_FullPathToFontsFolder + "/" + filename;
        relative_path = FONT_FOLDER_NAME + "/" + filename;

        resource = new FontResource( new_file_path, &m_Resources );
    }

    else if ( TEXT_EXTENSIONS.contains( extension ) )
    {
        new_file_path = m_FullPathToTextFolder + "/" + filename;
        relative_path = TEXT_FOLDER_NAME + "/" + filename;

        resource = new HTMLResource( new_file_path, &m_Resources, reading_order );
    }

    else if ( STYLE_EXTENSIONS.contains( extension ) )
    {
        new_file_path = m_FullPathToStylesFolder + "/" + filename;
        relative_path = STYLE_FOLDER_NAME + "/" + filename;

        if ( extension == "css" )

            resource = new CSSResource( new_file_path, &m_Resources );

        else

            resource = new XPGTResource( new_file_path, &m_Resources );
    }

    else
    {
        // Fallback mechanism
        new_file_path = m_FullPathToMiscFolder + "/" + filename;
        relative_path = MISC_FOLDER_NAME + "/" + filename;

        resource = new Resource( new_file_path, &m_Resources );
    }    

    m_Resources[ resource->GetIdentifier() ] = resource;

    // After we deal with the resource hash, other threads can continue.
    m_AccessMutex.unlock();

    QFile::copy( fullfilepath, new_file_path );

    if ( QThread::currentThread() != QApplication::instance()->thread() )

        resource->moveToThread( QApplication::instance()->thread() );

    return *resource;
}


int FolderKeeper::GetHighestReadingOrder() const
{
    int highest_reading_order = -1;

    foreach( Resource *resource, m_Resources.values() )
    {
        if ( resource->Type() == Resource::HTMLResource )
        {
            HTMLResource* html_resource = qobject_cast< HTMLResource* >( resource );

            Q_ASSERT( html_resource );

            int reading_order = html_resource->GetReadingOrder();

            if ( reading_order > highest_reading_order )

                highest_reading_order = reading_order;
        }
    }

    return highest_reading_order;
}


QString FolderKeeper::GetUniqueFilenameVersion( const QString &filename ) const
{
    const QStringList &filenames = GetAllFilenames();

    if ( !filenames.contains( filename ) )

        return filename;

    QString name_prefix = QFileInfo( filename ).baseName().remove( QRegExp( "\\d+$" ) );
    QString extension   = QFileInfo( filename ).completeSuffix();
    
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


// Returns a list of all the content files in the directory
// with a path relative to the OEBPS directory
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


// Returned in reading order
QList< HTMLResource* > FolderKeeper::GetSortedHTMLResources() const
{
    QList< HTMLResource* > html_resources;

    foreach( Resource *resource, m_Resources.values() )
    {
        if ( resource->Type() == Resource::HTMLResource )

            html_resources.append( qobject_cast< HTMLResource* >( resource ) );
    }

    qSort( html_resources.begin(), html_resources.end(), HTMLResource::LessThan );

    return html_resources;
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


// Returns the full path to the main folder of the publication
QString FolderKeeper::GetFullPathToMainFolder() const
{
    return m_FullPathToMainFolder;
}


// Returns the full path to the OEBPS folder of the publication
QString FolderKeeper::GetFullPathToOEBPSFolder() const
{
    return m_FullPathToOEBPSFolder;
}


// Returns the full path to the OEBPS/text folder of the publication
QString FolderKeeper::GetFullPathToTextFolder() const
{
    return m_FullPathToTextFolder;
}


// Performs common constructor duties
// for all constructors
void FolderKeeper::Initialize()
{
    QDir folder( Utility::GetNewTempFolderPath() );
    folder.mkpath( folder.absolutePath() );

    m_FullPathToMainFolder = folder.absolutePath();

    CreateFolderStructure();
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


void FolderKeeper::CopyFiles( const FolderKeeper &other )
{
    foreach( Resource *resource, other.m_Resources.values() )
    {
        QString filepath( other.GetFullPathToOEBPSFolder() + "/" + resource->GetRelativePathToOEBPS() );

        int reading_order = -1;

        if ( resource->Type() == Resource::HTMLResource )

            reading_order = qobject_cast< HTMLResource* >( resource )->GetReadingOrder();

        AddContentFileToFolder( filepath, reading_order );
    }
}


void FolderKeeper::DeleteAllResources( const QString &folderpath )
{
    foreach( Resource *resource, m_Resources.values() )
    {
        resource->Delete();
    }

    m_Resources.clear();

    QtConcurrent::run( Utility::DeleteFolderAndFiles, folderpath );
}


// Creates the required folder structure:
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


