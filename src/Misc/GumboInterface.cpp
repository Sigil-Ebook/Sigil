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
#include <QUrl>
#include <QFileInfo>

#include "Misc/Utility.h"
#include "GumboInterface.h"
#include "string_buffer.h"
#include "error.h"


static std::unordered_set<std::string> nonbreaking_inline  = { 
  "a","abbr","acronym","b","bdo","big","br","button","cite","code","del",
  "dfn","em","font","i","image","img","input","ins","kbd","label","map",
  "nobr","object","q","ruby","rt","s","samp","select","small","span","strike","strong",
  "sub","sup","textarea","tt","u","var","wbr", "mbp:nu"
};


static std::unordered_set<std::string> preserve_whitespace = {
  "pre","textarea","script","style"
};


static std::unordered_set<std::string> special_handling    = { 
  "html","body"
};


static std::unordered_set<std::string> no_entity_sub       = {
  "script","style"
};


static std::unordered_set<std::string> void_tags          = {
  "area","base","basefont","bgsound","br","col","command","embed",
  "event-source","frame","hr","img","input","keygen","link",
  "meta","param","source","spacer","track","wbr", 
  "mbp:pagebreak", "mglyph", "mspace", "mprescripts", "none",
  "maligngroup", "malignmark", "msline"
};


static std::unordered_set<std::string> structural_tags     = {
  "article","aside","blockquote","body","canvas","colgroup","div","dl",
  "figure","footer","head","header","hr","html","ol","section",
  "table","tbody","tfoot","thead","td","th","tr","ul"
};


static std::unordered_set<std::string> other_text_holders = {
  "address","caption","dd","div","dt","h1","h2","h3","h4","h5","h6",
  "legend","li","option","p","td","th","title"
};


static std::unordered_set<std::string> manifest_properties = {
  "math","nav","script","svg","epub:switch"
};


static std::unordered_set<std::string> href_src_tags       = {
  "a","area","audio","base","embed","font-face-uri","frame","iframe",
  "image","img","input","link","object","script","source","track","video"
};


static const QChar POUND_SIGN    = QChar::fromLatin1('#');
static const QChar FORWARD_SLASH = QChar::fromLatin1('/');
static const std::string aSRC = std::string("src");
static const std::string aHREF = std::string("href");
static const std::string aPOSTER = std::string("poster");
static const std::string aDATA = std::string("data");
QHash<QString,QString> EmptyHash = QHash<QString,QString>();

// These need to match the GumboAttributeNamespaceEnum sequence
static const char * attribute_nsprefixes[4] = { "", "xlink:", "xml:", "xmlns:" };
 
// Note: m_output contains the gumbo output tree which 
// has data structures with pointers into the original source
// buffer passed in!!!!!!

// This source buffer is provided by the m_utf8src std::string
// which should always exist unchanged alongside the output tree

// Do NOT change or delete m_utf8src once set until after you 
// have properly destroyed the gumbo output tree

GumboInterface::GumboInterface(const QString &source, const QString &version)
        : m_source(source),
          m_output(NULL),
          m_utf8src(""),
          m_sourceupdates(EmptyHash),
          m_newcsslinks(""),
          m_currentdir(""),
          m_newbody(""),
          m_hasnbsp(false),
          m_version(version)
{
}


GumboInterface::GumboInterface(const QString &source, const QString &version, const QHash<QString,QString> & source_updates)
        : m_source(source),
          m_output(NULL),
          m_utf8src(""),
          m_sourceupdates(source_updates),
          m_newcsslinks(""),
          m_currentdir(""),
          m_newbody(""),
          m_hasnbsp(false),
          m_version(version)
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
  
        if (!m_version.startsWith('3')) {
            m_hasnbsp = m_source.contains("&nbsp;");
        }
        m_utf8src = m_source.toStdString();
        // remove any xml header line and any trailing whitespace
        if (m_utf8src.compare(0,5,"<?xml") == 0) {
            size_t end = m_utf8src.find_first_of('>', 5);
            end = m_utf8src.find_first_not_of("\n\r\t\v\f ",end+1);
            m_utf8src.erase(0,end);
        }

        // In case we ever have to revert to earlier versions, please note the following
        // additional initialization is needed because Microsoft Visual Studio 2013 (and earlier?)
        // do not properly initialize myoptions from the static const kGumboDefaultOptions defined
        // in the gumbo library.  Instead whatever was in memory at the time is used causing random 
        // issues later on so if reverting remember to keep these specific changes as the bug 
        // they work around took a long long time to track down
        GumboOptions myoptions = kGumboDefaultOptions;
        myoptions.tab_stop = 4;
        myoptions.use_xhtml_rules = true;
        myoptions.stop_on_first_error = false;
        myoptions.max_errors = -1;

        // GumboInterface::m_mutex.lock();
        m_output = gumbo_parse_with_options(&myoptions, m_utf8src.data(), m_utf8src.length());
        // GumboInterface::m_mutex.unlock();
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
        rtrim(utf8out);
        result =  "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n" + QString::fromStdString(utf8out);
    }
    return result;
}


