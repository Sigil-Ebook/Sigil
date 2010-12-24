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
#include "ItemHrefValid.h"
#include <ToXercesStringConverter.h>
#include <FromXercesStringConverter.h>
#include <XmlUtils.h>
#include <xercesc/util/XMLUri.hpp>

namespace FlightCrew
{

std::vector<Result> ItemHrefValid::ValidateXml(
    const xc::DOMDocument &document,
    const fs::path& )
{
    std::vector< xc::DOMElement* > items = xe::GetElementsByQName( 
        document, QName( "item", OPF_XML_NAMESPACE ) );

    std::vector<Result> results;

    foreach( xc::DOMElement* item, items )
    {
        std::string href = fromX( item->getAttribute( toX( "href" ) ) );

        ResultId result_id = ALL_OK;

        if ( href.empty() || !ValidUri( href ) )
        
            result_id = ERROR_OPF_ITEM_HREF_INVALID_URI;

        else if ( UriHasFragment( href ) )
        
            result_id = ERROR_OPF_ITEM_HREF_HAS_FRAGMENT;

        if ( result_id != ALL_OK )
        {
            results.push_back( 
                ResultWithNodeLocation( result_id, *item )
                .AddMessageArgument( href ) );
        }
    }

    return results;
}


bool ItemHrefValid::ValidUri( const std::string &uri )
{
    return xc::XMLUri::isValidURI( true, toX( uri ) );
}


bool ItemHrefValid::UriHasFragment( const std::string &uri )
{
    return boost::contains( uri, "#" );
}

} // namespace FlightCrew

