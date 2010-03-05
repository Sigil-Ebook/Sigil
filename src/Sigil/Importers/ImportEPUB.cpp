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
#include "ImportEPUB.h"
#include "../Misc/Utility.h"
#include "../SourceUpdates/UniversalUpdates.h"
#include "ResourceObjects/HTMLResource.h"
#include "ResourceObjects/CSSResource.h"


// Constructor;
// The parameter is the file to be imported
ImportEPUB::ImportEPUB( const QString &fullfilepath )
    : ImportOEBPS( fullfilepath )
{

}


// Reads and parses the file 
// and returns the created Book
QSharedPointer< Book > ImportEPUB::GetBook()
{
    if ( !Utility::IsFileReadable( m_FullFilePath ) )

        boost_throw( CannotReadFile() << errinfo_file_fullpath( m_FullFilePath.toStdString() ) );

    // These read the EPUB file
    ExtractContainer();
    LocateOPF();
    ReadOPF();

    // These mutate the m_Book object
    LoadMetadata();

    const QHash< QString, QString > &updates = LoadFolderStructure();
    const QList< Resource* > &resources      = m_Book->GetFolderKeeper().GetResourceList();

    UniversalUpdates::PerformUniversalUpdates( false, resources, updates );

    return m_Book;
}

