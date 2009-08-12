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
#include "ExportSGF.h"
#include "FolderKeeper.h"
#include "Book.h"


// Constructor;
// the first parameter is the location where the book 
// should be save to, and the second is the book to be saved
ExportSGF::ExportSGF( const QString &fullfilepath, const Book &book  )
    : ExportEPUB( fullfilepath, book )
{

}


// Writes the book to the path 
// specified in the constructor
void ExportSGF::WriteBook()
{
    CreatePublication();

    SaveTo( m_FullFilePath );
}


// Creates the publication from the Book
// (creates XHTML, OPF, NCX files etc.)
void ExportSGF::CreatePublication()
{
    //CreateXHTMLFile();
    CreateOneTextFile( m_Book.source, "xhtml" );

    CreateContainerXML();
    CreateContentOPF();
    CreateTocNCX();
}


