/************************************************************************
**
**  Copyright (C) 2021  Kevin B. Hendricks, Stratford, ON, Canada
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
#ifndef CSS_PROPERTIES_H
#define CSS_PROPERTIES_H

#include <cstdlib>
#include <string> 
#include <map>

/**
 * Singleton.
 *
 * CSSProperties
 */
 

class CSSProperties
{

public:

    static CSSProperties *instance();

    bool contains(std::string pname);

    std::string levels(std::string pname);

private:

    CSSProperties();

    std::map<std::string, std::string> m_all_properties;
    static CSSProperties *m_instance;
};

#endif // CSS_PROPERTIES_H
