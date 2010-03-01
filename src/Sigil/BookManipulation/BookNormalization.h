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

#pragma once
#ifndef BOOKNORMALIZATION_H
#define BOOKNORMALIZATION_H

#include "../BookManipulation/Headings.h"
#include <QSharedPointer>

class Book;
class HTMLResource;

/**
 * Houses the book normalization process.
 */
class BookNormalization
{

public:

    /**
     * Normalizes the book's contents for export.
     * Performs all the operations necessary 
     * on the Book before it can be exported,
     * like adding ID's to all headings etc.
     *
     * @param book The book to normalize.
     */
    static void Normalize( QSharedPointer< Book > book ); 

private:

    /**
     * Gives ID's to all headings that don't have them.
     *
     * @param html_resource The HTMLResource on whose headings 
     *                      the operation will be performed.
     */
    static void GiveIDsToHeadings( HTMLResource *html_resource );

    /**
     * Returns the maximum Sigil heading ID index. 
     * 
     * @param headings The list of headings to search.
     * @return The highest index.
     */
    static int MaxSigilHeadingIDIndex( const QList< Headings::Heading > headings );
};

#endif // BOOKNORMALIZATION_H

