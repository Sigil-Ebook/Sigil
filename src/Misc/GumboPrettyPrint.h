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

#ifndef GUMBO_PRETTYPRINT
#define GUMBO_PRETTYPRINT

#include <stdlib.h>
#include <string>
#include "gumbo.h"

#include <QString>

class QString;

class GumboPrettyPrint
{
public:

    GumboPrettyPrint(const QString &source);
    ~GumboPrettyPrint();
    void    parse();
    QString prettyprint(QString indent_chars="  ");

private:

    std::string get_tag_name(GumboNode *node);

    std::string prettyprint(GumboNode* node, int lvl, const std::string indent_chars);
    std::string prettyprint_contents(GumboNode* node, int lvl, const std::string indent_chars);
    std::string build_doctype(GumboNode *node);
    std::string build_attributes(GumboAttribute * at, bool no_entities);
    std::string substitute_xml_entities_into_text(const std::string &text);
    std::string substitute_xml_entities_into_attributes(char quote, const std::string &text);
    void rtrim(std::string &s);
    void ltrim(std::string &s);
    void replace_all(std::string &s, const char * s1, const char * s2);

    QString                   m_source;
    GumboOutput*              m_output;
    std::string               m_utf8src;
    
};

#endif

