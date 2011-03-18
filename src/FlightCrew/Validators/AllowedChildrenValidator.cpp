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
#include "AllowedChildrenValidator.h"
#include "Misc/Utilities.h"
#include <FromXercesStringConverter.h>
#include <ToXercesStringConverter.h>
#include <XmlUtils.h>
#include <algorithm>

namespace FlightCrew
{

std::vector< Result > AllowedChildrenValidator::ValidateAllowedChildren( 
    const QName &parent_qname, 
    const std::vector< QName > &allowed_children, 
    const xc::DOMDocument &document )
{
    xc::DOMNodeList *parents_matching = document.getElementsByTagNameNS(
        toX( parent_qname.namespace_name ),  toX( parent_qname.local_name ) );

    std::vector< Result > results;

    if ( parents_matching->getLength() < 1 )
    
        return results;    

    xc::DOMElement* parent = static_cast< xc::DOMElement* >( parents_matching->item( 0 ) );
    std::vector< xc::DOMElement* > children = xe::GetElementChildren( *parent );

    for ( uint i = 0; i < children.size(); ++i )
    {
        xc::DOMElement *child = children[ i ];
        QName child_qname( fromX( child->getLocalName() ), fromX( child->getNamespaceURI() ) );

        if ( !Util::Contains< QName >( allowed_children, child_qname ) )

            results.push_back( NotAllowedChildResult( *children[ i ] ) );
    }

    return results;
}


Result AllowedChildrenValidator::NotAllowedChildResult( const xc::DOMNode &child )
{
    const xc::DOMElement* element = static_cast< const xc::DOMElement* >( &child );
    const xc::DOMElement* parent  = static_cast< const xc::DOMElement* >( child.getParentNode() );

    return ResultWithNodeLocation( ERROR_XML_CHILD_NOT_RECOGNIZED, child )
           .AddMessageArgument( fromX( element->getLocalName() ) )
           .AddMessageArgument( fromX( parent->getLocalName() ) );
}

} //namespace FlightCrew
