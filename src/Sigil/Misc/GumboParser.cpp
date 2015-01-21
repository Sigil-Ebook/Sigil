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

#include "GumboParser.h"

static std::string nonbreaking_inline  = "|a|abbr|acronym|b|bdo|big|cite|code|dfn|em|font|i|img|kbd|nobr|s|small|span|strike|strong|sub|sup|tt|";
static std::string empty_tags          = "|area|base|basefont|bgsound|br|command|col|embed|event-source|frame|hr|image|img|input|keygen|link|menuitem|meta|param|source|spacer|track|wbr|";
static std::string preserve_whitespace = "|pre|textarea|script|style|";
static std::string special_handling    = "|html|body|";
static std::string no_entity_sub       = "|script|style|";

static QStringList cdatatags = QStringList() << "title" << "textarea" << "style" << "script" << "xmp" << "iframe" << "noembed" << "noframes" << "noscript";
static QStringList convert_non_void_self_closing_tags = QStringList() << "a" << "span";

// Note: m_output contains the gumbo output tree which 
// has data structures with pointers into the original source
// buffer passed in!!!!!!

// This source buffer is provided by the m_utf8src std::string
// which should always exist unchanged alongside the output tree

// Do NOT change or delete m_utf8src once set until after you 
// have properly destroyed the gumbo output tree

GumboParser::GumboParser(const QString &source)
        : m_source(""),
          m_output(NULL),
          m_utf8src("")
{
    m_source = fix_self_closing_tags(source);
}


GumboParser::~GumboParser()
{
    if (m_output != NULL) {
        gumbo_destroy_output(&kGumboDefaultOptions, m_output);
        m_output = NULL;
        m_utf8src = "";
    }
}


void GumboParser::parse()
{
    if (!m_source.isEmpty() && (m_output == NULL)) {
        m_utf8src = m_source.toStdString();
        m_output = gumbo_parse(m_utf8src.c_str());
    }
}


QString GumboParser::repair()
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


QString GumboParser::prettyprint(QString indent_chars)
{
    QString result = "";
    if (!m_source.isEmpty()) {
        if (m_output == NULL) {
            parse();
        }
        std::string ind = indent_chars.toStdString();
        std::string utf8out = prettyprint(m_output->document, 0, ind);
        result = QString::fromStdString(utf8out);
    }
    return result;
}


QString GumboParser::fix_self_closing_tags(const QString &source)
{
    QString result = source;
    for (int i = 0; i < cdatatags.size(); ++i) {
        QString tag = cdatatags.at(i);
        QString oldtag = "<\\s*" + tag + "(\\s*[^>/]*)/\\s*>";
        QString newtag = "<" + tag + "\\1></" + tag + ">";
        QRegularExpression pat(oldtag);
        result = result.replace(pat, newtag);
    }
    for (int i = 0; i < convert_non_void_self_closing_tags.size(); ++i) {
        QString tag = convert_non_void_self_closing_tags.at(i);
        QString oldtag = "<\\s*" + tag + "(\\s*[^>/]*)/\\s*>";
        QString newtag = "<" + tag + "\\1></" + tag + ">";
        QRegularExpression pat(oldtag);
        result = result.replace(pat, newtag);
    }
    return result;
}


void GumboParser::rtrim(std::string &s) 
{
    s.erase(s.find_last_not_of(" \n\r\t")+1);
}


void GumboParser::ltrim(std::string &s)
{
    s.erase(0,s.find_first_not_of(" \n\r\t"));
}


void GumboParser::replace_all(std::string &s, const char * s1, const char * s2)
{
    std::string t1(s1);
    size_t len = t1.length();
    size_t pos = s.find(t1);
    while (pos != std::string::npos) {
      s.replace(pos, len, s2);
      pos = s.find(t1, pos + len);
    }
}


std::string GumboParser::substitute_xml_entities_into_text(const std::string &text)
{
    std::string result = text;
    // replacing & must come first 
    replace_all(result, "&", "&amp;");
    replace_all(result, "<", "&lt;");
    replace_all(result, ">", "&gt;");
    return result;
}


std::string GumboParser::substitute_xml_entities_into_attributes(char quote, const std::string &text)
{
    std::string result = substitute_xml_entities_into_text(text);
    if (quote == '"') {
        replace_all(result,"\"","&quot;");
    } else if (quote == '\'') {
        replace_all(result,"'","&apos;");
    }
    return result;
}


