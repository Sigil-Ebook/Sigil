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

#pragma once
#ifndef SEARCHABLE_H
#define SEARCHABLE_H

#include "PCRE/SPCRE.h"

class QString;
class QStringList;
class QWidget;


/**
 * The interface for searchable content.
 * Provides methods for searching, replacing
 * and counting search term occurrences.
 */
class Searchable
{
public:

    /**
     * The search direction.
     */
    enum Direction
    {
        Direction_Up,   /**< Search from the caret point upwards. */
        Direction_Down /**< Search from the caret point downwards. */
    };

    /**
     * Destructor.
     */
    virtual ~Searchable() {}

    /**
     * Finds the next occurrence of the search term in the document.
     * The matched string is selected.
     *
     * @param search_regex The regex to match with.
     * @param search_direction The direction of the search.
     * @return \c true if the term is found.
     */
    virtual bool FindNext( const QString &search_regex,
                           Direction search_direction,
                           bool misspelled_words = false,
                           bool ignore_selection_offset = false,
                           bool wrap = true ) = 0;

    /**
     * Returns the number of matching occurrences.
     *
     * @param search_regex The regex to match with.
     * @return The number of matching occurrences.
     */
    virtual int Count( const QString &search_regex ) = 0;

    /**
     * If the currently selected text matches the specified regex,
     * it is replaced by the specified replacement string.
     *
     * @param search_regex The regex to match with.
     * @param replacement The text with which to replace the matched string.
     * @return \c true if the searched term was successfully replaced.
     */
    virtual bool ReplaceSelected( const QString &search_regex, const QString &replacement, Searchable::Direction direction=Searchable::Direction_Down, bool keep_selection = false) = 0;

    /**
     * Replaces all occurrences of the specified regex.
     *
     * @param search_regex The regex to match with.
     * @param replacement The text with which to replace the matched string.
     * @return The number of performed replacements.
     */
    virtual int ReplaceAll( const QString &search_regex, const QString &replacement ) = 0;

    /**
     * Returns the currently selected text string.
     *
     * @return The currently selected text string.
     */
    virtual QString GetSelectedText() = 0;

    virtual void SetUpFindForSelectedText( const QString &search_regex ) = 0;

    static void ShowWrapIndicator( QWidget *parent );
};

#endif // SEARCHABLE_H
