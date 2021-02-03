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
#include <QDebug>

#include "Parsers/TagAtts.h"
#include "Misc/Utility.h"
#include "Parsers/QuickParser.h"

QuickParser::QuickParser(const QString &source, QString default_lang)
    : m_source(source),
      m_pos(0),
      m_next(0)
{
    m_LangPath << default_lang;
    m_TagPath << "root";
}



// public interface


void QuickParser::reload_parser(const QString& source, QString default_language)
{
    m_source = source;
    m_pos = 0;
    m_next = 0;
    m_LangPath = QStringList() << default_language;
    m_TagPath = QStringList() << "root";
}


QuickParser::MarkupInfo QuickParser::parse_next()
{
    MarkupInfo mi;
    mi.pos = -1;
    QStringRef markup = parseML();
    if (!markup.isNull()) {
        if ((markup.at(0) == "<") && (markup.at(markup.size() - 1) == ">")) {
            parseTag(markup, mi);
            if (mi.ttype == "begin") {
                m_TagPath << mi.tname;
                QString lang = mi.tattr.value("lang", QString());
                if (lang.isEmpty()) lang = mi.tattr.value("xml:lang", QString());
                if (lang.isEmpty()) lang = m_LangPath.last();
                m_LangPath << lang;
            } else if (mi.ttype == "end") {
                m_TagPath.removeLast();
                m_LangPath.removeLast();
            }
        } else {
            mi.text = markup.toString();
        }
        mi.pos = m_pos;
        mi.lang = m_LangPath.last();
        mi.tpath = m_TagPath.join(".");
    }
    return mi;
}


// Note all text and attribute values must be properly xml encoded if needed
QString QuickParser::serialize_markup(const QuickParser::MarkupInfo& mi)
{
    QString res;
    // handle leading text
    if (!mi.text.isEmpty()) {
        res = mi.text;
    }

    // if not tag info provided return just the text
    if (mi.ttype.isEmpty() || mi.tname.isEmpty()) return res;
    
    // handle any end tags
    if (mi.ttype == "end") {
        res = res + "</" + mi.tname + ">";
        return res;
    }
    
    // handle the special cases
    if ((mi.ttype == "xmlheader") || (mi.ttype == "doctype") || (mi.ttype == "pi")) {
        res = res + "<" + mi.tname + mi.tattr["special"] + ">";
        return res;
    }
    if (mi.ttype == "comment") {
        res = res + "<" + mi.tname + mi.tattr["special"] + "-->";
        return res;
    }
    if (mi.ttype == "cdata") {
        res = res + "<" + mi.tname + mi.tattr["special"] + "]]>";
        return res;
    }
    // finally begin and single tags
    res = res + "<" + mi.tname;
    foreach(QString key, mi.tattr.keys()) {
        QString val = mi.tattr[key];
        if (val.contains("\"")) {
            res = res + " " + key + "=" + "'" + val + "'";
        } else {
            res = res + " " + key + "=" + "\"" + val + "\"";
        }
    }
    if (mi.ttype == "single") {
        res = res + "/>";
    } else {
        res = res + ">";
    }
    return res;
}


// private routines


QStringRef QuickParser::parseML()
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


void QuickParser::parseTag(const QStringRef& tagstring, QuickParser::MarkupInfo& mi)
{
    Q_ASSERT(tagstring.at(0) == "<");
    Q_ASSERT(tagstring.at(tagstring.size() - 1) == ">");
    int taglen = tagstring.length();
    QChar c = tagstring.at(1);
    int p = 0;

    // first handle special cases
    if (c == '?') {
        if (tagstring.startsWith("<?xml")) {
            mi.tname = "?xml";
            mi.ttype = "xmlheader";
            mi.tattr["special"] = Utility::Substring(5, taglen-1, tagstring);
        } else {
            mi.tname = "?";
            mi.ttype = "pi";
            mi.tattr["special"] = Utility::Substring(1, taglen-1, tagstring);
        }
        return;
    }
    if (c == '!') {
        if (tagstring.startsWith("<!--")) {
            mi.tname = "!--";
            mi.ttype = "comment"; 
            mi.tattr["special"] = Utility::Substring(4, taglen-3, tagstring);
        } else if (tagstring.startsWith("<!DOCTYPE") || tagstring.startsWith("<!doctype")) {
            mi.tname = "!DOCTYPE";
            mi.ttype = "doctype";
            mi.tattr["special"] = Utility::Substring(9, taglen-1, tagstring);
        } else if (tagstring.startsWith("<![CDATA[") || tagstring.startsWith("<![cdata[")) {
            mi.tname = "![CDATA[";
            mi.ttype = "cdata";
            mi.tattr["special"] = Utility::Substring(9, taglen-3, tagstring);
        }
        return;
    }

    // normal tag, extract tag name
    p = skipAnyBlanks(tagstring, 1);
    if (tagstring.at(p) == "/") {
        mi.ttype = "end";
        p++;
        p = skipAnyBlanks(tagstring, p);
    }
    int b = p;
    p = stopWhenContains(tagstring, ">/ \f\t\r\n", p);
    mi.tname = Utility::Substring(b, p, tagstring);

    // handle the possibility of attributes (so begin or single tag type, not end)
    if (mi.ttype.isEmpty()) {
        while (tagstring.indexOf("=", p) != -1) {
            p = skipAnyBlanks(tagstring, p);
            b = p;
            p = stopWhenContains(tagstring, "=", p);
            QString aname = Utility::Substring(b, p, tagstring).trimmed();
            QString avalue;
            p++;
            p = skipAnyBlanks(tagstring, p);
            if ((tagstring.at(p) == "'") || (tagstring.at(p) == "\"")) {
                QString qc = tagstring.at(p);
                p++;
                b = p;
                p = stopWhenContains(tagstring, qc, p);
                avalue = Utility::Substring(b, p, tagstring);
                p++;
            } else {
                b = p;
                p = stopWhenContains(tagstring, ">/ ", p);
                avalue = Utility::Substring(b, p, tagstring);
            }
            mi.tattr[aname] = avalue;
        }
        mi.ttype = "begin";
        if (tagstring.indexOf("/", p) >= 0) mi.ttype = "single";
    }
    return;
}


int QuickParser::findTarget(const QString &tgt, int p, bool after)
{
    int nxt = m_source.indexOf(tgt, p);
    if (nxt == -1) return m_source.length();
    nxt = nxt + (tgt.length() -1);
    if (after) nxt++;
    return nxt;
}


int QuickParser::skipAnyBlanks(const QStringRef &tgt, int p)
{
    while((p < tgt.length()) && (tgt.at(p) == " ")) p++;
    return p;
}


int QuickParser::stopWhenContains(const QStringRef &tgt, const QString& stopchars, int p)
{
    while((p < tgt.length()) && !stopchars.contains(tgt.at(p))) p++;
    return p;
}
