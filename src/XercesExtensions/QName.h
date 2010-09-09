/************************************************************************
**
**  Copyright (C) 2010  Strahinja Markovic
**
**  This file is part of FlightCrew.
**
**  FlightCrew is free software: you can redistribute it and/or modify
**  it under the terms of the GNU Lesser General Public License as published
**  by the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  FlightCrew is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU Lesser General Public License for more details.
**
**  You should have received a copy of the GNU Lesser General Public License
**  along with FlightCrew.  If not, see <http://www.gnu.org/licenses/>.
**
*************************************************************************/

#pragma once
#ifndef QNAME_H
#define QNAME_H

#include <string>

namespace XercesExt
{

/**
 * A qualified name of a node/element/attribute in XML.
 */
struct QName
{
    /**
     * Constructor.
     *
     * @param new_local_name The local name.
     * @param new_namespace_name The namespace of the local name.
     */
    QName( const std::string &new_local_name, const std::string &new_namespace_name )
        : local_name( new_local_name ), namespace_name ( new_namespace_name ) {};
    
    /**
     * Implements the equality operator.
     * Two QNames are equal if they have the same local and namespace names.
     * The exceptions are names that equal "*", which match any name.
     */
    inline bool operator== ( const QName& other ) const 
    { 
        return ( local_name == other.local_name || local_name == "*" || other.local_name == "*" ) && 
               ( namespace_name == other.namespace_name || namespace_name == "*" || other.namespace_name == "*" );
    };

    /**
     * Implements the inequality operator.
     * Merely calls the equality operator and returns its inverted result. 
     */
    inline bool operator!= ( const QName& other ) const 
    { 
        return !operator==( other );
    };

    /**
     * The local name of the node.
     */
    std::string local_name;

    /**
     * The namespace name the local name is located in.
     */
    std::string namespace_name;
};

}

#endif // QNAME_H