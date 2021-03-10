/************************************************************************
**
**  Copyright (C) 2020-2021 Kevin B. Hendricks, Stratford Ontario
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
#include <QDebug>

#include "Misc/Utility.h"
#include "Parsers/TagLister.h"


// public interface

// Default Constructor
TagLister::TagLister()
    : m_source(""),
      m_pos(0),
      m_next(0),
      m_bodyStartPos(-1),
      m_bodyEndPos(-1),
      m_bodyOpenTag(-1),
      m_bodyCloseTag(-1)
{
    m_TagPath << "root";
    m_TagPos << -1;
    m_TagLen << 0;
}

// Normal Constructor
TagLister::TagLister(const QString &source)
    : m_source(source),
      m_pos(0),
      m_next(0)
{
    m_TagPath << "root";
    m_TagPos << -1;
    m_TagLen << 0;
    buildTagList();
}


void TagLister::reloadLister(const QString& source)
{
    m_source = source;
    m_pos = 0;
    m_next = 0;
    m_TagPath = QStringList() << "root";
    m_TagPos = QList<int>() << -1;
    m_TagLen = QList<int>() << 0;
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
    TagLister::TagInfo ti = at(i);
    while((ti.pos <= pos) && (ti.len != -1)) {
        i++;
        ti = m_Tags.at(i);
    }
    i--;
    return i;
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
void TagLister::parseAttribute(const QStringRef &tagstring, const QString &attribute_name, AttInfo &ainfo)
{
    int taglen = tagstring.length();
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
    if (tagstring.at(p) == "/") return; // end tag has no attributes
    // int s = p;
    p = stopWhenContains(tagstring, ">/ \f\t\r\n", p);
    // QString tagname = Utility::Substring(s, p, tagstring).trimmed();

    // handle the possibility of attributes (so begin or single tag type)
    while (tagstring.indexOf("=", p) != -1) {
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
        if ((tagstring.at(p) == "'") || (tagstring.at(p) == "\"")) {
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
QString TagLister::extractAllAttributes(const QStringRef &tagstring)
{
    int taglen = tagstring.length();
    QChar c = tagstring.at(1);
    int p = 0;

    // ignore comments, doctypes, cdata, pi, and xmlheaders
    if ((c == '?') || (c == '!')) return QString();
    // normal tag, skip over any blanks before tag name
    p = skipAnyBlanks(tagstring, 1);

    if (tagstring.at(p) == "/") return QString(); // end tag has no attributes

    // skip over tag name itself
    p = stopWhenContains(tagstring, ">/ \f\t\r\n", p);

    // skip any leading blanks before first attribute or tag end
    p = skipAnyBlanks(tagstring, p);

    // if any attributes exist
    // Note: xml/xhtml does not support boolean attribute values without =)
    if (tagstring.indexOf("=", p) == -1) return QString();
    // properly handle both begin and single tags
    QString res = tagstring.mid(p, taglen - 1 - p).toString(); // skip ending '>'
    res = res.trimmed();
    if (res.endsWith("/")) res = res.mid(0, res.length() - 1);
    res = res.trimmed();
    return res;
}


// private routines

TagLister::TagInfo TagLister::getNext()
{
    TagInfo mi;
    mi.pos = -1;
    mi.len = -1;
    mi.open_pos = -1;
    mi.open_len = -1;
    QStringRef markup = parseML();
    while (!markup.isNull()) {
        if ((markup.at(0) == "<") && (markup.at(markup.size() - 1) == ">")) {
            mi.pos = m_pos;
            parseTag(markup, mi);
            if (mi.ttype == "begin") {
                m_TagPath << mi.tname;
                m_TagPos << mi.pos;
                m_TagLen << mi.len;
            } else if (mi.ttype == "end") {
                QString tname = m_TagPath.last();
                if (tname == mi.tname) {
                    m_TagPath.removeLast();
                    mi.open_pos = m_TagPos.takeLast();
                    mi.open_len = m_TagLen.takeLast();
                } else {
                    qDebug() << "TagLister Error: Not well formed -  open close mismatch: ";
                    qDebug() << "   open Tag: " << tname << " at position: " << m_TagPos.last();
                    qDebug() << "   close Tag: " << mi.tname << " at position: " << mi.pos;
                    mi.open_pos = -1;
                    mi.open_len = -1;
                }
            }
            mi.tpath = m_TagPath.join(".");
            return mi;
        }
        // skip anything not a tag
        markup = parseML();
    }
    // done
    return mi;
}


QStringRef TagLister::parseML()
{
    int p = m_next;
    m_pos = p;
    if (p >= m_source.length()) return QStringRef();
    if (m_source.at(p) != "<") {
        // we have text leading up to a tag start
        m_next = findTarget("<", p+1);
        return Utility::SubstringRef(m_pos, m_next, m_source);
    }
    // we have a tag or special case
    // handle special cases first
    QString tstart = Utility::Substring(p, p+9, m_source);
    if (tstart.startsWith("<!--")) {
        // include ending > as part of the string
        m_next = findTarget("-->", p+4, true);
        return Utility::SubstringRef(m_pos, m_next, m_source);
    }
    if (tstart.startsWith("<![CDATA[")) {
        // include ending > as part of the string
        m_next = findTarget("]]>", p+9, true);
        return Utility::SubstringRef(m_pos, m_next, m_source);
    }
    // include ending > as part of the string
    m_next = findTarget(">", p+1, true);
    
    int ntb = findTarget("<", p+1);
    if ((ntb != -1) && (ntb < m_next)) {
        m_next = ntb;
    }
    return Utility::SubstringRef(m_pos, m_next, m_source);
}


void TagLister::parseTag(const QStringRef& tagstring, TagLister::TagInfo& mi)
{
    mi.len = tagstring.length();
    QChar c = tagstring.at(1);
    int p = 0;
    
    // first handle special cases
    if (c == '?') {
        if (tagstring.startsWith("<?xml")) {
            mi.tname = "?xml";
            mi.ttype = "xmlheader";
        } else {
            mi.tname = "?";
            mi.ttype = "pi";
        }
        return;
    }
    if (c == '!') {
        if (tagstring.startsWith("<!--")) {
            mi.tname = "!--";
            mi.ttype = "comment"; 
        } else if (tagstring.startsWith("<!DOCTYPE") || tagstring.startsWith("<!doctype")) {
            mi.tname = "!DOCTYPE";
            mi.ttype = "doctype";
        } else if (tagstring.startsWith("<![CDATA[") || tagstring.startsWith("<![cdata[")) {
            mi.tname = "![CDATA[";
            mi.ttype = "cdata";
        }
        return;
    }

    // normal tag, extract tag name
    p = skipAnyBlanks(tagstring, 1);
    if (tagstring.at(p) == "/") {
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
        if (tagstring.endsWith("/>") || tagstring.endsWith("/ >")) mi.ttype = "single";
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


int TagLister::skipAnyBlanks(const QStringRef &tgt, int p)
{
    while((p < tgt.length()) && (tgt.at(p) == " ")) p++;
    return p;
}


int TagLister::stopWhenContains(const QStringRef &tgt, const QString& stopchars, int p)
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
