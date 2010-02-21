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
#include "../BookManipulation/Book.h"
#include "../Misc/Utility.h"

// Constructor
Book::Book()
    : 
    m_PublicationIdentifier( Utility::CreateUUID() )
{
   
}


// Copy constructor
Book::Book( const Book& other )
{
    m_Metadata = other.m_Metadata;
    m_PublicationIdentifier = other.m_PublicationIdentifier;
    m_Mainfolder = other.m_Mainfolder;
}


// Assignment operator
Book& Book::operator = ( const Book& other )
{
    // Protect against invalid self-assignment
    if ( this != &other ) 
    {
        m_Metadata = other.m_Metadata;
        m_PublicationIdentifier = other.m_PublicationIdentifier;
        m_Mainfolder = other.m_Mainfolder;
    }

    // By convention, always return *this
    return *this;
}


// Returns the base url of the book,
// that is the location to the text folder
// within the main folder
QUrl Book::GetBaseUrl() const
{
    return QUrl::fromLocalFile( m_Mainfolder.GetFullPathToTextFolder() + "/" );
}


FolderKeeper& Book::GetFolderKeeper()
{
    return m_Mainfolder;
}


const FolderKeeper& Book::GetConstFolderKeeper()
{
    return m_Mainfolder;
}


QString Book::GetPublicationIdentifier()
{
    return m_PublicationIdentifier;
}

QHash< QString, QList< QVariant > > Book::GetMetadata()
{
    return m_Metadata;
}

void Book::SetMetadata( const QHash< QString, QList< QVariant > > metadata )
{
    m_Metadata = metadata;
}