QString GumboInterface::getxhtml()
{
    QString result = "";
    if (!m_source.isEmpty()) {
        if (m_output == NULL) {
            parse();
        }
        std::string utf8out = serialize(m_output->document);
        rtrim(utf8out);
        result =  "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n" + QString::fromStdString(utf8out);
    }
    return result;
}


QString GumboInterface::prettyprint(QString indent_chars)
{
    QString result = "";
    if (!m_source.isEmpty()) {
        if (m_output == NULL) {
            parse();
        }
        std::string ind = indent_chars.toStdString();
        std::string utf8out = prettyprint(m_output->document, 0, ind);
        rtrim(utf8out);
        result =  "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n" + QString::fromStdString(utf8out);
    }
    return result;
}


QStringList GumboInterface::get_all_properties()
{
    QStringList properties;
    if (!m_source.isEmpty()) {
        if (m_output == NULL) {
            parse();
        }
        properties = get_properties(m_output->root);
    }
    return properties;
}


QString GumboInterface::perform_source_updates(const QString& my_current_book_relpath)
{
    m_currentdir = QFileInfo(my_current_book_relpath).dir().path();
    QString result = "";
    if (!m_source.isEmpty()) {
        if (m_output == NULL) {
            parse();
        }
        enum UpdateTypes doupdates = SourceUpdates;
        std::string utf8out = serialize(m_output->document, doupdates);
        rtrim(utf8out);
        result =  "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n" + QString::fromStdString(utf8out);
    }
    return result;
}


QString GumboInterface::perform_style_updates(const QString& my_current_book_relpath)
{
    m_currentdir = QFileInfo(my_current_book_relpath).dir().path();
    QString result = "";
    if (!m_source.isEmpty()) {
        if (m_output == NULL) {
            parse();
        }
        enum UpdateTypes doupdates = StyleUpdates;
        std::string utf8out = serialize(m_output->document, doupdates);
        rtrim(utf8out);
        result =  "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n" + QString::fromStdString(utf8out);
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
        rtrim(utf8out);
        result =  "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n" + QString::fromStdString(utf8out);
    }
    return result;
}


GumboNode * GumboInterface::get_root_node() {
    if (!m_source.isEmpty()) {
        if (m_output == NULL) {
            parse();
        }
    }
    return m_output->root;
}


QString GumboInterface::get_body_contents() 
{
    if (!m_source.isEmpty()) {
        if (m_output == NULL) {
            parse();
        }
    }
    QList<GumboTag> tags = QList<GumboTag>() << GUMBO_TAG_BODY;
    QList<GumboNode*> nodes = get_all_nodes_with_tags(tags);
    if (nodes.count() != 1) {
        return QString();
    }
    enum UpdateTypes doupdates = NoUpdates;
    std::string results = serialize_contents(nodes.at(0), doupdates);
    return QString::fromStdString(results);
}


QStringList GumboInterface::get_properties(GumboNode* node)
{
    if (node->type != GUMBO_NODE_ELEMENT) {
        return QStringList();
    }
    QStringList properties;
    std::string tagname = get_tag_name(node);
    if (in_set(manifest_properties, tagname)) {
        properties.append(QString::fromStdString(tagname));
    }
    GumboAttribute* attr = gumbo_get_attribute(&node->v.element.attributes, "src");
    if (attr && !QUrl(QString::fromUtf8(attr->value)).isRelative()) {
        properties.append(QString("remote-resources"));
    }
    GumboVector* children = &node->v.element.children;
    for (unsigned int i = 0; i < children->length; ++i) {
        properties.append(get_properties(static_cast<GumboNode*>(children->data[i])));
    }
    return properties;
}


