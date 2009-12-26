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
#include "../Misc/Utility.h"
#include <QDomDocument>

#include <stdlib.h>
#include <time.h>

const QString WIN_PATH_SUFFIX = "/Sigil"; 
const QString NIX_PATH_SUFFIX = "/.Sigil";


// Uses QUuid to generate a random UUID but also removes
// the curly braces that QUuid::createUuid() adds
QString Utility::CreateUUID()
{
    return QUuid::createUuid().toString().remove( "{" ).remove( "}" );
}


// Returns true if the string is mixed case, false otherwise.
// For instance, "test" and "TEST" return false, "teSt" returns true.
// If the string is empty, returns false.
bool Utility::IsMixedCase( const QString &string )
{
    if ( string.isEmpty() || string.length() == 1 )

        return false;
    
    bool first_char_lower = string[ 0 ].isLower();

    for ( int i = 1; i < string.length(); ++i )
    {
        if ( string[ i ].isLower() != first_char_lower )

            return true;
    }

    return false;
}

// Returns a substring of a specified string;
// the characters included are in the interval: 
// [ start_index, end_index >
QString Utility::Substring( int start_index, int end_index, const QString &string )
{
    return string.mid( start_index, end_index - start_index);
}

// Replace the first occurrence of string "before"
// with string "after" in string "string"
QString Utility::ReplaceFirst( const QString &before, const QString &after, const QString &string )
{
    int start_index = string.indexOf( before );
    int end_index   = start_index + before.length();

    return Substring( 0, start_index, string ) + after + Substring( end_index, string.length(), string );
}


QStringList Utility::RecursiveGetFiles( const QString &fullfolderpath )
{
    QDir folder( fullfolderpath );
    QStringList files;

    foreach( QFileInfo file, folder.entryInfoList() )
    {
        if ( ( file.fileName() != "." ) && ( file.fileName() != ".." ) )
        {
            // If it's a file, add it to the list
            if ( file.isFile() == true )
            {
                files.append( file.absoluteFilePath() );
            }

            // Else it's a directory, so
            // we add all files from that dir
            else 
            {
                files.append( RecursiveGetFiles( file.absoluteFilePath() ) );                				
            }
        }
    }

    return files;
}


// Copies every file and folder in the source folder 
// to the destination folder; the paths to the folders are submitted;
// the destination folder needs to be created in advance
void Utility::CopyFiles( const QString &fullfolderpath_source, const QString &fullfolderpath_destination )
{
    QDir folder_source( fullfolderpath_source );
    QDir folder_destination( fullfolderpath_destination );

    // Erase all the files in this folder
    foreach( QFileInfo file, folder_source.entryInfoList() )
    {
        if ( ( file.fileName() != "." ) && ( file.fileName() != ".." ) )
        {
            // If it's a file, copy it
            if ( file.isFile() == true )
            {
                QFile::copy( file.absoluteFilePath(), fullfolderpath_destination + "/" + file.fileName() );
            }

            // Else it's a directory, copy everything in it
            // to a new folder of the same name in the destination folder
            else 
            {
                folder_destination.mkpath( file.fileName() );

                CopyFiles( file.absoluteFilePath(), fullfolderpath_destination + "/" + file.fileName() );				
            }
        }
    }
}

// Deletes the folder specified with fullfolderpath
// and all the files (and folders, recursively) in it
bool Utility::DeleteFolderAndFiles( const QString &fullfolderpath )
{
    // Make sure the path exists, otherwise very
    // bad things could happen
    if ( !QFileInfo( fullfolderpath ).exists() )

        return false;

    QDir folder( fullfolderpath );

    // Erase all the files in this folder
    foreach( QFileInfo file, folder.entryInfoList() )
    {
        if ( ( file.fileName() != "." ) && ( file.fileName() != ".." ) )
        {
            // If it's a file, delete it
            if ( file.isFile() == true )

                folder.remove( file.fileName() );

            // Else it's a directory, delete it recursively
            else 

                DeleteFolderAndFiles( file.absoluteFilePath() );
        }
    }

    // Delete the folder after it's empty
    folder.rmdir( folder.absolutePath() );

    return true;
}

// Deletes the specified file if it exists
bool Utility::DeleteFile( const QString &fullfilepath )
{
    // Make sure the path exists, otherwise very
    // bad things could happen
    if ( !QFileInfo( fullfilepath ).exists() )

        return false;

    QFile file( fullfilepath );
    return file.remove();
}


bool Utility::RenameFile( const QString &oldfilepath, const QString &newfilepath )
{
    // Make sure the path exists, otherwise very
    // bad things could happen
    if ( !QFileInfo( oldfilepath ).exists() )

        return false;

    QFile file( oldfilepath );
    return file.rename( newfilepath );
}


