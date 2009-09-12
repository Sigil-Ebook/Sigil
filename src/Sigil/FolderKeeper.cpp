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

#include "stdafx.h"
#include "FolderKeeper.h"
#include "Utility.h"

// Constructor
FolderKeeper::FolderKeeper()
{
    Initialize();	
}


// Copy constructor
FolderKeeper::FolderKeeper( const FolderKeeper& other )
{
    Initialize();
    
    Utility::CopyFiles( other.FullPathToMainFolder, FullPathToMainFolder );
}


// Assignment operator
FolderKeeper& FolderKeeper::operator = ( const FolderKeeper& other )
{
    // Protect against invalid self-assignment
    if ( this != &other ) 
    {
        QString path_to_old = FullPathToMainFolder;

        Initialize();

        Utility::DeleteFolderAndFiles( path_to_old );

        Utility::CopyFiles( other.FullPathToMainFolder, FullPathToMainFolder );
    }

    // By convention, always return *this
    return *this;
}


// Destructor
FolderKeeper::~FolderKeeper()
{
    if ( FullPathToMainFolder.isEmpty() == false )

        Utility::DeleteFolderAndFiles( FullPathToMainFolder );
}

// A dispatcher function that routes the given *infrastructure* file
// to the appropriate specific folder function
void FolderKeeper::AddInfraFileToFolder( const QString &fullfilepath, const QString &newfilename  )
{
    if ( newfilename == "container.xml" )
    
        AddFileToMetaInfFolder( fullfilepath, newfilename );

    else if ( ( newfilename == "content.opf" ) || ( newfilename == "toc.ncx" ) )

        AddFileToOEBPSFolder( fullfilepath, newfilename );

    else
    {
        // FIXME: throw exception
    }
}

// A dispatcher function that routes the given *content* file
// to the appropriate specific folder function.
// The file is recognized according to its extension: use force_extension
// to override the recognition and force an extension.
// Set preserve_filename to true to preserve the original filename.
// The function returns the new file's path relative to the OEBPS folder.
QString FolderKeeper::AddContentFileToFolder( const QString &fullfilepath, 
                                              const QString &force_extension,
                                              const bool preserve_filename    )
{
    QList< QString > images;
    images  << "jpg"    << "jpeg"   << "png"
            << "gif"    << "tif"    << "tiff"
            << "bm"     << "bmp"    << "svg";

    QList< QString > fonts;
    fonts << "ttf" << "otf";

    QList< QString > texts;
    texts << "xhtml" << "html" << "htm";

    QList< QString > styles;
    styles << "css" << "xpgt";

    QString extension = "";

    if ( force_extension.isNull() )
    {
        extension = QFileInfo( fullfilepath ).suffix().toLower();
    }      

    else
    {
        // If the caller specified for instance ".txt",
        // it is converted to "txt" (we remove the dot)
        if ( force_extension[ 0 ] == QChar( '.' ) )

            extension = Utility::Substring( 1, force_extension.length(), force_extension ).toLower();

        else

            extension = force_extension.toLower();;
    }        

    if ( images.contains( extension ) )

        return "images/" + AddFileToImagesFolder( fullfilepath, extension, preserve_filename );

    if ( fonts.contains( extension ) )
    
        return "fonts/" + AddFileToFontsFolder( fullfilepath, extension, preserve_filename );

    if ( texts.contains( extension ) )

        return "text/" + AddFileToTextFolder( fullfilepath, extension, preserve_filename );

    if ( styles.contains( extension ) )

        return "styles/" + AddFileToStylesFolder( fullfilepath, extension, preserve_filename );

    // Fallback mechanism
    return "misc/" + AddFileToMiscFolder( fullfilepath, extension, preserve_filename );
}


