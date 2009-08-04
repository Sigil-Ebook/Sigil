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
#include "ImporterFactory.h"
#include "ImportTXT.h"
#include "ImportHTML.h"
#include "ImportEPUB.h"
#include "ImportSGF.h"


// Constructor
ImporterFactory::ImporterFactory()
    : imImporter( NULL )
{

}


// Destructor
ImporterFactory::~ImporterFactory()
{
    delete imImporter;
}


// Returns a reference to the importer
// appropriate for the given filename
Importer& ImporterFactory::GetImporter( const QString &filename )
{
    QString extension = QFileInfo( filename ).suffix();

    if (	( extension == "xhtml" ) ||
            ( extension == "html" ) ||
            ( extension == "htm" )
        )
    {
        imImporter = new ImportHTML( filename );

        return *imImporter;
    }

    if ( ( extension == "txt" ) )
    {
        imImporter = new ImportTXT( filename );

        return *imImporter;
    }

    if ( ( extension == "epub" ) )
    {
        imImporter = new ImportEPUB( filename );

        return *imImporter;
    }

    if ( ( extension == "sgf" ) )
    {
        imImporter = new ImportSGF( filename );

        return *imImporter;
    }

    // FIXME: Tell the user that the extension wasn't
    // recognized and then offer a default method
    // of loading (or maybe a list of methods?)
    
    // FIXME: should probably just be a TXT importer
    imImporter = new ImportHTML( filename );

    return *imImporter;
}