// Returns the full path to a new temporary folder;
// the caller is responsible for creating and deleting the folder
QString Utility::GetNewTempFolderPath()
{
    QString token = Utility::CreateUUID();

    // The path used to store the folders depends on the OS used

#ifdef Q_WS_WIN

    QString folderpath = QDir::homePath() + WIN_PATH_SUFFIX + "/scratchpad/" + token;

#else

    QString folderpath = QDir::homePath() + NIX_PATH_SUFFIX + "/scratchpad/." + token;

#endif

    return folderpath;
}


// Creates a copy of the provided file with a random name in
// the systems TEMP directory and returns the full path to the new file.
// The extension of the original file is preserved. If the original file
// doesn't exist, an empty string is returned.
QString Utility::CreateTemporaryCopy( const QString &fullfilepath )
{
    if ( !QFileInfo( fullfilepath ).exists() || !QFileInfo( fullfilepath ).isFile() )

        return QString();

    QString temp_file_path = QDir::temp().absolutePath() + "/" + 
                             Utility::CreateUUID() + "." +
                             QFileInfo( fullfilepath ).suffix();

    QFile::copy( fullfilepath, temp_file_path );

    return temp_file_path;
}


// Returns true if the file can be read;
// shows an error dialog if it can't
// with a message elaborating what's wrong
bool Utility::IsFileReadable( const QString &fullfilepath )
{
    // Qt has <QFileInfo>.exists() and <QFileInfo>.isReadable()
    // functions, but then we would have to create our own error
    // message for each of those situations (and more). Trying to
    // actually open the file enables us to retrieve the exact
    // reason preventing us from reading the file in an error string.

    QFile file( fullfilepath );

    // Check if we can open the file
    if ( !file.open( QFile::ReadOnly ) )
    {
        QMessageBox::warning(	0,
                                QObject::tr( "Sigil" ),
                                QObject::tr( "Cannot read file %1:\n%2." )
                                .arg( fullfilepath )
                                .arg( file.errorString() ) 
                            );
        return false;
    }
    
    file.close();

    return true;
}


// Reads the text file specified with the full file path;
// text needs to be in UTF-8 or UTF-16; if the file cannot
// be read, an error dialog is shown and an empty string returned
QString Utility::ReadUnicodeTextFile( const QString &fullfilepath )
{
    // TODO: throw an exception instead of
    // returning an empty string 

    QFile file( fullfilepath );

    // Check if we can open the file
    if ( !file.open( QFile::ReadOnly ) )
    {
        QMessageBox::warning(	0,
                                QObject::tr( "Sigil" ),
                                QObject::tr( "Cannot read file %1:\n%2." )
                                .arg( fullfilepath )
                                .arg( file.errorString() ) 
                             );
        return "";
    }

    QTextStream in( &file );

    // Input should be UTF-8
    in.setCodec( "UTF-8" );

    // This will automatically switch reading from
    // UTF-8 to UTF-16 if a BOM is detected
    in.setAutoDetectUnicode( true );

    return ConvertLineEndings( in.readAll() );
}


// Writes the provided text variable to the specified
// file; if the file exists, it is truncated
void Utility::WriteUnicodeTextFile( const QString &text, const QString &fullfilepath )
{
    QFile file( fullfilepath );

    if ( file.open(  QIODevice::WriteOnly | 
                     QIODevice::Truncate | 
                     QIODevice::Text  
                  ) 
       )
    {
        QTextStream out( &file );

        // We ALWAYS output in UTF-8
        out.setCodec( "UTF-8" );

        out << text;

        // Write to disk immediately
        out.flush();
        file.flush();
    }

    // TODO: throw error if not open    
}


// Converts Mac and Windows style line endings to Unix style
// line endings that are expected throughout the Qt framework
QString Utility::ConvertLineEndings( const QString &text )
{
    QString newtext( text );
    return newtext.replace( "\x0D\x0A", "\x0A" ).replace( "\x0D", "\x0A" );
}


// Returns a value for the environment variable name passed;
// if the env var isn't set, it returns an empty string
QString Utility::GetEnvironmentVar( const QString &variable_name )
{
    // Renaming this function (and all references to it)
    // to GetEnvironmentVariable gets you a linker error 
    // on MSVC 9. Funny, innit?

    QRegExp search_for_name( "^" + QRegExp::escape( variable_name ) + "=" );

    QString variable = QProcess::systemEnvironment().filter( search_for_name ).value( 0 );

    if ( !variable.isEmpty() )

        return variable.split( "=" )[ 1 ];

    else

        return QString();
}


