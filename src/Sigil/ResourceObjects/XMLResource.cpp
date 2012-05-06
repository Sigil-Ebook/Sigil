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

#include "BookManipulation/CleanSource.h"
#include "BookManipulation/XhtmlDoc.h"
#include "Misc/Utility.h"
#include "ResourceObjects/XMLResource.h"


XMLResource::XMLResource( const QString &fullfilepath, QObject *parent )
    : TextResource( fullfilepath, parent )
{

}


Resource::ResourceType XMLResource::Type() const
{
    return Resource::XMLResourceType;
}


bool XMLResource::FileIsWellFormed() const
{
    // TODO: expand this with a dialog to fix the problem

    QReadLocker locker( &GetLock() );

    XhtmlDoc::WellFormedError error = XhtmlDoc::WellFormedErrorForSource( GetText() );

    bool well_formed = error.line == -1;

    return well_formed;
}


XhtmlDoc::WellFormedError XMLResource::WellFormedErrorLocation() const
{
    QReadLocker locker( &GetLock() );

    return XhtmlDoc::WellFormedErrorForSource( GetText() );
}


void XMLResource::UpdateTextFromDom( const xc::DOMDocument &document )
{
    QWriteLocker locker( &GetLock() );
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

            new_value.replace( i, 1, "_" );

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
           character == QChar( '-' )    ||
           character == QChar( '_' )    ||
           character == QChar( '.' )
           ;
}

