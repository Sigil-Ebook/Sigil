/************************************************************************
**
**  Copyright (C) 2020-2021 Kevin B. Hendricks Stratford, ON, Canada 
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

#ifndef QUICK_PARSER
#define QUICK_PARSER

#include "Parsers/TagAtts.h"

class TagAtts;
class QString;
class QStringRef;
class QStringList;
 
class QuickParser
{
public:

    struct MarkupInfo {
        int     pos;
        QString text;
        QString lang;
        QString tpath;
        QString tname;
        QString ttype;
        TagAtts tattr;
    };

    QuickParser(const QString &source, const QString default_lang = "en");
    ~QuickParser() {};
    void reload_parser(const QString &source, const QString default_lang = "en");
    MarkupInfo parse_next();
    QString serialize_markup(const MarkupInfo &mi);
    
private:
    QStringRef parseML();
    void parseTag(const QStringRef &tagstring, MarkupInfo &mi);
    int findTarget(const QString &tgt, int p, bool after=false);
    int skipAnyBlanks(const QStringRef &segment, int p);
    int stopWhenContains(const QStringRef &segment, const QString& stopchars, int p);
    
    QString      m_source;
    int          m_pos;
    int          m_next;
    QStringList  m_TagPath;
    QStringList  m_LangPath;
};

#endif
