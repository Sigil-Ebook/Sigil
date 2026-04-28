/************************************************************************
**
**  Copyright (C) 2026  Kevin B. Hendricks, Stratford, ON, Canada
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
#ifndef PRETTYPRINTPROPS_H
#define PRETYPROPS_H

#include <stdlib.h>
#include <string>
#include <unordered_set>

class QString;

/**
 * Singleton.
 *
 * PrettyPrintProps
 */
 

class PrettyPrintProps
{
    Q_DECLARE_TR_FUNCTIONS(PrettyPrintProps)

public:

    static PrettyPrintProps *instance();
    bool inset_structural(const std::string &s) const;
    bool inset_inline(const std::string &s) const;
    bool inset_preservespace(const std::string &s) const;
    bool inset_noentitysub(const std::string &s) const;
    bool inset_void(const std::string &s) const;
    bool inset_textholder(const std::string &s) const;

    std::string  getIndentString() { return m_indent_string; } ;
    bool getSingleSpace() { return m_singlespace; };

  
private:

    PrettyPrintProps();
    void ParsePrettyPrintXml();

    std::unordered_set<std::string> m_structural;
    std::unordered_set<std::string> m_inline;
    std::unordered_set<std::string> m_preservespace;
    std::unordered_set<std::string> m_noentitysub;
    std::unordered_set<std::string> m_void;
    std::unordered_set<std::string> m_textholder;

    std::string m_indent_string = "  ";
    int m_singlespace = false;

    static PrettyPrintProps *m_instance;
};

#endif // PRETTYPRINTPROPS_H
