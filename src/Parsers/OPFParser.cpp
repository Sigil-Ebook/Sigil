/************************************************************************
**
**  Copyright (C) 2021-2022 Kevin B. Hendricks
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

#include "Misc/Utility.h"
#include "Parsers/OPFParser.h"
#include <QDebug>

// Note: all hrefs/urls should always be kept in URLEncoded form
// as decoding urls before splitting into component parts can lead
// to data loss (paths can legally contain url delimiters when decoded - such as #)

/**
 * Package tag
 */

PackageEntry::PackageEntry(const QString& version, const QString& uniqueid,
                           const TagAtts& atts)
    : m_version(version), m_uniqueid(uniqueid), m_atts(atts)
{
}

QString PackageEntry::convert_to_xml() const
{
    QStringList xmlres;
    xmlres << "<package version=\"" + m_version + "\"";
    xmlres << " unique-identifier=\"" + m_uniqueid + "\"";
    foreach (QString kv, m_atts.keys()) {
        QString val = m_atts.value(kv,"");
        val.replace("\"", "&quot;");
        xmlres <<  " " + kv + "=\"" + val + "\"";
    }
    xmlres << ">\n";
    return xmlres.join(QString(""));
}

/**
 * metadata namespace attributes
 */
 
MetaNSEntry::MetaNSEntry(const TagAtts& atts)
    : m_atts(atts)
{
}

QString MetaNSEntry::convert_to_xml() const
{
  QStringList xmlres;
  xmlres << "  <metadata";
  foreach (QString kv, m_atts.keys()) {
      QString val = m_atts.value(kv,"");
      val.replace("\"", "&quot;");
      xmlres <<  " " + kv + "=\"" + val + "\"";
  }
  xmlres << ">\n";
  return xmlres.join(QString(""));
}

/**
 * metadata tags
 */
 
MetaEntry::MetaEntry(const QString& name, const QString& content, const TagAtts& atts)
    : m_name(name), m_content(content), m_atts(atts)
{
}

QString MetaEntry::convert_to_xml() const
{
    QStringList xmlres;
    xmlres << "    <" + m_name;
    foreach (QString kv, m_atts.keys()) {
        QString val = m_atts.value(kv,"");
        val.replace("\"", "&quot;");
        xmlres <<  " " + kv + "=\"" + val + "\"";
    }
    if (m_content.isEmpty()) {
        xmlres << " />\n";
    } else {
        xmlres << ">" + m_content + "</" + m_name + ">\n";
    }
    return xmlres.join(QString(""));
}

/**
 * manifest tags
 */
 
ManifestEntry::ManifestEntry(const QString& id, const QString& href, const QString& mtype, const TagAtts& atts)
    : m_id(id), m_href(href), m_mtype(mtype), m_atts(atts)
{
}

QString ManifestEntry::convert_to_xml() const
{
  QStringList xmlres;
  xmlres << "    <item id=\"" + m_id + "\"";
  xmlres << " href=\"" + m_href + "\"";
  xmlres << " media-type=\"" + m_mtype+ "\"";
  foreach (QString kv, m_atts.keys()) {
      QString val = m_atts.value(kv,"");
      val.replace("\"", "&quot;");
      xmlres <<  " " + kv + "=\"" + val + "\"";
  }
  xmlres << "/>\n";
  return xmlres.join(QString(""));
}

/**
 * spine attributes
 */

SpineAttrEntry::SpineAttrEntry(const TagAtts& atts)
    : m_atts(atts)
{
}

QString SpineAttrEntry::convert_to_xml() const
{
    QStringList xmlres;
    xmlres << "  <spine";
    foreach (QString kv, m_atts.keys()) {
        QString val = m_atts.value(kv,"");
        val.replace("\"", "&quot;");
        xmlres <<  " " + kv + "=\"" + val + "\"";
    }
    xmlres << ">\n";
    return xmlres.join(QString(""));
}

/**
 * spine tags
 */
 
SpineEntry::SpineEntry(const QString& idref, const TagAtts& atts)
    : m_idref(idref), m_atts(atts)
{
}

QString SpineEntry::convert_to_xml() const
{
  QStringList xmlres;
  xmlres << "    <itemref idref=\"" + m_idref + "\"";
  foreach (QString kv, m_atts.keys()) {
      QString val = m_atts.value(kv,"");
      val.replace("\"", "&quot;");
      xmlres <<  " " + kv + "=\"" + val + "\"";
  }
  xmlres << "/>\n";
  return xmlres.join(QString(""));
}

/**
 * guide tags
 */
 
GuideEntry::GuideEntry(const QString& gtype, const QString& gtitle, const QString& ghref)
: m_type(gtype), m_title(gtitle), m_href(ghref)
{
}

