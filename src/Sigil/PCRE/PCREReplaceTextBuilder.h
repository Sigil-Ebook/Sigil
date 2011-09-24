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
#ifndef PCREREPLACETEXTBUILDER_H
#define PCREREPLACETEXTBUILDER_H

#include <QString>
#include "SPCRE.h"

class QChar;

/**
 * Build replacement text from a given replacement pattern.
 */
class PCREReplaceTextBuilder
{
public:
    /**
     * Constructor.
     */
    PCREReplaceTextBuilder();

    /**
     * Generate replacement text.
     *
     * @param sre The SPCRE.
     * @param text The matched text we are using as a replacement.
     * @param capture_groups_offsets The offsets within the matched text
     * representing the captured subpatterns.
     * @param replacement_pattern The replacement pattern. Can be text or text
     * and control characters.
     * @param[out] out The string to store the repacement.
     *
     * @return True if replacement text is created.
     */
    bool BuildReplacementText(SPCRE &sre,
                              const QString &text,
                              const QList<std::pair<int, int> > &capture_groups_offsets,
                              const QString &replacement_pattern,
                              QString &out);

private:
    /**
     * The state of case changes.
     */
    enum CaseChange {
        CaseChange_LowerNext,
        CaseChange_Lower,
        CaseChange_UpperNext,
        CaseChange_Upper,
        CaseChange_None
    };

    /**
     * Reset the state of the replacement process.
     */
    void resetState();

    /**
     * Add to the final replacement text we are building.
     *
     * Processes by calling processTextSegement before adding.
     *
     * @param ch The character to add.
     */
    void accumulateReplcementText(const QChar &ch);
    /**
     * Add to the final replacement text we are building.
     *
     * Processes by calling processTextSegement before adding.
     *
     * @param text The text to add.
     */
    void accumulateReplcementText(const QString &text);

    /**
     * Processes the text segment making any changes necessary based upon
     * the state.
     *
     * @param ch The character to process.
     *
     * @return The processed and transformed text.
     */
    QString processTextSegement(const QChar &ch);
    /**
     * Processes the text segment making any changes necessary based upon
     * the state.
     *
     * @param text The text to process.
     *
     * @return The processed and transformed text.
     */
    QString processTextSegement(const QString &text);

    /**
     * Change the case state if possible.
     *
     * Case changes can only occur if no case change is currently active.
     * This will only change the case change state if it is not already set.
     *
     * @param state The state to set if possible.
     */
    void trySetCaseChange(CaseChange state);

    // Case change state.
    CaseChange m_caseChangeState;
    // The final replacement text.
    QString m_finalText;
};

#endif // PCREREPLACETEXTBUILDER_H
