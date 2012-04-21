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
#include <vector>
#include "Result.h"
#include "Misc/TempFolder.h"
#include "Misc/Utilities.h"
#include "Validators/Xml/WellFormedXml.h"
#include <XmlUtils.h>
#include <XercesInit.h>
#include <FromXercesStringConverter.h>
#include <ToXercesStringConverter.h>
#include <xercesc/util/XMLUri.hpp>
#include "flightcrew_p.h"
#include "Validators/Ocf/ContainerSatisfiesSchema.h"
#include "Validators/Ocf/EncryptionSatisfiesSchema.h"
#include "Validators/Ocf/SignaturesSatisfiesSchema.h"
#include "Validators/Ocf/ContainerListsOpf.h"
#include "Validators/Ocf/ContainerListedOpfPresent.h"
#include "Validators/Ocf/MimetypeBytesValid.h"
#include "Validators/Xml/UsesUnicode.h"

namespace FlightCrew
{

const std::string CONTAINER_XML_NAMESPACE = "urn:oasis:names:tc:opendocument:xmlns:container";


std::vector< Result > ValidateMetaInf( const fs::path &path_to_meta_inf )
{
    fs::path container_xml(  path_to_meta_inf / "container.xml"  );
    fs::path signatures_xml( path_to_meta_inf / "signatures.xml" );
    fs::path manifest_xml(   path_to_meta_inf / "manifest.xml"   );
    fs::path rights_xml(     path_to_meta_inf / "rights.xml"     );
    fs::path metadata_xml(   path_to_meta_inf / "metadata.xml"   );
    fs::path encryption_xml( path_to_meta_inf / "encryption.xml" );

    std::vector< Result > results;

    if ( fs::exists( container_xml ) )
    {
        Util::Extend( results, ContainerSatisfiesSchema() .ValidateFile( container_xml ) );
        Util::Extend( results, ContainerListsOpf()        .ValidateFile( container_xml ) );
        Util::Extend( results, ContainerListedOpfPresent().ValidateFile( container_xml ) );
    }

    else
    {
        results.push_back( Result( ERROR_EPUB_NO_CONTAINER_XML ) );
    }
    
    if ( fs::exists( encryption_xml ) )
     
        Util::Extend( results, EncryptionSatisfiesSchema().ValidateFile( encryption_xml ) );

    if ( fs::exists( signatures_xml ) )
    
        Util::Extend( results, SignaturesSatisfiesSchema().ValidateFile( signatures_xml ) );

    std::vector< fs::path > all_files;
    all_files.push_back( container_xml  );
    all_files.push_back( signatures_xml );
    all_files.push_back( encryption_xml );
    all_files.push_back( manifest_xml   );
    all_files.push_back( rights_xml     );
    all_files.push_back( metadata_xml   );

    foreach( fs::path file, all_files )
    {
        if ( fs::exists( file ) )

            Util::Extend( results, UsesUnicode().ValidateFile( file ) );        
    }

    // i starts at 3 because we already (implicitly) checked well-formedness  
    // for container.xml, signatures.xml and encryption.xml so 
    // we don't want to check it again.
    for ( uint i = 3; i < all_files.size(); ++i )
    {
        if ( fs::exists( all_files[ i ] ) )

            Util::Extend( results, WellFormedXml().ValidateFile( all_files[ i ] ) );        
    }    

    // There are some possible duplicates
    Util::RemoveDuplicates( results );
    return results;
}


fs::path GetRelativePathToNcx( const xc::DOMDocument &opf )
{
    std::vector< xc::DOMElement* > items = xe::GetElementsByQName( 
        opf, QName( "item", OPF_XML_NAMESPACE ) );

    foreach( xc::DOMElement* item, items )
    {
        std::string href       = fromX( item->getAttribute( toX( "href" )       ) );
        std::string media_type = fromX( item->getAttribute( toX( "media-type" ) ) );

        if ( xc::XMLUri::isValidURI( true, toX( href ) ) &&
             media_type == NCX_MIME )
        {
            return Util::Utf8PathToBoostPath( Util::UrlDecode( href ) );  
        }
    }

    return fs::path();
}


std::vector< fs::path > GetRelativePathsToXhtmlDocuments( const xc::DOMDocument &opf )
{
    std::vector< xc::DOMElement* > items = xe::GetElementsByQName( 
        opf, QName( "item", OPF_XML_NAMESPACE ) );

    std::vector< fs::path > paths;

    foreach( xc::DOMElement* item, items )
    {
        std::string href       = fromX( item->getAttribute( toX( "href" )       ) );
        std::string media_type = fromX( item->getAttribute( toX( "media-type" ) ) );

        if ( xc::XMLUri::isValidURI( true, toX( href ) ) &&
             ( media_type == XHTML_MIME || media_type == OEB_DOC_MIME ) )
        {                    
            paths.push_back( Util::Utf8PathToBoostPath( Util::UrlDecode( href ) ) );
        }
    }

    return paths;
}


std::vector< Result > DescendToOpf( const fs::path &path_to_opf )
{
    WellFormedXml wf_validator;

    // We can't continue if the OPF is not well-formed.
    // ValidateOpf will take care of returning any 
    // validation results for the OPF
    if ( !wf_validator.ValidateFile( path_to_opf ).empty() )
  
        return std::vector< Result >();

    xc::DOMDocument& opf = wf_validator.GetDocument();
    std::vector< Result > results;

    fs::path opf_parent    = path_to_opf.parent_path();
    fs::path rel_ncx_path  = GetRelativePathToNcx( opf );
    fs::path full_ncx_path = opf_parent / GetRelativePathToNcx( opf );

    if ( !rel_ncx_path.empty() && fs::exists( full_ncx_path ) )

        Util::Extend( results, ValidateNcx( full_ncx_path ) );

    std::vector< fs::path > xhtml_paths = GetRelativePathsToXhtmlDocuments( opf );
    
    foreach( fs::path rel_xhtml_path, xhtml_paths )
    {
        fs::path full_xhtml_path = opf_parent / rel_xhtml_path;

        if ( !rel_xhtml_path.empty() && fs::exists( full_xhtml_path ) )

            Util::Extend( results, ValidateXhtml( full_xhtml_path ) );
    }
    
    return results;
}


fs::path GetRelativeOpfPath( const xc::DOMDocument &content_xml )
{
    std::vector< xc::DOMElement* > rootfiles = xe::GetElementsByQName( 
        content_xml, QName( "rootfile", CONTAINER_XML_NAMESPACE ) );

    foreach( xc::DOMElement* rootfile, rootfiles )
    {
        std::string full_path_attribute = fromX( rootfile->getAttribute( toX( "full-path"  ) ) );
        std::string media_type          = fromX( rootfile->getAttribute( toX( "media-type" ) ) );
        
        if ( media_type == OEBPS_MIME )                 
                       
            return Util::Utf8PathToBoostPath( full_path_attribute );         
    }

    return fs::path();
}


std::vector< Result > DescendToContentXml( const fs::path &path_to_content_xml )
{
    WellFormedXml wf_validator;   

    // We can't continue if content.xml is not well-formed.
    // ValidateMetaInf will take care of returning any 
    // validation results for content.xml
    if ( !wf_validator.ValidateFile( path_to_content_xml ).empty() )
  
        return std::vector< Result >();

    // The base path for the OPF is the publication root path
    fs::path root_path     = path_to_content_xml.parent_path().parent_path();
    fs::path rel_opf_path  = GetRelativeOpfPath( wf_validator.GetDocument() );
    fs::path full_opf_path = root_path / rel_opf_path;

    std::vector< Result > results;

    if ( !rel_opf_path.empty() && fs::exists( full_opf_path ) )
    {
        Util::Extend( results, ValidateOpf( full_opf_path ) );
        Util::Extend( results, DescendToOpf( full_opf_path ) );
    }

    return results;
}

void RemoveBasePathFromResultPaths( std::vector< Result > &results, const fs::path &basepath )
{
    std::string path_prefix = Util::BoostPathToUtf8Path( basepath );

    foreach( Result &result, results )
    {
        std::string result_path = result.GetFilepath();

        if ( !result_path.empty() )
        {
            std::string relative_path = boost::erase_first_copy( result_path, path_prefix );

            // We don't want it to look like an absolute path
            // because it's not.
            if ( boost::starts_with( relative_path, "/" ) )

                boost::erase_first( relative_path, "/" );

            result.SetFilepath( relative_path );
        }        
    }
}


void AddEpubFilenameToResultPaths( std::vector< Result > &results, const std::string &epub_name )
{
    foreach( Result &result, results )
    {
        std::string result_path = result.GetFilepath();

        if ( !result_path.empty() )
        
            result.SetFilepath( epub_name + "/" + result_path );        

        else
        
            result.SetFilepath( epub_name );        
    }
}


std::vector< Result > ValidateEpubRootFolder( const fs::path &root_folder_path )
{
    xe::XercesInit init;

    if ( !fs::exists( root_folder_path ) )

        boost_throw( FileDoesNotExistEx() << ei_FilePath( Util::BoostPathToUtf8Path( root_folder_path ) ) );

    std::vector< Result > results;   
    Util::Extend( results, ValidateMetaInf( root_folder_path / "META-INF" ) );

    fs::path path_to_content_xml = root_folder_path / "META-INF/container.xml";

    if ( !fs::exists( path_to_content_xml ) )
    {
        return results;
    }
    
    Util::Extend( results, DescendToContentXml( path_to_content_xml ) );

    RemoveBasePathFromResultPaths( results, root_folder_path );
    return results;
}


std::vector< Result > ValidateEpub( const fs::path &filepath )
{
    TempFolder temp_folder;

    std::vector< Result > results;

    try
    {
        throw(std::exception());
    }
    
    catch ( std::exception& exception )
    {
        results.push_back( Result( ERROR_EPUB_NOT_VALID_ZIP_ARCHIVE )
                           .SetCustomMessage( exception.what() ) );
        return results;
    }
    
    Util::Extend( results, MimetypeBytesValid().ValidateFile( filepath ) );
    RemoveBasePathFromResultPaths( results, temp_folder.GetPath() );

    Util::Extend( results, ValidateEpubRootFolder( temp_folder.GetPath() ) );    
    AddEpubFilenameToResultPaths( results, Util::BoostPathToUtf8Path( filepath.filename() ) );
    return results;
}

} // namespace FlightCrew
