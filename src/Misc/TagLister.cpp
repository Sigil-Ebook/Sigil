/************************************************************************
**
**  Copyright (C) 2020  Kevin B. Hendricks, Stratford Ontario
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
#include "Misc/TagLister.h"

TagLister::TagLister(const QString &source)
    : m_source(source),
      m_pos(0),
      m_next(0)
{
    m_TagPath << "root";
    m_TagPos << -1;
    m_TagLen << 0;
}

// public interface


void TagLister::reload_lister(const QString& source)
{
    m_source = source;
    m_pos = 0;
    m_next = 0;
    m_TagPath = QStringList() << "root";
    m_TagPos = QList<int>() << -1;
    m_TagLen = QList<int>() << 0;
}


TagLister::TagInfo TagLister::get_next()
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

// private routines

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
