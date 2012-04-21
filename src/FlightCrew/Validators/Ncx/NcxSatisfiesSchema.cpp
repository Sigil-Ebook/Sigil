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
#include "NcxSatisfiesSchema.h"
#include "Result.h"
#include <ToXercesStringConverter.h>

namespace FlightCrew
{

NcxSatisfiesSchema::NcxSatisfiesSchema()
    :
    m_NcxDtd( NCX_2005_1_DTD,
              NCX_2005_1_DTD_LEN,
              toX( NCX_2005_1_DTD_ID ) ),
    m_NcxSchema( NCX_XSD,
                 NCX_XSD_LEN,
                 toX( NCX_XSD_ID ) ),
    m_XmlSchema( XML_XSD, 
                 XML_XSD_LEN,
                 toX( XML_XSD_ID ) )
{

}


std::vector< Result > NcxSatisfiesSchema::ValidateFile( const fs::path &filepath )
{
    std::string location = std::string( NCX_XSD_NS )
                           .append( " " )
                           .append( NCX_XSD_ID );

    std::vector< const xc::MemBufInputSource* > schemas;
    schemas.push_back( &m_XmlSchema );
    schemas.push_back( &m_NcxSchema );

    std::vector< const xc::MemBufInputSource* > dtds;
    dtds.push_back( &m_NcxDtd );

    return ValidateAgainstSchema( filepath, location, schemas, dtds ); 
}

} //namespace FlightCrew
