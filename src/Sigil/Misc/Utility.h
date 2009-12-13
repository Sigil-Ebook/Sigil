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

class QDomNode;

class Utility
{	

public:

    // Returns a random string of length characters
    static QString GetRandomString( int length );

    // Returns true if the string is mixed case, false otherwise.
    // For instance, "test" and "TEST" return false, "teSt" returns true.
    static bool IsMixedCase( const QString &string );

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
    static bool DeleteFolderAndFiles( const QString &fullfolderpath ); 

    // Deletes the specified file if it exists
    static bool DeleteFile( const QString &fullfilepath );

    // Returns the full path to a new temporary folder;
    // the caller is responsible for creating and deleting the folder
    static QString GetNewTempFolderPath(); 

    // Creates a copy of the provided file with a random name in
    // the systems TEMP directory and returns the full path to the new file.
    // The extension of the original file is preserved. If the original file
    // doesn't exist, an empty string is returned.
    static QString CreateTemporaryCopy( const QString &fullfilepath ); 

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

    // Returns a value for the environment variable name passed;
    // if the env var isn't set, it returns an empty string
    static QString GetEnvironmentVar( const QString &variable_name );
    
    // Accepts a string with HTML and returns the text
    // in that HTML fragment. For instance 
    //   <h1>Hello <b>Qt</b> <![CDATA[<xml is cool>]]></h1>
    // returns
    //   Hello Qt <xml is cool>
    static QString GetTextInHtml( const QString &source );

    // Resolves HTML entities in the provided string.
    // For instance: 
    //    Bonnie &amp; Clyde
    // returns
    //    Bonnie & Clyde
    static QString ResolveHTMLEntities( const QString &text );

    // Returns the same number, but rounded to one decimal place
    static float RoundToOneDecimal( float number );

    // This function goes through the entire byte array 
    // and tries to see whether this is a valid UTF-8 sequence.
    // If it's valid, this is probably a UTF-8 string.
    static bool IsValidUtf8( const QByteArray &string );
};

#endif // UTILITY_H


