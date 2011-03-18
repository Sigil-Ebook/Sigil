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
#include "PackageVersionCorrect.h"
#include <FromXercesStringConverter.h>
#include <ToXercesStringConverter.h>
#include <XmlUtils.h>

static const std::string VALID_PACKAGE_VERSION = "2.0";

namespace FlightCrew
{

std::vector< Result > PackageVersionCorrect::ValidateXml(
    const xc::DOMDocument &document,
    const fs::path& )
{
    xc::DOMElement *package_element = document.getDocumentElement();
    std::vector< Result > results;

    std::string version = fromX( package_element->getAttribute( toX( "version" ) ) );

    if ( version != VALID_PACKAGE_VERSION )
    {
        results.push_back( 
            ResultWithNodeLocation( ERROR_OPF_BAD_PACKAGE_VERSION, *package_element )
            .AddMessageArgument( VALID_PACKAGE_VERSION )
            .AddMessageArgument( version )            
            );    
    }

    return results;
}

} // namespace FlightCrew

