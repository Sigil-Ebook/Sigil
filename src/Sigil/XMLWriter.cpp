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
#include "XMLWriter.h"
#include "Book.h"


// Constructor;
// The first parameter is the book for which this XML file
// is being written, and the second is the list of files
// in the folder that will become the exported book
XMLWriter::XMLWriter( const Book &book, const FolderKeeper &fkeeper )
    : 
    m_Book( book ), 
    m_Folder( fkeeper ),
    m_Files( fkeeper.GetContentFilesList() ),
    m_Source( "" ),
    m_Writer( new QXmlStreamWriter( &m_Source ) )
{

}

    
// Destructor
XMLWriter::~XMLWriter( )
{
    delete m_Writer;
}





