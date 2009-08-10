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

class Book;

class BookNormalization
{

public:

    // Performs all the operations necessary
    // on the Book before it is exported,
    // like adding ID's to all headings etc.
    static Book Normalize( const Book &book ); 

private:

    // Gives ID's to all headings that don't have them
    static QString GiveIDsToHeadings( const QString &source );

    // Returns the maximum index for Sigil heading IDs
    // present in the provided XHTML source
    static int MaxSigilHeadingIDIndex( const QString &source );
};

#endif // BOOKNORMALIZATION_H