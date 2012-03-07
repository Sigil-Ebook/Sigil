/************************************************************************
**
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

#include <QtCore/QRegExp>
#include <QtCore/QString>
#include <QtCore/QTextCodec>

#include "Misc/HTMLEncodingResolver.h"
#include "Misc/Utility.h"

#include "PCRE/PCRECache.h"
#include "Misc/SpellCheck.h"
#include "Misc/HTMLSpellCheck.h"
#include "sigil_constants.h"
#include "sigil_exception.h"

const int MAX_WORD_LENGTH  = 90;

QList< HTMLSpellCheck::MisspelledWord > HTMLSpellCheck::GetMisspelledWords( const QString &orig_text,
                                                     int start_offset,
                                                     int end_offset,
                                                     const QString &search_regex, 
                                                     bool first_only )
{
    SpellCheck *sc = SpellCheck::instance();

    bool in_tag = false;
    bool in_invalid_word = false;
    bool in_entity = false;
    int word_start = 0;

    SPCRE *pcre = PCRECache::instance()->getObject( search_regex );

    QList< HTMLSpellCheck::MisspelledWord > misspellings;

    // Make sure text has beginning/end boundary markers for easier parsing
    QString text = QChar(' ') + orig_text + QChar(' ');

    for ( int i = 0; i < text.count(); i++ )
    {
        QChar c = text.at(i); 
        if ( !in_tag )
        {
            QChar prev_c = i > 0 ? text.at( i - 1 ): QChar( ' ' );
            QChar next_c = i < text.count() - 1 ? text.at( i + 1 ): QChar( ' ' );

            if ( IsBoundary( prev_c, c, next_c, in_entity ) )
            {
                // If we're in an entity and we hit a boundary and it isn't
                // part of an entity then this is an invalid entity.
                if ( in_entity && c != QChar(';') )
                {
                    in_entity = false;
                }
                // Check possibilities that would mean this isn't a word worth considering.
                if ( !in_invalid_word && !in_entity && word_start != -1 && ( i - word_start ) > 0 )
                {
                    QString word = Utility::Substring( word_start, i, text );

                    if ( !word.isEmpty() && word_start >= start_offset && word_start <= end_offset && !sc->spell( word ) )
                    {
                        SPCRE::MatchInfo match;
                        if ( !search_regex.isEmpty() )
                        {
                            match = pcre->getFirstMatchInfo( word );
                        }
                        if ( search_regex.isEmpty() || match.offset.first != -1 )
                        {
                            struct MisspelledWord misspelled_word;
                            misspelled_word.text = word;
                            // Make sure we account for the extra boundary added at the beginning
                            misspelled_word.offset = word_start - 1;
                            misspelled_word.length = i - word_start ;
                            misspellings.append( misspelled_word );

                            if ( first_only )
                            {
                                return misspellings;
                            }
                        }
                    }
                }
                // We want to start the word with the character after the boundary.
                // If the next character is another boundary we'll just move forwad one.
                word_start = i + 1;
                in_invalid_word = false;
            }
            else
            {
                // Ensure we're not dealing with some crazy run on text that isn't worth
                // considering as an actual word.
                if ( !in_invalid_word && ( i - word_start ) > MAX_WORD_LENGTH )
                {
                    in_invalid_word = true;
                }
            }
            if ( c == QChar( '&' ) )
            {
                in_entity = true;
            }
            if ( c == QChar( ';' ) )
            {
                in_entity = false;
            }
        }
        if ( c == QChar( '<' ) )
        {
            in_tag = true;
            word_start = -1;
        }
        if ( in_tag && c == QChar( '>' ) )
        {
            word_start = i + 1;
            in_tag = false;
        }
    }

    return misspellings;
}


bool HTMLSpellCheck::IsBoundary( QChar prev_c, QChar c, QChar next_c, bool in_entity )
{
    return c.isSpace() || c == QChar('<') || c == QChar('>') || c == QChar('&') || ( c == QChar(';') && in_entity) || (!c.isLetter() && ( !prev_c.isLetter() || !next_c.isLetter() ) );
}


QList< HTMLSpellCheck::MisspelledWord > HTMLSpellCheck::GetMisspelledWords( const QString &text )
{
    return GetMisspelledWords( text, 0, text.count(), "" );
}


HTMLSpellCheck::MisspelledWord HTMLSpellCheck::GetFirstMisspelledWord( const QString &text, 
                                                                       int start_offset, 
                                                                       int end_offset, 
                                                                       const QString &search_regex )
{
    QList< HTMLSpellCheck::MisspelledWord > misspelled_words = GetMisspelledWords( text, start_offset, end_offset, search_regex, true );
    HTMLSpellCheck::MisspelledWord misspelled_word;

    if ( !misspelled_words.isEmpty() )
    {
        misspelled_word = misspelled_words.first();
    }

    return misspelled_word;
}


HTMLSpellCheck::MisspelledWord HTMLSpellCheck::GetLastMisspelledWord( const QString &text, 
                                                                      int start_offset, 
                                                                      int end_offset, 
                                                                      const QString &search_regex )
{
    QList< HTMLSpellCheck::MisspelledWord > misspelled_words = GetMisspelledWords( text, start_offset, end_offset, search_regex );
    HTMLSpellCheck::MisspelledWord misspelled_word;

    if ( !misspelled_words.isEmpty() )
    {
        misspelled_word = misspelled_words.last();
    }

    return misspelled_word;
}


int HTMLSpellCheck::CountMisspelledWords( const QString &text, 
                                          int start_offset, 
                                          int end_offset, 
                                          const QString &search_regex )
{
    return GetMisspelledWords( text, start_offset, end_offset, search_regex ).count();
}


int HTMLSpellCheck::CountMisspelledWords( const QString &text )
{
    return CountMisspelledWords( text, 0, text.count(), "" );
}