QString GumboInterface::get_qwebpath_to_node(GumboNode* node) 
{
    QStringList path_pieces;
    GumboNode* anode = node;
    while (anode && !((anode->type == GUMBO_NODE_ELEMENT) && (anode->v.element.tag == GUMBO_TAG_HTML))) {
        GumboNode* myparent = anode->parent;
        QString parent_name = QString::fromStdString(get_tag_name(myparent));
        int index;
        QString aname = QString::fromStdString(get_tag_name(anode));
        if (aname == "#text") {
            index = anode->index_within_parent;
        } else {
            // need to find child num in parent as if only elements exist
            GumboVector* children = &myparent->v.element.children;
            int elnum = 0;
            for (unsigned int i=0; i < children->length; i++) {
                GumboNode* child = static_cast<GumboNode*>(children->data[i]);
                if (i == anode->index_within_parent) {
                    break;
                }
                if ((child->type == GUMBO_NODE_ELEMENT) || (child->type == GUMBO_NODE_TEMPLATE)) {
                    elnum++;
                } 
            }
            index = elnum;
        }
        path_pieces.prepend(parent_name + " " +  QString::number(index));
        anode = myparent;
    }
    return path_pieces.join(",");
}


GumboNode* GumboInterface::get_node_from_qwebpath(QString webpath) 
{
    QStringList path_pieces = webpath.split(",", QString::SkipEmptyParts);
    GumboNode* node = get_root_node();
    GumboNode* end_node = node;
    for (int i=0; i < path_pieces.count() - 1 ; ++i) {
        QString piece = path_pieces.at(i);
        QString name = piece.split(" ")[0];
        int index = piece.split(" ")[1].toInt();
        GumboVector* children = &node->v.element.children;
        GumboNode * next_node = NULL;
        if (children->length > 0) {
            if (path_pieces.at(i+1).startsWith("#text")) {
                next_node = static_cast<GumboNode*>(children->data[index]);
            } else {
                // need to index correct child index when only counting elements
                int elnum = -1;
                for (unsigned int j=0; j < children->length; j++) {
                    GumboNode* child = static_cast<GumboNode*>(children->data[j]);
                    if ((child->type == GUMBO_NODE_ELEMENT) || (child->type == GUMBO_NODE_TEMPLATE)) {
                        elnum++;
                    }
                    if (elnum == index) {
                        next_node = child;
                        break;
                    }
                }
            }
            if (next_node) {
                end_node = next_node;
                node = next_node;
            } else {
                break;
            }
         }
     }
     return end_node;
}

QList<unsigned int> GumboInterface::get_path_to_node(GumboNode* node) 
{
  QList<unsigned int> apath = QList<unsigned int>();
  GumboNode* anode = node;
  while (anode && !((anode->type == GUMBO_NODE_ELEMENT) && (anode->v.element.tag == GUMBO_TAG_HTML))) {
      apath.prepend(anode->index_within_parent);
      anode = anode->parent;
  }
  return apath;
}


GumboNode* GumboInterface::get_node_from_path(QList<unsigned int> & apath) 
{
   GumboNode* dest = get_root_node();
   foreach(unsigned int childnum, apath) {
       if ((dest->type == GUMBO_NODE_ELEMENT) || (dest->type == GUMBO_NODE_TEMPLATE)) {
           GumboVector* children = &dest->v.element.children;
           if (childnum < children->length) {
               dest = static_cast<GumboNode*>(children->data[childnum]);
           } else {
             break;
           }
       } else {
           break;
       }
   }
   return dest;
}


QString GumboInterface::perform_body_updates(const QString & new_body) 
{
    QString result = "";
    if (!m_source.isEmpty()) {
        if (m_output == NULL) {
            parse();
        }
    }
    QList<GumboTag> tags = QList<GumboTag>() << GUMBO_TAG_BODY;
    QList<GumboNode*> nodes = get_all_nodes_with_tags(tags);
    if (nodes.count() != 1) {
        return QString();
    }
    m_newbody = new_body.toStdString();
    enum UpdateTypes doupdates = BodyUpdates;
    std::string utf8out = serialize(m_output->document, doupdates);
    result =  "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n" + QString::fromStdString(utf8out);
    m_newbody= "";
    return result;
}
    

