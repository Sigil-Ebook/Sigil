/************************************************************************
**
**  Copyright (C) 2011  John Schember <john@nachtimwald.com>
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
#include "Searchable.h"
#include "PCRE/PCRECache.h"

void Searchable::UpdateSearchCache( const QString &search_regex, const QString &text )
{
    if ( search_regex != m_FindPattern || m_MatchOffsets.isEmpty() )
    {
        m_FindPattern = search_regex;
        m_MatchOffsets = PCRECache::instance()->getObject(m_FindPattern)->getMatchOffsets( text );
    }
}


std::pair<int, int> Searchable::NearestMatch( const QList<std::pair<int, int> > &matches,
                                    int position,
                                    Searchable::Direction search_direction )
{
    std::pair<int, int> nearest_match(-1, -1);

    if ( matches.isEmpty() )
    {
        return nearest_match;
    }

    int first_after = -1;
    for ( int i = 0; i < matches.count(); i++ )
    {
        if ( matches.at( i ).first >= position )
        {
            first_after = i;
            break;
        }
    }

    if ( search_direction == Searchable::Direction_Down )
    {
        if ( first_after != -1 )
        {
            nearest_match = matches.at( first_after );
        }
    }
    else if ( search_direction == Searchable::Direction_Up )
    {
        // No maths after our position so we are at the end.
        if ( first_after == -1 )
        {
            nearest_match = matches.at( matches.count() -1 );
        }
        // There is a match after we're somewhere in the middle
        else
        {
            int first_before = first_after - 1;
            // We have matches before.
            if ( first_before >= 0 )
            {
                nearest_match = matches.at( first_before );
            }
        }
    }
    else if ( search_direction == Searchable::Direction_All )
    {
        if ( first_after != -1 )
        {
            nearest_match = matches.at( first_after );
        }
        else
        {
            nearest_match = matches.at( 0 );
        }
    }

    return nearest_match;
}


bool Searchable::IsMatchSelected( const QList<std::pair<int, int> > &matches,
                                int start,
                                int end)
{
    for ( int i = 0; i < matches.count(); i++ )
    {
        if ( matches.at( i ).first == start && matches.at( i ).second == end )
        {
            return true;
        }
    }

    return false;
}