std::string GumboParser::handle_unknown_tag(GumboStringPiece *text)
{
    std::string tagname = "";
    if (text->data == NULL) {
        return tagname;
    }
    // work with copy GumboStringPiece to prevent asserts 
    // if try to read same unknown tag name more than once
    GumboStringPiece gsp = *text;
    gumbo_tag_from_original_text(&gsp);
    tagname = std::string(gsp.data, gsp.length);
    return tagname; 
}


std::string GumboParser::get_tag_name(GumboNode *node)
{
    std::string tagname;
    // work around lack of proper name for document node
    if (node->type == GUMBO_NODE_DOCUMENT) {
        tagname = "document";
    } else {
        tagname = gumbo_normalized_tagname(node->v.element.tag);
    }
    if (tagname.empty()) {
        tagname = handle_unknown_tag(&node->v.element.original_tag);
    }
    return tagname;
}


std::string GumboParser::build_doctype(GumboNode *node)
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


std::string GumboParser::build_attributes(GumboAttribute * at, bool no_entities)
{
    std::string atts = " ";
    atts.append(at->name);

    // how do we want to handle attributes with empty values
    // <input type="checkbox" checked />  or <input type="checkbox" checked="" /> 

    if ( (!std::string(at->value).empty())   || 
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
            atts.append(at->value);
        } else {
            atts.append(substitute_xml_entities_into_attributes(quote, std::string(at->value)));
        }
        atts.append(qs);
    }
    return atts;
}


// serialize children of a node
// may be invoked recursively

std::string GumboParser::serialize_contents(GumboNode* node) {
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

        } else if (child->type == GUMBO_NODE_ELEMENT) {
             contents.append(serialize(child));

        } else if (child->type == GUMBO_NODE_WHITESPACE) {
            // keep all whitespace to keep as close to original as possible
            contents.append(std::string(child->v.text.text));

        } else if (child->type != GUMBO_NODE_COMMENT) {
            // Does this actually exist: (child->type == GUMBO_NODE_CDATA)
            fprintf(stderr, "unknown element of type: %d\n", child->type); 
        }
    }
    return contents;
}


// serialize a GumboNode back to html/xhtml
// may be invoked recursively

std::string GumboParser::serialize(GumboNode* node) {
    // special case the document node
    if (node->type == GUMBO_NODE_DOCUMENT) {
        std::string results = build_doctype(node);
        results.append(serialize_contents(node));
        return results;
    }

    std::string close = "";
    std::string closeTag = "";
    std::string atts = "";
    std::string tagname            = get_tag_name(node);
    std::string key                = "|" + tagname + "|";
    bool need_special_handling     =  special_handling.find(key) != std::string::npos;
    bool is_empty_tag              = empty_tags.find(key) != std::string::npos;
    bool no_entity_substitution    = no_entity_sub.find(key) != std::string::npos;
    bool is_inline                 = nonbreaking_inline.find(key) != std::string::npos;

    // build attr string  
    const GumboVector * attribs = &node->v.element.attributes;
    for (int i=0; i< attribs->length; ++i) {
        GumboAttribute* at = static_cast<GumboAttribute*>(attribs->data[i]);
        atts.append(build_attributes(at, no_entity_substitution));
    }

    // determine closing tag type
    if (is_empty_tag) {
        close = "/";
    } else {
        closeTag = "</" + tagname + ">";
    }

    // serialize your contents
    std::string contents = serialize_contents(node);

    if (need_special_handling) {
        ltrim(contents);
        rtrim(contents);
        contents.append("\n");
    }

    // build results
    std::string results;
    results.append("<"+tagname+atts+close+">");
    if (need_special_handling) results.append("\n");
    results.append(contents);
    results.append(closeTag);
    if (need_special_handling) results.append("\n");
    return results;
}


// prettyprint children of a node
// may be invoked recursively

