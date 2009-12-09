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
#include "Searchable.h"
#include "../Misc/Utility.h"


// Accepts a regex, the full text to search,
// the starting offset and the search direction.
// Runs the regex through the text.
// MODIFIES search_regex IN PLACE
void Searchable::RunSearchRegex(    QRegExp &search_regex, 
                                    const QString &full_text, 
                                    int selection_offset, 
                                    Direction search_direction )
{
    if (    search_direction == Searchable::Direction_Down || 
            search_direction == Searchable::Direction_All 
        )
    {
        full_text.indexOf( search_regex, selection_offset );

        // If we need to search through the whole doc,
        // then we also wrap around and search from the 
        // beginning to the old search start point.
        if (    search_direction == Searchable::Direction_All &&
                search_regex.pos() == -1 
            )
        {
            QString upper_half = Utility::Substring( 0, selection_offset, full_text );
            upper_half.indexOf( search_regex );
        }
    }

    else // search_direction == Searchable::Direction_Up 
    {
        // If we don't subtract 1 from the offset, we get stuck
        // in that position if we are already on a matching substring
        full_text.lastIndexOf( search_regex, selection_offset - 1 );
    }
}


// Accepts a list of text capture groups and a replacement string,
// and returns a new replacement string with capture group references
// replaced with capture group contents.
QString Searchable::FillWithCapturedTexts( const QStringList captured_texts, const QString &replacement )
{
    QString filled_string( replacement );

    int index = 0;

    // We can't just call replace in a loop because
    // the replacement string could contain capture
    // group references! ("\#")
    // So we make sure we ignore the pasted-in text.
    while ( index < filled_string.length() )
    {
        for ( int i = 0; i < captured_texts.length(); ++i )
        {
            QString group_marker = "\\" + QString::number( i );

            if ( filled_string.midRef( index, group_marker.length() ) == group_marker )
            {
                filled_string.replace( index, group_marker.length(), captured_texts[ i ] );
                index += captured_texts[ i ].length() - 1;
                break;
            }                
        }       

        ++index;
    }

    return filled_string;
}
