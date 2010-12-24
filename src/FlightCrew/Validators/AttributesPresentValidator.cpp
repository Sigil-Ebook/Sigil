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
#include "AttributesPresentValidator.h"
#include "ToXercesStringConverter.h"
#include "FromXercesStringConverter.h"
#include "Misc/Utilities.h"


namespace FlightCrew
{

std::vector<Result> AttributesPresentValidator::HasOnlyAllowedAttributes( 
    const QName &element_qname,
    const std::vector< QName > &attribute_qnames, 
    const xc::DOMDocument &document )
{
    xc::DOMNodeList *elements = document.getElementsByTagNameNS(
        toX( element_qname.namespace_name ),  toX( element_qname.local_name ) );

    std::vector<Result> results;

    for ( uint i = 0; i < elements->getLength(); ++i )
    {
        xc::DOMElement* element         = static_cast< xc::DOMElement* >( elements->item( i ) );
        xc::DOMNamedNodeMap *attributes = element->getAttributes();

        for ( uint j = 0; j < attributes->getLength(); ++j )
        {
            xc::DOMAttr* attribute = static_cast< xc::DOMAttr* >( attributes->item( j ) );

            if ( !IsAllowedAttribute( *attribute, attribute_qnames ) )
            {
                results.push_back( 
                    ResultWithNodeLocation( ERROR_XML_ATTRIBUTE_NOT_RECOGNIZED, *element )
                    .AddMessageArgument( fromX( attribute->getName() ) )
                    .AddMessageArgument( element_qname.local_name )
                    );
            }
        }
    }    

    return results;
}


std::vector<Result> AttributesPresentValidator::HasMandatoryAttributes( 
    const QName &element_qname, 
    const std::vector< QName > &attribute_qnames, 
    const xc::DOMDocument &document )
{
    xc::DOMNodeList *elements = document.getElementsByTagNameNS(
        toX( element_qname.namespace_name ),  toX( element_qname.local_name ) );

    std::vector<Result> results;

    for ( uint i = 0; i < elements->getLength(); ++i )
    {
        xc::DOMElement* element = static_cast< xc::DOMElement* >( elements->item( i ) );

        foreach( QName attribute_qname, attribute_qnames )
        {
            if ( !element->hasAttributeNS( 
                    toX( attribute_qname.namespace_name ), toX( attribute_qname.local_name ) ) )
            {
                results.push_back( 
                    ResultWithNodeLocation( ERROR_XML_REQUIRED_ATTRIBUTE_MISSING, *element )
                    .AddMessageArgument( attribute_qname.local_name )
                    .AddMessageArgument( element_qname.local_name )
                    );
            }
        }
    }

    return results;
}

bool AttributesPresentValidator::IsAllowedAttribute( 
    const xc::DOMAttr &attribute, 
    const std::vector< QName > &allowed_attribute_qnames )
{
    QName attribute_qname( fromX( attribute.getLocalName() ), fromX( attribute.getNamespaceURI() ) );
 
    bool allowed_name = Util::Contains< QName >( 
                            allowed_attribute_qnames, attribute_qname );

    // This includes the namespace prefix,
    // so e.g. opf:scheme
    std::string full_attribute_name = fromX( attribute.getName() );

    bool is_xmlns = boost::starts_with( full_attribute_name, "xmlns" );

    return allowed_name || is_xmlns;
}

} //namespace FlightCrew
