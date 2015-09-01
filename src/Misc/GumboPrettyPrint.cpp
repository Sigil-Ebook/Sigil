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


#include "GumboPrettyPrint.h"

static std::string nonbreaking_inline  = "|a|abbr|acronym|b|bdo|big|br|cite|code|del|dfn|em|font|i|img|kbd|nobr|s|small|span|strike|strong|sub|sup|tt|u|wbr|";
static std::string empty_tags          = "|area|base|basefont|bgsound|br|command|col|embed|event-source|frame|hr|image|img|input|keygen|link|menuitem|meta|param|source|spacer|track|wbr|";
static std::string preserve_whitespace = "|pre|textarea|script|style|";
static std::string structural_tags     = "|article|aside|blockquote|body|canvas|div|dl|figure|footer|head|header|hr|html|ol|section|script|style|table|ul|";
static std::string special_handling    = "|html|head|body|";
static std::string no_entity_sub       = "|script|style|";

// Note: m_output contains the gumbo output tree which 
// has data structures with pointers into the original source
// buffer passed in!!!!!!

// This source buffer is provided by the m_utf8src std::string
// which should always exist unchanged alongside the output tree

// Do NOT change or delete m_utf8src once set until after you 
// have properly destroyed the gumbo output tree

GumboPrettyPrint::GumboPrettyPrint(const QString &source)
        : m_source(source),
          m_output(NULL),
          m_utf8src("")
{
}


GumboPrettyPrint::~GumboPrettyPrint()
{
    if (m_output != NULL) {
        gumbo_destroy_output(m_output);
        m_output = NULL;
        m_utf8src = "";
    }
}


void GumboPrettyPrint::parse()
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


QString GumboPrettyPrint::prettyprint(QString indent_chars)
{
    QString result = "";
    if (!m_source.isEmpty()) {
        if (m_output == NULL) {
            parse();
        }
        std::string ind = indent_chars.toStdString();
        std::string utf8out = prettyprint(m_output->document, 0, ind);
        result =  "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n" + QString::fromStdString(utf8out);
    }
    return result;
}

void GumboPrettyPrint::rtrim(std::string &s) 
{
    s.erase(s.find_last_not_of(" \n\r\t")+1);
}


void GumboPrettyPrint::ltrim(std::string &s)
{
    s.erase(0,s.find_first_not_of(" \n\r\t"));
}


void GumboPrettyPrint::replace_all(std::string &s, const char * s1, const char * s2)
{
    std::string t1(s1);
    size_t len = t1.length();
    size_t pos = s.find(t1);
    while (pos != std::string::npos) {
      s.replace(pos, len, s2);
      pos = s.find(t1, pos + len);
    }
}


std::string GumboPrettyPrint::substitute_xml_entities_into_text(const std::string &text)
{
    std::string result = text;
    // replacing & must come first 
    replace_all(result, "&", "&amp;");
    replace_all(result, "<", "&lt;");
    replace_all(result, ">", "&gt;");
    return result;
}


std::string GumboPrettyPrint::substitute_xml_entities_into_attributes(char quote, const std::string &text)
{
    std::string result = substitute_xml_entities_into_text(text);
    if (quote == '"') {
        replace_all(result,"\"","&quot;");
    } else if (quote == '\'') {
        replace_all(result,"'","&apos;");
    }
    return result;
}


std::string GumboPrettyPrint::get_tag_name(GumboNode *node)
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
      // since case change only - length must be same as original.
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

std::string GumboPrettyPrint::build_doctype(GumboNode *node)
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


std::string GumboPrettyPrint::build_attributes(GumboAttribute * at, bool no_entities)
{
    std::string atts = " ";
    atts.append(" ");
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


// prettyprint children of a node
// may be invoked recursively

std::string GumboPrettyPrint::prettyprint_contents(GumboNode* node, int lvl, const std::string indent_chars) 
{
  std::string contents        = "";
  std::string tagname         = get_tag_name(node);
  std::string key             = "|" + tagname + "|";
  bool no_entity_substitution = no_entity_sub.find(key) != std::string::npos;
  bool keep_whitespace        = preserve_whitespace.find(key) != std::string::npos;
  bool is_inline              = nonbreaking_inline.find(key) != std::string::npos;
  bool is_structural          = structural_tags.find(key) != std::string::npos;
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

      contents.append(val);

    } else if (child->type == GUMBO_NODE_ELEMENT || child->type == GUMBO_NODE_TEMPLATE) {

      std::string val = prettyprint(child, lvl, indent_chars);
      contents.append(val);

    } else if (child->type == GUMBO_NODE_WHITESPACE) {

      if (keep_whitespace || is_inline) {
        std::string wspace = std::string(child->v.text.text);
        contents.append(wspace);
      }

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


// prettyprint a GumboNode back to html/xhtml
// may be invoked recursively

std::string GumboPrettyPrint::prettyprint(GumboNode* node, int lvl, const std::string indent_chars)
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
  std::string parentname         = "|" + get_tag_name(node->parent) + "|";
  std::string key                = "|" + tagname + "|";
  bool need_special_handling     =  special_handling.find(key) != std::string::npos;
  bool is_empty_tag              = empty_tags.find(key) != std::string::npos;
  bool no_entity_substitution    = no_entity_sub.find(key) != std::string::npos;
  bool keep_whitespace           = preserve_whitespace.find(key) != std::string::npos;
  bool is_inline                 = (nonbreaking_inline.find(key) != std::string::npos) && (structural_tags.find(parentname) == std::string::npos);
  bool is_structural             = structural_tags.find(key) != std::string::npos;
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
  std::string contents;

  // prettyprint your contents 
  if (is_structural) {
    contents = prettyprint_contents(node, lvl+1, indent_chars);
  } else {
    contents = prettyprint_contents(node, lvl, indent_chars);
  }

  if (is_structural) {
    rtrim(contents);
    if (!contents.empty()) contents.append("\n");
  }

  char last_char = ' ';
  if (!contents.empty()) {
    last_char = contents.at(contents.length()-1);
  } 

  // build results
  std::string results;

  if (!is_inline) {
    results.append(indent_space);
  }

  results.append("<"+tagname+atts+close+">");

  if (pp_okay && is_structural && !contents.empty()) {
    results.append("\n");
  }

  results.append(contents);

  if (pp_okay && (last_char != '\n') && !contents.empty() && is_structural) {
    results.append("\n");
  }

  // handle any indent before structural close tags
  if (!is_inline && is_structural && !closeTag.empty() && !contents.empty()) {
    results.append(indent_space);
  }

  results.append(closeTag);

  if (pp_okay) {
    results.append("\n\n");
  }

  return results;
}
