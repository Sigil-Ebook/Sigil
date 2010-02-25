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
#include <QDomDocument>
#include "../Misc/Utility.h"
#include "../BookManipulation/Book.h"
#include "BookNormalization.h"
#include "../BookManipulation/CleanSource.h"
#include "../BookManipulation/Headings.h"
#include "../BookManipulation/XHTMLDoc.h"
#include "ResourceObjects/HTMLResource.h"

static const QString SIGIL_HEADING_ID_PREFIX = "heading_id_";
static const QString SIGIL_HEADING_ID_REG    = SIGIL_HEADING_ID_PREFIX + "(\\d+)";


// Performs all the operations necessary
// on the Book before it is exported,
// like adding ID's to all headings etc.
void BookNormalization::Normalize( QSharedPointer< Book > book )
{
    QList< HTMLResource* > html_resources = book->GetFolderKeeper().GetSortedHTMLResources();

    QtConcurrent::blockingMap( html_resources, GiveIDsToHeadings );
}


// Gives ID's to all headings that don't have them
void BookNormalization::GiveIDsToHeadings( HTMLResource *html_resource )
{
    QReadLocker locker( &html_resource->GetLock() );

    QList< Headings::Heading > headings = Headings::GetHeadingListForOneFile( html_resource );

    int heading_id_index = MaxSigilHeadingIDIndex( headings ) + 1;

    for ( int index = 0; index < headings.count(); index++ )
    {
        QDomElement element = headings.at( index ).element;
        
        if ( !element.hasAttribute( "id" ) )
        {
            element.setAttribute( "id", SIGIL_HEADING_ID_PREFIX + QString::number( heading_id_index ) );
            
            heading_id_index++;
        }
    }    
}


// Returns the maximum index for Sigil heading IDs
// present in the provided XHTML source
int BookNormalization::MaxSigilHeadingIDIndex( const QList< Headings::Heading > headings )
{
    int maxindex = 1;
    
    for ( int index = 0; index < headings.count(); index++ )
    {
        QDomElement element = headings.at( index ).element;

        QRegExp suffix( SIGIL_HEADING_ID_REG );

        if ( element.attribute( "id" ).contains( suffix ) )
        {
            int index = suffix.cap( 1 ).toInt();

            if ( index > maxindex )

                maxindex = index;
        }
    }

    return maxindex;
}


