/************************************************************************
**
**  Copyright (C) 2025 Kevin B. Hendricks, Stratford Ontario Canada
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
#ifndef SEARCHUTILS_H
#define SEARCHUTILS_H

#include <utility>
#include <QString>
#include <QList>
#include "PCRE2/SPCRE.h"

class SearchUtils
{

public:


    static QList<std::pair<int,int> > UTF16to32_PositionTable(const QString& text);


    static int ConvertUTF16Posto32(const QList<std::pair<int,int> > &table, int pos); 


    static SPCRE::MatchInfo ConvertMatchInfotoUTF32(const QString& text,
                                                    const SPCRE::MatchInfo& mi);

    static QList<std::pair<int, int> > ConvertCaptureGroupstoUTF32(const QString& text,
                                                                   const QList<std::pair<int, int> > &cgs);


};
#endif // SEARCHUTILS_H

