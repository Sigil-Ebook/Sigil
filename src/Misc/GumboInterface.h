/************************************************************************
**
**  Copyright (C) 2015 Kevin B. Hendricks Stratford, ON, Canada 
**  Copyright (C) 2012  John Schember <john@nachtimwald.com>
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

#ifndef GUMBO_INTERFACE
#define GUMBO_INTERFACE

#include <stdlib.h>
#include <string>
#include <unordered_set>

#include "gumbo.h"
#include "gumbo_edit.h"

#include <QString>
#include <QList>
#include <QHash>
// #include <QMutex>

class QString;

struct GumboWellFormedError {
  int line;
  int column;
  QString message;
};

class GumboInterface
{
public:

    GumboInterface(const QString &source);
    GumboInterface(const QString &source, const QHash<QString, QString> &source_updates);
    ~GumboInterface();

    void    parse();
    QString repair();
    QString getxhtml();
    QString prettyprint(QString indent_chars="  ");

    // returns list tags that match manifest properties
    QStringList get_all_properties();

    // returns "html" node
    GumboNode * get_root_node();

    // routines for working with gumbo paths
    GumboNode* get_node_from_path(QList<unsigned int> & apath);
    QList<unsigned int> get_path_to_node(GumboNode* node);

    // routines for working with qwebpaths
    GumboNode* get_node_from_qwebpath(QString webpath);
    QString get_qwebpath_to_node(GumboNode* node);

    // routines for updating while serializing (see SourceUpdates and AnchorUpdates
    QString perform_source_updates(const QString & my_current_book_relpath);
    QString perform_link_updates(const QString & newlinks);
    QString get_body_contents();
    QString perform_body_updates(const QString & new_body);

    // routines for working with nodes with specific attributes
    QList<GumboNode*> get_all_nodes_with_attribute(const QString & attname);
    QStringList get_all_values_for_attribute(const QString & attname);
    QHash<QString,QString> get_attributes_of_node(GumboNode* node);

    // routines for working with nodes with specific tags
    QList<GumboNode*> get_all_nodes_with_tag(GumboTag tag);
    QList<GumboNode*> get_all_nodes_with_tags(const QList<GumboTag> & tags);

    // utility routines 
    std::string get_tag_name(GumboNode *node);
    QString get_local_text_of_node(GumboNode* node);

    // routine to check if well-formed
    QList<GumboWellFormedError> error_check();

private:

    enum UpdateTypes {
        NoUpdates      = 0, 
        SourceUpdates  = 1 <<  0,
        LinkUpdates    = 1 <<  1,
        BodyUpdates    = 1 <<  2
    };

    QStringList get_properties(GumboNode* node);

    QList<GumboNode*> get_nodes_with_attribute(GumboNode* node, const char * att_name);

    QStringList get_values_for_attr(GumboNode* node, const char* attr_name);

    QList<GumboNode*> get_nodes_with_tags(GumboNode* node, const QList<GumboTag> & tags);

    std::string serialize(GumboNode* node, enum UpdateTypes doupdates = NoUpdates);

    std::string serialize_contents(GumboNode* node, enum UpdateTypes doupdates = NoUpdates);

    std::string prettyprint(GumboNode* node, int lvl, const std::string indent_chars);

    std::string prettyprint_contents(GumboNode* node, int lvl, const std::string indent_chars);

    std::string build_doctype(GumboNode *node);

    std::string get_attribute_name(GumboAttribute * at);

    std::string build_attributes(GumboAttribute * at, bool no_entities, bool runupdates = false);

    std::string update_attribute_value(std::string href);

    std::string substitute_xml_entities_into_text(const std::string &text);

    std::string substitute_xml_entities_into_attributes(char quote, const std::string &text);

    bool in_set(std::unordered_set<std::string> &s, std::string &key);

    void rtrim(std::string &s);

    void ltrim(std::string &s);

    void ltrimnewlines(std::string &s);

    void newlinetrim(std::string &s);

    void condense_whitespace(std::string &s);

    void replace_all(std::string &s, const char * s1, const char * s2);

    // Hopefully now unneeded
    // QString fix_self_closing_tags(const QString & source);

    QString                         m_source;
    GumboOutput*                    m_output;
    std::string                     m_utf8src;
    const QHash<QString, QString> & m_sourceupdates;
    std::string                     m_newcsslinks;
    QString                         m_currentdir;
    std::string                     m_newbody;
    bool                            m_hasnbsp;
    
};

#endif
