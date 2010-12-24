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
#include "ContainerListsOpf.h"
#include <FromXercesStringConverter.h>
#include <ToXercesStringConverter.h>
#include <XmlUtils.h>
#include "Misc/Utilities.h"

namespace FlightCrew
{


std::vector<Result> ContainerListsOpf::ValidateXml( 
    const xc::DOMDocument &document,
    const fs::path& )
{
    std::vector< xc::DOMElement* > rootfiles = xe::GetElementsByQName( 
        document, QName( "rootfile", CONTAINER_XML_NAMESPACE ) );

    std::vector< Result > results;

    foreach( xc::DOMElement* rootfile, rootfiles )
    {
        std::string media_type = fromX( rootfile->getAttribute( toX( "media-type" ) ) );
        
        if ( media_type == OEBPS_MIME )             
                       
            return results;
    }

    std::vector< xc::DOMElement* > rootfiles_elements = xe::GetElementsByQName( 
        document, QName( "rootfiles", CONTAINER_XML_NAMESPACE ) );

    if ( rootfiles_elements.empty() )

        return results;

    results.push_back( 
        ResultWithNodeLocation( ERROR_OCF_CONTAINER_DOESNT_LIST_OPF, *rootfiles_elements[ 0 ] )
        );

    return results; 
}

} // namespace FlightCrew