std::string GumboParser::prettyprint_contents(GumboNode* node, int lvl, const std::string indent_chars) 
{

  std::string contents        = "";
  std::string tagname         = get_tag_name(node);
  std::string key             = "|" + tagname + "|";
  bool no_entity_substitution = no_entity_sub.find(key) != std::string::npos;
  bool keep_whitespace        = preserve_whitespace.find(key) != std::string::npos;
  bool is_inline              = nonbreaking_inline.find(key) != std::string::npos;
  bool pp_okay                = !is_inline && !keep_whitespace;

  GumboVector* children = &node->v.element.children;

  for (unsigned int i = 0; i < children->length; ++i) {
    GumboNode* child = static_cast<GumboNode*> (children->data[i]);

    if (child->type == GUMBO_NODE_TEXT) {

      std::string val;

      if (no_entity_substitution) {
        val = std::string(child->v.text.text);
      } else {
        val = substitute_xml_entities_into_text(std::string(child->v.text.text));
      }

      if (pp_okay) rtrim(val);

      if (pp_okay && (contents.length() == 0)) {
        // add required indentation
        char c = indent_chars.at(0);
        int n  = indent_chars.length();
        contents.append(std::string((lvl-1)*n,c));
      }

      contents.append(val);


    } else if (child->type == GUMBO_NODE_ELEMENT) {

      std::string val = prettyprint(child, lvl, indent_chars);

      // remove any indentation if this child is inline and not first child
      std::string childname = get_tag_name(child);
      std::string childkey = "|" + childname + "|";
      if ((nonbreaking_inline.find(childkey) != std::string::npos) && (contents.length() > 0)) {
        ltrim(val);
      }

      contents.append(val);

    } else if (child->type == GUMBO_NODE_WHITESPACE) {

      if (keep_whitespace || is_inline) {
        contents.append(std::string(child->v.text.text));
      }

    } else if (child->type != GUMBO_NODE_COMMENT) {

      // Does this actually exist: (child->type == GUMBO_NODE_CDATA)
      fprintf(stderr, "unknown element of type: %d\n", child->type); 

    }

  }

  return contents;
}


// prettyprint a GumboNode back to html/xhtml
// may be invoked recursively

std::string GumboParser::prettyprint(GumboNode* node, int lvl, const std::string indent_chars)
{

  // special case the document node
  if (node->type == GUMBO_NODE_DOCUMENT) {
    std::string results = build_doctype(node);
    results.append(prettyprint_contents(node,lvl+1,indent_chars));
    return results;
  }

  std::string close              = "";
  std::string closeTag           = "";
  std::string atts               = "";
  std::string tagname            = get_tag_name(node);
  std::string key                = "|" + tagname + "|";
  bool need_special_handling     =  special_handling.find(key) != std::string::npos;
  bool is_empty_tag              = empty_tags.find(key) != std::string::npos;
  bool no_entity_substitution    = no_entity_sub.find(key) != std::string::npos;
  bool keep_whitespace           = preserve_whitespace.find(key) != std::string::npos;
  bool is_inline                 = nonbreaking_inline.find(key) != std::string::npos;
  bool pp_okay                   = !is_inline && !keep_whitespace;
  char c                         = indent_chars.at(0);
  int  n                         = indent_chars.length(); 

  // build attr string
  const GumboVector * attribs = &node->v.element.attributes;
  for (int i=0; i< attribs->length; ++i) {
    GumboAttribute* at = static_cast<GumboAttribute*>(attribs->data[i]);
    atts.append(build_attributes(at, no_entity_substitution));
  }

  // determine closing tag type
  if (is_empty_tag) {
      close = "/";
  } else {
      closeTag = "</" + tagname + ">";
  }

  std::string indent_space = std::string((lvl-1)*n,c);

  // prettyprint your contents 
  std::string contents = prettyprint_contents(node, lvl+1, indent_chars);

  if (need_special_handling) {
    rtrim(contents);
    contents.append("\n");
  }

  char last_char = ' ';
  if (!contents.empty()) {
    last_char = contents.at(contents.length()-1);
  } 

  // build results
  std::string results;
  if (pp_okay) {
    results.append(indent_space);
  }
  results.append("<"+tagname+atts+close+">");
  if (pp_okay) {
    results.append("\n");
  }
  results.append(contents);
  if (pp_okay && !contents.empty() && (last_char != '\n')) {
    results.append("\n");
  }
  if (pp_okay && !closeTag.empty()) {
    results.append(indent_space);
  }
  results.append(closeTag);
  if (pp_okay && !closeTag.empty()) {
    results.append("\n");
  }

  return results;
}

