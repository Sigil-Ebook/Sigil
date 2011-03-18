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
#include "EncryptionSatisfiesSchema.h"
#include "Result.h"
#include <ToXercesStringConverter.h>


namespace FlightCrew
{


EncryptionSatisfiesSchema::EncryptionSatisfiesSchema()
    :
    m_EncryptionSchema( ENCRYPTION_XSD,
                        ENCRYPTION_XSD_LEN,
                        toX( ENCRYPTION_XSD_ID ) ),
    m_XencSchema( XENC_SCHEMA_XSD,
                  XENC_SCHEMA_XSD_LEN,
                  toX( XENC_SCHEMA_XSD_ID ) ),
    m_XmldsigSchema( XMLDSIG_CORE_SCHEMA_XSD,
                     XMLDSIG_CORE_SCHEMA_XSD_LEN,
                     toX( XMLDSIG_CORE_SCHEMA_XSD_ID ) )
{

}


std::vector< Result > EncryptionSatisfiesSchema::ValidateFile( const fs::path &filepath )
{
    std::string location = std::string( CONTAINER_XSD_NS )
                                        .append( " " )
                                        .append( ENCRYPTION_XSD_ID );

    std::vector< const xc::MemBufInputSource* > schemas;
    schemas.push_back( &m_XmldsigSchema    );
    schemas.push_back( &m_XencSchema       );
    schemas.push_back( &m_EncryptionSchema );

    return ValidateAgainstSchema( filepath, location, schemas ); 
}

} //namespace FlightCrew
