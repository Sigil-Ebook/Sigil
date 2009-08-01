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
#include "Book.h"
#include "BookNormalization.h"
#include "CleanSource.h"


// Performs all the operations necessary
// on the Book before it is exported,
// like adding ID's to all headings etc.
Book BookNormalization::Normalize( const Book &book )
{
    Book newbook = book;

    newbook.source = GiveIDsToHeadings( newbook.source );
    newbook.source = RemoveXMLCarriageReturns( newbook.source );
    newbook.source = CleanSource::Clean( newbook.source );

    return newbook;
}

// Gives ID's to all headings that don't have them
QString BookNormalization::GiveIDsToHeadings( const QString &source )
{
    QDomDocument document;

    document.setContent( source );

    int heading_id = 1;

    for ( int level = 1; level < 7; level++ )
    {
        QString tag = "h" + QString::number( level );

        QDomNodeList headings = document.elementsByTagName( tag );

        for ( int index = 0; index < headings.count(); index++ )
        {
            QDomElement element = headings.at( index ).toElement();
            
            if ( !element.hasAttribute( "id" ) )
            {
                element.setAttribute( "id", "heading_id_" + QString::number( heading_id ) );
                
                heading_id++;
            }
        }
    }

    return document.toString();
}


// Removes the XML carriage returns ("&#xD" sequences )
// that <QDomDocument>.toString() kindly left for us
QString BookNormalization::RemoveXMLCarriageReturns( const QString &source )
{
    QString newsource = source;

    return newsource.replace( "&#xd;", "" );
}