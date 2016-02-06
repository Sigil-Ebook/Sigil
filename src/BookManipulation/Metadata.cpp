/************************************************************************
**
**  Copyright (C) 2016 Kevin B. Hendricks Stratford, ON, Canada 
**  Copyright (C) 2009, 2010, 2011  Strahinja Markovic  <strahinja.markovic@gmail.com>
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

#include <QtCore/QDate>
#include <QtCore/QMutex>
#include <QtCore/QString>
#include <QtCore/QStringList>

#include "BookManipulation/Metadata.h"
#include "BookManipulation/XhtmlDoc.h"
#include "Misc/Utility.h"
#include "Misc/Language.h"

static const QStringList EVENT_LIST           = QStringList() << "creation" << "publication" << "modification";
static const QStringList MODIFICATION_ALIASES = QStringList() << "modified" << "modification";
static const QStringList CREATION_ALIASES     = QStringList() << "created"  << "creation";
static const QStringList PUBLICATION_ALIASES  = QStringList() << "issued"   << "published" << "publication";
static const QStringList SCHEME_LIST          = QStringList() << "ISBN" << "ISSN" << "DOI";

QMutex Metadata::s_AccessMutex;
Metadata *Metadata::m_Instance = NULL;

Metadata *Metadata::Instance()
{
    // We use a static local variable
    // to hold our singleton instance; using a pointer member
    // variable creates problems with object destruction;
    QMutexLocker locker(&s_AccessMutex);

    if (m_Instance == 0) {
        m_Instance = new Metadata();
    }

    return m_Instance;
}

QString Metadata::GetName(QString code)
{
    QString name = "";

    // Codes are unique between basic/relator
    if (m_Basic.contains(code)) {
        name = m_Basic[ code ].name;
    } else if (m_Relator->isRelatorCode(code)) {
        name = m_Relator->GetName(code);
    }
    return name;
}

QString Metadata::GetCode(QString name)
{
    QString code = "";

    // Names are sufficiently unique between basic/relator
    // Except Publisher which is handled as an exception elsewhere
    if (m_BasicFullNames.contains(name)) {
        code = m_BasicFullNames[ name ];
    } else if (m_Relator->isRelatorName(name)) {
        code = m_Relator->GetCode(name);
    }

    return code;
}

QString Metadata::GetText(QString text)
{
    if (m_Text.contains(text)) {
        return m_Text.value(text);
    }

    return text;
}

bool Metadata::IsRelator(QString code)
{
  return m_Relator->isRelatorCode(code);
}

const QHash<QString, DescriptiveMetaInfo> & Metadata::GetRelatorMap()
{
    return m_Relator->GetCodeMap();
}

const QHash<QString, DescriptiveMetaInfo> & Metadata::GetBasicMetaMap()
{
    return m_Basic;
}

QStringList Metadata::GetSortedNames(QString infotype) 
{
    if (infotype == "role") {
        return m_Relator->GetSortedNames();
    }
    QStringList names = m_BasicFullNames.keys();
    names.sort();
    return names;
}

QString Metadata::GetDescriptionByName(QString infotype, QString name)
{  
    if (infotype == "role") {
        return m_Relator->GetDescriptionByName(name);
    }
    QString description;
    if (m_BasicFullNames.contains(name)) {
        QString code = m_BasicFullNames[ name ];
        if (m_Basic.contains(code)) {
            description = m_Basic[ code ].description;
        }
    }
    return description;
}


// Processes metadata from inside xhtml files for the gui
Metadata::MetaElement Metadata::MapToBookMetadata(GumboNode* node, GumboInterface & gi)
{
    Metadata::MetaElement meta;
    if (node->v.element.tag == GUMBO_TAG_META) {

        GumboAttribute* attr = gumbo_get_attribute(&node->v.element.attributes, "name");
        if (attr) meta.name  = QString::fromUtf8(attr->value);

        attr = gumbo_get_attribute(&node->v.element.attributes, "content");
        if (attr) meta.value  = QString::fromUtf8(attr->value);

        attr = gumbo_get_attribute(&node->v.element.attributes, "scheme");
        if (attr) meta.attributes[ "scheme"] = QString::fromUtf8(attr->value);

        attr = gumbo_get_attribute(&node->v.element.attributes, "opf:scheme");
        if (attr) meta.attributes[ "scheme"] = QString::fromUtf8(attr->value);

        attr = gumbo_get_attribute(&node->v.element.attributes, "id");
        if (attr) meta.attributes[ "id"] = QString::fromUtf8(attr->value);

        if ((!meta.name.isEmpty()) && (!meta.value.toString().isEmpty())) {
            return MapToBookMetadata(meta , false);
        }

    } else {
        meta.name = QString::fromStdString(gi.get_tag_name(node));
        // Metadata class works only with local names so strip off any dc: prefix
        if (meta.name.startsWith("dc:")) meta.name.remove(0,3);
        meta.attributes = gi.get_attributes_of_node(node);
        // strip off any namespace prefixes from attribute keys
        QStringList keys = meta.attributes.keys();
        foreach(QString key, keys){
            QString value = meta.attributes.value(key,"");
            int i = key.indexOf(":");
            if (i != -1) {
                meta.attributes.remove(key);
                key.remove(0,i+1);
                meta.attributes[key] = value;
            }
        }
        QString element_text = gi.get_local_text_of_node(node);
        meta.value = element_text;
        if (!element_text.isEmpty()) {
            return MapToBookMetadata(meta , true);
        }
    }
    return meta;
}


// Processes OPF Metadata MetaEntry for use in GUI
Metadata::MetaElement Metadata::MapMetaEntryToBookMetadata(const MetaEntry& me)
{
    Metadata::MetaElement meta;
    QString element_name = me.m_name;
    if (element_name.startsWith("dc:")) element_name.remove(0,3);
    if (element_name == "meta") {
        meta.name  = Utility::DecodeXML(me.m_atts.value("name","")); 
        meta.value = Utility::DecodeXML(me.m_atts.value("content", ""));
        QString schemeval = Utility::DecodeXML(me.m_atts.value("scheme", ""));
        if (schemeval.isEmpty()) {
          schemeval = Utility::DecodeXML(me.m_atts.value("opf:scheme", ""));
        }
        meta.attributes[ "scheme" ] = schemeval;
        meta.attributes[ "id" ] = me.m_atts.value("id", "");

        if ((!meta.name.isEmpty()) && (!meta.value.toString().isEmpty())) {
            return MapToBookMetadata(meta , false);
        }
    } else {
        meta.attributes = me.m_atts;
        // The Metadata class requires removal the ns prefixes from attribute keys
        QStringList keys = meta.attributes.keys();
        foreach(QString key, keys){
            QString value = meta.attributes.value(key,"");
            int i = key.indexOf(":");
            if (i != -1) {
                meta.attributes.remove(key);
                key.remove(0,i+1);
                meta.attributes[key] = value;
            }
        }
        meta.name = element_name;
        QString element_text = Utility::DecodeXML(me.m_content);
        meta.value = element_text;

        if (!element_text.isEmpty()) {
            return MapToBookMetadata(meta , true);
        }
    }

    return meta;
}


// Maps Dublic Core metadata to internal book meta format
Metadata::MetaElement Metadata::MapToBookMetadata(const Metadata::MetaElement &meta, bool is_dc_element)
{
    QString name = meta.name.toLower();

    if (!is_dc_element &&
        !name.startsWith("dc.") &&
        !name.startsWith("dcterms.")) {
        return FreeFormMetadata(meta);
    }

    // Dublin Core
    // Transform HTML based Dublin Core to OPF style meta element
    MetaElement working_copy_meta = is_dc_element ? meta : HtmlToOpfDC(meta);
    name = working_copy_meta.name.toLower();

    if ((name == "creator") || (name == "contributor")) {
        return CreateContribMetadata(working_copy_meta);
    }

    if (name == "date") {
        return DateMetadata(working_copy_meta);
    }

    if (name == "identifier") {
        return IdentifierMetadata(working_copy_meta);
    }

    QString value = meta.value.toString();

    if (name == "language") {
        value = Language::instance()->GetLanguageName(value);
        // fall through
    }

    MetaElement book_meta;

    if ((!name.isEmpty()) && (!value.isEmpty())) {
        book_meta.name = name;
        book_meta.value = value;
    }

    return book_meta;
}


Metadata::Metadata() :
  m_Relator(MarcRelators::instance())
{
    LoadBasicMetadata();
    LoadText();
}

void Metadata::LoadText()
{
    m_Text[ "creator" ] = tr("Creator");
    m_Text[ "contributor" ] = tr("Contributor");
    m_Text[ "date" ] = tr("Date");
    m_Text[ "identifier" ] = tr("Identifier");
}


// Loads the basic metadata types, names, and descriptions
void Metadata::LoadBasicMetadata()
{
    // If the basic metadata has already been loaded
    // by a previous Meta Editor, then don't load them again
    if (!m_Basic.isEmpty()) {
        return;
    }

    // These descriptions are standard EPUB descriptions and should not be changed.
    // Names and codes must be unique between basic and advanced (except Publisher)
    // Abbreviations are not translated.
    QStringList data;
    data <<
         tr("Subject") << "subject" << tr("An arbitrary phrase or keyword describing the subject in question. Use multiple 'subject' elements if needed.") <<
         tr("Description") << "description" << tr("Description of the publication's content.") <<
         tr("Publisher") << "publisher" << tr("An entity responsible for making the publication available.") <<
         tr("Date: Publication") << "publication" << tr("The date of publication.") <<
         tr("Date: Creation") << "creation" << tr("The date of creation.") <<
         tr("Date: Modification") << "modification" << tr("The date of modification.") <<
         tr("Date (custom)") << "customdate" << tr("Enter your own event name in the File As column, e.g. updated.") <<
         tr("Type") << "type" << tr("The nature or genre of the content of the resource.") <<
         tr("Format") << "format" << tr("The media type or dimensions of the publication. Best practice is to use a value from a controlled vocabulary (e.g. MIME media types).") <<
         tr("Source") << "source" << tr("A reference to a resource from which the present publication is derived.") <<
         tr("Language") << "language" << tr("An optional extra language of the publication.  Use a value from the Language drop down menu.  For example use 'English' instead of the language code 'en'.") <<
         tr("Relation") << "relation" << tr("A reference to a related resource. The recommended best practice is to identify the referenced resource by means of a string or number conforming to a formal identification system.") <<
         tr("Coverage") << "coverage" << tr("The extent or scope of the content of the publication's content.") <<
         tr("Rights") << "rights" << tr("Information about rights held in and over the publication. Rights information often encompasses Intellectual Property Rights (IPR), Copyright, and various Property Rights. If the Rights element is absent, no assumptions may be made about any rights held in or over the publication.") <<
         tr("Title") << "title" << tr("An optional extra title of the publication in addition to the main title already entered.") <<
         tr("Identifier") + ": DOI"   << "DOI" << tr("Digital Object Identifier") <<
         tr("Identifier") + ": ISBN"  << "ISBN" << tr("International Standard Book Number") <<
         tr("Identifier") + ": ISSN"  << "ISSN" << tr("International Standard Serial Number") <<
         tr("Identifier (custom)") << "customidentifier" << tr("Enter your own custom identifier name in the File As column, e.g. stocknumber");

    for (int i = 0; i < data.count(); i++) {
        QString name = data.at(i++);
        QString code = data.at(i++);
        QString description = data.at(i);
        DescriptiveMetaInfo meta;
        meta.name = name;
        meta.description  = description;
        m_Basic.insert(code, meta);
        m_BasicFullNames.insert(name, code);
    }
}


// Converts HTML sourced Dublin Core metadata to OPF style metadata
Metadata::MetaElement Metadata::HtmlToOpfDC(const Metadata::MetaElement &meta)
{
    // Dublin Core from html file with the original 15 element namespace or
    // expanded DCTerms namespace. Allows qualifiers as refinements
    // prefix.name[.refinement]
    QStringList fields = QString(meta.name.toLower() + "..").split(".");
    QString name       = fields[ 1 ];
    QString refinement = fields[ 2 ];
    QString dc_event;

    if (MODIFICATION_ALIASES.contains(name) || MODIFICATION_ALIASES.contains(refinement)) {
        name     = "date";
        dc_event = "modification";
    } else if (CREATION_ALIASES.contains(name) || CREATION_ALIASES.contains(refinement)) {
        name     = "date";
        dc_event = "creation";
    } else if (PUBLICATION_ALIASES.contains(name) || PUBLICATION_ALIASES.contains(refinement)) {
        name     = "date";
        dc_event = "publication";
    }

    QString role   = (name == "creator") || (name == "contributor") ? refinement : QString();
    QString scheme = meta.attributes.value("scheme");

    if ((name == "identifier") && (scheme.isEmpty())) {
        scheme = refinement;
    }

    if (!scheme.isEmpty()) {
        if (SCHEME_LIST.contains(scheme, Qt::CaseInsensitive)) {
            scheme = SCHEME_LIST.filter(scheme, Qt::CaseInsensitive)[ 0 ];
        }
    }

    MetaElement opf_meta;
    opf_meta.name  = name;
    opf_meta.value = meta.value;

    if (!scheme.isEmpty()) {
        opf_meta.attributes[ "scheme" ] = scheme;
    }

    if (!dc_event.isEmpty()) {
        opf_meta.attributes[ "event" ] = dc_event;
    }

    if (!role.isEmpty()) {
        opf_meta.attributes[ "role" ] = role;
    }

    return opf_meta;
}


// Converts free form metadata into internal book metadata
Metadata::MetaElement Metadata::FreeFormMetadata(const Metadata::MetaElement &meta)
{
    // non - dublin core meta info from html file, if this maps to
    // one of the metadata basic fields used internally pass it through
    // i.e. Author, Title, Publisher, Rights/CopyRight, EISBN/ISBN
    QString name = meta.name.toLower();

    // Remap commonly used meta values to match internal names
    if (name == "copyright") {
        name = "rights";
    } else if (name  == "eisbn") {
        name = "ISBN";
    } else if (name  == "issn") {
        name = "ISSN";
    } else if (name == "doi") {
        name = "DOI";
    }

    MetaElement book_meta;
    book_meta.name  = name;
    book_meta.value = meta.value;
    return book_meta;
}


// Converts dc:creator and dc:contributor metadata to book internal metadata
Metadata::MetaElement Metadata::CreateContribMetadata(const Metadata::MetaElement &meta)
{
    QString role    = meta.attributes.value("role", "aut");

    // Some epub exporters set incorrect opf:role attributes
    // and we need to handle that. Otherwise, Sigil bugs out on export.
    // Since we can't tell what the role is, just guess author.
    if (!IsRelator(role)) {
        role = "aut";
    }

    MetaElement book_meta;
    book_meta.name  = role;
    book_meta.value = meta.value.toString();
    book_meta.file_as = meta.attributes.value("file-as");
    // Save contributor or creator
    book_meta.role_type = meta.name.toLower();
    return book_meta;
}


// Converts dc:date metadata to book internal metadata
Metadata::MetaElement Metadata::DateMetadata(const Metadata::MetaElement &meta)
{
    QString dc_event = meta.attributes.value("event");
    // Dates are in YYYY[-MM[-DD]] format
    QStringList date_parts = meta.value.toString().split("-", QString::SkipEmptyParts);

    if (date_parts.count() < 1) {
        date_parts.append(QString::number(QDate::currentDate().year()));
    }

    if (date_parts.count() < 2) {
        date_parts.append("01");
    }

    if (date_parts.count() < 3) {
        date_parts.append("01");
    }

    QVariant value = QDate(date_parts[ 0 ].toInt(),
                           date_parts[ 1 ].toInt(),
                           date_parts[ 2 ].toInt());
    MetaElement book_meta;
    book_meta.name  = meta.name;
    book_meta.value = value;
    book_meta.file_as = dc_event;
    return book_meta;
}


// Converts dc:identifier metadata to book internal metadata
Metadata::MetaElement Metadata::IdentifierMetadata(const Metadata::MetaElement &meta)
{
    QString scheme = meta.attributes.value("scheme");
    QString id = meta.attributes.value("id");
    MetaElement book_meta;

    // Ignore any identifiers with an id as id can't be edited in dialog
    // And skip the uuid identifier in case it made it through without an id
    if (id.isEmpty() && scheme.toLower() != "uuid") {
        book_meta.name = meta.name;
        book_meta.value = meta.value;
        book_meta.file_as = scheme;
    }

    return book_meta;
}

