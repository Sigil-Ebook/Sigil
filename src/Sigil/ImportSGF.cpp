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


// Constructor;
// The parameter is the file to be imported
ImportSGF::ImportSGF( const QString &fullfilepath )
    : ImportEPUB( fullfilepath )
{

}

// Loads the source code into the Book
void ImportSGF::LoadSource()
{
    QString fullpath = QFileInfo( m_OPFFilePath ).absolutePath() + "/" + m_Files.values().first();

    QFile file( fullpath );

    // Check if we can open the file
    if ( !file.open( QFile::ReadOnly | QFile::Text ) )
    {
        QMessageBox::warning(	0,
            QObject::tr( "Sigil" ),
            QObject::tr("Cannot read file %1:\n%2.")
            .arg( fullpath )
            .arg( file.errorString() ) 
            );
        return;
    }

    QTextStream in( &file );

    // Input should be UTF-8
    in.setCodec( "UTF-8" );

    m_Book.source = in.readAll();
}
