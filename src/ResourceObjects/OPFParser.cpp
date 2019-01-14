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

#include "Misc/EmbeddedPython.h"
#include "Misc/Utility.h"
#include "ResourceObjects/OPFParser.h"

/**
 * Package tag
 */

PackageEntry::PackageEntry(const QString& version, const QString& uniqueid, 
  					       const QStringList& keylist, const QStringList& vallist) 
    : m_version(version), m_uniqueid(uniqueid)
{
    int n = keylist.size();
	if (n == vallist.size()) {
        for (int i=0; i < n; i++) {
            m_atts[keylist.at(i)] = vallist.at(i);
        }
	}
}

PackageEntry::PackageEntry(const QVariant& qv)
    : m_version(""), m_uniqueid(""), m_atts(QHash<QString,QString>())
{
    // tuple: (version, uniqueid, keylist, vallist)
    QList<QVariant> tup = qv.toList();
    if (tup.size() == 4) {
        m_version  = tup.at(0).toString();
        m_uniqueid = tup.at(1).toString();
        QStringList keylist = tup.at(2).toStringList();
        QStringList vallist = tup.at(3).toStringList();
        int n = keylist.size();
        if (n == vallist.size()) {
            for (int i=0; i < n; i++) {
                m_atts[keylist.at(i)] = vallist.at(i);
            }
        }
    }
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
 
MetaNSEntry::MetaNSEntry(const QStringList& keylist, const QStringList& vallist) 
{
    int n = keylist.size();
	if (n == vallist.size()) {
		for (int i=0; i < n; i++) {
            m_atts[keylist.at(i)] = vallist.at(i);
		}
	}
}

MetaNSEntry::MetaNSEntry(const QVariant& qv)
    : m_atts(QHash<QString,QString>())
{
    // tuple: (keylist, vallist)
    QList<QVariant> tup = qv.toList();
    if (tup.size() == 2) {
        QStringList keylist = tup.at(0).toStringList();
        QStringList vallist = tup.at(1).toStringList();
        int n = keylist.size();
        if (n == vallist.size()) {
            for (int i=0; i < n; i++) {
                m_atts[keylist.at(i)] = vallist.at(i);
            }
        }
    }
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
 
MetaEntry::MetaEntry(const QString& name, const QString& content, 
  					 const QStringList& keylist, const QStringList& vallist) 
    : m_name(name), m_content(content)
{
    int n = keylist.size();
	if (n == vallist.size()) {
		for (int i=0; i < n; i++) {
            m_atts[keylist.at(i)] = vallist.at(i);
		}
	}
}

MetaEntry::MetaEntry(const QVariant& qv)
    : m_name(""), m_content("")
{
    // tuple: (tagname, content, keylist, vallist)
    QList<QVariant> tup = qv.toList();
    if (tup.size() == 4) {
        m_name = tup.at(0).toString();
        m_content = tup.at(1).toString();
        QStringList keylist = tup.at(2).toStringList();
        QStringList vallist = tup.at(3).toStringList();
        int n = keylist.size();
        if (n == vallist.size()) {
            for (int i=0; i < n; i++) {
                m_atts[keylist.at(i)] = vallist.at(i);
            }
        }
    }
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
 
ManifestEntry::ManifestEntry(const QString& id, const QString& href, const QString& mtype, 
  					         const QStringList& keylist, const QStringList& vallist) 
: m_id(id), m_href(href), m_mtype(mtype)
{
    int n = keylist.size();
	if (n == vallist.size()) {
		for (int i=0; i < n; i++) {
            m_atts[keylist.at(i)] = vallist.at(i);
		}
	}
}

ManifestEntry::ManifestEntry(const QVariant& qv)
    : m_id(""), m_href(""), m_mtype("")
{
    QList<QVariant> tup = qv.toList();
    // tuple (id, href, media-type, keylist, vallist)
    if (tup.size() == 5) {
        m_id    = tup.at(0).toString();
        m_href  = tup.at(1).toString();
        m_mtype = tup.at(2).toString();
        QStringList keylist = tup.at(3).toStringList();
        QStringList vallist = tup.at(4).toStringList();
        int n = keylist.size();
        if (n == vallist.size()) {
            for (int i=0; i < n; i++) {
                m_atts[keylist.at(i)] = vallist.at(i);
            }
        }
    }
}

QString ManifestEntry::convert_to_xml() const
{
  QStringList xmlres;
  xmlres << "    <item id=\"" + m_id + "\"";
  xmlres << " href=\"" + Utility::URLEncodePath(m_href) + "\"";
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
 
SpineAttrEntry::SpineAttrEntry(const QStringList& keylist, const QStringList& vallist) 
{
    int n = keylist.size();
	if (n == vallist.size()) {
		for (int i=0; i < n; i++) {
            m_atts[keylist.at(i)] = vallist.at(i);
		}
	}
}

SpineAttrEntry::SpineAttrEntry(const QVariant& qv)
    : m_atts(QHash<QString,QString>())
{
    // tuple (keylist, vallist)
    QList<QVariant> tup = qv.toList();
    if (tup.size() == 2) {
        QStringList keylist = tup.at(0).toStringList();
        QStringList vallist = tup.at(1).toStringList();
        int n = keylist.size();
        if (n == vallist.size()) {
            for (int i=0; i < n; i++) {
                m_atts[keylist.at(i)] = vallist.at(i);
            }
        }
    }
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
 
SpineEntry::SpineEntry(const QString& idref, const QStringList& keylist, const QStringList& vallist) 
: m_idref(idref)
{
    int n = keylist.size();
	if (n == vallist.size()) {
		for (int i=0; i < n; i++) {
            m_atts[keylist.at(i)] = vallist.at(i);
		}
	}
}

SpineEntry::SpineEntry(const QVariant& qv)
    : m_idref("")
{
    QList<QVariant> tup = qv.toList();
    // tuple (idref, keylist, vallist)
    if (tup.size() == 3) {
        m_idref = tup.at(0).toString();
        QStringList keylist = tup.at(1).toStringList();
        QStringList vallist = tup.at(2).toStringList();
        int n = keylist.size();
        if (n == vallist.size()) {
            for (int i=0; i < n; i++) {
                m_atts[keylist.at(i)] = vallist.at(i);
            }
        }
    }
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

GuideEntry::GuideEntry(const QVariant& qv)
    : m_type(""), m_title(""), m_href("")
{
    // tuple (type, title, href)
    QList<QVariant> tup = qv.toList();
    if (tup.size() == 3) {
        m_type  = tup.at(0).toString();
        m_title = tup.at(1).toString();
        m_href  = tup.at(2).toString();
    }
}

QString GuideEntry::convert_to_xml() const
{
  QStringList xmlres;
  xmlres << "    <reference type=\"" + m_type + "\"";
  xmlres << " title=\"" + m_title + "\"";
  xmlres << " href=\"" + Utility::URLEncodePath(m_href) + "\"";
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

BindingsEntry::BindingsEntry(const QVariant& qv)
    : m_mtype(""), m_handler("")
{
    // tuple (media-type, handler)
    QList<QVariant> tup = qv.toList();
    if (tup.size() == 2) {
        m_mtype   = tup.at(0).toString();
        m_handler = tup.at(1).toString();
    }
}

QString BindingsEntry::convert_to_xml() const
{
  QStringList xmlres;
  xmlres << "    <mediaType  media-type=\"" + m_mtype + "\"";
  xmlres << " handler=\"" + m_handler + "\"";
  xmlres << "/>\n";
  return xmlres.join(QString(""));
}


void OPFParser::parse(const QString& source)
{
  int rv = 0;
  QString traceback;

  QList<QVariant> args;
  args.append(QVariant(source));
  EmbeddedPython* epp = EmbeddedPython::instance();
  QVariant res = epp->runInPython( QString("opf_newparser"), QString("parseopf"), args, &rv, traceback, true);
  if (rv) fprintf(stderr, "setext parseropf error %d traceback %s\n",rv, traceback.toStdString().c_str());

  PyObjectPtr mpo = PyObjectPtr(res);

  args.clear();
  res = epp->callPyObjMethod(mpo, QString("get_package"), args, &rv, traceback);
  if (rv) fprintf(stderr, "setext package error %d traceback %s\n",rv, traceback.toStdString().c_str());
  m_package = PackageEntry(res);

  res = epp->callPyObjMethod(mpo, QString("get_metadata_attr"), args, &rv, traceback);
  if (rv) fprintf(stderr, "setext meta_attr error %d traceback %s\n",rv, traceback.toStdString().c_str());
  m_metans = MetaNSEntry(res);

  res = epp->callPyObjMethod(mpo, QString("get_metadata"), args, &rv, traceback);
  if (rv) fprintf(stderr, "setext metadata error %d traceback %s\n",rv, traceback.toStdString().c_str());
  m_metadata.clear();
  QList<QVariant> lst = res.toList();
  foreach(QVariant qv, lst) {
    m_metadata.append(MetaEntry(qv));
  }

  m_idpos.clear();
  m_hrefpos.clear();

  res = epp->callPyObjMethod(mpo, QString("get_manifest"), args, &rv, traceback);
  if (rv) fprintf(stderr, "setext manifest error %d traceback %s\n",rv, traceback.toStdString().c_str());
  m_manifest.clear();
  lst = res.toList();
  for (int i = 0; i < lst.count(); i++) {
    ManifestEntry me = ManifestEntry(lst.at(i));
    m_idpos[me.m_id] = i;
    m_hrefpos[me.m_href] = i;
    m_manifest.append(me);
  }

  res = epp->callPyObjMethod(mpo, QString("get_spine_attr"), args, &rv, traceback);
  if (rv) fprintf(stderr, "setext spineattr error %d traceback %s\n",rv, traceback.toStdString().c_str());
  m_spineattr = SpineAttrEntry(res);

  res = epp->callPyObjMethod(mpo, QString("get_spine"), args, &rv, traceback);
  if (rv) fprintf(stderr, "setext spine error %d traceback %s\n",rv, traceback.toStdString().c_str());
  m_spine.clear();
  lst = res.toList();
  foreach(QVariant qv, lst) {
    m_spine.append(SpineEntry(qv));
  }

  res = epp->callPyObjMethod(mpo, QString("get_guide"), args, &rv, traceback);
  if (rv) fprintf(stderr, "setext guide error %d traceback %s\n",rv, traceback.toStdString().c_str());
  m_guide.clear();
  lst = res.toList();
  foreach(QVariant qv, lst) {
    m_guide.append(GuideEntry(qv));
  }

  res = epp->callPyObjMethod(mpo, QString("get_bindings"), args, &rv, traceback);
  if (rv) fprintf(stderr, "setext bindings error %d traceback %s\n",rv, traceback.toStdString().c_str());
  m_bindings.clear();
  lst = res.toList();
  foreach(QVariant qv, lst) {
    m_bindings.append(BindingsEntry(qv));
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
