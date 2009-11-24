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
#include "ImportTXT.h"
#include "../BookManipulation/CleanSource.h"
#include "../Misc/Utility.h"



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
    m_Book.source = Utility::ReadUnicodeTextFile( m_FullFilePath );    
}


// Accepts a list of text lines and returns
// a string with paragraphs wrapped into <p> tags
QString ImportTXT::CreateParagraphs( const QStringList &lines ) const
{
    QString text = "";

    QString paragraph = "<p>";

    foreach( QString line, lines )
    {
        if ( line.isEmpty() || line[ 0 ].isSpace() )
        {
            text.append( paragraph.append( "</p>\n" ) );

            paragraph = "<p>";
        }

        paragraph.append( line.prepend( " " ) );
    }

    text.append( paragraph.append( "</p>\n" ) );

    return text;
}


