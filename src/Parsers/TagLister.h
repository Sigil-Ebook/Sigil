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

    struct AttInfo {
        int     pos;      // position of attribute relative to tag start
        int     len;      // length of attribute in tag
        int     vpos;     // position of attribute value relative to tag start
        int     vlen;     // length of attribute value
        QString aname;    // attribute name
        QString avalue;   // attribute value
    };


    TagLister();
    
    TagLister(const QString &source);
    ~TagLister() {};

    void reloadLister(const QString &source);

    const TagInfo& at(int i);
    size_t size();

    bool isPositionInBody(int pos);
    bool isPositionInTag(int pos);
    bool isPositionInOpenTag(int pos);
    bool isPositionInCloseTag(int pos);

    int findLastTagOnOrBefore(int pos);
    int findFirstTagOnOrAfter(int pos);
    int findOpenTagForClose(int i);
    int findCloseTagForOpen(int i);
    int findBodyOpenTag();
    int findBodyCloseTag();

    const QString& getSource();

    static void parseAttribute(const QStringRef &tagstring, const QString &attribute_name, AttInfo& ainfo);
    static QString serializeAttribute(const QString &aname, const QString &avalue);
    static QString extractAllAttributes(const QStringRef &tagstring);
    
private:
    TagInfo getNext();
    void  buildTagList();

    QStringRef parseML();

    void parseTag(const QStringRef &tagstring, TagInfo &mi);

    int findTarget(const QString &tgt, int p, bool after=false);
    static int skipAnyBlanks(const QStringRef &segment, int p);
    static int stopWhenContains(const QStringRef &segment, const QString& stopchars, int p);
    
    QString        m_source;
    int            m_pos;
    int            m_next;
    QStringList    m_TagPath;
    QList<int>     m_TagPos;
    QList<int>     m_TagLen;
    QList<TagInfo> m_Tags;
    int            m_bodyStartPos;
    int            m_bodyEndPos;
    int            m_bodyOpenTag;
    int            m_bodyCloseTag;
};

#endif
