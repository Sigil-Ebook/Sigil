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
#include "IdsUnique.h"
#include <ToXercesStringConverter.h>
#include <FromXercesStringConverter.h>
#include <XmlUtils.h>


namespace FlightCrew
{

std::vector< Result > IdsUnique::ValidateXml(
    const xc::DOMDocument &document,
    const fs::path& )
{
    xc::DOMNodeList *elements = document.getElementsByTagName( toX( "*" ) );

    boost::unordered_set< std::string > ids;
    std::vector< Result > results;

    for ( uint i = 0; i < elements->getLength(); ++i )
    {
        xc::DOMElement *element = static_cast< xc::DOMElement* >( elements->item( i ) );
        std::string id = fromX( element->getAttribute( toX( "id" ) ) );

        if ( !id.empty() )
        {
            if ( ids.count( id ) == 0 )
            {
                ids.insert( id );
            }

            else
            {
                results.push_back( 
                    ResultWithNodeLocation( ERROR_XML_ID_NOT_UNIQUE, *element )
                    .AddMessageArgument( id ) 
                    );
            }
        }
    }

    return results;
}

} // namespace FlightCrew