// Accepts a string with HTML and returns the text
// in that HTML fragment. For instance: 
//   <h1>Hello <b>Qt</b> <![CDATA[<xml is cool>]]></h1>
// returns
//   Hello Qt <xml is cool>
QString Utility::GetTextInHtml( const QString &source )
{
    QDomDocument document;
    document.setContent( source );
    QDomElement document_element = document.documentElement();

    return document_element.text();
}


// Resolves HTML entities in the provided string.
// For instance: 
//    Bonnie &amp; Clyde
// returns
//    Bonnie & Clyde
QString Utility::ResolveHTMLEntities( const QString &text )
{
    // Faking some HTML... this is the easiest way to do it
    QString newsource = "<div>" + text + "</div>";

    return GetTextInHtml( newsource );
}


QString Utility::GetEntityEscapedString( const QString &text )
{
    QString new_text( text );
    return new_text.replace( "&", "&amp;" ).replace( "<", "&lt;" ).replace( ">", "&gt;" );
}


// Returns the same number, but rounded to one decimal place
float Utility::RoundToOneDecimal( float number )
{
    return QString::number( number, 'f', 1 ).toFloat();
}


// This function goes through the entire byte array 
// and tries to see whether this is a valid UTF-8 sequence.
// If it's valid, this is probably a UTF-8 string.
bool Utility::IsValidUtf8( const QByteArray &string )
{
    // This is an implementation of the Perl code written here:
    //   http://www.w3.org/International/questions/qa-forms-utf-8
    //
    // Basically, UTF-8 has a very specific byte-pattern. This function
    // checks if the sent byte-sequence conforms to this pattern.
    // If it does, chances are *very* high that this is UTF-8.
    //
    // This function is written to be fast, not pretty.    

    if ( string.isNull() )

        return false;

    int index = 0;
    const unsigned char * bytes = NULL;

    while ( index < string.size() )
    {
        QByteArray dword = string.mid( index, 4 );

        if ( dword.size() < 4 )

            dword = dword.leftJustified( 4, '\0' );

        bytes = (const unsigned char *) dword.constData();

        // ASCII
        if (   bytes[0] == 0x09 ||
               bytes[0] == 0x0A ||
               bytes[0] == 0x0D ||
               ( 0x20 <= bytes[0] && bytes[0] <= 0x7E )                    
           ) 
        {
            index += 1;
        }

        // non-overlong 2-byte
        else if (  ( 0xC2 <= bytes[0] && bytes[0] <= 0xDF ) &&
                   ( 0x80 <= bytes[1] && bytes[1] <= 0xBF )            
                ) 
        {
            index += 2;
        }
           
        else if (  (     bytes[0] == 0xE0                         &&         // excluding overlongs 
                         ( 0xA0 <= bytes[1] && bytes[1] <= 0xBF ) &&
                         ( 0x80 <= bytes[2] && bytes[2] <= 0xBF )        ) || 
                  
                   (     (   ( 0xE1 <= bytes[0] && bytes[0] <= 0xEC ) ||     // straight 3-byte
                             bytes[0] == 0xEE                         ||
                             bytes[0] == 0xEF                     ) &&
                    
                         ( 0x80 <= bytes[1] && bytes[1] <= 0xBF )   &&
                         ( 0x80 <= bytes[2] && bytes[2] <= 0xBF )        ) ||

                   (     bytes[0] == 0xED                         &&         // excluding surrogates
                         ( 0x80 <= bytes[1] && bytes[1] <= 0x9F ) &&
                         ( 0x80 <= bytes[2] && bytes[2] <= 0xBF )        )
                 ) 
        {
            index += 3;
        }
 
          
        else if (    (   bytes[0] == 0xF0                         &&         // planes 1-3
                         ( 0x90 <= bytes[1] && bytes[1] <= 0xBF ) &&
                         ( 0x80 <= bytes[2] && bytes[2] <= 0xBF ) &&
                         ( 0x80 <= bytes[3] && bytes[3] <= 0xBF )      ) ||

                     (   ( 0xF1 <= bytes[0] && bytes[0] <= 0xF3 ) &&         // planes 4-15
                         ( 0x80 <= bytes[1] && bytes[1] <= 0xBF ) &&
                         ( 0x80 <= bytes[2] && bytes[2] <= 0xBF ) &&
                         ( 0x80 <= bytes[3] && bytes[3] <= 0xBF )      ) ||
                
                     (   bytes[0] == 0xF4                         &&         // plane 16
                         ( 0x80 <= bytes[1] && bytes[1] <= 0x8F ) &&
                         ( 0x80 <= bytes[2] && bytes[2] <= 0xBF ) &&
                         ( 0x80 <= bytes[3] && bytes[3] <= 0xBF )      )
                ) 
        {
            index += 4;
        }

        else
        {
            return false;
        }
    }

    return true;
}


