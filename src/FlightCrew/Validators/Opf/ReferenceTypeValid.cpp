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

#include <stdafx.h>
#include "ReferenceTypeValid.h"
#include <ToXercesStringConverter.h>
#include <FromXercesStringConverter.h>
#include <XmlUtils.h>

namespace FlightCrew
{

std::vector< Result > ReferenceTypeValid::ValidateXml(
    const xc::DOMDocument &document,
    const fs::path& )
{
    std::vector< xc::DOMElement* > elements = xe::GetElementsByQName( 
        document, QName( "reference", OPF_XML_NAMESPACE ) );

    std::vector< Result > results;
    boost::unordered_set< std::string > types = GetReferenceTypesSet();

    foreach( xc::DOMElement* element, elements )
    {
        std::string type = fromX( element->getAttribute( toX( "type" ) ) );

        // The "type" is required to have a value
        if ( types.count( type ) == 0 &&
             !boost::starts_with( type, "other." ) )
        {
            results.push_back( 
                ResultWithNodeLocation( ERROR_OPF_BAD_REFERENCE_TYPE_VALUE, *element )
                .AddMessageArgument( type )
                );
        }     
    }

    return results;
}


boost::unordered_set< std::string > ReferenceTypeValid::GetReferenceTypesSet()
{
    boost::unordered_set< std::string > types;
    types.insert( "cover" );
    types.insert( "title-page" );
    types.insert( "toc" );
    types.insert( "index" );
    types.insert( "glossary" );
    types.insert( "acknowledgements" );
    types.insert( "bibliography" );
    types.insert( "colophon" );
    types.insert( "copyright-page" );
    types.insert( "dedication" );
    types.insert( "epigraph" );
    types.insert( "foreword" );
    types.insert( "loi" );
    types.insert( "lot" );
    types.insert( "notes" );
    types.insert( "preface" );
    types.insert( "text" );

    return types;
}

} // namespace FlightCrew

