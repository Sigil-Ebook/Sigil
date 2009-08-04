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
#include "ImportHTML.h"
#include "Utility.h"
#include "CleanSource.h"



// Constructor;
// The parameter is the file to be imported
ImportHTML::ImportHTML( const QString &fullfilepath )
    : ImportTXT( fullfilepath )
{

}

// Reads and parses the file 
// and returns the created Book
Book ImportHTML::GetBook()
{
    if ( !IsFileReadable() )

        return Book();

    LoadSource();
    LoadFolderStructure();

    m_Book.source = CleanSource::Clean( m_Book.source );

    return m_Book;
}


// Returns a style tag created 
// from the provided path to a CSS file
QString ImportHTML::CreateStyleTag( const QString &fullfilepath )
{
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

    QString style_tag = "";

    if ( QFileInfo( fullfilepath ).suffix() == "css" )
    {
        style_tag = "<style type=\"text/css\">\n" + in.readAll() + "\n</style>\n";
    }

    else // XPGT stylesheet
    {
        style_tag = "<style type=\"application/vnd.adobe-page-template+xml\">\n" + in.readAll() + "\n</style>\n";
    }

    return style_tag;
}


// Updates all references to the resource specified with oldpath
// to the path of the new resource specified with newpath
void ImportHTML::UpdateReferences( const QString &oldpath, const QString &newpath )
{
    QString filename = QFileInfo( oldpath ).fileName();

    QRegExp reference;

    // Fonts get searched for differently than the other resources
    if ( ( filename.contains( ".ttf" ) ) || ( filename.contains( ".otf" ) ) )

        reference = QRegExp( "src:\\s*\\w+\\(([^\\)]*" + filename + ")\\)" );

    else

        reference = QRegExp( "<[^>]*\"([^\">]*" + filename + ")\"[^>]*>" );

    int index = -1;

    while ( true )
    {
        int newindex = m_Book.source.indexOf( reference );

        // We need to make sure we don't end up
        // replacing the same thing over and over again
        if ( ( index == newindex ) || ( newindex == -1 ) )

            break;

        m_Book.source.replace( reference.cap( 1 ), newpath );

        index = newindex;
    }
}


// Resolves custom ENTITY declarations
QString ImportHTML::ResolveCustomEntities( const QString &html_source )
{
    QString source = html_source;

    QRegExp entity_search( "<!ENTITY\\s+(\\w+)\\s+\"([^\"]+)\">" );

    QHash< QString, QString > entities;

    int main_index = 0;

    // Catch all custom entity declarations...
    while ( true )
    {
        main_index = source.indexOf( entity_search, main_index );

        if ( main_index == -1 )

            break;

        entities[ "&" + entity_search.cap( 1 ) + ";" ] = entity_search.cap( 2 );

        // Erase the entity declaration
        source.replace( entity_search.cap( 0 ), "" );
    }

    // ...and now replace all occurrences
    foreach( QString key, entities.keys() )
    {
        source.replace( key, entities[ key ] );
    }

    // Clean up what's left of the custom entity declaration field
    source.replace( QRegExp( "\\[\\s*\\]>" ), "" );

    return source;
}

// Loads the source code into the Book
void ImportHTML::LoadSource()
{
    QFile file( m_FullFilePath );

    // Check if we can open the file
    if ( !file.open( QFile::ReadOnly | QFile::Text ) )
    {
        QMessageBox::warning(	0,
            QObject::tr( "Sigil" ),
            QObject::tr("Cannot read file %1:\n%2.")
            .arg( m_FullFilePath )
            .arg( file.errorString() ) 
            );
        return;
    }

    QByteArray data = file.readAll();

    // Qt docs say Qt will take care of deleting
    // any QTextCodec objects on application exit
    m_Book.source = QTextCodec::codecForHtml( data, QTextCodec::codecForName( "UTF-8" ) )->toUnicode( data );
    m_Book.source = ResolveCustomEntities( m_Book.source );
}


// Loads the referenced files into the main folder of the book;
// as the files get a new name, the references are updated
void ImportHTML::LoadFolderStructure()
{
    int index = 0;

    QString image   = "<\\s*img[^>]*src\\s*=\\s*";
    QString linkel  = "<\\s*link[^>]*href\\s*=\\s*";
    QString theurl  = "\"([^\">]+)\"";
    QString tail    = "[^>]*>";

    while ( true )
    {         
        QRegExp fileurl( "(?:" + image + "|" + linkel + ")" + theurl + tail );

        index = m_Book.source.indexOf( fileurl, index ) + fileurl.matchedLength();

        if ( index < 0 )

            break;

        QDir folder( QFileInfo( m_FullFilePath ).absoluteDir() );

        QString fullfilepath = QFileInfo( folder, fileurl.cap( 1 ) ).absoluteFilePath();

        if ( !fullfilepath.contains( ".css" ) )
        {
            QString newpath = "../" + m_Book.mainfolder.AddContentFileToFolder( fullfilepath );
        
            UpdateReferences( fileurl.cap( 1 ), newpath );
        }

        else
        {
            QString style_tag = CreateStyleTag( fullfilepath );

            m_Book.source.replace( fileurl.cap( 0 ), style_tag );          
        }      
    }      
}


