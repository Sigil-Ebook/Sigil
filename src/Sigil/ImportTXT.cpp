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
#include "ImportTXT.h"
#include "CleanSource.h"
#include "Utility.h"



// Constructor;
// The parameter is the file to be imported
ImportTXT::ImportTXT( const QString &fullfilepath )
    : m_FullFilePath( fullfilepath )
{

}

// Reads and parses the file 
// and returns the created Book
Book ImportTXT::GetBook()
{
    if ( !Utility::IsFileReadable( m_FullFilePath ) )
        
        return Book();
    
    LoadSource();
    
    m_Book.source = CreateParagraphs( m_Book.source.split( QChar( '\n' ) ) );
    m_Book.source = CleanSource::Clean( m_Book.source );

    return m_Book;
}


// Loads the source code into the Book
void ImportTXT::LoadSource()
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

    QTextStream in( &file );
    
    // Input should be UTF-8
    in.setCodec( "UTF-8" );

    // This will automatically switch reading from
    // UTF-8 to UTF-16 if a BOM is detected
    in.setAutoDetectUnicode( true );

    m_Book.source = in.readAll();    
}


// Accepts a list of text lines and returns
// a string with paragraphs wrapped into <p> tags
QString ImportTXT::CreateParagraphs( const QStringList &lines )
{
    QString text = "";

    QString paragraph = "<p>";

    foreach( QString line, lines )
    {
        if ( ( line.isEmpty() ) || ( line[ 0 ].isSpace() ) )
        {
            text += paragraph + "</p>\n";

            paragraph = "<p>";
        }

        paragraph += line;
    }

    return text;
}
