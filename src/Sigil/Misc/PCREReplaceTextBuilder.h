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

class QChar;

class PCREReplaceTextBuilder
{
public:
    PCREReplaceTextBuilder();

    bool BuildReplacementText(const QString &search_regex,
                              const QString &matched_text,
                              const QString &replacement_pattern,
                              QString &replacement_text);

private:
    enum CaseChange {
        CaseChange_LowerNext,
        CaseChange_Lower,
        CaseChange_UpperNext,
        CaseChange_Upper,
        CaseChange_None
    };

    void resetState();

    void accumulateReplcementText(const QChar &ch);
    void accumulateReplcementText(const QString &text);

    void processTextSegement(const QChar &ch);
    void processTextSegement(const QString &text);

    void trySetCaseChange(CaseChange state);

    // Vairables
    QString m_finalText;

    bool m_inControl;
    bool m_inBackref;

    CaseChange m_caseChangeState;
};

#endif // PCREREPLACETEXTBUILDER_H