QList<GumboWellFormedError> GumboInterface::error_check()
{
    QList<GumboWellFormedError> errlist;
    int line_offset = 0;

    // In case we ever have to revert to earlier versions, please note the following
    // additional initialization is needed because Microsoft Visual Studio 2013 (and earlier?)
    // do not properly initialize myoptions from the static const kGumboDefaultOptions defined
    // in the gumbo library.  Instead whatever was in memory at the time is used causing random 
    // issues later on so if reverting remember to keep these specific changes as the bug 
    // they work around took a long long time to track down
    GumboOptions myoptions = kGumboDefaultOptions;
    myoptions.tab_stop = 4;
    myoptions.use_xhtml_rules = true;
    myoptions.stop_on_first_error = false;
    myoptions.max_errors = -1;

    if (!m_source.isEmpty() && (m_output == NULL)) {

        m_utf8src = m_source.toStdString();
        // remove any xml header line and trailing whitespace
        if (m_utf8src.compare(0,5,"<?xml") == 0) {
            size_t end = m_utf8src.find_first_of('>', 0);
            end = m_utf8src.find_first_not_of("\n\r\t\v\f ",end+1);
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
    for (unsigned int i=0; i< errors->length; ++i) {
        GumboError* er = static_cast<GumboError*>(errors->data[i]);
        GumboWellFormedError gperror;
        gperror.line = er->position.line + line_offset;;
        gperror.column = er->position.column;
        // unsigned int typenum = er->type;
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
        nodes = get_nodes_with_attribute(m_output->root, attname.toUtf8().constData()); 
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
  for (unsigned int i = 0; i < children->length; ++i) {
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
        attrvals = get_values_for_attr(m_output->root, attname.toUtf8().constData()); 
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
    for (unsigned int i = 0; i < children->length; ++i) {
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
    for (unsigned int i=0; i< attribs->length; ++i) {
        GumboAttribute* attr = static_cast<GumboAttribute*>(attribs->data[i]);
        QString key = QString::fromStdString(get_attribute_name(attr));
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

        } else if (child->type == GUMBO_NODE_ELEMENT) {
            node_text += get_local_text_of_node(child);
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
  for (unsigned int i = 0; i < children->length; ++i) {
    nodes.append(get_nodes_with_tags(static_cast<GumboNode*>(children->data[i]), tags));
  }
  return nodes;
}


bool GumboInterface::in_set(std::unordered_set<std::string> &s, std::string &key)
{
  return s.find(key) != s.end();
}


void GumboInterface::rtrim(std::string &s) 
{
    s.erase(s.find_last_not_of(" \n\r\t\v\f")+1);
}


void GumboInterface::ltrim(std::string &s)
{
    s.erase(0,s.find_first_not_of(" \n\r\t\v\f"));
}


void GumboInterface::ltrimnewlines(std::string &s)
{
    s.erase(0,s.find_first_not_of("\n\r"));
}


// delete everything up to and including the newline
void GumboInterface::newlinetrim(std::string &s)
{
  size_t pos = s.find("\n");
  if (pos != std::string::npos) {
    s.erase(0, pos+1);
  }
}


void GumboInterface::condense_whitespace(std::string &s)
{
    size_t n = s.length();
    std::string val;
    val.reserve(n);
    std::string wspace = " \n\r\t\v\f";
    char last_c = 'x';
    for (size_t i=0; i < n; i++) {
        char c = s.at(i);
        if (wspace.find(c) != std::string::npos) {
            c = ' ';
        }
        if ((c != ' ') || (last_c != ' ')) {
            val.push_back(c);
        }
        last_c = c;
    }
    s = val;
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


std::string GumboInterface::update_attribute_value(const std::string &attvalue)
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
    QString new_href;
    if (m_sourceupdates.contains(search_key)) {
        new_href = m_sourceupdates.value(search_key);
    }
    if (!new_href.isEmpty()) {
        new_href += fragment;
        new_href = Utility::URLEncodePath(new_href);
        result =  new_href.toStdString();
    } 
    return result;
}


std::string GumboInterface::update_style_urls(const std::string &source)
{
    QString result = QString::fromStdString(source);
    // Now parse the text once looking urls and replacing them where needed
    QRegularExpression reference(
        "(?:(?:src|background|background-image|list-style|list-style-image|border-image|border-image-source|content)\\s*:|@import)\\s*"
        "[^;\\}\\(\"']*"
        "(?:"
        "url\\([\"']?([^\\(\\)\"']*)[\"']?\\)"
        "|"
        "[\"']([^\\(\\)\"']*)[\"']"
        ")");
    int start_index = 0;
    QRegularExpressionMatch mo = reference.match(result, start_index);
    do {
        for (int i = 1; i < reference.captureCount(); ++i) {
            if (mo.captured(i).trimmed().isEmpty()) {
                continue;
            }
            QString apath = Utility::URLDecodePath(mo.captured(i));
            QString search_key = QDir::cleanPath(m_currentdir + FORWARD_SLASH + apath);
            QString new_href;
            if (m_sourceupdates.contains(search_key)) {
                new_href = m_sourceupdates.value(search_key);
            }
            if (!new_href.isEmpty()) {
                new_href = Utility::URLEncodePath(new_href);
                result.replace(mo.capturedStart(i), mo.capturedLength(i), new_href);
            }
        }
        start_index += mo.capturedLength();
        mo = reference.match(result, start_index);
    } while (mo.hasMatch());

    return result.toStdString();
}



std::string GumboInterface::substitute_xml_entities_into_text(const std::string &text)
{
    std::string result = text;
    // replacing & must come first 
    replace_all(result, "&", "&amp;");
    replace_all(result, "<", "&lt;");
    replace_all(result, ">", "&gt;");
    // convert non-breaking spaces to entities to prevent their loss for later editing
    // See the strange//buggy behaviour of Qt QTextDocument toPlainText() routine 
    if (m_hasnbsp) {
        replace_all(result, "\xc2\xa0", "&nbsp;");
    } else {
        replace_all(result, "\xc2\xa0", "&#160;");
    }
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
    tagname = "#document";
    return tagname;
  } else if ((node->type == GUMBO_NODE_TEXT) || (node->type == GUMBO_NODE_WHITESPACE)) {
    tagname = "#text";
    return tagname;
  } else if (node->type == GUMBO_NODE_CDATA) {
    tagname = "#cdata";
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


// if missing leave it alone
// if epub3 docytpe use it otherwise set it to epub2 docytpe
std::string GumboInterface::build_doctype(GumboNode *node)
{
    std::string results = "";
    if (m_version.startsWith('2')) {
        results.append("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.1//EN\"\n");
        results.append("  \"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd\">\n\n");
        return results;
    } else if (m_version.startsWith('3')) {
        results.append("<!DOCTYPE html>\n\n");
        return results; 
    }
    if (node->v.document.has_doctype) {
        std::string name(node->v.document.name);
        std::string pi(node->v.document.public_identifier);
        std::string si(node->v.document.system_identifier);
        if ((name == "html") && pi.empty() && si.empty()) {
            results.append("<!DOCTYPE html>\n\n");
            return results;
        } else {
            results.append("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.1//EN\"\n");
            results.append("  \"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd\">\n\n");
        }
    }
    return results;
}


// deal properly with foreign namespaced attributes
std::string GumboInterface::get_attribute_name(GumboAttribute * at)
{
    std::string attr_name = at->name;
    GumboAttributeNamespaceEnum attr_ns = at->attr_namespace;
    if ((attr_ns == GUMBO_ATTR_NAMESPACE_NONE) || (attr_name == "xmlns")) {
        return attr_name;
    } 
    attr_name = std::string(attribute_nsprefixes[attr_ns]) + attr_name;
    return attr_name;
}


std::string GumboInterface::build_attributes(GumboAttribute * at, bool no_entities, 
                                             bool run_src_updates, bool run_style_updates)
{
    std::string atts = " ";
    std::string name = get_attribute_name(at);
    std::string local_name = at->name;
    atts.append(name);
    std::string attvalue = at->value;

    if (run_src_updates && (local_name == aHREF || local_name == aSRC || 
                            local_name == aPOSTER || local_name == aDATA)) {
        attvalue = update_attribute_value(attvalue);
    }

    if (run_style_updates && (local_name == "style")) {
        attvalue = update_style_urls(attvalue);
    }

    // we handle empty attribute values like so: alt=""
    char quote = '"';
    std::string qs="\"";

    // verify an original value existed since we create our own attributes
    // and if so determine the original quote character used if any

    if (at->original_value.data) {
        if ( (!attvalue.empty())   || 
             (at->original_value.data[0] == '"') || 
             (at->original_value.data[0] == '\'') ) {

          quote = at->original_value.data[0];
          if (quote == '\'') qs = std::string("'");
          if (quote == '"') qs = std::string("\"");
        }
    }

    atts.append("=");
    atts.append(qs);
    if (no_entities) {
        atts.append(attvalue);
    } else {
        atts.append(substitute_xml_entities_into_attributes(quote, attvalue));
    }
    atts.append(qs);
    return atts;
}


// serialize children of a node
// may be invoked recursively

std::string GumboInterface::serialize_contents(GumboNode* node, enum UpdateTypes doupdates) {
    std::string contents        = "";
    std::string tagname         = get_tag_name(node);
    bool no_entity_substitution = in_set(no_entity_sub, tagname);
    bool keep_whitespace        = in_set(preserve_whitespace, tagname);
    bool is_inline              = in_set(nonbreaking_inline, tagname);
    bool is_structural          = in_set(structural_tags, tagname);

    // build up result for each child, recursively if need be
    GumboVector* children = &node->v.element.children;

    bool inject_newline = false;
    bool in_head_without_title = (tagname == "head");

    for (unsigned int i = 0; i < children->length; ++i) {
        GumboNode* child = static_cast<GumboNode*> (children->data[i]);

        if (child->type == GUMBO_NODE_TEXT) {
            inject_newline = false;
            if (no_entity_substitution) {
                contents.append(std::string(child->v.text.text));
            } else {
                contents.append(substitute_xml_entities_into_text(std::string(child->v.text.text)));
            }

        } else if (child->type == GUMBO_NODE_ELEMENT || child->type == GUMBO_NODE_TEMPLATE) {
            contents.append(serialize(child, doupdates));
            inject_newline = false;
            std::string childname = get_tag_name(child);
            if (in_head_without_title && (childname == "title")) in_head_without_title = false;
            if (!is_inline && !keep_whitespace && !in_set(nonbreaking_inline,childname) && is_structural) {
                contents.append("\n");
                inject_newline = true;
            }

        } else if (child->type == GUMBO_NODE_WHITESPACE) {
            // try to keep all whitespace to keep as close to original as possible
            std::string wspace = std::string(child->v.text.text);
            if (inject_newline) {
                newlinetrim(wspace);
                inject_newline = false;
            }
            contents.append(wspace);
            inject_newline = false;

        } else if (child->type == GUMBO_NODE_CDATA) {
            contents.append("<![CDATA[" + std::string(child->v.text.text) + "]]>");
            inject_newline = false;

        } else if (child->type == GUMBO_NODE_COMMENT) {
            contents.append("<!--" + std::string(child->v.text.text) + "-->");
 
        } else {
            fprintf(stderr, "unknown element of type: %d\n", child->type); 
            inject_newline = false;
        }

    }
    if (in_head_without_title) contents.append("<title></title>");
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
    bool need_special_handling     = in_set(special_handling, tagname);
    bool is_void_tag              = in_set(void_tags, tagname);
    bool no_entity_substitution    = in_set(no_entity_sub, tagname);
    // bool is_inline                 = in_set(nonbreaking_inline, tagname);
    bool is_href_src_tag           = in_set(href_src_tags, tagname);

    // build attr string  
    const GumboVector * attribs = &node->v.element.attributes;
    for (unsigned int i=0; i< attribs->length; ++i) {
        GumboAttribute* at = static_cast<GumboAttribute*>(attribs->data[i]);
        atts.append(build_attributes(at, no_entity_substitution, ((doupdates & SourceUpdates) && is_href_src_tag), (doupdates & StyleUpdates)));
    }

    // Make sure that the xmlns attribute exists as an html tag attribute
    if (tagname == "html") {
      if (atts.find("xmlns=") == std::string::npos) {
        atts.append(" xmlns=\"http://www.w3.org/1999/xhtml\"");
      }
    }

    // determine closing tag type
    if (is_void_tag) {
        close = "/";
    } else {
        closeTag = "</" + tagname + ">";
    }

    std::string contents;

    if ((tagname == "body") && (doupdates & BodyUpdates)) {
        contents = m_newbody;
    } else {
        // serialize your contents
        contents = serialize_contents(node, doupdates);
    }

    if ((doupdates & StyleUpdates) && (tagname == "style") && 
        (node->parent->type == GUMBO_NODE_ELEMENT) && 
        (node->parent->v.element.tag == GUMBO_TAG_HEAD)) {
        contents = update_style_urls(contents);
    }

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



std::string GumboInterface::prettyprint_contents(GumboNode* node, int lvl, const std::string indent_chars) 
{
    std::string contents        = "";
    std::string tagname         = get_tag_name(node);
    bool no_entity_substitution = in_set(no_entity_sub, tagname);
    bool keep_whitespace        = in_set(preserve_whitespace, tagname);
    bool is_inline              = in_set(nonbreaking_inline, tagname);
    bool is_structural          = in_set(structural_tags, tagname);
    char c                      = indent_chars.at(0);
    int  n                      = indent_chars.length(); 
    std::string indent_space    = std::string((lvl-1)*n,c);
    char last_char              = 'x';
    bool contains_block_tags    = false;

    GumboVector* children = &node->v.element.children;

    if (is_structural || (tagname == "#document")) last_char = '\n';
    bool in_head_without_title = (tagname == "head");

    for (unsigned int i = 0; i < children->length; ++i) {

        GumboNode* child = static_cast<GumboNode*> (children->data[i]);

        if (child->type == GUMBO_NODE_TEXT) {
            std::string val;

            if (no_entity_substitution) {
                val = std::string(child->v.text.text);
            } else {
                val = substitute_xml_entities_into_text(std::string(child->v.text.text));
            }

            // if child of a structual element is text and follows a newline, indent it properly
            if (is_structural && last_char == '\n') {
                contents.append(indent_space);
                ltrim(val);
            }
            if (!keep_whitespace && !is_structural) {
                // okay to condense whitespace
                condense_whitespace(val);
            }
            contents.append(val);

        } else if (child->type == GUMBO_NODE_ELEMENT || child->type == GUMBO_NODE_TEMPLATE) {

            std::string val = prettyprint(child, lvl, indent_chars);
            std::string childname = get_tag_name(child);
            if (in_head_without_title && (childname == "title")) in_head_without_title = false;
            if (!in_set(nonbreaking_inline, childname)) {
                contains_block_tags = true;
                if (last_char != '\n') {
                    contents.append("\n");
                    if (tagname != "head" && tagname != "html") contents.append("\n");
                    last_char='\n';
                }
            }
            // if child of a structual element is inline and follows a newline, indent it properly
            if (is_structural && in_set(nonbreaking_inline, childname) && (last_char == '\n')) {
                contents.append(indent_space);
                ltrim(val);
            }    
            contents.append(val);

        } else if (child->type == GUMBO_NODE_WHITESPACE) {

            if (keep_whitespace) {
                std::string wspace = std::string(child->v.text.text);
                contents.append(wspace);
            } else if (is_inline || in_set(other_text_holders, tagname)) {
                if (std::string(" \t\v\f\r\n").find(last_char) == std::string::npos) {
                    contents.append(std::string(" "));
                }
            }

        } else if (child->type == GUMBO_NODE_CDATA) {
            contents.append("<![CDATA[" + std::string(child->v.text.text) + "]]>");

        } else if (child->type == GUMBO_NODE_COMMENT) {
            contents.append("<!--" + std::string(child->v.text.text) + "-->");
 
        } else {
            fprintf(stderr, "unknown element of type: %d\n", child->type); 
        }

        // update last character of current contents
        if (!contents.empty()) {
            last_char = contents.at(contents.length()-1);
        }

    }

    // inject epmpty title into head if one is missing
    if (in_head_without_title) {
        if (last_char != '\n') contents.append("\n");
        contents.append(indent_space + "<title></title>\n");
        last_char = '\n';
    }

    // treat inline tags containing block tags like a block tag
    if (is_inline && contains_block_tags) {
      if (last_char != '\n') contents.append("\n\n");
      contents.append(indent_space);
    }

    return contents;
}


// prettyprint a GumboNode back to html/xhtml
// may be invoked recursively

std::string GumboInterface::prettyprint(GumboNode* node, int lvl, const std::string indent_chars)
{

    // special case the document node
    if (node->type == GUMBO_NODE_DOCUMENT) {
      std::string results = build_doctype(node);
      results.append(prettyprint_contents(node,lvl+1,indent_chars));
      return results;
    }

    std::string tagname = get_tag_name(node);
    std::string parentname = get_tag_name(node->parent);
    bool in_head = (parentname == "head");

    bool is_structural = in_set(structural_tags, tagname);
    bool is_inline = in_set(nonbreaking_inline, tagname);

    // build attr string
    std::string atts = "";
    bool no_entity_substitution = in_set(no_entity_sub, tagname);
    const GumboVector * attribs = &node->v.element.attributes;
    for (unsigned int i=0; i< attribs->length; ++i) {
        GumboAttribute* at = static_cast<GumboAttribute*>(attribs->data[i]);
        atts.append(build_attributes(at, no_entity_substitution));
    }

    bool is_void_tag = in_set(void_tags, tagname);

    // get tag contents
    std::string contents = "";
    if (!is_void_tag) {
        if (is_structural && tagname != "html") {
            contents = prettyprint_contents(node, lvl+1, indent_chars);
        } else {
            contents = prettyprint_contents(node, lvl, indent_chars);
        }
    }

    bool keep_whitespace = in_set(preserve_whitespace, tagname);
    if (!keep_whitespace && !is_inline) {
        rtrim(contents);
    }

    bool single = is_void_tag;
    // for xhtml serialization that allows non-void tags to be self-closing
    // uncomment the following line
    // single = single || contents.empty();

    char c = indent_chars.at(0);
    int  n = indent_chars.length(); 
    std::string indent_space = std::string((lvl-1)*n,c);

    // handle self-closed tags with no contents first
    if (single) {
        std::string selfclosetag = "<" + tagname + atts + "/>";
        if (is_inline) {
            // always add newline after br tags when they are children of structural tags
            if ((tagname == "br") && in_set(structural_tags, parentname)) {
              selfclosetag.append("\n");
              if (!in_head && (tagname != "html")) selfclosetag.append("\n");
            }
            return selfclosetag;
        }
        if (!in_head && (tagname != "html")) selfclosetag.append("\n");
        return indent_space + selfclosetag + "\n";
    } 

    // Handle the general case
    std::string results;
    std::string starttag = "<" + tagname +  atts + ">";
    std::string closetag = "</" + tagname + ">";

    if (is_structural) {
        results = indent_space + starttag;
        if (!contents.empty()) {
            results.append("\n" + contents + "\n" + indent_space);
        }  
        results.append(closetag + "\n");
        if (!in_head && (tagname != "html")) results.append("\n");
    } else if (is_inline) {
        results = starttag;
        results.append(contents);
        results.append(closetag);
    } else /** all others */ {
        results = indent_space + starttag;
        if (!keep_whitespace) {
            ltrim(contents);
        }
        results.append(contents);
        results.append(closetag + "\n");
        if (!in_head && (tagname != "html")) results.append("\n");
    }
    return results;
}




// The below are hopefully obsolete now but keep them since
// they are a real pain to recreate and may be needed again
// if other corner cases in gumbo xhtml support are found
#if 0

// handle a few special cases that are hard to deal with insides of gumbo
static QStringList allowed_void_tags = QStringList() << "area"    << "base"     << "basefont" 
                                                     << "bgsound" << "br"       << "col" 
                                                     << "command" << "embed"    << "event-source" 
                                                     << "frame"   << "hr"       << "image" 
                                                     << "img"     << "input"    << "keygen" 
                                                     << "link"    << "menuitem" << "meta" 
                                                     << "param"   << "source"   << "spacer" 
                                                     << "track"   << "wbr";

// Handle the general case
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
        if (!allowed_void_tags.contains(name)) {
            QString newtag = "<" + name + atts + "></" + name + ">";
            newsource = newsource.replace(sp,n,newtag);
            nsp = sp + newtag.length();
        }
        match = selfclosed.match(newsource, nsp);
    }
    return newsource;
}

// Handle the specific problem of iframe being self-closed
QString GumboInterface::fix_self_closing_tags(const QString &source)
{
    QString newsource = source;
    QRegularExpression selfclosed("<\\s*iframe(\\s*[^>/]*)/\\s*>");
    QRegularExpressionMatch match = selfclosed.match(newsource, 0);
    while (match.hasMatch()) {
        if (match.capturedStart() == -1) {
            break;
        }
        QString tag = match.captured(0);
        int sp = match.capturedStart(0);
        int n = match.capturedLength(0);
        QString atts = match.captured(1);;
        atts = atts.trimmed();
        if (!atts.isEmpty()) {
            atts = " " + atts;
        }
        QString newtag = "<iframe" + atts + "></iframe>";
        newsource = newsource.replace(sp,n,newtag);
        int nsp = sp + newtag.length();
        match = selfclosed.match(newsource, nsp);
    }
    return newsource;
}
#endif


