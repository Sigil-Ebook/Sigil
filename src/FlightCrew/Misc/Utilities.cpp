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
#include "Utilities.h"
#include <fstream>
#include <utf8.h>
#include <xercesc/util/TransService.hpp>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <XmlUtils.h>
#include <LocationAwareDOMParser.h>
#include <boost/filesystem/detail/utf8_codecvt_facet.hpp>
#include <ToXercesStringConverter.h>


namespace FlightCrew
{

namespace Util
{

std::string ReadUnicodFile( const fs::path &filepath )
{
    fs::ifstream file( filepath, std::ios::in | std::ios::binary );

    if ( !file.is_open() )

          boost_throw( FileDoesNotExistEx() << ei_FilePath( BoostPathToUtf8Path( filepath ) ) );

    std::vector< char > contents( (std::istreambuf_iterator< char>( file )), 
                                   std::istreambuf_iterator< char>() );

    // May as well be empty
    if ( contents.size() < 2 )
    
        return std::string();

    if ( utf8::is_valid( contents.begin(), contents.end() ) )

        return std::string( contents.begin(), contents.end() );

    // UTF-16BE
    if ( static_cast< unsigned char >( contents[ 0 ] ) == 0xfeU &&
         static_cast< unsigned char >( contents[ 1 ] ) == 0xffU )
    {
        xc::TranscodeFromStr transcoder( 
            (const XMLByte*) &( *contents.begin() ), contents.size() , "UTF-16BE" );

        xc::TranscodeToStr transcoder_utf8( transcoder.str(), "UTF-8" );

        return std::string( (char*) transcoder_utf8.str() );
    }

    // UTF-16LE
    else if ( static_cast< unsigned char >( contents[ 0 ] ) == 0xffU &&
              static_cast< unsigned char >( contents[ 1 ] ) == 0xfeU )
    {
        xc::TranscodeFromStr transcoder( 
            (const XMLByte*) &( *contents.begin() ), contents.size(), "UTF-16LE" );

        xc::TranscodeToStr transcoder_utf8( transcoder.str(), "UTF-8" );

        return std::string( (char*) transcoder_utf8.str() );
    }

    else
    {
        boost_throw( FileNotInUnicodeEx() << ei_FilePath( filepath.generic_string() ) );
    }
}


std::string GetFirstNumChars( const std::string &string, uint num_chars )
{
    if ( string.empty() )

        return std::string();

    uint string_size   = static_cast< unsigned int >( string.size() );
    uint chars_to_copy = string_size < num_chars ? string_size : num_chars;

    std::string::const_iterator it = string.begin();
    std::advance( it, chars_to_copy );

    std::string line;
    line.resize( num_chars );

    std::copy( string.begin(), it, line.begin() );

    return line;
}


std::string GetFirstNumCharsFromFile( const fs::path &filepath, uint num_chars )
{
    try
    {
        // TODO: Let's not load the entire file
        std::string contents = Util::ReadUnicodFile( filepath );
        return GetFirstNumChars( contents, num_chars );
    }

    catch ( FileNotInUnicodeEx& )
    {
        return std::string();
    }  
}


int LineOfCharIndex( const std::string &string, unsigned int char_index )
{
    // \x0A is the line feed char, 
    // \x0D is the carriage return char

    std::string line_marker;

    if ( string.find( "\x0A" ) != std::string::npos )

        line_marker = "\x0A";

    else 

        line_marker = "\x0D";

    size_t search_start = 0;
    int count = 1;

    while ( true )
    {
        size_t position = string.find( line_marker, search_start );

        if ( position == std::string::npos || position > char_index )

            break;

        ++count;        
        search_start = position + 1;
    }

    return count;
}


boost::shared_ptr< xc::DOMDocument > RaiiWrapDocument( xc::DOMDocument *document )
{
    return boost::shared_ptr< xc::DOMDocument >( document, XercesExt::XercesDeallocator< xc::DOMDocument > );
}


boost::shared_ptr< xc::DOMDocument > LoadXmlDocument( const fs::path &filepath )
{
    if ( filepath.empty() )

        boost_throw( XercesParsingError() );  

    xe::LocationAwareDOMParser parser;

    // This scanner ignores schemas and DTDs
    parser.useScanner( xc::XMLUni::fgWFXMLScanner );
    parser.setValidationScheme( xc::AbstractDOMParser::Val_Never );
    parser.setDoNamespaces( true );

    parser.parse( toX( BoostPathToUtf8Path( filepath ) ) );

    xc::DOMDocument *document = parser.adoptDocument();

    if ( !document )

        boost_throw( XercesParsingError() );        

    return RaiiWrapDocument( document );
}


boost::shared_ptr< xc::DOMDocument > LoadXhtmlDocument( const fs::path &filepath )
{
    if ( filepath.empty() )

        boost_throw( XercesParsingError() );  

    xe::LocationAwareDOMParser parser;

    parser.setDoSchema(             false );
    parser.setLoadSchema(           false );
    parser.setSkipDTDValidation(    true  );
    parser.setDoNamespaces(         true  );
    parser.useCachedGrammarInParse( true  );

    parser.setValidationScheme( xc::AbstractDOMParser::Val_Never );

    // This scanner ignores schemas, but does use DTDs
    parser.useScanner( xc::XMLUni::fgDGXMLScanner );

    const xc::MemBufInputSource input( XHTML11_FLAT_DTD,
                                       XHTML11_FLAT_DTD_LEN,
                                       toX( XHTML11_FLAT_DTD_ID ) );

    parser.loadGrammar( input, xc::Grammar::DTDGrammarType, true ); 

    parser.parse( toX( BoostPathToUtf8Path( filepath ) ) );

    xc::DOMDocument *document = parser.adoptDocument();

    if ( !document )

        boost_throw( XercesParsingError() );        

    return RaiiWrapDocument( document );
}


char CharFromTwoHex( std::string two_hex_chars )
{
    std::istringstream stream( two_hex_chars );
    int int_value;
    stream >> std::hex >> int_value;

    return static_cast< char >( int_value );
}


std::string UrlDecode( const std::string &encoded_url )
{
    std::string decoded;
    decoded.reserve( encoded_url.size() );

    uint i = 0;
    while ( i < encoded_url.size() )
    {
        if ( encoded_url[ i ] == '%' &&
             i + 2 < encoded_url.size() )
        {
            decoded += CharFromTwoHex( encoded_url.substr( i + 1, 2 ) );
            i += 3;            
        }

        else 
        {
            decoded += encoded_url[ i ];
            ++i;
        }
    }

    return decoded;
}


std::string GetUrlFragment( const std::string &decoded_url )
{
    int hash_location = static_cast< int >( decoded_url.find( '#' ) );

    if ( hash_location != -1 && 
         hash_location + 1 < static_cast< int >( decoded_url.size() ) )
    {
        return decoded_url.substr( hash_location + 1, decoded_url.size() );
    }
    
    return std::string();
}


std::string UrlWithoutFragment( const std::string &decoded_url )
{
    int hash_location = static_cast< int >( decoded_url.find( '#' ) );

    if ( hash_location != -1 )
    
        return decoded_url.substr( 0, hash_location );    
    
    return decoded_url;
}


std::string UrlWithoutFileScheme( const std::string &decoded_url )
{
    if ( boost::starts_with( decoded_url, "file://" ) )

        return boost::erase_first_copy( decoded_url, "file://" );

    return decoded_url;
}



fs::path NormalizePath( const fs::path &filepath )
{
    std::string path_string = BoostPathToUtf8Path( filepath );        
    boost::regex up_dir_regex( "[^/]+/\\.\\./" );

    while ( true )
    {
        std::string old_path = path_string;
        path_string = boost::erase_all_regex_copy( path_string, up_dir_regex );

        if ( path_string == old_path )

            break;
    }

    boost::regex current_dir_regex( "/\\./" );

    while ( true )
    {
        std::string old_path = path_string;
        path_string = boost::erase_all_regex_copy( path_string, current_dir_regex );

        if ( path_string == old_path )

            break;
    }

    return Utf8PathToBoostPath( path_string );    
}


fs::path Utf8PathToBoostPath( const std::string &utf8_path )
{
    if ( utf8_path.empty() )

        return fs::path();

    if ( !utf8::is_valid( utf8_path.begin(), utf8_path.end() ) )
        
        boost_throw( PathNotInUtf8() << ei_FilePath( utf8_path ) );    

    boost::filesystem::detail::utf8_codecvt_facet utf8facet;
    return fs::path( utf8_path, utf8facet );
}


std::string BoostPathToUtf8Path( const fs::path &filepath )
{
    if ( filepath.empty() )

        return std::string();

    boost::filesystem::detail::utf8_codecvt_facet utf8facet;
    return filepath.generic_string( utf8facet );
}


// Taking by const ref and making a copy could be costly,
// but you know what they say about premature optimization.
// If the profiler ends up screaming at this, then we'll refactor.
std::vector< Result > AddPathToResults( const std::vector< Result > &results, const fs::path &filepath )
{
    std::vector< Result > mod_results = results;

    foreach( Result &result, mod_results )
    {
        if ( result.GetFilepath().empty() )

            result.SetFilepath( BoostPathToUtf8Path( filepath ) );
    }

    return mod_results;
}


} // namespace Util

} // namespace FlightCrew
