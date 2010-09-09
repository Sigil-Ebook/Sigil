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

#include "XmlUtils.h"
#include "ToXercesStringConverter.h"
#include "FromXercesStringConverter.h"
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/dom/DOMAttr.hpp>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMNodeList.hpp>
#include <xercesc/dom/DOMNamedNodeMap.hpp>
#include <boost/foreach.hpp> 
#define foreach BOOST_FOREACH

extern const char *LOCATION_INFO_KEY;

typedef unsigned int uint;

namespace XercesExt
{

NodeLocationInfo GetNodeLocationInfo( const xc::DOMNode &node )
{
    return *static_cast< NodeLocationInfo* >( node.getUserData( toX( LOCATION_INFO_KEY ) ) );
}


std::vector< xc::DOMElement* > GetElementsByQName( const xc::DOMElement &start_element, 
                                                   const QName &element_qname )
{
    xc::DOMNodeList *elements = start_element.getElementsByTagNameNS(
        toX( element_qname.namespace_name ),  toX( element_qname.local_name ) );

    return ExtractElementsFromNodeList( *elements );
}


std::vector< xc::DOMElement* > GetElementsByQName( const xc::DOMDocument &document, 
                                                   const QName &element_qname )
{
    xc::DOMNodeList *elements = document.getElementsByTagNameNS(
        toX( element_qname.namespace_name ),  toX( element_qname.local_name ) );

    return ExtractElementsFromNodeList( *elements );
}


std::vector< xc::DOMElement* > GetElementChildren( const xc::DOMElement &element )
{
    xc::DOMElement *child = element.getFirstElementChild();

    std::vector< xc::DOMElement* > children;

    if ( !child )

        return children;

    children.push_back( child );

    while (true)
    {
         child = child->getNextElementSibling();

         if ( !child )

             return children;
         
         children.push_back( child );
    }
}


std::vector< xc::DOMAttr* > GetAllAttributesFromElements( const QName &element_qname, 
                                                          const QName &attribute_qname,
                                                          const xc::DOMDocument &document )
{
    xc::DOMNodeList *elements = document.getElementsByTagNameNS(
        toX( element_qname.namespace_name ),  toX( element_qname.local_name ) );

    std::vector< xc::DOMAttr* > attributes;

    for ( uint i = 0; i < elements->getLength(); ++i )
    {
        xc::DOMNamedNodeMap *attribute_map = elements->item( i )->getAttributes();

        if ( !attribute_map )

            continue;

        for ( uint j = 0; j < attribute_map->getLength(); ++j )
        {
            xc::DOMAttr *attribute = static_cast< xc::DOMAttr* >( attribute_map->item( j ) );
            QName current_attribute_qname( fromX( attribute->getLocalName() ),
                                           fromX( attribute->getNamespaceURI() ) );

            if ( attribute_qname == current_attribute_qname )

                attributes.push_back( attribute );
        }
    }

    return attributes;
}


std::vector< xc::DOMElement* > ExtractElementsFromNodeList( const xc::DOMNodeList &node_list )
{
    std::vector< xc::DOMElement* > element_list;

    for ( uint i = 0; i < node_list.getLength(); ++i )
    {
        xc::DOMNode *node = node_list.item( i );

        if ( node->getNodeType() == xc::DOMNode::ELEMENT_NODE )

            element_list.push_back( static_cast< xc::DOMElement* >( node ) );
    }

    return element_list;
}


bool ElementListContains( std::vector< xc::DOMElement* > element_list,
                          const QName &element_qname )
{
    for ( uint i = 0; i < element_list.size(); ++i )
    {
        xc::DOMElement *element = element_list[ i ];
        QName current_qname( fromX( element->getLocalName() ), 
                             fromX( element->getNamespaceURI() ) );

        if ( current_qname == element_qname )
        
            return true;
    } 

    return false;
}

xc::DOMNode* GetFirstAvailableElement( const std::vector< QName > &element_qnames,
                                       const xc::DOMDocument &document )
{
    foreach( QName element_qname, element_qnames )
    {
        xc::DOMNodeList *matching_nodes = document.getElementsByTagNameNS(
               toX( element_qname.namespace_name ),  toX( element_qname.local_name ) );

        if ( matching_nodes->getLength() > 0 )

            return matching_nodes->item( 0 );
    }

    return NULL;
}

xc::DOMNode* GetFirstAvailableElement( const QName &element_qname, 
                                       const xc::DOMDocument &document )
{
    std::vector< QName > element_qnames;
    element_qnames.push_back( element_qname );

    return GetFirstAvailableElement( element_qnames, document );
}



} // namespace XercesExt
