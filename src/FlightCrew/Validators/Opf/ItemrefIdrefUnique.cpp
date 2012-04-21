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
#include "ItemrefIdrefUnique.h"
#include <ToXercesStringConverter.h>
#include <FromXercesStringConverter.h>
#include <XmlUtils.h>

namespace FlightCrew
{

std::vector< Result > ItemrefIdrefUnique::ValidateXml( 
    const xc::DOMDocument &document,
    const fs::path& )
{
    std::vector< xc::DOMAttr* > idrefs = xe::GetAllAttributesFromElements( 
        QName( "itemref", OPF_XML_NAMESPACE ),
        QName( "idref", "" ),
        document );

    std::vector< Result > results;

    boost::unordered_set< std::string > idref_values;

    foreach( xc::DOMAttr* idref, idrefs )
    {
        std::string idref_value = fromX( idref->getValue() );   

        if ( !idref_values.count( idref_value ) )
        {
            idref_values.insert( idref_value );              
        }

        else
        {
            results.push_back( 
                ResultWithNodeLocation( ERROR_OPF_IDREF_NOT_UNIQUE, *idref )
                .AddMessageArgument( idref_value ) 
                );  
        }
    }

    return results;
}

} // namespace FlightCrew

