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
#include "SatisfiesXhtmlSchema.h"
#include "Misc/ErrorResultCollector.h"
#include <ToXercesStringConverter.h>
#include <XmlUtils.h>
#include <LocationAwareDOMParser.h>
#include <xercesc/sax/SAXException.hpp>

namespace FlightCrew
{

SatisfiesXhtmlSchema::SatisfiesXhtmlSchema()
    :
    m_Dtd( XHTML11_FLAT_DTD,
           XHTML11_FLAT_DTD_LEN,
           toX( XHTML11_FLAT_DTD_ID ) ),
    m_OpsSchema( OPS201_XSD,
                 OPS201_XSD_LEN,
                 toX( OPS201_XSD_ID ) ),
    m_OpsSwitchSchema( OPS_SWITCH_XSD, 
                       OPS_SWITCH_XSD_LEN,
                       toX( OPS_SWITCH_XSD_ID ) ),
    m_SvgSchema( SVG11_XSD, 
                 SVG11_XSD_LEN,  
                 toX( SVG11_XSD_ID ) ),
    m_XlinkSchema( XLINK_XSD, 
                   XLINK_XSD_LEN,
                   toX( XLINK_XSD_ID ) ),
    m_XmlSchema( XML_XSD, 
                 XML_XSD_LEN,
                 toX( XML_XSD_ID ) )
{

}


std::vector< Result > SatisfiesXhtmlSchema::ValidateFile( const fs::path &filepath )
{
    std::string location = std::string( OPS201_XSD_NS )
                           .append( " " )
                           .append( OPS201_XSD_ID );

    std::vector< const xc::MemBufInputSource* > schemas;
    schemas.push_back( &m_XmlSchema       );
    schemas.push_back( &m_XlinkSchema     );
    schemas.push_back( &m_SvgSchema       );
    schemas.push_back( &m_OpsSwitchSchema );
    schemas.push_back( &m_OpsSchema       );

    std::vector< const xc::MemBufInputSource* > dtds;
    dtds.push_back( &m_Dtd );

    return ValidateAgainstSchema( filepath, location, schemas, dtds ); 
}

} //namespace FlightCrew
