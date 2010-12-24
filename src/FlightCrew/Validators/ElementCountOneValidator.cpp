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
#include "ElementCountOneValidator.h"
#include "ToXercesStringConverter.h"
#include <XmlUtils.h>

namespace FlightCrew
{

std::vector<Result> ElementCountOneValidator::VerifyElementCountOne( 
    const QName &element_qname, 
    const QName &parent_qname, 
    const xc::DOMDocument &document )
{
    xc::DOMNodeList *elements = document.getElementsByTagNameNS(
        toX( element_qname.namespace_name ),  toX( element_qname.local_name ) );

    std::vector<Result> results;

    if ( elements->getLength() != 1 )
    {
        Result result;

        if ( elements->getLength() < 1 )
        {
            xc::DOMNode *parent = xe::GetFirstAvailableElement( parent_qname, document );

            result = parent != NULL                                                   ?
                     ResultWithNodeLocation( ERROR_XML_WRONG_ELEMENT_COUNT, *parent ) :
                     Result( ERROR_XML_WRONG_ELEMENT_COUNT ); 
        }

        else
        {
            result = ResultWithNodeLocation( 
                ERROR_XML_WRONG_ELEMENT_COUNT, *elements->item( 1 ) );
        }

        result.AddMessageArgument( element_qname.local_name );
        results.push_back( result );
    }

    return results;
}

} //namespace FlightCrew