QString GuideEntry::convert_to_xml() const
{
    QStringList xmlres;
    xmlres << "    <reference type=\"" + m_type + "\"";
    xmlres << " title=\"" + m_title + "\"";
    xmlres << " href=\"" + m_href + "\"";
    xmlres << "/>\n";
    return xmlres.join(QString(""));
}

/**
 * bindings tags
 */

BindingsEntry::BindingsEntry(const QString& mtype, const QString& handler)
: m_mtype(mtype), m_handler(handler)
{
}

QString BindingsEntry::convert_to_xml() const
{
    QStringList xmlres;
    xmlres << "    <mediaType  media-type=\"" + m_mtype + "\"";
    xmlres << " handler=\"" + m_handler + "\"";
    xmlres << "/>\n";
    return xmlres.join(QString(""));
}


BaseParser::BaseParser(const QString &source)
    : m_source(source), m_pos(0), m_next(0), m_ns_remap(false)
{
    m_TagPath << "root";
}


void BaseParser::parse_next(MarkupInfo& mi)
{
    mi.pos = -1;
    QStringRef markup = parseML();
    if (!markup.isNull()) {
        if ((markup.at(0) == '<') && (markup.at(markup.size() - 1) == '>')) {
            parseTag(markup, mi);
            if (mi.tname.endsWith(":package") && (mi.ttype == "begin")) {
                m_ns_remap = true;
                m_oldprefix = mi.tname.split(':').at(0);
            }
            if (m_ns_remap && mi.tname.startsWith(m_oldprefix + ':')) {
                mi.tname = mi.tname.mid(m_oldprefix.length()+1,-1);
            }
            if (mi.ttype == "begin") {
                m_TagPath << mi.tname;
            } else if (mi.ttype == "end") {
                m_TagPath.removeLast();
            }
        } else {
            mi.text = markup.toString();
        }
        mi.pos = m_pos;
        mi.tpath = m_TagPath.join(".");
    }
}

