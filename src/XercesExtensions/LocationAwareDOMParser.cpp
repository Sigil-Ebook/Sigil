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

#include <xercesc/internal/XMLScanner.hpp>
#include <xercesc/dom/DOMNamedNodeMap.hpp>
#include "LocationAwareDOMParser.h"
#include "LocationInfoDataHandler.h"

// This can't be a member variable of the parser since it needs to be around
// even after the parser is destroyed. The cleanup methods of the parser's
// super class call the handle() method of the handler.
// We only ever need a single handler and a const one at that... this could
// also easily go into a singleton, but this approach is simpler.
static const XercesExt::LocationInfoDataHandler LOCATION_DATA_HANDLER;
const char *LOCATION_INFO_KEY = "LocationInfoKey";
typedef unsigned int uint; 

namespace XercesExt
{

LocationAwareDOMParser::LocationAwareDOMParser( xc::XMLValidator   *const valToAdopt,
                                                xc::MemoryManager  *const manager,
                                                xc::XMLGrammarPool *const gramPool )
    :
    xc::XercesDOMParser( valToAdopt, manager, gramPool )
{
    m_LocationInfoKey = xc::XMLString::transcode( LOCATION_INFO_KEY );
}


LocationAwareDOMParser::~LocationAwareDOMParser()
{
    xc::XMLString::release( &m_LocationInfoKey );
}


void LocationAwareDOMParser::startElement( const xc::XMLElementDecl &elemDecl,
                                           const unsigned int uriId,
                                           const XMLCh *const prefixName,
                                           const xc::RefVectorOf< xc::XMLAttr > &attrList,
                                           const XMLSize_t attrCount,
                                           const bool isEmpty,
                                           const bool isRoot )
{
    xc::XercesDOMParser::startElement(
            elemDecl, uriId, prefixName, attrList, attrCount, isEmpty, isRoot );

    const xc::Locator* locator = getScanner()->getLocator();
    int line_number   = (int) locator->getLineNumber();
    int column_number = (int) locator->getColumnNumber();

    xc::DOMNode *current_node = getCurrentNode();

    // It's OK to const_cast the handler since the parser will only ever
    // call the handler's handle() method, which doesn't mutate the handler.
    // In fact, this function not accepting a const handler is probably a design
    // error on the part of Xerces developers.
    current_node->setUserData( 
        m_LocationInfoKey,
        new NodeLocationInfo( line_number, column_number ),
        const_cast< XercesExt::LocationInfoDataHandler* >( &LOCATION_DATA_HANDLER ) );

    // Attribute nodes get the same location as the opening tag
    // of the element they were declared in... it's the best we can do.
    xc::DOMNamedNodeMap *attribute_map = current_node->getAttributes();
    if ( !attribute_map )

        return;

    for ( uint i = 0; i < attribute_map->getLength(); ++i )
    {
        attribute_map->item( i )->setUserData( 
            m_LocationInfoKey,
            new NodeLocationInfo( line_number, column_number ),
            const_cast< XercesExt::LocationInfoDataHandler* >( &LOCATION_DATA_HANDLER ) );
    }
}

}
