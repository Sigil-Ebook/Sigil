/************************************************************************
**
**  Copyright (C) 2016 Kevin B. Hendricks Stratford, ON, Canada 
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

#include <QMutex>
#include <QString>
#include <QStringList>

#include "BookManipulation/HTMLMetadata.h"

static const QStringList EVENT_LIST           = QStringList() << "creation" << "publication" << "modification";
static const QStringList MODIFICATION_ALIASES = QStringList() << "modified" << "modification";
static const QStringList CREATION_ALIASES     = QStringList() << "created"  << "creation";
static const QStringList PUBLICATION_ALIASES  = QStringList() << "issued"   << "published" << "publication";
static const QStringList SCHEME_LIST          = QStringList() << "ISBN" << "ISSN" << "DOI";

QMutex HTMLMetadata::s_AccessMutex;
HTMLMetadata *HTMLMetadata::m_Instance = NULL;

HTMLMetadata *HTMLMetadata::Instance()
{
    // We use a static local variable
    // to hold our singleton instance; using a pointer member
    // variable creates problems with object destruction;
    QMutexLocker locker(&s_AccessMutex);

    if (m_Instance == 0) {
        m_Instance = new HTMLMetadata();
    }

    return m_Instance;
}



// Processes metadata from inside xhtml files for the gui
// try to extract whatever dc metadata possible
MetaEntry HTMLMetadata::MapHTMLToOPFMetadata(GumboNode* node, GumboInterface & gi)
{
    MetaEntry meta;
    if (node->v.element.tag == GUMBO_TAG_META) {
        QString name;
        QString value;
        QHash<QString,QString> matts;

        GumboAttribute* attr = gumbo_get_attribute(&node->v.element.attributes, "name");
        if (attr) name = QString::fromUtf8(attr->value);

        attr = gumbo_get_attribute(&node->v.element.attributes, "content");
        if (attr) value = QString::fromUtf8(attr->value);

        attr = gumbo_get_attribute(&node->v.element.attributes, "scheme");
        if (attr) matts[QString("scheme")] = QString::fromUtf8(attr->value);

        attr = gumbo_get_attribute(&node->v.element.attributes, "opf:scheme");
        if (attr) matts[QString("opf:scheme")] = QString::fromUtf8(attr->value);

        attr = gumbo_get_attribute(&node->v.element.attributes, "id");
        if (attr) matts[QString("id")] = QString::fromUtf8(attr->value);

        if ((!name.isEmpty()) && (!value.isEmpty())) {
            meta = FixupHTMLMetadata(name, value, matts);
        }
    }
    return meta;
}


// Maps Dublic Core metadata to internal book meta format
MetaEntry HTMLMetadata::FixupHTMLMetadata(QString name, QString value, const QHash<QString,QString> & matts)
{
    QString lowname = name.toLower();
    MetaEntry me;

    if (!lowname.startsWith("dc.") &&
        !lowname.startsWith("dcterms.")) {
        if (lowname == "copyright") {
            me.m_name = "dc:rights";
            me.m_content = value;
        } else if (lowname == "author") {
            me.m_name = "dc:creator";
            me.m_content = value;
        } else if (lowname == "publisher") {
            me.m_name = "dc:publisher";
            me.m_content = value;
        } else if (lowname == "source") {
            me.m_name = "dc:source";
            me.m_content = value;
        } else if (lowname == "description") {
            me.m_name = "dc:description";
            me.m_content = value;
        } else if (lowname == "date" || lowname == "published") {
            me.m_name = "dc:date";
            me.m_content = value;
        } else if (lowname  == "eisbn") {
            me.m_name = "dc:identifier";
            me.m_content = "urn:isbn:" + value;
        } else if (lowname  == "issn") {
            me.m_name = "dc:identifier";
            me.m_content = "urn:issn:" + value;
        } else if (lowname == "doi") {
            me.m_name = "dc:identifier";
            me.m_content = "urn:doi:" + value;
        }
        return me;
    }
    return HtmlToOpfDC(name, value, matts);
}



// Converts HTML sourced Dublin Core metadata to OPF style metadata
//
// Sample of HTML Based DC Metadata:
//   <meta name="DC.Title" content="The Title"/>
//   <meta name="DC.Language" content="en"/>
//   <meta name="DC.Creator" content=""/>
//   <meta name="DC.Publisher" content="Publisher Name"/>
//   <meta name="DC.Date" content="2016-03-01"/>
//   <meta name="DC.Identifier" content="978-0-00000-000-0" scheme="ISBN"/>
//   <meta name="DC.Relation" content="978-0-00000-000-0" scheme="ISBN"/>

MetaEntry HTMLMetadata::HtmlToOpfDC(QString mname, QString mvalue, const QHash<QString,QString> & matts)
{
    // Dublin Core from html file with the original 15 element namespace or
    // expanded DCTerms namespace. Allows qualifiers as refinements
    // prefix.name[.refinement]
    QStringList fields = QString(mname.toLower() + "..").split(".");
    QString name       = fields[ 1 ];
    QString refinement = fields[ 2 ];
    QString dc_event;

    if (MODIFICATION_ALIASES.contains(name) || MODIFICATION_ALIASES.contains(refinement)) {
        name = "dc:date";
        dc_event = "modification";
    } else if (CREATION_ALIASES.contains(name) || CREATION_ALIASES.contains(refinement)) {
        name     = "dc:date";
        dc_event = "creation";
    } else if (PUBLICATION_ALIASES.contains(name) || PUBLICATION_ALIASES.contains(refinement)) {
        name     = "dc:date";
        dc_event = "publication";
    }

    QString role   = (name == "creator") || (name == "contributor") ? refinement : QString();

    QString scheme = matts.value("scheme");
    if ((name == "identifier") && (scheme.isEmpty())) {
        scheme = refinement;
    }
    if (!scheme.isEmpty()) {
        if (SCHEME_LIST.contains(scheme, Qt::CaseInsensitive)) {
            scheme = SCHEME_LIST.filter(scheme, Qt::CaseInsensitive)[ 0 ];
        }
    }

    MetaEntry me;
    me.m_name  = "dc:" + name;
    me.m_content = mvalue;
    if (!scheme.isEmpty()) {
        me.m_atts[ "opf:scheme" ] = scheme;
    }
    if (!dc_event.isEmpty()) {
        me.m_atts[ "opf:event" ] = dc_event;
    }
    if (!role.isEmpty()) {
        me.m_atts[ "opf:role" ] = role;
    }
    return me;
}
