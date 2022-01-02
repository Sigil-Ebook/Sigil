/************************************************************************
**
**  Copyright (C) 2015-2022 Kevin B. Hendricks, John Schember
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
#ifndef _OPFENTRY_H
#define _OPFENTRY_H

#include <QString>
#include <QStringList>
#include <QStringRef>
#include <QHash>
#include "Parsers/TagAtts.h"

// Note: all hrefs should always be kept in URLEncoded form
// as decoding urls before splitting into component parts can lead
// to data loss (paths can legally contain url delimiters when decoded - such as #)

struct PackageEntry
{
    QString m_version;
    QString m_uniqueid;
    TagAtts m_atts;

    PackageEntry() : m_version("2.0"), m_uniqueid("Bookid") {};
    PackageEntry(const QString&, const QString&, const TagAtts&);
    PackageEntry(const PackageEntry& entry) : m_version(entry.m_version),
      m_uniqueid(entry.m_uniqueid), m_atts(entry.m_atts) {};
    QString convert_to_xml() const;
};


struct MetaNSEntry
{
    TagAtts m_atts;

    MetaNSEntry() {};
    MetaNSEntry(const TagAtts& atts);
    MetaNSEntry(const MetaNSEntry& entry) : m_atts(entry.m_atts) {};
    QString convert_to_xml() const;
};


struct MetaEntry
{
    QString m_name;
    QString m_content;
    TagAtts m_atts;

    MetaEntry() :  m_name(""), m_content("") {};
    MetaEntry(const QString&, const QString&, const TagAtts&);
    MetaEntry(const MetaEntry& entry) : m_name(entry.m_name), m_content(entry.m_content), m_atts(entry.m_atts) {};
    QString convert_to_xml() const;
};


struct ManifestEntry
{
    QString m_id;
    QString m_href;
    QString m_mtype;
    TagAtts m_atts;

    ManifestEntry() : m_id(""), m_href(""), m_mtype("") {} ;
    ManifestEntry(const QString&, const QString&, const QString&, const TagAtts&);
    ManifestEntry(const ManifestEntry& entry) : m_id(entry.m_id), m_href(entry.m_href),
          m_mtype(entry.m_mtype), m_atts(entry.m_atts) {};
    QString convert_to_xml() const;
};


struct SpineAttrEntry
{
    TagAtts m_atts;

    SpineAttrEntry() {};
    SpineAttrEntry(const TagAtts& atts);
    SpineAttrEntry(const SpineAttrEntry& entry) : m_atts(entry.m_atts) {};
    QString convert_to_xml() const;
};


struct SpineEntry
{
    QString m_idref;
    TagAtts m_atts;

    SpineEntry() : m_idref("") {};
    SpineEntry(const QString&, const TagAtts& atts);
    SpineEntry(const SpineEntry& entry) : m_idref(entry.m_idref), m_atts(entry.m_atts) {};
    QString convert_to_xml() const;
};


struct GuideEntry
{
    QString m_type;
    QString m_title;
    QString m_href;

    GuideEntry() : m_type(""), m_title(""), m_href("") {};
    GuideEntry(const QString&, const QString&, const QString&);
    GuideEntry(const GuideEntry& entry) : m_type(entry.m_type), m_title(entry.m_title), m_href(entry.m_href) {};
    QString convert_to_xml() const;
};


struct BindingsEntry
{
    QString m_mtype;
    QString m_handler;

    BindingsEntry() : m_mtype(""), m_handler("") {};
    BindingsEntry(const QString&, const QString&);
    BindingsEntry(const BindingsEntry& entry) : m_mtype(entry.m_mtype), m_handler(entry.m_handler) {};
    QString convert_to_xml() const;
};


class BaseParser
{
public:
    struct MarkupInfo {
        int     pos;
        QString text;
        QString tpath;
        QString tname;
        QString ttype;
        TagAtts tattr;
    };

    BaseParser(const QString &source);
    ~BaseParser() {};
    void parse_next(MarkupInfo&);
    bool ns_remap_needed() { return m_ns_remap; }
    QString oldprefix() { return m_oldprefix; }

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
    bool         m_ns_remap;
    QString      m_oldprefix;
};


struct OPFParser
{
    PackageEntry         m_package;
    MetaNSEntry          m_metans;
    QList<MetaEntry>     m_metadata;
    QList<ManifestEntry> m_manifest;
    SpineAttrEntry       m_spineattr;
    QList<SpineEntry>    m_spine;
    QList<GuideEntry>    m_guide;
    QList<BindingsEntry> m_bindings;
    QHash<QString,int>   m_idpos;
    QHash<QString,int>   m_hrefpos;

    OPFParser(): m_idpos(QHash<QString,int>()), m_hrefpos(QHash<QString,int>()) {};
    void parse(const QString & source);

    QString convert_to_xml() const;
};

#endif
