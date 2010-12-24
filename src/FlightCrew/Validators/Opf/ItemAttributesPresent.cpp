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
#include "ItemAttributesPresent.h"
#include "Misc/Utilities.h"

namespace FlightCrew
{

std::vector<Result> ItemAttributesPresent::ValidateXml(
    const xc::DOMDocument &document,
    const fs::path& )
{
    std::vector< QName > allowed_attributes;
    allowed_attributes.push_back( QName( "id",                 "" ) );
    allowed_attributes.push_back( QName( "href",               "" ) );
    allowed_attributes.push_back( QName( "media-type",         "" ) );
    allowed_attributes.push_back( QName( "fallback",           "" ) );
    allowed_attributes.push_back( QName( "fallback-style",     "" ) );
    allowed_attributes.push_back( QName( "required-namespace", "" ) );
    allowed_attributes.push_back( QName( "required-modules",   "" ) );
    
    QName element_qname( "item", OPF_XML_NAMESPACE );

    std::vector< Result > allowed_results = HasOnlyAllowedAttributes( 
        element_qname, allowed_attributes, document );

    std::vector< QName > mandatory_attributes;
    mandatory_attributes.push_back( QName( "id",         "" ) );
    mandatory_attributes.push_back( QName( "href",       "" ) );
    mandatory_attributes.push_back( QName( "media-type", "" ) );

    std::vector< Result > mandatory_results = HasMandatoryAttributes(   
        element_qname, mandatory_attributes, document );

    return Util::Extend( allowed_results, mandatory_results );
}

} // namespace FlightCrew

