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
#include "ContentTargetsPresent.h"
#include <FromXercesStringConverter.h>
#include <XmlUtils.h>
#include "Misc/Utilities.h"

namespace FlightCrew
{

std::vector< Result > ContentTargetsPresent::ValidateXml( 
    const xc::DOMDocument &document,
    const fs::path &filepath )
{
    std::vector< Result > results;
    std::vector< ContentTargetsPresent::ContentTarget > targets = 
        GetContentTargets( document, filepath.parent_path() );

    fs::path current_xhtml_path;
    boost::unordered_set< std::string > document_ids;

    foreach( const ContentTargetsPresent::ContentTarget &target, targets )
    {
        // They're sorted by content file path, so until
        // the path changes, they all refer to the same file
        if ( target.content_file != current_xhtml_path )
        {
            if ( !fs::exists( target.content_file ) )
            {
                results.push_back( 
                    Result( ERROR_NCX_CONTENT_FILE_DOES_NOT_EXIST, target.content_node )
                    .AddMessageArgument( target.raw_src_path )
                    );

                continue;
            }

            current_xhtml_path = target.content_file;
            document_ids       = GetAllIdsFromDocument( current_xhtml_path );                        
        }

        if ( !target.fragment.empty() && !document_ids.count( target.fragment ) )
        {
            results.push_back( 
                Result( ERROR_NCX_CONTENT_FRAGMENT_DOES_NOT_EXIST, target.content_node )
                .AddMessageArgument( target.raw_src_path )
                );
        }
    }

    return results;
}

std::vector< ContentTargetsPresent::ContentTarget > ContentTargetsPresent::GetContentTargets( 
    const xc::DOMDocument &document, 
    const fs::path &folderpath )
{
    std::vector< xc::DOMAttr* > srcs = xe::GetAllAttributesFromElements( 
        QName( "content", NCX_XML_NAMESPACE ),
        QName( "src", "" ),
        document );

    std::vector< ContentTargetsPresent::ContentTarget > targets;

    foreach( xc::DOMAttr* src, srcs )
    {
        ContentTargetsPresent::ContentTarget target;
        
        target.raw_src_path     = fromX( src->getValue() );
        std::string decoded_url = Util::UrlDecode( target.raw_src_path );

        fs::path resource_path = 
            Util::Utf8PathToBoostPath(
                Util::UrlWithoutFragment( decoded_url ) ); 

        target.content_file = Util::NormalizePath( folderpath / resource_path );        
        target.fragment     = Util::GetUrlFragment( decoded_url );
        target.content_node = xe::GetNodeLocationInfo( *src );

        targets.push_back( target );
    }

    std::sort( targets.begin(), targets.end() );

    return targets;
}


boost::unordered_set< std::string > ContentTargetsPresent::GetAllIdsFromDocument( 
    const fs::path &filepath )
{
    boost::shared_ptr< xc::DOMDocument > document;

    try
    {
        document = Util::LoadXhtmlDocument( filepath );
    }

    catch ( std::exception& )
    {
        // If the file doesn't exist or some other
        // snafu, then there are obviously no ids.
        return boost::unordered_set< std::string > ();
    }

    std::vector< xc::DOMAttr* > ids = xe::GetAllAttributesFromElements( 
        QName( "*", "*" ),
        QName( "id", "*" ),
        *document );

    boost::unordered_set< std::string > id_values;

    foreach( xc::DOMAttr* id, ids )
    {        
        std::string id_value = fromX( id->getValue() );

        id_values.insert( id_value );
    }

    return id_values;
}

} // namespace FlightCrew

