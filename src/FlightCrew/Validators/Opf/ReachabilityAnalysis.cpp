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
#include "ReachabilityAnalysis.h"
#include <ToXercesStringConverter.h>
#include <FromXercesStringConverter.h>
#include <XmlUtils.h>
#include "Misc/DetermineMimetype.h"
#include "Misc/Utilities.h"

namespace boost
{

namespace filesystem3
{    
    // This overload of the boost hash_value func
    // is necessary so that we can put fs::paths
    // in boost::unordered_sets
    std::size_t hash_value( const fs::path &mypath ) 
    {
#ifdef _WIN32
        boost::hash< std::wstring > hasher;
        return hasher( mypath.generic_wstring() );
#else
        boost::hash< std::string > hasher;
        return hasher( mypath.generic_string() );
#endif 
    }
} // namespace filesystem3

} // namespace boost


namespace FlightCrew
{


std::vector< Result > ReachabilityAnalysis::ValidateXml( 
    const xc::DOMDocument &document,
    const fs::path &filepath )
{
    const fs::path opf_folder_path = filepath.parent_path();

    boost::unordered_map< std::string, fs::path > manifest_items =
        GetManifestItems( document, opf_folder_path );

    boost::unordered_set< fs::path > starting_set = 
        StartingSetOpsPaths( document, manifest_items, opf_folder_path );

    boost::unordered_set< fs::path > reachable_resources =
        DetermineReachableResources( starting_set );

    std::vector< Result > results;

    Util::Extend( results, ResultsForOpsDocsNotInSpine( document, manifest_items, reachable_resources ) );
    Util::Extend( results, ResultsForResourcesNotInManifest( manifest_items, reachable_resources ) );
    Util::Extend( results, ResultsForUnusedResources( manifest_items, reachable_resources ) );

    return results;
}


std::vector< Result > ReachabilityAnalysis::ResultsForOpsDocsNotInSpine( 
    const xc::DOMDocument &document,
    const boost::unordered_map< std::string, fs::path > &manifest_items,
    const boost::unordered_set< fs::path > &reachable_resources )
{
    boost::unordered_set< fs::path > spine_paths = SpinePaths( document, manifest_items );
    boost::unordered_set< fs::path > ops_docs    = GetOnlyOpsDocs( reachable_resources );

    std::vector< Result > results;

    foreach( const fs::path &ops_path, ops_docs )
    {
        if ( !spine_paths.count( ops_path ) )
        {
            results.push_back( 
                Result( ERROR_OPF_REACHABLE_OPS_DOC_NOT_IN_SPINE )
                .SetFilepath( Util::BoostPathToUtf8Path( ops_path ) ) 
                );
        }
    }

    return results;
}


std::vector< Result > ReachabilityAnalysis::ResultsForResourcesNotInManifest( 
    const boost::unordered_map< std::string, fs::path > &manifest_items,
    const boost::unordered_set< fs::path > &reachable_resources )
{
    boost::unordered_set< fs::path > manifest_paths =
        GetPathsFromItems( manifest_items );

    std::vector< Result > results;

    foreach( const fs::path &resource_path, reachable_resources )
    {
        if ( !manifest_paths.count( resource_path ) )
        {
            results.push_back(
                Result( ERROR_OPF_REACHABLE_RESOURCE_NOT_IN_MANIFEST )
                .SetFilepath( Util::BoostPathToUtf8Path( resource_path ) ) 
                );
        }
    }

    return results;
}


std::vector< Result > ReachabilityAnalysis::ResultsForUnusedResources(
    const boost::unordered_map< std::string, fs::path > &manifest_items,
    const boost::unordered_set< fs::path > &reachable_resources )
{
    std::vector< Result > results;

    boost::unordered_set< fs::path > manifest_paths =
        GetPathsFromItems( manifest_items );

    foreach( const fs::path &manifest_path, manifest_paths )
    {
        if ( !reachable_resources.count( manifest_path ) &&
             !AllowedToBeNotReachable( manifest_path ) )
        {
            results.push_back(
                Result( WARNING_OPF_RESOURCE_IN_MANIFEST_NOT_REACHABLE )
                .SetFilepath( Util::BoostPathToUtf8Path( manifest_path ) ) 
                );
        }
    }

    return results;
}


bool ReachabilityAnalysis::AllowedToBeNotReachable( const fs::path &filepath )
{
    // As per spec, the only file that is allowed to be unreachable is the NCX file.
    return DetermineMimetype( filepath ) == NCX_MIME;
}


boost::unordered_map< std::string, fs::path > ReachabilityAnalysis::GetManifestItems( 
    const xc::DOMDocument &document,
    const fs::path &opf_folder_path )
{
    boost::unordered_map< std::string, fs::path > manifest_items;

    std::vector< xc::DOMElement* > items = xe::GetElementsByQName( 
        document, QName( "item", OPF_XML_NAMESPACE ) );    

    foreach( xc::DOMElement* item, items )
    {
        std::string id     = fromX( item->getAttribute( toX( "id" ) ) );
        std::string href   = fromX( item->getAttribute( toX( "href" ) ) );
        fs::path item_path = opf_folder_path /
            Util::Utf8PathToBoostPath( Util::UrlDecode( href ) );

        manifest_items[ id ] = item_path;
    }

    return manifest_items;
}


boost::unordered_set< fs::path > ReachabilityAnalysis::StartingSetOpsPaths( 
    const xc::DOMDocument &document,
    const boost::unordered_map< std::string, fs::path > &manifest_items,
    const fs::path &opf_folder_path )
{
    boost::unordered_set< fs::path > starting_set = SpinePaths( document, manifest_items );
    starting_set = Util::SetUnion( starting_set, GuidePaths( document, opf_folder_path ) );
    starting_set = Util::SetUnion( starting_set, ToursPaths( document, opf_folder_path ) );
    starting_set = Util::SetUnion( starting_set, NcxPaths( document, manifest_items ) );    
            
    return starting_set;
}


boost::unordered_set< fs::path > ReachabilityAnalysis::SpinePaths( 
    const xc::DOMDocument &document,
    const boost::unordered_map< std::string, fs::path > &manifest_items )
{
    boost::unordered_set< fs::path > spine_paths;

    std::vector< xc::DOMElement* > items = xe::GetElementsByQName( 
        document, QName( "itemref", OPF_XML_NAMESPACE ) );    

    foreach( xc::DOMElement* item, items )
    {
        std::string idref = fromX( item->getAttribute( toX( "idref" ) ) );

        if ( manifest_items.count( idref ) > 0 )

            spine_paths.insert( manifest_items.at( idref ) );
    }
            
    return spine_paths;
}


boost::unordered_set< fs::path > ReachabilityAnalysis::GuidePaths( 
    const xc::DOMDocument &document, 
    const fs::path &opf_folder_path )
{
    boost::unordered_set< fs::path > guide_paths;

    std::vector< xc::DOMElement* > references = xe::GetElementsByQName( 
        document, QName( "reference", OPF_XML_NAMESPACE ) );    

    foreach( xc::DOMElement* reference, references )
    {
        std::string href        = fromX( reference->getAttribute( toX( "href" ) ) );
        fs::path reference_path = opf_folder_path /
            Util::Utf8PathToBoostPath( Util::UrlWithoutFragment( Util::UrlDecode( href ) ) );

        guide_paths.insert( reference_path );
    }
            
    return guide_paths;
}


boost::unordered_set< fs::path > ReachabilityAnalysis::ToursPaths(
    const xc::DOMDocument &document, 
    const fs::path &opf_folder_path )
{
    boost::unordered_set< fs::path > tours_paths;

    std::vector< xc::DOMElement* > sites = xe::GetElementsByQName( 
        document, QName( "site ", OPF_XML_NAMESPACE ) );    

    foreach( xc::DOMElement* site, sites )
    {
        std::string href   = fromX( site->getAttribute( toX( "href" ) ) );
        fs::path site_path = opf_folder_path / 
            Util::Utf8PathToBoostPath( Util::UrlWithoutFragment( Util::UrlDecode( href ) ) );

        tours_paths.insert( site_path );
    }
            
    return tours_paths;
}


fs::path ReachabilityAnalysis::GetPathToNcx( 
    const xc::DOMDocument &document, 
    const boost::unordered_map< std::string, fs::path > &manifest_items )
{
    std::vector< xc::DOMAttr* > tocs = xe::GetAllAttributesFromElements( 
        QName( "spine", OPF_XML_NAMESPACE ),
        QName( "toc", "" ),
        document );

    if ( tocs.empty() )

        return fs::path();

    std::string toc_id = fromX( tocs[ 0 ]->getValue() );

    if ( !manifest_items.count( toc_id ) )

        return fs::path();

    return manifest_items.at( toc_id );
}


boost::unordered_set< fs::path > ReachabilityAnalysis::NcxPaths( 
    const xc::DOMDocument &document, 
    const boost::unordered_map< std::string, fs::path > &manifest_items )
{
    fs::path ncx_path = GetPathToNcx( document, manifest_items );
    boost::shared_ptr< xc::DOMDocument > ncx_document;

    try
    {
        ncx_document = Util::LoadXmlDocument( ncx_path );
    }

    catch ( std::exception& )
    {
        // If the file doesn't exist or some other
        // snafu, then there are obviously no links.
        return boost::unordered_set< fs::path > ();
    }

    boost::unordered_set< fs::path > ncx_paths;

    std::vector< xc::DOMAttr* > srcs = xe::GetAllAttributesFromElements( 
        QName( "content", NCX_XML_NAMESPACE ),
        QName( "src", "" ),
        *ncx_document );

    fs::path ncx_folder = ncx_path.parent_path();

    foreach( xc::DOMAttr* src, srcs )
    {
        fs::path resource_path = 
            Util::Utf8PathToBoostPath(
                Util::UrlWithoutFragment( 
                    Util::UrlDecode( fromX( src->getValue() ) ) ) ); 
        
        ncx_paths.insert( ncx_folder / resource_path );
    }

    return ncx_paths;
}


boost::unordered_set< fs::path > ReachabilityAnalysis::DetermineReachableResources(
    const boost::unordered_set< fs::path > &starting_ops_paths )
{
    boost::unordered_set< fs::path > current_resource_set = starting_ops_paths;
    boost::unordered_set< fs::path > new_resource_set = current_resource_set;

    while ( true )
    {
        boost::unordered_set< fs::path > reachable_resource_set =
            GetDirectlyReachableResources( new_resource_set );
        
        boost::unordered_set< fs::path > next_resource_set =
            Util::SetUnion( reachable_resource_set, current_resource_set );

        if ( next_resource_set == current_resource_set )

            break;

        new_resource_set = Util::SetSubtraction( next_resource_set, current_resource_set );
        current_resource_set = next_resource_set;        
    }

    return current_resource_set;
}


boost::unordered_set< fs::path > ReachabilityAnalysis::GetDirectlyReachableResources( 
    const boost::unordered_set< fs::path > &resources )
{
    return Util::SetUnion( 
        GetLinkedResourcesFromAllOps( GetOnlyOpsDocs( resources ) ),
        GetLinkedResourcesFromAllCss( GetOnlyCssDocs( resources ) ) );
}


boost::unordered_set< fs::path > ReachabilityAnalysis::GetOnlyOpsDocs( 
    const boost::unordered_set< fs::path > &resources )
{
    boost::unordered_set< fs::path > ops_docs;

    foreach( const fs::path &resource, resources )
    {
        std::string mimetype = DetermineMimetype( resource );

        if ( mimetype == XHTML_MIME  ||
             mimetype == DTBOOK_MIME ||
             mimetype == OEB_DOC_MIME )
        {
            ops_docs.insert( resource );
        }
    }

    return ops_docs;
}


boost::unordered_set< fs::path > ReachabilityAnalysis::GetOnlyCssDocs( 
    const boost::unordered_set< fs::path > &resources )
{
    boost::unordered_set< fs::path > ops_docs;

    foreach( const fs::path &resource, resources )
    {
        std::string mimetype = DetermineMimetype( resource );

        if ( mimetype == CSS_MIME )
        {
            ops_docs.insert( resource );
        }
    }

    return ops_docs;
}


boost::unordered_set< fs::path > ReachabilityAnalysis::GetLinkedResourcesFromAllOps( 
    const boost::unordered_set< fs::path > &ops_docs )
{
    boost::unordered_set< fs::path > all_linked_resources;

    foreach( const fs::path &ops_doc, ops_docs )
    {
        all_linked_resources = Util::SetUnion( 
            all_linked_resources, GetLinkedResourcesFromOps( ops_doc ) );
    }

    return all_linked_resources;
}


boost::unordered_set< fs::path > ReachabilityAnalysis::GetLinkedResourcesFromAllCss( 
    const boost::unordered_set< fs::path > &css_docs )
{
    boost::unordered_set< fs::path > all_linked_resources;

    foreach( const fs::path &css_doc, css_docs )
    {
        all_linked_resources = Util::SetUnion( 
            all_linked_resources, GetLinkedResourcesFromCss( css_doc ) );
    }

    return all_linked_resources;
}


boost::unordered_set< fs::path > ReachabilityAnalysis::GetLinkedResourcesFromOps(
    const fs::path &ops_document )
{
    boost::shared_ptr< xc::DOMDocument > document;

    try
    {
        document = Util::LoadXhtmlDocument( ops_document );
    }

    catch ( std::exception& )
    {
        // If the file doesn't exist or some other
        // snafu, then there are obviously no links.
        return boost::unordered_set< fs::path > ();
    }

    xc::DOMNodeList *elements = document->getElementsByTagNameNS(
        toX( "*" ),  toX( "*" ) );

    boost::unordered_set< fs::path > linked_resources;
    fs::path ops_doc_folder = ops_document.parent_path();

    for ( uint i = 0; i < elements->getLength(); ++i )
    {
        xc::DOMNamedNodeMap *attribute_map = elements->item( i )->getAttributes();

        if ( !attribute_map )

            continue;

        for ( uint j = 0; j < attribute_map->getLength(); ++j )
        {
            xc::DOMAttr *attribute = static_cast< xc::DOMAttr* >( attribute_map->item( j ) );
            std::string attribute_name = fromX( attribute->getLocalName() );

            if ( attribute_name == "href" ||
                 attribute_name == "src" )
            {
                std::string attribute_value = fromX( attribute->getValue() );
                fs::path resource_path =
                    Util::Utf8PathToBoostPath(
                        Util::UrlWithoutFileScheme(
                            Util::UrlWithoutFragment( 
                                Util::UrlDecode( attribute_value ) ) ) );

                if ( !IsFilesystemPath( resource_path ) || resource_path.empty() )

                    continue;

                linked_resources.insert( Util::NormalizePath( ops_doc_folder / resource_path ) );
            }
        }
    }

    return linked_resources;
}


boost::unordered_set< fs::path > ReachabilityAnalysis::GetLinkedResourcesFromCss( 
    const fs::path &css_document )
{
    std::string contents;

    try
    {
        contents = Util::ReadUnicodFile( css_document );
    }

    catch ( std::exception& )
    {
        // If the file doesn't exist or some other
        // snafu, then there are obviously no links.
        return boost::unordered_set< fs::path > ();
    }

    boost::unordered_set< fs::path > linked_resources;
    fs::path css_doc_folder = css_document.parent_path();

    // We have to erase all comments first, because we don't want
    // to count commented-out links.
    boost::erase_all_regex( contents, boost::regex( "/\\*.*?\\*/" ) );

    std::string::const_iterator start = contents.begin();
    std::string::const_iterator end   = contents.end(); 

    boost::match_results< std::string::const_iterator > matches; 
    boost::regex expression( 
        "(?:(?:src|background|background-image)\\s*:|@import)\\s*"
        "[^;\\}\\(\"']*"
        "(?:"
            "url\\([\"']?([^\\)\"']+)[\"']?\\)"
            "|"
            "[\"']([^\"']+)[\"']"
        ")"
        "[^;\\}]*"
        "(?:;|\\})" );

    while ( boost::regex_search( start, end, matches, expression ) ) 
    {   
        start = matches[ 0 ].second;

        for ( int i = 1; i < matches.size(); ++i )
        {             
            std::string matched_path = matches[ i ];
            boost::trim( matched_path );

            if ( matched_path.empty() )

                continue;

            fs::path resource_path = Util::Utf8PathToBoostPath( matched_path );        
            linked_resources.insert( Util::NormalizePath( css_doc_folder / resource_path ) );  
        }      
    }

    return linked_resources;
}


boost::unordered_set< fs::path > ReachabilityAnalysis::GetPathsFromItems( 
    const boost::unordered_map< std::string, fs::path > &manifest_items )
{
    boost::unordered_set< fs::path > manifest_paths;

    // Using boost_foreach gives us a warning here,
    // so we use the normal for loop 
    for ( boost::unordered_map< std::string, fs::path >::const_iterator it = manifest_items.begin();
          it != manifest_items.end(); ++it )
    {
        manifest_paths.insert( it->second );
    }

    return manifest_paths;
}


bool ReachabilityAnalysis::IsFilesystemPath( const fs::path &path )
{   
    // If the attribute value in a href has ':', it's because
    // this is a non-filesystem path. We already removed the
    // "file://" prefix if it existed.
    
    return path.string().find( ':' ) == std::string::npos;
}


} // namespace FlightCrew

