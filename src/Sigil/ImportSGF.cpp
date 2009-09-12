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
#include "ImportSGF.h"
#include "Utility.h"
#include "CleanSource.h"


// Constructor;
// The parameter is the file to be imported
ImportSGF::ImportSGF( const QString &fullfilepath )
    : ImportEPUB( fullfilepath )
{

}


// Reads and parses the file 
// and returns the created Book;
// Overrides;
Book ImportSGF::GetBook()
{
    // These read the EPUB file
    ExtractContainer();
    LocateOPF();
    ReadOPF();

    // These mutate the m_Book object
    LoadMetadata();
    LoadSource();
    LoadFolderStructure();

    m_Book.source = CleanSource::Clean( m_Book.source );

    return m_Book;
}


// Loads the source code into the Book
void ImportSGF::LoadSource()
{
    QString fullpath = QFileInfo( m_OPFFilePath ).absolutePath() + "/" + m_Files.values().first();
    m_Book.source    = Utility::ReadUnicodeTextFile( fullpath ); 
}


// Loads the referenced files into the main folder of the book
void ImportSGF::LoadFolderStructure()
{
    foreach( QString key, m_Files.keys() )
    {
        QString path = m_Files[ key ];

        // We skip over the book text
        if ( !m_ReadingOrderIds.contains( key ) )
        {
            QString fullfilepath = QFileInfo( m_OPFFilePath ).absolutePath() + "/" + path;

            m_Book.mainfolder.AddContentFileToFolder( fullfilepath, NULL, true );
        }        
    }
}

