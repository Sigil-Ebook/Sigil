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
#include "ContainerSatisfiesSchema.h"
#include "Result.h"
#include <ToXercesStringConverter.h>


namespace FlightCrew
{


ContainerSatisfiesSchema::ContainerSatisfiesSchema()
    :
    m_ContainerSchema( CONTAINER_XSD,
                       CONTAINER_XSD_LEN,
                       toX( CONTAINER_XSD_ID ) )
{

}


std::vector< Result > ContainerSatisfiesSchema::ValidateFile( const fs::path &filepath )
{
    std::string location = std::string( CONTAINER_XSD_NS )
                                        .append( " " )
                                        .append( CONTAINER_XSD_ID );

    std::vector< const xc::MemBufInputSource* > schemas;
    schemas.push_back( &m_ContainerSchema );

    return ValidateAgainstSchema( filepath, location, schemas ); 
}

} //namespace FlightCrew