QStringRef BaseParser::parseML()
{
    int p = m_next;
    m_pos = p;
    if (p >= m_source.length()) return QStringRef();
    if (m_source.at(p) != '<') {
        // we have text leading up to a tag start
        m_next = findTarget("<", p+1);
        return Utility::SubstringRef(m_pos, m_next, m_source);
    }
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


void BaseParser::parseTag(const QStringRef& tagstring, MarkupInfo& mi)
{
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
        } else if (tagstring.startsWith("<![CDATA[") || tagstring.startsWith("<![cdata[")) {
            mi.tname = "![CDATA[";
            mi.ttype = "cdata";
            mi.tattr["special"] = Utility::Substring(9, taglen-3, tagstring);
        }
        return;
    }

    // normal tag, extract tag name
    p = skipAnyBlanks(tagstring, 1);
    if (tagstring.at(p) == '/') {
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
            if ((tagstring.at(p) == '\'') || (tagstring.at(p) == '"')) {
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


int BaseParser::findTarget(const QString &tgt, int p, bool after)
{
    int nxt = m_source.indexOf(tgt, p);
    if (nxt == -1) return m_source.length();
    nxt = nxt + (tgt.length() -1);
    if (after) nxt++;
    return nxt;
}


int BaseParser::skipAnyBlanks(const QStringRef &tgt, int p)
{
    while((p < tgt.length()) && (tgt.at(p) == ' ')) p++;
    return p;
}


int BaseParser::stopWhenContains(const QStringRef &tgt, const QString& stopchars, int p)
{
    while((p < tgt.length()) && !stopchars.contains(tgt.at(p))) p++;
    return p;
}


void OPFParser::parse(const QString& source)
{
    BaseParser fxp(source);
    QString tcontent;
    BaseParser::MarkupInfo matching_begin_tag;
    int count = 0;
    int manifest_position = 0;
    bool get_content = false;
    m_idpos.clear();
    m_hrefpos.clear();
    while(true) {

        BaseParser::MarkupInfo mi;
        fxp.parse_next(mi);

        if (mi.pos < 0) break;

        if (!mi.text.isEmpty()) {
            if (get_content) tcontent = mi.text.trimmed();
            continue;
        }

        // handle manifest
        if (mi.tpath.contains("manifest") && (mi.ttype == "single" || mi.ttype == "begin")) {
            if (mi.tname == "item") {
                QString nid = QString("xid%03d").arg(count);
                count++;
                QString mid = mi.tattr.value("id",nid);
                mi.tattr.remove("id");
                // must keep all hrefs in urlencoded form here
                // if relative then no fragments so decode and encode for safety
                QString href = mi.tattr.value("href","");
                if (href.indexOf(':') == -1) {
                    href = Utility::URLDecodePath(href);
                    href = Utility::URLEncodePath(href);
                }
                mi.tattr.remove("href");
                QString mtype = mi.tattr.value("media-type","");
                mi.tattr.remove("media-type");
                m_manifest << ManifestEntry(mid, href, mtype, mi.tattr);
                m_idpos[mid] = manifest_position;
                m_hrefpos[href] = manifest_position;
                manifest_position++;
            }
            continue;
        }

        // handle spine
        if ((mi.tname == "spine") && (mi.ttype == "begin")) {
            m_spineattr = SpineAttrEntry(mi.tattr);
            continue;
        }
        if (mi.tpath.contains("spine") && (mi.ttype == "single" || mi.ttype == "begin")) {
            if (mi.tname == "itemref") {
                QString idref = mi.tattr.value("idref","");
                mi.tattr.remove("idref");
                m_spine << SpineEntry(idref, mi.tattr);
            }
            continue;
        }

        // handle metadata
        if ((mi.tname == "metadata") && (mi.ttype == "begin")) {
            if (!mi.tattr.contains("xmlns:opf")) {
                mi.tattr["xmlns:opf"] = "http://www.idpf.org/2007/opf";
            }
            m_metans = MetaNSEntry(mi.tattr);
            continue;
        }
        if (mi.tpath.contains("metadata") && mi.ttype == "begin") {
            if (mi.tname == "meta" || mi.tname == "link" || mi.tname.startsWith("dc:")) {
                matching_begin_tag = mi;
                get_content = true;
            }
            continue;
        }
        if (mi.tpath.contains("metadata") && (mi.ttype == "single" || mi.ttype == "end")) {
            if (mi.tname == "meta" || mi.tname == "link" || mi.tname.startsWith("dc:")) {
                if (mi.ttype == "single") {
                    m_metadata << MetaEntry(mi.tname, "", mi.tattr);
                } else {
                    m_metadata << MetaEntry(matching_begin_tag.tname, tcontent, matching_begin_tag.tattr);
                    matching_begin_tag = BaseParser::MarkupInfo();
                    tcontent = "";
                    get_content = false;
                }
            }
            continue;
        }

        // handle package tag
        if ((mi.tname == "package") && (mi.ttype == "begin")) {
            QString version = mi.tattr.value("version", "2.0");
            mi.tattr.remove("version");
            QString uid = mi.tattr.value("unique-identifier","bookid");
            mi.tattr.remove("unique-identifier");
            if (fxp.ns_remap_needed()) {
                mi.tattr.remove("xmlns:" + fxp.oldprefix());
                mi.tattr["xmlns"] = "http://www.idpf.org/2007/opf";
            }
            m_package = PackageEntry(version, uid, mi.tattr);
            continue;
        }

        // handle guide
        if (mi.tpath.contains("guide") && (mi.ttype == "single" || mi.ttype == "begin")) {
            if (mi.tname == "reference") {
                QString gtype = mi.tattr.value("type","");
                QString gtitle = mi.tattr.value("title","");
                QString ghref = mi.tattr.value("href","");
                m_guide << GuideEntry(gtype, gtitle, ghref);
            }
            continue;
        }

        // handle bindings
        if (mi.tpath.contains("bindings") && (mi.ttype == "single" || mi.ttype == "begin")) {
            if (mi.tname == "mediaType" || mi.tname == "mediatype") {
                QString btype = mi.tattr.value("media-type","");
                QString bhandler = mi.tattr.value("handler","");
                m_bindings << BindingsEntry(btype, bhandler);
            }
            continue;
        }
    }
}


QString OPFParser::convert_to_xml() const
{
    QStringList xmlres;
    xmlres <<  "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
    xmlres << m_package.convert_to_xml();
    xmlres << m_metans.convert_to_xml();
    foreach (MetaEntry me, m_metadata) {
        xmlres << me.convert_to_xml();
    }
    xmlres << "  </metadata>\n";
    xmlres << "  <manifest>\n";
    foreach (ManifestEntry me, m_manifest) {
        xmlres << me.convert_to_xml();
    }
    xmlres << "  </manifest>\n";
    xmlres << m_spineattr.convert_to_xml();
    foreach(SpineEntry sp, m_spine) {
        xmlres << sp.convert_to_xml();
    }
    xmlres << "  </spine>\n";
    if (m_guide.size() > 0) {
        xmlres << "  <guide>\n";
        foreach(GuideEntry ge, m_guide) {
            xmlres << ge.convert_to_xml();
        }
        xmlres << "  </guide>\n";
    }
    if ((m_bindings.size() > 0) && (!m_package.m_version.startsWith("2"))) {
        xmlres << "  <bindings>\n";
        foreach(BindingsEntry be, m_bindings) {
            xmlres << be.convert_to_xml();
        }
        xmlres << "  </bindings>\n";
    }
    xmlres << "</package>\n";
    return xmlres.join("");
}
