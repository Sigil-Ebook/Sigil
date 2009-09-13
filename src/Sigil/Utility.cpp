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
#include "Utility.h"
#include <QDomDocument>

#include <stdlib.h>
#include <time.h>

const QString WIN_PATH_SUFFIX = "/My Documents/Sigil"; 
const QString NIX_PATH_SUFFIX = "/.Sigil";

static const int TEMPFOLDER_NUM_RANDOM_CHARS = 10;
static const int TEMPFILE_NUM_RANDOM_CHARS   = 10;


// Returns a random string of "length" characters
QString Utility::GetRandomString( int length )
{
    static bool seed_flag = false;

    QString chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

    QString token;

    // This is probably not thread-safe
    if ( seed_flag == false )
    {
        srand( time( NULL ) );

        seed_flag = true;
    }

    for ( int i = 0; i < length; i++ )
    {
        token += chars[ rand() % 36 ];
    }

    return token;
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
void Utility::DeleteFolderAndFiles( const QString &fullfolderpath )
{
    // Make sure the path exists, otherwise very
    // bad things could happen
    if ( !QFileInfo( fullfolderpath ).exists() )

        return;

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
}


// Returns the full path to a new temporary folder;
// the caller is responsible for creating and deleting the folder
QString Utility::GetNewTempFolderPath()
{
    QString token = Utility::GetRandomString( TEMPFOLDER_NUM_RANDOM_CHARS );

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
                             Utility::GetRandomString( TEMPFILE_NUM_RANDOM_CHARS ) + "." +
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
    if ( !file.open( QFile::ReadOnly | QFile::Text ) )
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

    return in.readAll();
}


// Writes the provided text variable to the specified
// file; if the file exists, it is truncated
void Utility::WriteUnicodeTextFile( const QString &text, const QString &fullfilepath )
{
    QFile file( fullfilepath );

    if ( file.open(     QIODevice::WriteOnly | 
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


// We need to remove the XML carriage returns ("&#xD" sequences)
// that the default toString() method creates so we wrap it in this function
QString Utility::GetQDomDocumentAsString( const QDomDocument &document )
{
    // This function used to be just this one line:
    //
    //    return document.toString().replace( "&#xd;", "" );
    //
    // But Qt has a bug with the toString() method if the XML
    // encoding is specified as "us-ascii"... so we work around it.

    QString document_text;
    QTextStream stream( &document_text );
    stream.setCodec( "UTF-8" );

    document.save( stream, 1, QDomNode::EncodingFromTextStream );

    return document_text.replace( "&#xd;", "" );   
}


