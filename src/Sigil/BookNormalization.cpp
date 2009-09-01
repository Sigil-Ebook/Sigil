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
#include <QDomDocument>
#include "Utility.h"
#include "Book.h"
#include "BookNormalization.h"
#include "CleanSource.h"
#include "Headings.h"

static const QString SIGIL_HEADING_ID_PREFIX = "heading_id_";
static const QString SIGIL_HEADING_ID_REG    = SIGIL_HEADING_ID_PREFIX + "(\\d+)";


// Performs all the operations necessary
// on the Book before it is exported,
// like adding ID's to all headings etc.
Book BookNormalization::Normalize( const Book &book )
{
    Book newbook = book;

    newbook.source = GiveIDsToHeadings( newbook.source );
    newbook.source = CleanSource::Clean( newbook.source );

    return newbook;
}

// Gives ID's to all headings that don't have them
QString BookNormalization::GiveIDsToHeadings( const QString &source )
{
    QDomDocument document;

    document.setContent( source );

    int heading_id_index = MaxSigilHeadingIDIndex( source ) + 1;

    for ( int level = 1; level < 7; level++ )
    {
        QString tag = "h" + QString::number( level );

        QDomNodeList headings = document.elementsByTagName( tag );

        for ( int index = 0; index < headings.count(); index++ )
        {
            QDomElement element = headings.at( index ).toElement();
            
            if ( !element.hasAttribute( "id" ) )
            {
                element.setAttribute( "id", SIGIL_HEADING_ID_PREFIX + QString::number( heading_id_index ) );
                
                heading_id_index++;
            }
        }
    }

    return Utility::GetQDomDocumentAsString( document );
}


// Returns the maximum index for Sigil heading IDs
// present in the provided XHTML source
int BookNormalization::MaxSigilHeadingIDIndex( const QString &source )
{
    int maxindex = 1;

    QList< Headings::Heading > headings = Headings::GetHeadingList( source );
    
    foreach( Headings::Heading heading, headings )
    {
        QRegExp suffix( SIGIL_HEADING_ID_REG );

        if ( heading.id.contains( suffix ) )
        {
            int index = suffix.cap( 1 ).toInt();

            if ( index > maxindex )

                maxindex = index;
        }
    }

    return maxindex;
}


