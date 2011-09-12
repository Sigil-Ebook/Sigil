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
#include "Misc/Utility.h"


static const int MAX_GROUPS = 16;
static const int OVECTOR_SIZE = ( 1 + MAX_GROUPS ) * 3;

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
    while ( rc >= 0 )
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
        } while( rc >= 0 );

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
    pcre *re;
    const char *error;
    int erroroffset;
    bool replacement_made = false;

    re = pcre_compile( search_regex.toUtf8().data(), PCRE_UTF8 | PCRE_MULTILINE, &error, &erroroffset, NULL );
    // compile faliure
    if ( re == NULL )
    {
        return replacement_made;
    }

    int rc = 0;
    // The vector needs to be a multiple of 3.
    // N match items * 3 = our total size.
    // We only want the first match which is the entire string.
    int ovector[OVECTOR_SIZE];
    QByteArray utf_str = selected_text.toUtf8();

    rc = pcre_exec( re, NULL, utf_str.data(), utf_str.length(), 0, 0, ovector, OVECTOR_SIZE );

    // Match succeeded. Go forward with replacing the matched text with
    // the replacement pattern.
    if ( rc >= 0 )
    {
        // If rc is 0 then we have more groups then our maximum allowed number.
        if ( rc == 0 )
        {
            rc = MAX_GROUPS;
        }

        // Tempory character used as we loop though all characters so we can
        // determine if it is a control character or text.
        QChar c;
        // Named back referecnes can be in within {} or <>. Store which
        // character we've matched so we can match the appropriate closing
        // character.
        QChar backref_bracket_start_char;
        // The state of our progress through the replacment string.
        bool in_backref = false;
        bool in_backref_bracket = false;
        // Stores a named back reference we build as we parse the string.
        QString backref_name;
        QString invalid_backref;

        // We are going to parse the replacment pattern one character at a time
        // to build the final replacement string. We need to replace subpatterns
        // numbered and named with the text matched by the regex.
        //
        // We do a linear replacement one character at a time instead of using
        // a regex because we don't want false positives or replacments
        // within subpatterns. E.G. replace \g{1a} with back reference 1 by
        // accident. It's also faster to do one pass through the string instead
        // of multiple times by using multiple regexes.
        //
        // Back references can be:
        // \# where # is 1 to 9.
        // \g# where # is 1 to 9.
        // \g{#s} where #s can be 1 to MAX_GROUPS.
        // \g<#s>  where #s can be 1 to MAX_GROUPS.
        // \g{text} where text can be any string.
        // \g<text> where text can be any string.
        for ( int i = 0; i < replacement_pattern.length(); i++ )
        {
            c = replacement_pattern.at( i );

            if ( in_backref )
            {
                invalid_backref += c;

                if ( c.isDigit() && !in_backref_bracket )
                {
                    int backref_number = c.digitValue();

                    // Check if there is we have a back reference we can
                    // actually get.
                    if ( backref_number > 0 && backref_number <= rc )
                    {
                        full_replacement += Utility::Substring( ovector[2 * backref_number], ovector[2 * backref_number + 1], selected_text );
                    }
                    else
                    {
                        full_replacement += invalid_backref;
                    }

                    in_backref = false;
                }
                else if ( c == 'g' && !in_backref_bracket )
                {
                    continue;
                }
                else if ( ( c == '{' || c == '<' ) && !in_backref_bracket )
                {
                    backref_name.clear();
                    backref_bracket_start_char = c;
                    in_backref_bracket = true;
                }
                else if ( ( ( c == '}' && backref_bracket_start_char == '{' ) || ( c == '>' && backref_bracket_start_char == '<' ) ) && in_backref_bracket )
                {
                    // Either we have a back reference number in the bracket
                    // or we have a name which we will convert to a number.
                    int backref_number;

                    // Try to convert the backref name to a number.
                    backref_number = backref_name.toInt();
                    // The backref wasn't a number so get the number for the name.
                    if ( backref_number == 0 )
                    {
                        backref_number = pcre_get_stringnumber( re, backref_name.toUtf8().data() );
                    }

                    // Check if there is we have a back reference we can
                    // actually get.
                    if ( backref_number > 0 && backref_number <= rc )
                    {
                        full_replacement += Utility::Substring( ovector[2 * backref_number], ovector[2 * backref_number + 1], selected_text );
                    }
                    else
                    {
                        full_replacement += invalid_backref;
                    }

                    in_backref_bracket = false;
                    in_backref = false;
                }
                // Accumulating the name of the back reference.
                else if ( in_backref_bracket )
                {
                    backref_name += c;
                }
                // Invalid back reference. Put it into the text.
                else
                {
                    full_replacement += invalid_backref;
                    in_backref = false;
                    in_backref_bracket = false;
                }
            }
            // We're not in a back reference.
            else
            {
                // Control character to start of a back reference.
                if ( c == '\\' )
                {
                    invalid_backref = c;
                    in_backref = true;
                }
                // Normal text.
                else
                {
                    full_replacement += c;
                }
            }
        }
        // If we ended reading the replacement string and we're still in
        // a back reference then we have an invalid back reference because
        // it never ended. Put the invalid reference into the replacment string.
        if ( in_backref || in_backref_bracket )
        {
            full_replacement += invalid_backref;
        }

        replacement_made = true;
    }

    pcre_free( re );

    return replacement_made;
}
