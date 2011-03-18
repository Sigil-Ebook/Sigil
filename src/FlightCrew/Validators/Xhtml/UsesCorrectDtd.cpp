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
#include "Misc/Utilities.h"
#include "Result.h"
#include "UsesCorrectDtd.h"


namespace FlightCrew
{

static const int NUM_PEEK_CHARS_FOR_DTD = 300;
const std::string XHTML11_SYSTEM_ID = "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd";
const std::string XHTML11_PUBLIC_ID = "-//W3C//DTD XHTML 1.1//EN";


std::vector< Result > UsesCorrectDtd::ValidateFile( const fs::path &filepath )
{
    std::vector< Result > results;

    try
    {
        std::string inital_chars = Util::GetFirstNumCharsFromFile( filepath, NUM_PEEK_CHARS_FOR_DTD );

        if ( !DtdCorrect( inital_chars ) )
        {
            int dtd_start  = DtdStartLocation( inital_chars );
            int error_line = dtd_start == -1 ? -1 : Util::LineOfCharIndex( inital_chars, dtd_start ); 

            results.push_back( Result( ERROR_XHTML_BAD_DTD )
                .SetErrorLine( error_line ) );
        }
    }

    catch ( ExceptionBase& )
    {
        results.clear();
        results.push_back( Result( UNABLE_TO_PERFORM_VALIDATION ) );        
    }  

    return Util::AddPathToResults( results, filepath );
}


bool UsesCorrectDtd::DtdCorrect( const std::string &inital_chars )
{
    boost::regex expression( "<!DOCTYPE[^\"']+(?:\"|')([^\"']+)(?:\"|')\\s*(?:\"|')([^\"']+)(?:\"|')" );
    boost::match_results< std::string::const_iterator > matches;

    // If no dtd was found, we return true... the standard
    // says the dtd is optional.
    if ( !boost::regex_search( inital_chars, matches, expression ) )

        return true;
    
    if ( matches[ 1 ] == XHTML11_PUBLIC_ID &&
         matches[ 2 ] == XHTML11_SYSTEM_ID )
    {
        return true;
    }

    return false;    
}


int UsesCorrectDtd::DtdStartLocation( const std::string &inital_chars )
{
    return static_cast< int >( inital_chars.find( "<!DOCTYPE" ) );
}

} // namespace FlightCrew
