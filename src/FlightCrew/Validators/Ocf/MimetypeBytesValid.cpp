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
#include "MimetypeBytesValid.h"
#include "Misc/Utilities.h"

static const std::string MIMETYPE_BYTES = "mimetypeapplication/epub+zipPK";

namespace FlightCrew
{

std::vector< Result > MimetypeBytesValid::ValidateFile( const fs::path &filepath )
{
    fs::ifstream file( filepath, std::ios::in | std::ios::binary );

    // As per the OCF spec, bytes 30 to 60 of an epub archive 
    // have to match what we have in MIMETYPE_BYTES
    char bytes30to60[ 30 + 1 ] = { 0 };
    file.seekg( 30 );
    file.read( bytes30to60, 30 );

    std::string string_bytes( bytes30to60 );
    std::vector< Result > results;

    if ( string_bytes != MIMETYPE_BYTES )

        results.push_back( Result( ERROR_EPUB_MIMETYPE_BYTES_INVALID ) );

    return results;
}

} // namespace FlightCrew