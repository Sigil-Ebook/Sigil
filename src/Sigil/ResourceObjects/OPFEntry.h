/************************************************************************
**
**  Copyright (C) 2015 Kevin B. Hendricks, John Schember
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
#include <QHash>
#include <QVariant>

struct PackageEntry
{
  QString m_version;
  QString m_uniqueid;
  QHash<QString,QString> m_atts;

PackageEntry() : m_version("2.0"), m_uniqueid("Bookid"), m_atts(QHash<QString,QString>()) {};
  PackageEntry(const QVariant&);
  PackageEntry(const QString&, const QString&, const QStringList&, const QStringList&);
  PackageEntry(const PackageEntry& entry) : m_version(entry.m_version), m_uniqueid(entry.m_uniqueid), m_atts(entry.m_atts) {};

  QString convert_to_xml() const;
};


struct MetaNSEntry
{
  QHash<QString,QString> m_atts;

  MetaNSEntry() : m_atts(QHash<QString,QString>()) {}; 
  MetaNSEntry(const QVariant&);
  MetaNSEntry(const QStringList&, const QStringList&);
  MetaNSEntry(const MetaNSEntry& entry) : m_atts(entry.m_atts) {};

  QString convert_to_xml() const;
};


struct MetaEntry
{
  QString m_name;
  QString m_content;
  QHash<QString,QString> m_atts;
  MetaEntry() :  m_name(""), m_content(""), m_atts(QHash<QString,QString>()) {};
  MetaEntry(const QVariant&);
  MetaEntry(const QString&, const QString&, const QStringList&, const QStringList&);
  MetaEntry(const MetaEntry& entry) : m_name(entry.m_name), m_content(entry.m_content), m_atts(entry.m_atts) {};

  QString convert_to_xml() const;
};


struct ManifestEntry
{
  QString m_id;
  QString m_href;
  QString m_mtype;
  QHash<QString,QString> m_atts;

  ManifestEntry() : m_id(""), m_href(""), m_mtype(""), m_atts(QHash<QString,QString>()) {} ;
  ManifestEntry(const QVariant&);
  ManifestEntry(const QString&, const QString&, const QString&, const QStringList&, const QStringList&);
  ManifestEntry(const ManifestEntry& entry) : m_id(entry.m_id), m_href(entry.m_href),
        m_mtype(entry.m_mtype), m_atts(entry.m_atts) {};

  QString convert_to_xml() const;
};


struct SpineAttrEntry
{
  QHash<QString,QString> m_atts;

  SpineAttrEntry() : m_atts(QHash<QString,QString>()) {};
  SpineAttrEntry(const QVariant&);
  SpineAttrEntry(const QStringList&, const QStringList&);
  SpineAttrEntry(const SpineAttrEntry& entry) : m_atts(entry.m_atts) {};

  QString convert_to_xml() const;
};


struct SpineEntry
{
  QString m_idref;
  QHash<QString,QString> m_atts;

  SpineEntry() : m_idref(""), m_atts(QHash<QString,QString>()) {};
  SpineEntry(const QVariant&);
  SpineEntry(const QString&, const QStringList&, const QStringList&);
  SpineEntry(const SpineEntry& entry) : m_idref(entry.m_idref), m_atts(entry.m_atts) {};

  QString convert_to_xml() const;
};


struct GuideEntry
{
  QString m_type;
  QString m_title;
  QString m_href;

  GuideEntry() : m_type(""), m_title(""), m_href("") {};
  GuideEntry(const QVariant&);
  GuideEntry(const QString&, const QString&, const QString&);
  GuideEntry(const GuideEntry& entry) : m_type(entry.m_type), m_title(entry.m_title), m_href(entry.m_href) {};

  QString convert_to_xml() const;
};


struct BindingsEntry
{
  QString m_mtype;
  QString m_handler;

  BindingsEntry() : m_mtype(""), m_handler("") {};
  BindingsEntry(const QVariant&);
  BindingsEntry(const QString&, const QString&);
  BindingsEntry(const BindingsEntry& entry) : m_mtype(entry.m_mtype), m_handler(entry.m_handler) {};

  QString convert_to_xml() const;
};

#endif

