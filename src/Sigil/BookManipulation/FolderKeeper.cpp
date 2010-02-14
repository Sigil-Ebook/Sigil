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

static const QStringList IMAGES = QStringList() << "jpg"   << "jpeg"  << "png"
                                                << "gif"   << "tif"   << "tiff"
                                                << "bm"    << "bmp"   << "svg";

static const QStringList FONTS  = QStringList() << "ttf"   << "otf";
static const QStringList TEXTS  = QStringList() << "xhtml" << "html"  << "htm" << "xml";
static const QStringList STYLES = QStringList() << "css"   << "xpgt";

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
    if ( newfilename == "container.xml" )
    
        QFile::copy( fullfilepath, m_FullPathToMetaInfFolder + "/" + newfilename );

    else if ( ( newfilename == "content.opf" ) || ( newfilename == "toc.ncx" ) )

        QFile::copy( fullfilepath, m_FullPathToOEBPSFolder + "/" + newfilename );

    else
    {
        Q_ASSERT( false );
    }
}

// Adds the given *content* file to the appropriate specific folder.
// The file is recognized according to its extension.
// The function returns the file's new path relative to the OEBPS folder.
QString FolderKeeper::AddContentFileToFolder( const QString &fullfilepath, int reading_order )
{
    // TODO: check if file with this filename exists,
    // and append a suffix if it does

    QString filename  = QFileInfo( fullfilepath ).fileName();
    QString extension = QFileInfo( fullfilepath ).suffix().toLower();

    QString new_file_path;
    QString relative_path;
    Resource *resource = NULL;

    if ( IMAGES.contains( extension ) )
    {
        new_file_path = m_FullPathToImagesFolder + "/" + filename;
        relative_path = IMAGE_FOLDER_NAME + "/" + filename;

        resource = new ImageResource( new_file_path, &m_Resources );
    }

    else if ( FONTS.contains( extension ) )
    {
        new_file_path = m_FullPathToFontsFolder + "/" + filename;
        relative_path = FONT_FOLDER_NAME + "/" + filename;

        resource = new FontResource( new_file_path, &m_Resources );
    }

    else if ( TEXTS.contains( extension ) )
    {
        new_file_path = m_FullPathToTextFolder + "/" + filename;
        relative_path = TEXT_FOLDER_NAME + "/" + filename;

        resource = new HTMLResource( new_file_path, &m_Resources, reading_order );
    }

    else if ( STYLES.contains( extension ) )
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
        new_file_path = fullfilepath, m_FullPathToMiscFolder + "/" + filename;
        relative_path = MISC_FOLDER_NAME + "/" + filename;

        resource = new Resource( new_file_path, &m_Resources );
    }

    QFile::copy( fullfilepath, new_file_path );

    m_Resources[ resource->GetIdentifier() ] = resource;

    return relative_path;
}

// Returns a list of all the content files in the directory
// with a path relative to the OEBPS directory
QStringList FolderKeeper::GetContentFilesList() const
{
    QStringList filelist;

    foreach( Resource* resource, m_Resources.values() )
    {
        filelist.append( resource->GetRelativePathToOEBPS() );
    }

    return filelist;
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


QList< Resource* > FolderKeeper::GetResourceList() const
{
    return m_Resources.values();
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



