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
#ifndef UTILITY_H
#define UTILITY_H

#include <QString>
#include <QDir>

class Utility
{	

public:

    // Returns a random string of length characters
    static QString GetRandomString( int length );

    // Returns a substring of a specified string;
    // the characters included are in the interval: 
    // [ start_index, end_index >
    static QString Substring( int start_index, int end_index, const QString &string );

    // Replace the first occurrence of string "before"
    // with string "after" in string "string"
    static QString ReplaceFirst( const QString &before, const QString &after, const QString &string );
   
    // Copies every file and folder in the source folder 
    // to the destination folder; the paths to the folders are submitted;
    // the destination folder needs to be created in advance
    static void CopyFiles( const QString &fullfolderpath_source, const QString &fullfolderpath_destination );

    // Deletes the folder specified with fullfolderpath
    // and all the files (and folders, recursively) in it
    static void DeleteFolderAndFiles( const QString &fullfolderpath );  

    // Returns the full path to a new temporary folder;
    // the caller is responsible for creating and deleting the folder
    static QString GetNewTempFolderPath(); 

    // Returns true if the file can be read;
    // shows an error dialog if it can't
    // with a message elaborating what's wrong
    static bool IsFileReadable( const QString &fullfilepath );

    // Reads the text file specified with the full file path;
    // text needs to be in UTF-8 or UTF-16; if the file cannot
    // be read, an error dialog is shown and an empty string returned
    static QString ReadUnicodeTextFile( const QString &fullfilepath );

    // Writes the provided text variable to the specified
    // file; if the file exists, it is truncated
    static void WriteUnicodeTextFile( const QString &text, const QString &fullfilepath );
};

#endif // UTILITY_H


