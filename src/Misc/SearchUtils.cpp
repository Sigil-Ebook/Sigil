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

#include <QDebug>
#include "sigil_constants.h"
#include "Misc/SearchUtils.h"


QList<std::pair<int,int> > SearchUtils::UTF16to32_PositionTable(const QString& text)
{
    QList<std::pair<int,int> > low_surrogates;
    int pos = 0;
    int count = 0;
    for (int i = 0; i < text.size(); i++) {
         QChar c = text.at(i);
         if (c.isLowSurrogate()) {
         std:pair<int, int> si;
             count += 1;
             si.first = pos;
             si.second = count;
             low_surrogates << si;             
         }
         pos += 1;
    }
    return low_surrogates;
}


int SearchUtils::ConvertUTF16Posto32(const QList<std::pair<int,int> > &table, int pos) 
{
    if (table.isEmpty()) return pos;  // no table
    if (pos < table.at(0).first) return pos; // before table
    if (pos >= table.last().first) return pos - table.last().second; // last or after table
    // pos lies in table someplace
    int i = 0;
    while (i < table.size() - 1) {
         if ((pos >= table.at(i).first) && (pos < table.at(i+1).first)) return pos - table.at(i).second;
         i++;
    }
    // should never reach here
    return pos;
}


SPCRE::MatchInfo SearchUtils::ConvertMatchInfotoUTF32(const QString& text,
                                                      const SPCRE::MatchInfo& mi)
{
    SPCRE::MatchInfo nm;
    int start = mi.offset.first;
    int end = mi.offset.second;
    if (start == -1) return nm;
    QList<std::pair<int, int>> table = UTF16to32_PositionTable(text);
    nm.offset.first = ConvertUTF16Posto32(table, start);
    nm.offset.second = ConvertUTF16Posto32(table, end);
    nm.capture_groups_offsets.append(std::pair<int, int>(0, nm.offset.second - nm.offset.first));
    int capture_pattern_count = mi.capture_groups_offsets.size();
    for (int i = 1; i < capture_pattern_count; i++) {
        int old_full_start = mi.capture_groups_offsets.at(i).first + start;
        int old_full_end = mi.capture_groups_offsets.at(i).second + start;
        int new_start = ConvertUTF16Posto32(table, old_full_start) - nm.offset.first;
        int new_end = ConvertUTF16Posto32(table, old_full_end) - nm.offset.first;
        nm.capture_groups_offsets.append(std::pair<int,int>(new_start, new_end));
    }
    return nm;
}


QList<std::pair<int, int> > SearchUtils::ConvertCaptureGroupstoUTF32(const QString& text,
                                                                    const QList<std::pair<int, int> > &cgs) 
{
    // text was found and selected by previous search so the match always starts at 0
    QList<std::pair<int, int>> ncgs;
    QList<std::pair<int, int>> table = UTF16to32_PositionTable(text);
    for (int i = 0; i < cgs.size(); i++) {
        int new_start = ConvertUTF16Posto32(table, cgs.at(i).first);
        int new_end = ConvertUTF16Posto32(table, cgs.at(i).second);
        ncgs.append(std::pair<int,int>(new_start, new_end));
    }
    return ncgs;
}
