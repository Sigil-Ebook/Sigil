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
#include "pcre.h"
#include "Searchable.h"
#include "Misc/PCREReplaceTextBuilder.h"
#include "Misc/Utility.h"

const int PCRE_MAX_GROUPS = 16;

int Searchable::Count( const QString &search_regex,
                       const QString &full_text)
{
    pcre *re;
    const char *error;
    int erroroffset;

    re = pcre_compile( search_regex.toUtf8().data(), PCRE_UTF8 | PCRE_MULTILINE, &error, &erroroffset, NULL );
    // compile faliure
    if ( re == NULL )
    {
        return 0;
    }

    int rc = 0;
    int last_end = 0;
    int count = 0;
    QByteArray utf_str = full_text.toUtf8();

    // The vector needs to be a multiple of 3.
    // N match items * 3 = our total size.
    // We only want the first match which is the entire string.
    int ovector[3] = { 0, 0, 0 };

    // Run until no matches are found.
    rc = pcre_exec( re, NULL, utf_str.data(), utf_str.length(), last_end, 0, ovector, 3 );
    while ( rc >= 0 && ovector[0] != ovector[1])
    {
        count++;
        last_end = ovector[1];
        rc = pcre_exec( re, NULL, utf_str.data(), utf_str.length(), last_end, 0, ovector, 3 );
    }

    pcre_free( re );

    return count;
}

tuple< int, int > Searchable::RunSearchRegex( const QString &search_regex,
                                 const QString &full_text, 
                                 int selection_offset, 
                                 Direction search_direction )
{
    pcre *re;
    const char *error;
    int erroroffset;

    re = pcre_compile( search_regex.toUtf8().data(), PCRE_UTF8 | PCRE_MULTILINE, &error, &erroroffset, NULL );
    // compile faliure
    if ( re == NULL )
    {
        return make_tuple( -1, -1 );
    }

    int rc = 0;
    // The vector needs to be a multiple of 3.
    // N match items * 3 = our total size.
    // We only want the first match which is the entire string.
    int ovector[3] = { 0, 0, 0 };
    int start = -1;
    int end = -1;
    QByteArray utf_str = full_text.toUtf8();

    if ( search_direction == Searchable::Direction_Down || 
         search_direction == Searchable::Direction_All 
        )
    {
        rc = pcre_exec( re, NULL, utf_str.data(), utf_str.length(), selection_offset, 0, ovector, 3 );

        // Match succeeded.
        if ( rc >= 0 )
        {
            start = ovector[0];
            end = ovector[1];
        }

        // If we need to search through the whole doc,
        // then we also wrap around and search from the 
        // beginning to the old search start point.
        if ( search_direction == Searchable::Direction_All &&
            start == -1
            )
        {
            QByteArray upper_half = Utility::Substring( 0, selection_offset, full_text ).toUtf8();

            rc = pcre_exec( re, NULL, upper_half.data(), upper_half.length(), 0, 0, ovector, 3 );

            // Success
            if ( rc >= 0 )
            {
                start = ovector[0];
                end = ovector[1];
            }
        }
    }
    else // search_direction == Searchable::Direction_Up 
    {
        // Only holds first and second positions of ovector
        // so we know if the last iteration was a real match.
        int last_ovector[2] = { 0, 0 };

        // Run until no matches are found.
        do
        {
            last_ovector[0] = ovector[0];
            last_ovector[1] = ovector[1];
            rc = pcre_exec( re, NULL, utf_str.data(), selection_offset, last_ovector[1], 0, ovector, 3 );
        } while( rc >= 0  && ovector[0] != ovector[1] );

        start = last_ovector[0];
        end = last_ovector[1];
    }

    pcre_free( re );

    // 0 lengh strings are not a match.
    if ( start == end )
    {
        start = end = -1;
    }

    return make_tuple( start, end );
}


bool Searchable::FillWithCapturedTexts( const QString &search_regex,
                                        const QString &selected_text,
                                        const QString &replacement_pattern,
                                        QString &full_replacement )
{
    PCREReplaceTextBuilder builder;
    return builder.BuildReplacementText(search_regex, selected_text, replacement_pattern, full_replacement);
}
