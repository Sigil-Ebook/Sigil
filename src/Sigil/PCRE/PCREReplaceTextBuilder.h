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

class PCREReplaceTextBuilder
{
public:
    PCREReplaceTextBuilder();

    bool BuildReplacementText(SPCRE &sre,
                              const QString &text,
                              const QList<std::pair<int, int> > &capture_groups_offsets,
                              const QString &replacement_pattern,
                              QString &out);

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

    QString processTextSegement(const QChar &ch);
    QString processTextSegement(const QString &text);

    void trySetCaseChange(CaseChange state);

    // Vairables
    CaseChange m_caseChangeState;
    QString m_finalText;
};

#endif // PCREREPLACETEXTBUILDER_H
