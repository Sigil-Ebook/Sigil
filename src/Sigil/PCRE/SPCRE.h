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

class SPCRE
{
public:
    SPCRE(const QString &patten);
    ~SPCRE();

    bool isValid();

    QString getPattern();
    pcre *getCompiledPattern();
    pcre_extra *getStudy();
    int getCaptureSubpatternCount();

    QList<std::pair<int, int> > getMatchOffsets(const QString &text);
    bool replaceText(const QString &text, const QString &replacement_pattern, QString &out);

private:
    bool m_valid;
    QString m_pattern;
    pcre *m_re;
    pcre_extra *m_study;
    int m_captureSubpatternCount;
};

#endif // SPCRE_H
