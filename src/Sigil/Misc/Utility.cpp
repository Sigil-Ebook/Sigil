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
#include "Misc/Utility.h"

#include <stdlib.h>
#include <time.h>


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


QStringList Utility::GetAbsolutePathsToFolderDescendantFiles( const QString &fullfolderpath )
{
    QDir folder( fullfolderpath );
    QStringList files;

    foreach( QFileInfo file, folder.entryInfoList() )
    {
        if ( ( file.fileName() != "." ) && ( file.fileName() != ".." ) )
        {
            // If it's a file, add it to the list
            if ( file.isFile() )
            {
                files.append( file.absoluteFilePath() );
            }

            // Else it's a directory, so
            // we add all files from that dir
            else 
            {
                files.append( GetAbsolutePathsToFolderDescendantFiles( file.absoluteFilePath() ) );                				
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
            if ( file.isFile() )
            {
                QString destination = fullfolderpath_destination + "/" + file.fileName();
                bool success = QFile::copy( file.absoluteFilePath(), destination );

                if ( !success )
                {
                    boost_throw( CannotCopyFile() 
                                 << errinfo_file_fullpath( file.absoluteFilePath().toStdString() )
                                 << errinfo_file_copypath( destination.toStdString() ) 
                               );
                }
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
        QMessageBox::critical( 0,
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
        boost_throw( CannotOpenFile() 
                     << errinfo_file_fullpath( fullfilepath.toStdString() )
                     << errinfo_file_errorstring( file.errorString().toStdString() ) 
                   );
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

    if ( !file.open( QIODevice::WriteOnly | 
                     QIODevice::Truncate  | 
                     QIODevice::Text  
                  ) 
       )
    {
        boost_throw( CannotOpenFile() 
                     << errinfo_file_fullpath( file.fileName().toStdString() )
                     << errinfo_file_errorstring( file.errorString().toStdString() ) 
                   );
    }

    QTextStream out( &file );

    // We ALWAYS output in UTF-8
    out.setCodec( "UTF-8" );
    out << text;
}


// Converts Mac and Windows style line endings to Unix style
// line endings that are expected throughout the Qt framework
QString Utility::ConvertLineEndings( const QString &text )
{
    QString newtext( text );
    return newtext.replace( "\x0D\x0A", "\x0A" ).replace( "\x0D", "\x0A" );
}


QString Utility::URLEncodePath( const QString &path )
{
    QByteArray encoded_url = QUrl::toPercentEncoding( path, QByteArray( "/#" ) );
    return QString::fromUtf8( encoded_url.constData(), encoded_url.count() );
}


QString Utility::URLDecodePath( const QString &path )
{
    return QUrl::fromPercentEncoding( path.toUtf8() );
}


void Utility::DisplayExceptionErrorDialog( const QString &error_info )
{
    QMessageBox message_box;
    message_box.setIcon( QMessageBox::Critical );
    message_box.setWindowTitle( "Sigil" );

    // Spaces are added to the end because otherwise the dialog is too small.
    message_box.setText( QObject::tr( "Sigil has encountered a problem.               " ) );
    message_box.setInformativeText( QObject::tr( "Please <a href=\"http://code.google.com/p/sigil/wiki/ReportingIssues\">report it</a> " 
                                    "on the issue tracker, including the details from this dialog." ) );

    message_box.setStandardButtons( QMessageBox::Close );

    QStringList detailed_text;
    detailed_text << "Error info: "    + error_info
                  << "Sigil version: " + QString( SIGIL_FULL_VERSION )
                  << "Runtime Qt: "    + QString( qVersion() )
                  << "Compiled Qt: "   + QString( QT_VERSION_STR );

#if defined Q_WS_WIN
    detailed_text << "Platform: Windows SysInfo ID " + QString::number( QSysInfo::WindowsVersion );
#elif defined Q_WS_MAC
    detailed_text << "Platform: Mac SysInfo ID " + QString::number( QSysInfo::MacintoshVersion);
#else
    detailed_text << "Platform: Linux";
#endif

    message_box.setDetailedText( detailed_text.join( "\n" ) );
    message_box.exec();
}


void Utility::DisplayStdErrorDialog( const QString &error_message, const QString &detailed_text )
{
    QMessageBox message_box;
    message_box.setIcon( QMessageBox::Critical );
    message_box.setWindowTitle( "Sigil" );
    message_box.setText( error_message );

    if ( !detailed_text.isEmpty() )
       
        message_box.setDetailedText( detailed_text );
    
    message_box.setStandardButtons( QMessageBox::Close );
    message_box.exec();
}


QString Utility::GetExceptionInfo( const ExceptionBase &exception )
{
    return QString::fromStdString( diagnostic_information( exception ) );
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


// Returns the same number, but rounded to one decimal place
float Utility::RoundToOneDecimal( float number )
{
    return QString::number( number, 'f', 1 ).toFloat();
}




