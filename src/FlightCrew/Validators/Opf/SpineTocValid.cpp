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
#include "SpineTocValid.h"
#include <ToXercesStringConverter.h>
#include <FromXercesStringConverter.h>
#include <XmlUtils.h>

namespace FlightCrew
{

std::vector<Result> SpineTocValid::ValidateXml(
    const xc::DOMDocument &document,
    const fs::path& )
{
    std::vector< xc::DOMAttr* > tocs = xe::GetAllAttributesFromElements(
        QName( "spine", OPF_XML_NAMESPACE ),
        QName( "toc", "" ),
        document );

    std::vector< xc::DOMAttr* > item_ids = xe::GetAllAttributesFromElements(
        QName( "item", OPF_XML_NAMESPACE ),
        QName( "id", "" ),
        document );

    boost::unordered_set< std::string > item_id_set;

    foreach( xc::DOMAttr* item_id, item_ids )
    {
        item_id_set.insert( fromX( item_id->getValue() ) );
    }

    std::vector<Result> results;

    if ( tocs.empty() )

        return results;

    std::string toc_item_id = fromX( tocs[ 0 ]->getValue() );

    if ( item_id_set.count( toc_item_id ) == 0 )
    {
        results.push_back( 
            ResultWithNodeLocation( ERROR_OPF_BAD_SPINE_TOC_VALUE, *tocs[ 0 ] )
            .AddMessageArgument( toc_item_id ) 
            );    
    }

    return results;
}

} // namespace FlightCrew

