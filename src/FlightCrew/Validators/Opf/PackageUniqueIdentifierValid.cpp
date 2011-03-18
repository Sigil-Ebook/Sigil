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
#include "PackageUniqueIdentifierValid.h"
#include <ToXercesStringConverter.h>
#include <FromXercesStringConverter.h>
#include <XmlUtils.h>

namespace FlightCrew
{

std::vector< Result > PackageUniqueIdentifierValid::ValidateXml(
    const xc::DOMDocument &document,
    const fs::path& )
{
    xc::DOMElement *package_element = document.getDocumentElement();
    std::string unique_id = fromX( package_element->getAttribute( toX( "unique-identifier" ) ) );

    std::vector< xc::DOMAttr* > identifier_ids = xe::GetAllAttributesFromElements( 
        QName( "identifier", DC_XML_NAMESPACE ),
        QName( "id", "" ),
        document );

    std::vector< Result > results;
    bool id_found = false;

    foreach( xc::DOMAttr* identifier_id, identifier_ids )
    {
        std::string id_value = fromX( identifier_id->getValue() ); 
        if ( id_value == unique_id )
        {
            id_found = true;
            break;
        }
    }

    if ( !id_found )
    {
        results.push_back( 
            ResultWithNodeLocation( 
                ERROR_OPF_PACKAGE_UNIQUE_IDENTIFIER_DOES_NOT_EXIST, *package_element )
            .AddMessageArgument( unique_id )
            );
    }   

    return results;
}

} // namespace FlightCrew

