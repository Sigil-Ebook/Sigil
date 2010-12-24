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

#include "zipios++/zipios-config.h"

#include "zipios++/meta-iostreams.h"
#include <memory>
#include <stdlib.h>
#include <boost/scoped_ptr.hpp>

#include "zipios++/zipfile.h"
#include "zipios++/zipinputstream.h"

#include "zipios++/zipextraction.h"
#include "zipios++/fcollexceptions.h"

namespace zipios 
{


void WriteEntryToFile( const std::istream &stream, const fs::path &filepath )
{
    fs::ofstream ofs( filepath, ios::out | ios::binary );
    ofs << stream.rdbuf();
    ofs.close();
}


void CreateFilepath( const fs::path &filepath )
{
    if ( filepath.empty() )

        throw IOException();

    if ( !fs::exists( filepath.parent_path() ) )

        fs::create_directories( filepath.parent_path() );

    if ( fs::is_regular_file( filepath ) )
    
        fs::ofstream( filepath );

    else if ( fs::is_directory( filepath ) )

        fs::create_directory( filepath );
}


void ExtractZipToFolder( const fs::path &path_to_zip, const fs::path &path_to_folder )
{
    ZipFile zip( path_to_zip );

    ConstEntries entries = zip.entries();
    for ( ConstEntries::iterator it = entries.begin(); it != entries.end(); ++it )
    {
        boost::scoped_ptr< std::istream > stream( zip.getInputStream( *it ) );

        fs::path new_file_path = path_to_folder / (*it)->getName();

        CreateFilepath( new_file_path );
        WriteEntryToFile( *stream, new_file_path );
    }
}


} // namespace zipios
