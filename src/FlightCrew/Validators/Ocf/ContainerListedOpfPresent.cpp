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
#include "ContainerListedOpfPresent.h"
#include <FromXercesStringConverter.h>
#include <ToXercesStringConverter.h>
#include <XmlUtils.h>
#include "Misc/Utilities.h"

namespace FlightCrew
{

std::vector< Result > ContainerListedOpfPresent::ValidateFile( const fs::path &filepath )
{
    // TODO: try/catch for loaddoc
    boost::shared_ptr< xc::DOMDocument > document = Util::LoadXmlDocument( filepath );

    std::vector< xc::DOMElement* > rootfiles = xe::GetElementsByQName( 
        *document, QName( "rootfile", CONTAINER_XML_NAMESPACE ) );

    std::vector< Result > results;
    fs::path opf_path;

    foreach( xc::DOMElement* rootfile, rootfiles )
    {
        std::string full_path_attribute = fromX( rootfile->getAttribute( toX( "full-path"  ) ) );
        std::string media_type          = fromX( rootfile->getAttribute( toX( "media-type" ) ) );
        
        if ( media_type == OEBPS_MIME )                 
        {
            opf_path = filepath.parent_path().parent_path() / 
                       Util::Utf8PathToBoostPath( full_path_attribute );

            if ( !fs::exists( opf_path ) )
            {
                results.push_back(
                    Result( ERROR_OCF_CONTAINER_SPECIFIED_OPF_DOESNT_EXIST,
                            xe::GetNodeLocationInfo( *rootfile ) )
                        .AddMessageArgument( full_path_attribute )
                    );
            }
            
            break;
        }
    }    

    return Util::AddPathToResults( results, filepath );
}

} // namespace FlightCrew