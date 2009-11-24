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
#ifndef SEARCHABLE_H
#define SEARCHABLE_H

class QRegExp;
class QString;

class Searchable
{

public:

    enum Direction
    {
        Direction_Up,
        Direction_Down,
        Direction_All,
    };

    // Destructor
    virtual ~Searchable() {}

    // Finds the next occurrence of the search term in the document,
    // and selects the matched string. The first argument is the matching
    // regex, the second is the direction of the search.
    virtual bool FindNext( const QRegExp &search_regex, Direction search_direction ) = 0;

    // Returns the number of times that the specified
    // regex matches in the document.
    virtual int Count( const QRegExp &search_regex ) = 0;

    // If the currently selected text matches the specified regex, 
    // it is replaced by the specified replacement string.
    virtual bool ReplaceSelected( const QRegExp &search_regex, const QString &replacement ) = 0;

    // Replaces all occurrences of the specified regex in 
    // the document with the specified replacement string.
    virtual int ReplaceAll( const QRegExp &search_regex, const QString &replacement ) = 0;

protected:

    // Accepts a regex, the full text to search,
    // the starting offset and the search direction.
    // Runs the regex through the text.
    // MODIFIES search_regex IN PLACE
    static void RunSearchRegex( QRegExp &search_regex, 
                                const QString &full_text, 
                                int selection_offset, 
                                Searchable::Direction search_direction );

    // Accepts a list of text capture groups and a replacement string,
    // and returns a new replacement string with capture group references
    // replaced with capture group contents.
    static QString FillWithCapturedTexts( const QStringList captured_texts, const QString &replacement );
};

#endif // SEARCHABLE_H