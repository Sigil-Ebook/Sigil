/************************************************************************
**
**  Copyright (C) 2009, 2010, 2011  Strahinja Markovic  <strahinja.markovic@gmail.com>
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
#include "XMLResource.h"
#include "BookManipulation/XhtmlDoc.h"
#include "BookManipulation/CleanSource.h"
#include "Misc/Utility.h"


XMLResource::XMLResource( const QString &fullfilepath, QObject *parent )
    : TextResource( fullfilepath, parent )
{

}


Resource::ResourceType XMLResource::Type() const
{
    return Resource::XMLResource;
}


bool XMLResource::FileIsWellFormed()
{
    // TODO: expand this with a dialog to fix the problem

    QReadLocker locker( &m_ReadWriteLock );

    int error_line, error_column = -1;
    tie( error_line, error_column ) = XhtmlDoc::WellFormedErrorLocation( m_TextDocument->toPlainText() );

    return error_line == -1; 
}


void XMLResource::UpdateTextFromDom( const xc::DOMDocument &document )
{
    QWriteLocker locker( &m_ReadWriteLock );
    SetText( CleanSource::ProcessXML( XhtmlDoc::GetDomDocumentAsString( document ) ) );
}


QString XMLResource::GetValidID( const QString &value )
{
    QString new_value = value.simplified();
    int i = 0;

    // Remove all forbidden characters.
    while ( i < new_value.size() )
    {
        if ( !IsValidIDCharacter( new_value.at( i ) ) )
        
            new_value.remove( i, 1 );
        
        else
        
            ++i;        
    }

    if ( new_value.isEmpty() )

        return Utility::CreateUUID();

    QChar first_char = new_value.at( 0 );

    // IDs cannot start with a number, a dash or a dot
    if ( first_char.isNumber()      ||
         first_char == QChar( '-' ) ||
         first_char == QChar( '.' )
       )
    {
        new_value.prepend( "x" );
    }

    return new_value;
}


// This is probably more rigorous 
// than the XML spec, but it's simpler.
// (spec ref: http://www.w3.org/TR/xml-id/#processing)
bool XMLResource::IsValidIDCharacter( const QChar &character )
{
    return character.isLetterOrNumber() ||
           character == QChar( '=' )    ||
           character == QChar( '-' )    ||
           character == QChar( '_' )    ||
           character == QChar( '.' )    ||
           character == QChar( ':' )
           ;
}

