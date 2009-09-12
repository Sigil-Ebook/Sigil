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

#pragma once
#ifndef FOLDERKEEPER_H
#define FOLDERKEEPER_H

#include <QString>


class FolderKeeper
{

public:

    // Constructor
    FolderKeeper();

    // Copy constructor
    FolderKeeper( const FolderKeeper& other );

    // Assignment operator
    FolderKeeper& operator = ( const FolderKeeper& other );

    // Destructor
    ~FolderKeeper();	

    // A dispatcher function that routes the given *infrastructure* file
    // to the appropriate specific folder function; the name of the new file
    // needs to be specified
    void AddInfraFileToFolder( const QString &fullfilepath, const QString &newfilename );

    // A dispatcher function that routes the given *content* file
    // to the appropriate specific folder function.
    // The file is recognized according to its extension: use force_extension
    // to override the recognition and force an extension.
    // Set preserve_filename to true to preserve the original filename.
    // The function returns the new file's path relative to the OEBPS folder.
    QString AddContentFileToFolder( const QString &fullfilepath, 
                                    const QString &force_extension = NULL,
                                    const bool preserve_filename = false );
    
    // Returns a list of all the content files in the directory
    // with a path relative to the OEBPS directory
    QStringList GetContentFilesList() const;

    // Returns the full path to the main folder of the publication
    QString GetFullPathToMainFolder() const;	

    // Returns the full path to the OEBPS folder of the publication
    QString GetFullPathToOEBPSFolder() const;	

    // Returns the full path to the OEBPS folder of the publication
    QString GetFullPathToTextFolder() const;	

private:

    // Copies the file specified with fullfilepath
    // to the META-INF folder with the name newfilename
    void AddFileToMetaInfFolder(    const QString &fullfilepath, const QString &newfilename );

    // Copies the file specified with fullfilepath
    // to the OEBPS folder with the name newfilename
    void AddFileToOEBPSFolder(      const QString &fullfilepath, const QString &newfilename );

    // Copies the file specified with fullfilepath
    // to the OEBPS/images folder with a generated name;
    // the generated name is returned.
    // Set preserve_filename to true to preserve the original filename.
    QString AddFileToImagesFolder(  const QString &fullfilepath, const QString &extension, const bool preserve_filename );

    // Copies the file specified with fullfilepath
    // to the OEBPS/fonts folder with a generated name;
    // the generated name is returned.
    // Set preserve_filename to true to preserve the original filename.
    QString AddFileToFontsFolder(   const QString &fullfilepath, const QString &extension, const bool preserve_filename );	

    // Copies the file specified with fullfilepath
    // to the OEBPS/text folder with a generated name;
    // the generated name is returned.
    // Set preserve_filename to true to preserve the original filename.
    QString AddFileToTextFolder(    const QString &fullfilepath, const QString &extension, const bool preserve_filename );

    // Copies the file specified with fullfilepath
    // to the OEBPS/styles folder with a generated name;
    // the generated name is returned.
    // Set preserve_filename to true to preserve the original filename.
    QString AddFileToStylesFolder(  const QString &fullfilepath, const QString &extension, const bool preserve_filename );

    // Copies the file specified with fullfilepath
    // to the OEBPS/misc folder with a generated name;
    // the generated name is returned.
    // Set preserve_filename to true to preserve the original filename.
    QString AddFileToMiscFolder(    const QString &fullfilepath, const QString &extension, const bool preserve_filename );

    // Performs common constructor duties
    // for all constructors
    void Initialize();

    // Creates the required folder structure:
    //	 META-INF
    //	 OEBPS
    //	    images
    //	    fonts
    //	    text
    //      styles
    //      misc
    void CreateFolderStructure();


    ///////////////////////////////
    // PRIVATE MEMBER VARIABLES
    ///////////////////////////////

    // Full paths to all the folders in the publication
    QString FullPathToMainFolder;
    QString FullPathToMetaInfFolder;
    QString FullPathToOEBPSFolder;

    QString FullPathToImagesFolder;
    QString FullPathToFontsFolder;
    QString FullPathToTextFolder;
    QString FullPathToStylesFolder;
    QString FullPathToMiscFolder;
};

#endif // FOLDERKEEPER_H


