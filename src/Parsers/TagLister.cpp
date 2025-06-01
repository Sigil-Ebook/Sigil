/************************************************************************
**
**  Copyright (C) 2020-2025 Kevin B. Hendricks, Stratford Ontario
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

#include <QChar>
#include <QString>
#include <QStringList>
#include <QList>
#include <QStringView>
#include <QDebug>

#include "Misc/Utility.h"
#include "Parsers/TagLister.h"
#include "sigil_constants.h"

const QString WHITESPACE_CHARS=" \t\n\r";  // valid in pure xml

// public interface

// Default Constructor
TagLister::TagLister()
    : m_source(""),
      m_pos(0),
      m_next(0),
      m_child(-1),
      m_bodyStartPos(-1),
      m_bodyEndPos(-1),
      m_bodyOpenTag(-1),
      m_bodyCloseTag(-1)
{
    m_TagPath << "root";
    m_TagPos << -1;
    m_TagLen << 0;
    m_TagChild << -1;
}

// Normal Constructor
TagLister::TagLister(const QString &source)
    : m_source(source),
      m_pos(0),
      m_next(0),
      m_child(-1)
{
    m_TagPath << "root";
    m_TagPos << -1;
    m_TagLen << 0;
    m_TagChild << -1;
    buildTagList();
}


void TagLister::reloadLister(const QString& source)
{
    m_source = source;
    m_pos = 0;
    m_next = 0;
    m_child = -1;
    m_TagPath = QStringList() << "root";
    m_TagPos = QList<int>() << -1;
    m_TagLen = QList<int>() << 0;
    m_TagChild = QList<int>() << -1;
    buildTagList();
}

const TagLister::TagInfo& TagLister::at(int i)
{
    if ((i < 0) || (i >= m_Tags.size())) {
        i = m_Tags.size() - 1; // last entry in list is a dummy entry
    }
    return m_Tags.at(i);
}


size_t TagLister::size() { return m_Tags.size(); }


const QString& TagLister::getSource() { return m_source; }

bool TagLister::isPositionInBody(int pos)
{
    if ((pos < m_bodyStartPos) || (pos > m_bodyEndPos)) {
        return false;
    }
    return true;
}

bool TagLister::isPositionInTag(int pos)
{
    int i = findFirstTagOnOrAfter(pos);
    TagLister::TagInfo ti = m_Tags.at(i);
    if ((pos >= ti.pos) && (pos < ti.pos + ti.len)) {
        return true;
    }
    return false;
}

bool TagLister::isPositionInOpenTag(int pos)
{
    int i = findFirstTagOnOrAfter(pos);
    TagLister::TagInfo ti = m_Tags.at(i);
    if ((pos >= ti.pos) && (pos < ti.pos + ti.len)) {
        if ((ti.ttype == "begin") || (ti.ttype == "single")) return true;
    }
    return false;
}

bool TagLister::isPositionInCloseTag(int pos)
{
    int i = findFirstTagOnOrAfter(pos);
    TagLister::TagInfo ti = m_Tags.at(i);
    if ((pos >= ti.pos) && (pos < ti.pos + ti.len)) {
        if (ti.ttype == "end") return true;
    }
    return false;
}


int TagLister::findOpenTagForClose(int i)
{
    if ((i < 0) || (i >= m_Tags.size())) return -1;
    TagLister::TagInfo ti = m_Tags.at(i);
    if (ti.ttype != "end") return -1;
    int open_pos = ti.open_pos;
    for (int j=i-1; j >= 0; j--) {
        TagInfo tb = m_Tags.at(j);
        if (tb.pos == open_pos) return j;
    }
    return -1;
}

int TagLister::findCloseTagForOpen(int i)
{
    if ((i < 0) || (i >= m_Tags.size())) return -1;
    TagLister::TagInfo ti = m_Tags.at(i);
    if (ti.ttype != "begin") return -1;
    int open_pos = ti.pos;
    for (int j=i+1; j < m_Tags.size(); j++) {
        TagInfo te = m_Tags.at(j);
        if (te.open_pos == open_pos) return j;
    }
    return -1;
}

// There may not be one here if no tags exists because
// the front of m_Tags is not padded with a dummy tag
// so this can return -1 meaning none exists
int TagLister::findLastTagOnOrBefore(int pos)
{
    // find that tag that starts immediately **after** pos and then
    // then use its predecessor 
    int i = 0;
    TagLister::TagInfo ti = m_Tags.at(i);
    while((ti.pos <= pos) && (ti.len != -1)) {
        i++;
        ti = m_Tags.at(i);
    }
    i--;
    return i;
}

// this routine can return -1 meaning none exists
// but both the body and html tags exist this should not happen
int TagLister::findLastOpenOrSingleTagThatContainsYou(int pos)
{
    // to sync to Preview the position must be inside the body tag someplace
    int bpos = pos;
    if (bpos > m_bodyEndPos) bpos = m_bodyEndPos;
    if (bpos < m_bodyStartPos) bpos = m_bodyStartPos;

    int k = findLastTagOnOrBefore(bpos);
    TagLister::TagInfo ti = m_Tags.at(k);

    // test if it contains you
    // if bpos inside a single tag use it
    if (ti.ttype == "single") {
        if ((bpos >= ti.pos) && (bpos < ti.pos + ti.len)) return k;
    }

    // if bpos inside a begin tag and a child of it, use it
    if (ti.ttype == "begin") {
        int ci  = findCloseTagForOpen(k);
        if (ci != -1) {
            TagLister::TagInfo cls = m_Tags.at(ci);
            if ((bpos >= ti.pos) && (bpos < (cls.pos + cls.len))) return k;
        }
    }

    // ow. start search at last tag on or before and stop for closest single or begin tag
    int i = k;
    bool found = false;
    while ((i >= 0) && !found) {
        TagLister::TagInfo ti = m_Tags.at(i);
        if (ti.ttype == "single") {
            found = true;
        }
        if (ti.ttype == "begin") {
            found = true;
        }
        // if not found try the preceding tag
        if (!found) i = i - 1;
    }
    if (!found) return -1;
    return i;
}

// this routine can return -1 meaning none exists
// but both the body and html tags exist so this should never happen
int TagLister::findLastOpenTagOnOrBefore(int pos)
{
    // to sync to Preview the position must be inside the body tag someplace
    int bpos = pos;
    if (bpos >= m_bodyEndPos) bpos = m_bodyEndPos;
    if (bpos <= m_bodyStartPos) bpos = m_bodyStartPos;

    int i = findLastTagOnOrBefore(bpos);
    bool found = false;
    while ((i >= 0) && !found) {
        if (m_Tags.at(i).ttype == "begin") found = true;
        if (!found) i = i - 1;
    }
    if (!found) {
        return -1;
    }
    return i;
}

QString TagLister::GeneratePathToTag(int pos)
{
    int i = findLastOpenOrSingleTagThatContainsYou(pos);
    // int i = findLastOpenTagOnOrBefore(pos);
    if (i < 0) return "html -1";
    TagInfo ti = m_Tags.at(i);
    return ti.tpath;
}

// m_Tags is padded with an ending dummy tag
// So finding first tag on or after a pos will always work
int TagLister::findFirstTagOnOrAfter(int pos)
{
    int i = 0;
    TagLister::TagInfo ti = m_Tags.at(i);
    while((ti.pos + ti.len <= pos) && (ti.len != -1)) {
        i++;
        ti = m_Tags.at(i);
    }
    return i;
}


int TagLister::findBodyOpenTag() { return m_bodyOpenTag; }

int TagLister::findBodyCloseTag() { return m_bodyCloseTag; }


// static
QString TagLister::serializeAttribute(const QString& aname, const QString &avalue)
{
    QString qc = "\"";
    if (avalue.contains("\"")) qc = "'";
    QString res = aname + "=" + qc + avalue + qc;
    return res;
}

// static
void TagLister::parseAttribute(const QStringView tagstring, const QString &attribute_name, AttInfo &ainfo)
{
    QChar c = tagstring.at(1);
    int p = 0;

    ainfo.pos = -1;
    ainfo.len = -1;
    ainfo.vpos = -1;
    ainfo.vlen = -1;
    ainfo.aname = QString();
    ainfo.avalue = QString();

    // ignore comments, doctypes, cdata, pi, and xmlheaders
    if ((c == '?') || (c == '!')) return;

    // normal tag, skip over tag name
    p = skipAnyBlanks(tagstring, 1);
    if (tagstring.at(p) == '/') return; // end tag has no attributes
    // int s = p;
    p = stopWhenContains(tagstring, ">/ \f\t\r\n", p);
    // QString tagname = Utility::Substring(s, p, tagstring).trimmed();

    // handle the possibility of attributes (so begin or single tag type)
    while (tagstring.indexOf(QChar('='), p) != -1) {
        p = skipAnyBlanks(tagstring, p);
        int s = p;
        p = stopWhenContains(tagstring, "=", p);
        QString aname = Utility::Substring(s, p, tagstring).trimmed();
        if (aname == attribute_name) {
            ainfo.pos = s;
            ainfo.aname = aname;
        }
        QString avalue;
        p++;
        p = skipAnyBlanks(tagstring, p);
        if ((tagstring.at(p) == '\'') || (tagstring.at(p) == '"')) {
            QString qc = tagstring.at(p);
            p++;
            int b = p;
            p = stopWhenContains(tagstring, qc, p);
            avalue = Utility::Substring(b, p, tagstring);
            if (aname == attribute_name) {
                ainfo.avalue = avalue;
                ainfo.len = p - s + 1;
                ainfo.vpos = b;
                ainfo.vlen = p - b;
            }
            p++;
        } else {
            int b = p;
            p = stopWhenContains(tagstring, ">/ ", p);
            avalue = Utility::Substring(b, p, tagstring);
            if (aname == attribute_name) {
                ainfo.avalue = avalue;
                ainfo.len = p - s;
                ainfo.vpos = b;
                ainfo.vlen = p - b;
            }
        }
    }
    return;
}

//static
// extracts a copy of all attributes if any exist o.w. returns empty string
QString TagLister::extractAllAttributes(const QStringView tagstring)
{
    int taglen = tagstring.length();
    QChar c = tagstring.at(1);
    int p = 0;

    // ignore comments, doctypes, cdata, pi, and xmlheaders
    if ((c == '?') || (c == '!')) return QString();
    // normal tag, skip over any blanks before tag name
    p = skipAnyBlanks(tagstring, 1);

    if (tagstring.at(p) == '/') return QString(); // end tag has no attributes

    // skip over tag name itself
    p = stopWhenContains(tagstring, ">/ \f\t\r\n", p);

    // skip any leading blanks before first attribute or tag end
    p = skipAnyBlanks(tagstring, p);

    // if any attributes exist
    // Note: xml/xhtml does not support boolean attribute values without =)
    if (tagstring.indexOf(QChar('='), p) == -1) return QString();
    // properly handle both begin and single tags
    QString res = tagstring.mid(p, taglen - 1 - p).toString(); // skip ending '>'
    res = res.trimmed();
    if (res.endsWith(QChar('/'))) res = res.mid(0, res.length() - 1);
    res = res.trimmed();
    return res;
}


// private routines

QString TagLister::makePathToTag()
{
    int i = 1; // skip over root
    QStringList tagpath;
    while (i < m_TagPath.size()) {
        int child_index = -1;
        if (i+1 < m_TagPath.size()) child_index = m_TagChild.at(i+1);
        tagpath << m_TagPath.at(i) + " " + QString::number(child_index);
        i = i + 1;
    }
    return tagpath.join(",");
}

TagLister::TagInfo TagLister::getNext()
{
    TagInfo mi;
    mi.pos = -1;
    mi.len = -1;
    mi.open_pos = -1;
    mi.open_len = -1;
    mi.child = -1;
    QStringView markup = parseML();
    while (!markup.isNull()) {
        if ((markup.at(0) == '<') && (markup.at(markup.size() - 1) == '>')) {
            mi.pos = m_pos;
            parseTag(markup, mi);
            if (mi.ttype == "begin") {
                m_TagPos << mi.pos;
                m_TagLen << mi.len;
                mi.child = ++m_child;
                m_TagChild << mi.child;
                m_child = -1;
                m_TagPath << mi.tname;
                mi.tpath = makePathToTag();

            } else if (mi.ttype == "single") {
                // for path purposes temporarily treat like open tag
                // until makePathToTag is calculated
                mi.child = ++m_child;
                m_TagChild << mi.child;
                m_TagPath << mi.tname;
                mi.tpath = makePathToTag();
                // then remove it from tagpath since single and has no children
                m_TagPath.removeLast();
                m_TagChild.removeLast();

            } else if (mi.ttype == "end") {
                QString pathnode = m_TagPath.last();
                if (pathnode.startsWith(mi.tname)) {
                    m_TagPath.removeLast();
                    mi.open_pos = m_TagPos.takeLast();
                    mi.open_len = m_TagLen.takeLast();
                    mi.child = m_TagChild.takeLast();
                    m_child = mi.child;
                } else {
                    qDebug() << "TagLister Error: Not well formed -  open close mismatch: ";
                    qDebug() << "   open Tag: " << pathnode << " at position: " << m_TagPos.last();
                    qDebug() << "   close Tag: " << mi.tname << " at position: " << mi.pos;
                    mi.open_pos = -1;
                    mi.open_len = -1;
                    mi.child = -1;
                }
                mi.tpath = makePathToTag();
            }
            return mi;
        }
        // skip anything not a tag
        markup = parseML();
    }
    // done
    return mi;
}


QStringView TagLister::parseML()
{
    int p = m_next;
    m_pos = p;
    if (p >= m_source.length()) return QStringView();
    if (m_source.at(p) != '<') {
        // we have text leading up to a tag start
        m_next = findTarget("<", p+1);
        return Utility::SubstringView(m_pos, m_next, m_source);
    }
    // we have a tag or special case
    // handle special cases first
    QString tstart = Utility::Substring(p, p+9, m_source);
    if (tstart.startsWith("<!--")) {
        // include ending > as part of the string
        m_next = findTarget("-->", p+4, true);
        return Utility::SubstringView(m_pos, m_next, m_source);
    }
    if (tstart.startsWith("<![CDATA[")) {
        // include ending > as part of the string
        m_next = findTarget("]]>", p+9, true);
        return Utility::SubstringView(m_pos, m_next, m_source);
    }
    // include ending > as part of the string
    m_next = findTarget(">", p+1, true);
    
    int ntb = findTarget("<", p+1);
    if ((ntb != -1) && (ntb < m_next)) {
        m_next = ntb;
    }
    return Utility::SubstringView(m_pos, m_next, m_source);
}


void TagLister::parseTag(const QStringView tagstring, TagLister::TagInfo& mi)
{
    mi.len = tagstring.length();
    QChar c = tagstring.at(1);
    int p = 0;
    
    // first handle special cases
    if (c == '?') {
        if (tagstring.startsWith(QL1SV("<?xml"))) {
            mi.tname = "?xml";
            mi.ttype = "xmlheader";
        } else {
            mi.tname = "?";
            mi.ttype = "pi";
        }
        return;
    }
    if (c == '!') {
        if (tagstring.startsWith(QL1SV("<!--"))) {
            mi.tname = "!--";
            mi.ttype = "comment"; 
        } else if (tagstring.startsWith(QL1SV("<!DOCTYPE")) || tagstring.startsWith(QL1SV("<!doctype"))) {
            mi.tname = "!DOCTYPE";
            mi.ttype = "doctype";
        } else if (tagstring.startsWith(QL1SV("<![CDATA[")) || tagstring.startsWith(QL1SV("<![cdata["))) {
            mi.tname = "![CDATA[";
            mi.ttype = "cdata";
        }
        return;
    }

    // normal tag, extract tag name
    p = skipAnyBlanks(tagstring, 1);
    if (tagstring.at(p) == '/') {
        mi.ttype = "end";
        p++;
        p = skipAnyBlanks(tagstring, p);
    };
    int b = p;
    p = stopWhenContains(tagstring, ">/ \f\t\r\n", p);
    mi.tname = Utility::Substring(b, p, tagstring);

    // fill in tag type
    if (mi.ttype.isEmpty()) {
        mi.ttype = "begin";
        if (tagstring.endsWith(QL1SV("/>")) || tagstring.endsWith(QL1SV("/ >"))) {
            mi.ttype = "single";
        }
    }
    return;
}


int TagLister::findTarget(const QString &tgt, int p, bool after)
{
    int nxt = m_source.indexOf(tgt, p);
    if (nxt == -1) return m_source.length();
    nxt = nxt + (tgt.length() -1);
    if (after) nxt++;
    return nxt;
}


int TagLister::skipAnyBlanks(const QStringView tgt, int p)
{
    while((p < tgt.length()) && (WHITESPACE_CHARS.contains(tgt.at(p)))) p++;
    return p;
}


int TagLister::stopWhenContains(const QStringView tgt, const QString& stopchars, int p)
{
    while((p < tgt.length()) && !stopchars.contains(tgt.at(p))) p++;
    return p;
}


void TagLister::buildTagList()
{
        m_Tags.clear();
        m_bodyStartPos = -1;
        m_bodyEndPos = -1;
        m_bodyOpenTag = -1;
        m_bodyCloseTag = -1;
        int i = 0;
        TagLister::TagInfo ti = getNext();
        while(ti.len != -1) {
            if ((ti.tname == "body") && (ti.ttype == "begin")) {
                m_bodyStartPos = ti.pos + ti.len;
                m_bodyOpenTag = i;
            }
            if ((ti.tname == "body") && (ti.ttype == "end")) {
                m_bodyEndPos = ti.pos - 1;
                m_bodyCloseTag = i;
            }
            TagLister::TagInfo temp = ti;
            m_Tags << temp;
            i++;
            ti = getNext();
        }
        // set stop indicator as last record
        TagLister::TagInfo temp;
        temp.pos = -1;
        temp.len = -1;
        temp.open_pos = -1;
        temp.open_len = -1;
        m_Tags << temp;
}
