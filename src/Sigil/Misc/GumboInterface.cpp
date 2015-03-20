/************************************************************************
**
**  Copyright (C) 2015  Kevin B. Hendricks, Stratford Ontario
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

#include <QString>
#include <QStringList>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QDir>
#include <QFileInfo>

#include "Misc/Utility.h"

#include "GumboInterface.h"
#include "string_buffer.h"
#include "error.h"

static std::string preserve_whitespace = "|pre|textarea|script|style|";
static std::string special_handling    = "|html|body|";
static std::string no_entity_sub       = "|script|style|";
static std::string href_src_tags       = "|a|audio|image|img|link|script|video|";
static std::string nonbreaking_inline  = "|a|abbr|acronym|b|bdo|big|cite|code|dfn|em|font|i|img|kbd|nobr|s|small|span|strike|strong|sub|sup|tt|";
static std::string empty_tags          = "|area|base|basefont|bgsound|br|command|col|embed|event-source|frame|hr|image|img|input|keygen|link|menuitem|meta|param|source|spacer|track|wbr|";

static const QChar POUND_SIGN    = QChar::fromLatin1('#');
static const QChar FORWARD_SLASH = QChar::fromLatin1('/');
static const std::string SRC = std::string("src");
static const std::string HREF = std::string("href");
static QHash<QString,QString> EmptyHash = QHash<QString,QString>();

// Note: m_output contains the gumbo output tree which 
// has data structures with pointers into the original source
// buffer passed in!!!!!!

// This source buffer is provided by the m_utf8src std::string
// which should always exist unchanged alongside the output tree

// Do NOT change or delete m_utf8src once set until after you 
// have properly destroyed the gumbo output tree

GumboInterface::GumboInterface(const QString &source)
        : m_source(source),
          m_output(NULL),
          m_sourceupdates(EmptyHash),
          m_newcsslinks(""),
          m_currentdir(""),
          m_utf8src("")
{
}


GumboInterface::~GumboInterface()
{
    if (m_output != NULL) {
        gumbo_destroy_output(m_output);
        m_output = NULL;
        m_utf8src = "";
    }
}


void GumboInterface::parse()
{
    if (!m_source.isEmpty() && (m_output == NULL)) {
        m_utf8src = m_source.toStdString();
        // remove any xml header line and any trailing whitespace
        if (m_utf8src.compare(0,5,"<?xml") == 0) {
            size_t end = m_utf8src.find_first_of('>', 5);
            end = m_utf8src.find_first_not_of("\n\r\t ",end+1);
            m_utf8src.erase(0,end);
        }
        GumboOptions myoptions = kGumboDefaultOptions;
        myoptions.use_xhtml_rules = true;
        myoptions.tab_stop = 4;
        m_output = gumbo_parse_with_options(&myoptions, m_utf8src.data(), m_utf8src.length());
    }
}


QString GumboInterface::repair()
{
    QString result = "";
    if (!m_source.isEmpty()) {
        if (m_output == NULL) {
            parse();
        }
        std::string utf8out = serialize(m_output->document);
        result = QString::fromStdString(utf8out);
    }
    return result;
}


QString GumboInterface::gettext()
{
    QString result = "";
    if (!m_source.isEmpty()) {
        if (m_output == NULL) {
            parse();
        }
        std::string utf8out = serialize(m_output->document);
        result = QString::fromStdString(utf8out);
    }
    return result;
}


QString GumboInterface::perform_source_updates(const QHash<QString, QString>& updates, const QString& my_current_book_relpath)
{
    m_sourceupdates = updates;
    m_currentdir = QFileInfo(my_current_book_relpath).dir().path();
    QString result = "";
    if (!m_source.isEmpty()) {
        if (m_output == NULL) {
            parse();
        }
        enum UpdateTypes doupdates = SourceUpdates;
        std::string utf8out = serialize(m_output->document, doupdates);
        result = QString::fromStdString(utf8out);
    }
    return result;
}


QString GumboInterface::perform_link_updates(const QString& newcsslinks)
{
    m_newcsslinks = newcsslinks.toStdString();
    QString result = "";
    if (!m_source.isEmpty()) {
        if (m_output == NULL) {
            parse();
        }
        enum UpdateTypes doupdates = LinkUpdates;
        std::string utf8out = serialize(m_output->document, doupdates);
        result = QString::fromStdString(utf8out);
    }
    return result;
}


QList<GumboWellFormedError> GumboInterface::error_check()
{
    QList<GumboWellFormedError> errlist;
    int line_offset = 0;
    GumboOptions myoptions = kGumboDefaultOptions;
    myoptions.use_xhtml_rules = true;
    myoptions.tab_stop = 4;
    // leave this as false to prevent pre-mature stopping when no error exists
    myoptions.stop_on_first_error = false;

    if (!m_source.isEmpty() && (m_output == NULL)) {
        m_utf8src = m_source.toStdString();
        // remove any xml header line and trailing whitespace
        if (m_utf8src.compare(0,5,"<?xml") == 0) {
            size_t end = m_utf8src.find_first_of('>', 0);
            end = m_utf8src.find_first_not_of("\n\r\t ",end+1);
            m_utf8src.erase(0,end);
            line_offset++;
        }
        // add in doctype if missing
        if ((m_utf8src.compare(0,9,"<!DOCTYPE") != 0) && (m_utf8src.compare(0,9,"<!doctype") != 0)) {
            m_utf8src.insert(0,"<!DOCTYPE html>\n");
            line_offset--;
        }
        m_output = gumbo_parse_with_options(&myoptions, m_utf8src.data(), m_utf8src.length());
    }
    const GumboVector* errors  = &m_output->errors;
    for (int i=0; i< errors->length; ++i) {
        GumboError* er = static_cast<GumboError*>(errors->data[i]);
        GumboWellFormedError gperror;
        gperror.line = er->position.line + line_offset;;
        gperror.column = er->position.column;
        unsigned int typenum = er->type;
        GumboStringBuffer text;
        gumbo_string_buffer_init(&text);
        gumbo_error_to_string(er, &text);
        std::string errmsg(text.data, text.length);
        gperror.message = QString::fromStdString(errmsg);
        gumbo_string_buffer_destroy(&text);
        errlist.append(gperror);
    }
    return errlist;
}


QList<GumboNode*> GumboInterface::get_all_nodes_with_attribute(const QString& attname)
{
    QList<GumboNode*> nodes;
    if (!m_source.isEmpty()) {
        if (m_output == NULL) {
            parse();
        }
        nodes = get_nodes_with_attribute(m_output->root, attname.toUtf8()); 
    }
    return nodes;
}


QList<GumboNode*> GumboInterface::get_nodes_with_attribute(GumboNode* node, const char * attname)
{
  if (node->type != GUMBO_NODE_ELEMENT) {
    return QList<GumboNode*>();
  }
  QList<GumboNode*> nodes;
  GumboAttribute* attr = gumbo_get_attribute(&node->v.element.attributes, attname);
  if (attr) {
      nodes.append(node);
  }
  GumboVector* children = &node->v.element.children;
  for (int i = 0; i < children->length; ++i) {
      nodes.append(get_nodes_with_attribute(static_cast<GumboNode*>(children->data[i]), attname));
  }
  return nodes;
}


QStringList GumboInterface::get_all_values_for_attribute(const QString& attname)
{
    QStringList attrvals;
    if (!m_source.isEmpty()) {
        if (m_output == NULL) {
            parse();
        }
        attrvals = get_values_for_attr(m_output->root, attname.toUtf8()); 
    }
    return attrvals;
}


QStringList  GumboInterface::get_values_for_attr(GumboNode* node, const char* attr_name) 
{
    if (node->type != GUMBO_NODE_ELEMENT) {
        return QStringList();
    }
    QStringList attr_vals;
    GumboAttribute* attr = gumbo_get_attribute(&node->v.element.attributes, attr_name);
    if (attr != NULL) {
        attr_vals.append(QString::fromUtf8(attr->value));
    }
    GumboVector* children = &node->v.element.children;
    for (int i = 0; i < children->length; ++i) {
        attr_vals.append(get_values_for_attr(static_cast<GumboNode*>(children->data[i]), attr_name));
    }
    return attr_vals;
}

QHash<QString,QString> GumboInterface::get_attributes_of_node(GumboNode* node)
{
    QHash<QString,QString> node_atts;
    if (node->type != GUMBO_NODE_ELEMENT) {
        return node_atts;
    }
    const GumboVector * attribs = &node->v.element.attributes;
    for (int i=0; i< attribs->length; ++i) {
        GumboAttribute* attr = static_cast<GumboAttribute*>(attribs->data[i]);
        QString key = QString::fromUtf8(attr->name);
        QString val = QString::fromUtf8(attr->value);
        node_atts[key] = val;
    }
    return node_atts;
}


QString GumboInterface::get_local_text_of_node(GumboNode* node)
{
    QString  node_text;
    if (node->type != GUMBO_NODE_ELEMENT) {
        return node_text;
    }
    GumboVector* children = &node->v.element.children;
    for (unsigned int i = 0; i < children->length; ++i) {
        GumboNode* child = static_cast<GumboNode*> (children->data[i]);

        if (child->type == GUMBO_NODE_TEXT) {
            node_text += QString::fromUtf8(child->v.text.text);

        } else if (child->type == GUMBO_NODE_WHITESPACE) {
            // keep all whitespace to keep as close to original as possible
            node_text += QString::fromUtf8(child->v.text.text);

        } else if (child->type == GUMBO_NODE_CDATA) {
            node_text += QString::fromUtf8(child->v.text.text);
        }
    }
    return node_text;
}


QList<GumboNode*> GumboInterface::get_all_nodes_with_tag(GumboTag tag)
{
    QList<GumboTag> tags;
    tags << tag;
    return  get_all_nodes_with_tags(tags); 
}


QList<GumboNode*> GumboInterface::get_all_nodes_with_tags(const QList<GumboTag> & tags )
{
  QList<GumboNode*> nodes;
  if (!m_source.isEmpty()) {
    if (m_output == NULL) {
      parse();
    }
    nodes = get_nodes_with_tags(m_output->root, tags); 
  }
  return nodes;
}


QList<GumboNode*>  GumboInterface::get_nodes_with_tags(GumboNode* node, const QList<GumboTag> & tags) 
{
  if (node->type != GUMBO_NODE_ELEMENT) {
    return QList<GumboNode*>();
  }
  QList<GumboNode*> nodes;
  GumboTag tag = node ->v.element.tag;
  if (tags.contains(tag)) {
    nodes.append(node);
  }
  GumboVector* children = &node->v.element.children;
  for (int i = 0; i < children->length; ++i) {
    nodes.append(get_nodes_with_tags(static_cast<GumboNode*>(children->data[i]), tags));
  }
  return nodes;
}


void GumboInterface::rtrim(std::string &s) 
{
    s.erase(s.find_last_not_of(" \n\r\t")+1);
}


void GumboInterface::ltrim(std::string &s)
{
    s.erase(0,s.find_first_not_of(" \n\r\t"));
}


void GumboInterface::ltrimnewlines(std::string &s)
{
    s.erase(0,s.find_first_not_of("\n\r"));
}


void GumboInterface::replace_all(std::string &s, const char * s1, const char * s2)
{
    std::string t1(s1);
    size_t len = t1.length();
    size_t pos = s.find(t1);
    while (pos != std::string::npos) {
      s.replace(pos, len, s2);
      pos = s.find(t1, pos + len);
    }
}


std::string GumboInterface::update_attribute_value(std::string attvalue)
{
    std::string result = attvalue; 
    QString attpath = Utility::URLDecodePath(QString::fromStdString(attvalue));
    int fragpos = attpath.lastIndexOf(POUND_SIGN);
    bool has_fragment = fragpos != -1;
    QString fragment = "";
    if (has_fragment) {
        fragment = attpath.mid(fragpos, -1);
        attpath = attpath.mid(0, fragpos);
    }
    QString search_key = QDir::cleanPath(m_currentdir + FORWARD_SLASH + attpath);
    QString new_href = m_sourceupdates.value(search_key, QString());
    if (!new_href.isEmpty()) {
        new_href += fragment;
        new_href = Utility::URLEncodePath(new_href);
        result =  new_href.toStdString();
    } 
    return result;
}


std::string GumboInterface::substitute_xml_entities_into_text(const std::string &text)
{
    std::string result = text;
    // replacing & must come first 
    replace_all(result, "&", "&amp;");
    replace_all(result, "<", "&lt;");
    replace_all(result, ">", "&gt;");
    return result;
}


std::string GumboInterface::substitute_xml_entities_into_attributes(char quote, const std::string &text)
{
    std::string result = substitute_xml_entities_into_text(text);
    if (quote == '"') {
        replace_all(result,"\"","&quot;");
    } else if (quote == '\'') {
        replace_all(result,"'","&apos;");
    }
    return result;
}


std::string GumboInterface::get_tag_name(GumboNode *node)
{
  std::string tagname;
  if (node->type == GUMBO_NODE_DOCUMENT) {
    tagname = "document";
    return tagname;
  }
  tagname = gumbo_normalized_tagname(node->v.element.tag);
  if ((tagname.empty()) ||
      (node->v.element.tag_namespace == GUMBO_NAMESPACE_SVG)) {

    // set up to examine original text of tag.
    GumboStringPiece gsp = node->v.element.original_tag;
    gumbo_tag_from_original_text(&gsp);

    // special handling for some svg tag names.
    if (node->v.element.tag_namespace  == GUMBO_NAMESPACE_SVG) {
      const char * data = gumbo_normalize_svg_tagname(&gsp);
      // NOTE: data may not be null-terminated!
      // since case change only - length must be same as original
      // if no replacement found returns null, not original tag!
      if (data != NULL) {
        return std::string(data, gsp.length);
      }
    }
    if (tagname.empty()) {
      return std::string(gsp.data, gsp.length);
    }
  }
  return tagname;
}


std::string GumboInterface::build_doctype(GumboNode *node)
{
    std::string results = "";
    if (node->v.document.has_doctype) {
        results.append("<!DOCTYPE ");
        results.append(node->v.document.name);
        std::string pi(node->v.document.public_identifier);
        if ((node->v.document.public_identifier != NULL) && !pi.empty() ) {
            results.append(" PUBLIC \"");
            results.append(node->v.document.public_identifier);
            results.append("\"\n  \"");
            results.append(node->v.document.system_identifier);
            results.append("\"");
        }
        results.append(">\n");
    } else {
      results.append("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.1//EN\"\n");
      results.append("  \"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd\">\n");
    }
    return results;
}


std::string GumboInterface::build_attributes(GumboAttribute * at, bool no_entities, bool runupdates)
{
    std::string atts = " ";
    atts.append(at->name);
    std::string attvalue = at->value;

    if (runupdates && (at->name == HREF || at->name == SRC)) {
        attvalue = update_attribute_value(attvalue);
    }

    // how do we want to handle attributes with empty values
    // <input type="checkbox" checked />  or <input type="checkbox" checked="" /> 

    if ( (!attvalue.empty())   || 
         (at->original_value.data[0] == '"') || 
         (at->original_value.data[0] == '\'') ) {

        // determine original quote character used if it exists
        char quote = at->original_value.data[0];
        std::string qs = "";
        if (quote == '\'') qs = std::string("'");
        if (quote == '"') qs = std::string("\"");
        atts.append("=");
        atts.append(qs);
        if (no_entities) {
            atts.append(attvalue);
        } else {
            atts.append(substitute_xml_entities_into_attributes(quote, attvalue));
        }
        atts.append(qs);
    }
    return atts;
}


// serialize children of a node
// may be invoked recursively

std::string GumboInterface::serialize_contents(GumboNode* node, enum UpdateTypes doupdates) {
    std::string contents        = "";
    std::string tagname         = get_tag_name(node);
    std::string key             = "|" + tagname + "|";
    bool no_entity_substitution = no_entity_sub.find(key) != std::string::npos;
    bool keep_whitespace        = preserve_whitespace.find(key) != std::string::npos;
    bool is_inline              = nonbreaking_inline.find(key) != std::string::npos;

    // build up result for each child, recursively if need be
    GumboVector* children = &node->v.element.children;

    for (unsigned int i = 0; i < children->length; ++i) {
        GumboNode* child = static_cast<GumboNode*> (children->data[i]);

        if (child->type == GUMBO_NODE_TEXT) {
            if (no_entity_substitution) {
                contents.append(std::string(child->v.text.text));
            } else {
                contents.append(substitute_xml_entities_into_text(std::string(child->v.text.text)));
            }

        } else if (child->type == GUMBO_NODE_ELEMENT || child->type == GUMBO_NODE_TEMPLATE) {
          contents.append(serialize(child, doupdates));

        } else if (child->type == GUMBO_NODE_WHITESPACE) {
          // keep all whitespace to keep as close to original as possible
          contents.append(std::string(child->v.text.text));

        } else if (child->type == GUMBO_NODE_CDATA) {
          contents.append("<![CDATA[" + std::string(child->v.text.text) + "]]>");

        } else if (child->type == GUMBO_NODE_COMMENT) {
          contents.append("<!--" + std::string(child->v.text.text) + "-->");
 
        } else {
          fprintf(stderr, "unknown element of type: %d\n", child->type); 
        }

    }

    return contents;
}


// serialize a GumboNode back to html/xhtml
// may be invoked recursively

std::string GumboInterface::serialize(GumboNode* node, enum UpdateTypes doupdates) {
    // special case the document node
    if (node->type == GUMBO_NODE_DOCUMENT) {
        std::string results = build_doctype(node);
        results.append(serialize_contents(node, doupdates));
        return results;
    }

    std::string close = "";
    std::string closeTag = "";
    std::string atts = "";
    std::string tagname            = get_tag_name(node);
    std::string key                = "|" + tagname + "|";
    bool need_special_handling     = special_handling.find(key) != std::string::npos;
    bool is_empty_tag              = empty_tags.find(key) != std::string::npos;
    bool no_entity_substitution    = no_entity_sub.find(key) != std::string::npos;
    bool is_inline                 = nonbreaking_inline.find(key) != std::string::npos;
    bool is_href_src_tag           = href_src_tags.find(key) != std::string::npos;

    // build attr string  
    const GumboVector * attribs = &node->v.element.attributes;
    for (int i=0; i< attribs->length; ++i) {
        GumboAttribute* at = static_cast<GumboAttribute*>(attribs->data[i]);
        atts.append(build_attributes(at, no_entity_substitution, ((doupdates & SourceUpdates) && is_href_src_tag) ));
    }

    // determine closing tag type
    if (is_empty_tag) {
        close = "/";
    } else {
        closeTag = "</" + tagname + ">";
    }

    // serialize your contents
    std::string contents = serialize_contents(node, doupdates);

    if (need_special_handling) {
        ltrimnewlines(contents);
        rtrim(contents);
        contents.append("\n");
    }

    // build results
    std::string results;

    if ((doupdates & LinkUpdates) && (tagname == "link") && 
        (node->parent->type == GUMBO_NODE_ELEMENT) && 
        (node->parent->v.element.tag == GUMBO_TAG_HEAD)) {
      return "";
    }

    results.append("<"+tagname+atts+close+">");
    if (need_special_handling) results.append("\n");
    results.append(contents);

    if ((doupdates & LinkUpdates) && (tagname == "head")) {
        results.append(m_newcsslinks);
    }

    results.append(closeTag);
    if (need_special_handling) results.append("\n");
    return results;
}


#if 0
// This should no longer be needed but keep it around in case my xhtml parsing
// support changes in gumbo cause problems later

static QStringList allowed_void_tags = QStringList() << "area"    << "base"     << "basefont" 
                                                     << "bgsound" << "br"       << "command" 
                                                     << "col"     << "embed"    << "event-source" 
                                                     << "frame"   << "hr"       << "image" 
                                                     << "img"     << "input"    << "keygen" 
                                                     << "link"    << "menuitem" << "meta" 
                                                     << "param"   << "source"   << "spacer" 
                                                     << "track"   << "wbr";

QString GumboInterface::fix_self_closing_tags(const QString &source)
{
    QString newsource = source;
    QRegularExpression selfclosed("<\\s*([a-zA-Z]+)(\\s*[^>/]*)/\\s*>");
    QRegularExpressionMatch match = selfclosed.match(newsource, 0);
    while (match.hasMatch()) {
        if (match.capturedStart() == -1) {
            break;
        }
        QString tag = match.captured(0);
        int sp = match.capturedStart(0);
        int n = match.capturedLength(0);
        QString name = match.captured(1);
        QString atts = match.captured(2);;
        atts = atts.trimmed();
        if (!atts.isEmpty()) {
            atts = " " + atts;
        }
        int nsp = sp + n;
        if (!allowed_void_tags.contains(tag)) {
            QString newtag = "<" + name + atts + "></" + name + ">";
            newsource = newsource.replace(sp,n,newtag);
            nsp = sp + newtag.length();
        }
        match = selfclosed.match(newsource, nsp);
    }
    return newsource;
}
#endif
