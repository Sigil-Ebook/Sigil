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

bool Book::s_IgnoreCalibreEnvFlag = false;

// Constructor
Book::Book()
    : 
    PublicationIdentifier( Utility::CreateUUID() ),
    m_ReportToCalibre( false )
{
    QMutexLocker locker( &m_IgnoreCalibreEnvFlagSync );

    if ( !Utility::GetEnvironmentVar( "CALLED_FROM_CALIBRE" ).isEmpty() &&
         !s_IgnoreCalibreEnvFlag 
        )
    {
        SetReportToCalibreStatus( true );

        s_IgnoreCalibreEnvFlag = true;
    }
}


// Copy constructor
Book::Book( const Book& other )
{
    source = other.source;
    metadata = other.metadata;
    PublicationIdentifier = other.PublicationIdentifier;
    mainfolder = other.mainfolder;

    // We do NOT copy the m_ReportToCalibre value
}


// Assignment operator
Book& Book::operator = ( const Book& other )
{
    // Protect against invalid self-assignment
    if ( this != &other ) 
    {
        source = other.source;
        metadata = other.metadata;
        PublicationIdentifier = other.PublicationIdentifier;
        mainfolder = other.mainfolder;

        // We do NOT copy the m_ReportToCalibre value
    }

    // By convention, always return *this
    return *this;
}


// Returns the base url of the book,
// that is the location to the text folder
// within the main folder
QUrl Book::GetBaseUrl() const
{
    return QUrl::fromLocalFile( mainfolder.GetFullPathToTextFolder() + "/" );
}


// Returns the status of the m_ReportToCalibre
// variable. Thread-safe.
bool Book::GetReportToCalibreStatus()
{
    QMutexLocker locker( &m_ReportToCalibreSync );

    return m_ReportToCalibre;
}


// Sets the status of the m_ReportToCalibre
// variable. Thread-safe.
void Book::SetReportToCalibreStatus( bool new_status )
{
    QMutexLocker locker( &m_ReportToCalibreSync );

    m_ReportToCalibre = new_status;
}