// Returns a list of all the content files in the directory
// with a path relative to the OEBPS directory
QStringList FolderKeeper::GetContentFilesList() const
{
    QStringList filelist;

    QDir images(    FullPathToImagesFolder  );
    QDir fonts(     FullPathToFontsFolder   );
    QDir texts(     FullPathToTextFolder    );
    QDir styles(    FullPathToStylesFolder  );
    QDir misc(      FullPathToMiscFolder    );

    foreach( QString file, images.entryList() )
    {
        if ( ( file != "." ) && ( file != ".." ) )

            filelist.append( "images/" + file );
    }

    foreach( QString file, fonts.entryList() )
    {
        if ( ( file != "." ) && ( file != ".." ) )

            filelist.append( "fonts/" + file );
    }

    foreach( QString file, texts.entryList() )
    {
        if ( ( file != "." ) && ( file != ".." ) )

            filelist.append( "text/" + file );
    }

    foreach( QString file, styles.entryList() )
    {
        if ( ( file != "." ) && ( file != ".." ) )

            filelist.append( "styles/" + file );
    }

    foreach( QString file, misc.entryList() )
    {
        if ( ( file != "." ) && ( file != ".." ) )

            filelist.append( "misc/" + file );
    }

    return filelist;
}


// Returns the full path to the main folder of the publication
QString FolderKeeper::GetFullPathToMainFolder() const
{
    return FullPathToMainFolder;
}


// Returns the full path to the OEBPS folder of the publication
QString FolderKeeper::GetFullPathToOEBPSFolder() const
{
    return FullPathToOEBPSFolder;
}


// Returns the full path to the OEBPS/text folder of the publication
QString FolderKeeper::GetFullPathToTextFolder() const
{
    return FullPathToTextFolder;
}


// Copies the file specified with fullfilepath
// to the META-INF folder with the name newfilename
void FolderKeeper::AddFileToMetaInfFolder( const QString &fullfilepath, const QString &newfilename )
{
    QFile::copy( fullfilepath, FullPathToMetaInfFolder + "/" + newfilename );	
}


// Copies the file specified with fullfilepath
// to the OEBPS folder with the name newfilename
void FolderKeeper::AddFileToOEBPSFolder( const QString &fullfilepath, const QString &newfilename )
{
    QFile::copy( fullfilepath, FullPathToOEBPSFolder + "/" + newfilename );	
}


// Copies the file specified with fullfilepath
// to the OEBPS/images folder with a generated name;
// the generated name is returned
QString FolderKeeper::AddFileToImagesFolder(    const QString &fullfilepath, 
                                                const QString &extension, 
                                                const bool preserve_filename )
{
    QString filename;    

    if ( preserve_filename )
    {
        filename = QFileInfo( fullfilepath ).fileName();
    }

    else
    {
        // Count - 2 because we don't care for the "." and ".." folders
        // + 1 because we are adding a file
        int index = QDir( FullPathToImagesFolder ).count() - 2 + 1;

        // We add a number to the file, so e.g. we get "img0001.png", "img0035.jpg" etc.
        filename = QString( "img" ) + QString( "%1" ).arg( index, 4, 10, QChar( '0' ) ) + "." + extension;
    }

    QFile::copy( fullfilepath, FullPathToImagesFolder + "/" + filename );

    return filename;
}


// Copies the file specified with fullfilepath
// to the OEBPS/images folder with a generated name;
// the generated name is returned
QString FolderKeeper::AddFileToFontsFolder(    const QString &fullfilepath, 
                                               const QString &extension, 
                                               const bool preserve_filename )
{
    QString filename;    

    if ( preserve_filename )
    {
        filename = QFileInfo( fullfilepath ).fileName();
    }

    else
    {
        // Count - 2 because we don't care for the "." and ".." folders
        // + 1 because we are adding a file
        int index = QDir( FullPathToFontsFolder ).count() - 2 + 1;

        // We add a number to the file, so e.g. we get "font001.otf", "font035.ttf" etc.
        filename = QString( "font" ) + QString( "%1" ).arg( index, 3, 10, QChar( '0' ) ) + "." + extension;
    }

    QFile::copy( fullfilepath, FullPathToFontsFolder + "/" + filename );

    return filename;
}


