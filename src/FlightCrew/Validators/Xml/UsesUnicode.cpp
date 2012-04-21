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
#include <fstream>
#include <utf8.h>
#include "UsesUnicode.h"
#include "Misc/Utilities.h"
#include "Result.h"


static const int NUM_PEEK_CHARS_FOR_XML_DECLARATION = 80;

namespace FlightCrew
{

std::vector< Result > UsesUnicode::ValidateFile( const fs::path &filepath )
{
    std::vector< Result > results;

    try
    {
        std::string inital_chars = Util::GetFirstNumCharsFromFile( filepath, NUM_PEEK_CHARS_FOR_XML_DECLARATION );

        if ( FileIsValidUtf8( filepath ) )
        {
            if ( !FileDeclaresUtf8( inital_chars ) )
            {
                results.push_back( Result( ERROR_XML_SPECIFIES_NEITHER_UTF8_NOR_UTF16 )
                    .SetErrorLine( 1 )
                    .AddMessageArgument( GetDeclaredEncoding( inital_chars ) ) );
            }

            // else everything ok
        }

        else
        {            
            if ( !inital_chars.empty() )
            {
                // It's in UTF-16

                if ( !FileDeclaresUtf16( inital_chars ) )
                {
                    results.push_back( Result( ERROR_XML_SPECIFIES_NEITHER_UTF8_NOR_UTF16 )
                        .SetErrorLine( 1 )
                        .AddMessageArgument( GetDeclaredEncoding( inital_chars ) ) );
                }

                // else everything ok
            }

            else
            {
                results.push_back( Result( ERROR_XML_BYTESTREAM_NEITHER_UTF8_NOR_UTF16 )
                    .SetErrorLine( 1 ) );
            }
        }

        return results;
    }

    catch ( ExceptionBase& )
    {
        results.clear();
        results.push_back( Result( UNABLE_TO_PERFORM_VALIDATION ) );        
    }  

    return Util::AddPathToResults( results, filepath );
}


bool UsesUnicode::FileIsValidUtf8( const fs::path &filepath )
{
    fs::ifstream file( filepath, std::ios::in | std::ios::binary );
    std::istreambuf_iterator<char> it( file.rdbuf() );
    std::istreambuf_iterator<char> eos;

    return utf8::is_valid( it, eos );
}


bool UsesUnicode::FileDeclaresUtf8( const std::string &line )
{   
    if ( HasXmlDeclaration( line ) )
    {
        std::string encoding = boost::to_upper_copy( GetDeclaredEncoding( line ) );

        // Empty still counts as utf-8 as per spec
        if ( encoding.empty() || encoding == "UTF-8" )
                
            return true;

        return false;
    }

    // No xml declaration means
    // UTF-8 according to the spec
    return true;
}


bool UsesUnicode::FileDeclaresUtf16( const std::string &line )
{   
    if ( HasXmlDeclaration( line ) )
    {
        if ( boost::to_upper_copy( GetDeclaredEncoding( line ) ) == "UTF-16" )
                
            return true;

        return false;
    }

    return false;
}


bool UsesUnicode::HasXmlDeclaration( const std::string &line )
{
    return boost::contains( line, "<?" );
}


std::string UsesUnicode::GetDeclaredEncoding( const std::string &line )
{
    boost::regex expression( "encoding\\s*=\\s*(?:\"|')([^\"']+)(?:\"|')" );
    boost::match_results< std::string::const_iterator > matches;

    // FIXME: return only when regex_search returns true
    boost::regex_search( line, matches, expression );
    return matches[ 1 ];
}

} //namespace FlightCrew
