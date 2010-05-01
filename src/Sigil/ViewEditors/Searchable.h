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
class QStringList;


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
        Direction_Down, /**< Search from the caret point downwards. */
        Direction_All,  /**< Search from the caret point down, then wrap around. */
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
    virtual bool FindNext( const QRegExp &search_regex, Direction search_direction ) = 0;

    /**
     * Returns the number of matching occurrences.
     *
     * @param search_regex The regex to match with.
     * @return The number of matching occurrences.
     */
    virtual int Count( const QRegExp &search_regex ) = 0;

    /**
     * If the currently selected text matches the specified regex, 
     * it is replaced by the specified replacement string.
     *
     * @param search_regex The regex to match with. 
     * @param replacement The text with which to replace the matched string.
     * @return \c true if the searched term was successfully replaced.
     */
    virtual bool ReplaceSelected( const QRegExp &search_regex, const QString &replacement ) = 0;

    /**
     * Replaces all occurrences of the specified regex.
     *
     * @param search_regex The regex to match with. 
     * @param replacement The text with which to replace the matched string.
     * @return The number of performed replacements.
     */
    virtual int ReplaceAll( const QRegExp &search_regex, const QString &replacement ) = 0;

protected:

    /**
     * Runs the regex through the text.
     *
     * @param[in,out] search_regex The regex to match with. IT IS MODIFIED IN PLACE!
     * @param full_text The text through which the search regex will be run.
     * @param selection_offset The offset from which the search starts.
     * @param search_direction The direction of the search.
     */
    static void RunSearchRegex( QRegExp &search_regex, 
                                const QString &full_text, 
                                int selection_offset, 
                                Searchable::Direction search_direction );

    /**
     * Fills the replacement text with the captured groups.
     * Accepts a list of text capture groups and a replacement string,
     * and returns a new replacement string with capture group references
     * replaced with capture group contents.
     * 
     * @param captured_texts The list of captured texts.
     * @param replacement The replacement string that potentially 
     *                    has capture group references.
     * @return The replacement string with the capture group references replaced
     *         with their actual values.
     */
    static QString FillWithCapturedTexts( const QStringList &captured_texts, 
                                          const QString &replacement );
};

#endif // SEARCHABLE_H