// Copies the file specified with fullfilepath
// to the OEBPS/text folder with a generated name;
// the generated name is returned
QString FolderKeeper::AddFileToTextFolder(    const QString &fullfilepath, 
                                              const QString &extension, 
                                              const bool preserve_filename )
{
    QString filename;    

    if ( preserve_filename )
    {
        filename = QFileInfo( fullfilepath ).fileName();
    }

    else
    {
        // Count - 2 because we don't care for the "." and ".." folders
        // + 1 because we are adding a file
        int index = QDir( FullPathToTextFolder ).count() - 2 + 1;

        // We add a number to the file, so e.g. we get "content001.xhtml", "content035.xhtml" etc.
        filename = QString( "content" ) + QString( "%1" ).arg( index, 3, 10, QChar( '0' ) ) + "." + extension;
    }

    QFile::copy( fullfilepath, FullPathToTextFolder + "/" + filename );

    return filename;
}

// Copies the file specified with fullfilepath
// to the OEBPS/styles folder with a generated name;
// the generated name is returned
QString FolderKeeper::AddFileToStylesFolder(    const QString &fullfilepath, 
                                                const QString &extension, 
                                                const bool preserve_filename )
{
    QString filename;    

    if ( preserve_filename )
    {
        filename = QFileInfo( fullfilepath ).fileName();
    }

    else
    {
        // Count - 2 because we don't care for the "." and ".." folders
        // + 1 because we are adding a file
        int index = QDir( FullPathToStylesFolder ).count() - 2 + 1;

        // We add a number to the file, so e.g. we get "style001.css", "style035.css" etc.
        filename = QString( "style" ) + QString( "%1" ).arg( index, 3, 10, QChar( '0' ) ) + "." + extension;
    }

    QFile::copy( fullfilepath, FullPathToStylesFolder + "/" + filename );

    return filename;
}

// Copies the file specified with fullfilepath
// to the OEBPS/misc folder with a generated name;
// the generated name is returned
QString FolderKeeper::AddFileToMiscFolder(    const QString &fullfilepath, 
                                              const QString &extension, 
                                              const bool preserve_filename )
{
    QString filename;    

    if ( preserve_filename )
    {
        filename = QFileInfo( fullfilepath ).fileName();
    }

    else
    {
        // Count - 2 because we don't care for the "." and ".." folders
        // + 1 because we are adding a file
        int index = QDir( FullPathToMiscFolder ).count() - 2 + 1;

        // We add a number to the file, so e.g. we get "misc001.xxx", "misc035.xxx" etc.
        filename = QString( "misc" ) + QString( "%1" ).arg( index, 3, 10, QChar( '0' ) ) + "." + extension;
    }

    QFile::copy( fullfilepath, FullPathToMiscFolder + "/" + filename );

    return filename;
}


// Performs common constructor duties
// for all constructors
void FolderKeeper::Initialize()
{
    QDir folder( Utility::GetNewTempFolderPath() );

    folder.mkpath( folder.absolutePath() );

    FullPathToMainFolder = folder.absolutePath();

    CreateFolderStructure();
}


// Creates the required folder structure:
//	 META-INF
//	 OEBPS
//	    images
//	    fonts
//	    text
//      styles
//      misc
void FolderKeeper::CreateFolderStructure()
{
    QDir folder( FullPathToMainFolder );

    folder.mkdir( "META-INF" );
    folder.mkdir( "OEBPS" );

    folder.mkpath( "OEBPS/fonts" );
    folder.mkpath( "OEBPS/images" );
    folder.mkpath( "OEBPS/text" );
    folder.mkpath( "OEBPS/styles" );
    folder.mkpath( "OEBPS/misc" );

    FullPathToMetaInfFolder		= FullPathToMainFolder + "/META-INF";
    FullPathToOEBPSFolder		= FullPathToMainFolder + "/OEBPS";

    FullPathToImagesFolder		= FullPathToOEBPSFolder + "/images";
    FullPathToFontsFolder		= FullPathToOEBPSFolder + "/fonts";
    FullPathToTextFolder		= FullPathToOEBPSFolder + "/text";
    FullPathToStylesFolder		= FullPathToOEBPSFolder + "/styles";
    FullPathToMiscFolder        = FullPathToOEBPSFolder + "/misc";
}


