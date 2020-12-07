/************************************************************************
**
**  Copyright (C) 2020 Kevin B. Hendricks Stratford, ON, Canada 
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

#ifndef TAG_LISTER
#define TAG_LISTER

#include <QList>

class QString;
class QStringRef;
class QStringList;

class TagLister
{
public:

    struct TagInfo {
        int     pos;      // position of tag in source
        int     len;      // length of tag in source
        QString tpath;    // path of tag names to this tag ("." joined) 
        QString tname;    // tag name, ?xml, ?, !--, !DOCTYPE, ![CDATA[
        QString ttype;    // xmlheader, pi, comment, doctype, cdata, begin, single, end
        int     open_pos; // set if end tag to position of its corresponding begin tag
        int     open_len; // set if end tag to length of its corresponding begin tag
    };

    TagLister(const QString &source);
    ~TagLister() {};
    void reload_lister(const QString &source);
    TagInfo get_next();
    
private:
    QStringRef parseML();
    void parseTag(const QStringRef &tagstring, TagInfo &mi);
    int findTarget(const QString &tgt, int p, bool after=false);
    int skipAnyBlanks(const QStringRef &segment, int p);
    int stopWhenContains(const QStringRef &segment, const QString& stopchars, int p);
    
    QString      m_source;
    int          m_pos;
    int          m_next;
    QStringList  m_TagPath;
    QList<int>   m_TagPos;
    QList<int>   m_TagLen;
};

#endif
