/************************************************************************
**
**  Copyright (C) 2011  John Schember <john@nachtimwald.com>
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
#ifndef SPCRE_H
#define SPCRE_H

#include <QList>
#include <QString>
#include "pcre.h"
#include <utility>

using std::pair;

/**
 * Sigil Regular Expression object.
 *
 * Used to find matches within a string and create replacements.
 *
 * This class is a wrapper for the PCRE C library. The C API is used instead
 * of the C++ API because the C++ API does not return offsets within the matched
 * String.
 */
class SPCRE
{
public:
    /**
     * Constructor.
     *
     * @param pattern The search pattern.
     */
    SPCRE(const QString &patten);
    ~SPCRE();

    /**
     * Defines an offset in a string of matching instances. Also, defines
     * offsets within the matched string that relate to captured patterns from
     * the matched pattern. The offset of the captured patterns is relative
     * to the matched substring.
     */
    struct MatchInfo {
        // Offset within the full text where this match occurs.
        std::pair<int, int> offset;
        // Each offset representes the porition of text that the capture
        // represents inside of the matched string. This is normalized so that
        // 0 represents the start of the string represented by offset.
        QList<std::pair<int, int> > capture_groups_offsets;

        MatchInfo() {
            offset.first = -1;
            offset.second = -1;
        }
    };

    /**
     * Is the pattern valid.
     *
     * @return True if the pattern in valid.
     */
    bool isValid();

    /**
     * The string that was used to create the regular expression.
     *
     * @return The pattern string that was given in the constructor.
     */
    QString getPattern();
    /**
     * The compiled PCRE pattern created from the string pattern given in the
     * constructor.
     *
     * @return The compiled pattern.
     */
    pcre *getCompiledPattern();
    /**
     * The result of the study of the compiled pattern.
     *
     * @return The study result.
     */
    pcre_extra *getStudy();
    /**
     * The total number of capture subpatterns within the pattern.
     *
     * @return The total number of capture subpatterns referenced within the
     * pattern.
     */
    int getCaptureSubpatternCount();
    /**
     * Convert a named capture group to its absolute numbered group equivelent.
     *
     * @param name The named catpure group.
     * @return The absolute numbered group represented by the name. -1 if the
     * named group does not exist within the pattern.
     */
    int getCaptureStringNumber(const QString &name);

    /**
     * Generate match information from a segment of text. Finds all matching
     * instances of pattern within the given text.
     *
     * @param text The text to get matching information from.
     *
     * @return A list of MatchInfo objects.
     */
    QList<MatchInfo> getEveryMatchInfo(const QString &text);
    MatchInfo getFirstMatchInfo(const QString &text);
    MatchInfo getLastMatchInfo(const QString &text);

    /**
     * Replaces the given text using a replacement pattern. The matched text is
     * required because the replacement pattern can references the capture
     * groups from within the matched text.
     *
     * @param text The matched text.
     * @param capture_group_offsets Offsets within the string that represents
     * the capture groups.
     * @param replacement_pattern The pattern / text to use to create the
     * final replacement text.
     * @param[out] out The final replacement string.
     *
     * @return true if the replacement string was created successfully.
     */
    bool replaceText(const QString &text, const QList<std::pair<int, int> > &capture_groups_offsets, const QString &replacement_pattern, QString &out);

private:
    MatchInfo generateMatchInfo(const QString &text, int ovector[], int ovector_count);

    // Store if the pattern is valid.
    bool m_valid;
    // The regular expression as a string.
    QString m_pattern;
    // The compiled regular expression.
    pcre *m_re;
    // The result of a study of the pcre.
    pcre_extra *m_study;
    // The number of capture subpatterns with the expression.
    int m_captureSubpatternCount;
};

#endif // SPCRE_H
