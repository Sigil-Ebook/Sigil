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
#include "TempFolder.h"
#include <cstdio>

namespace FlightCrew
{

static const char *UNIQUE_PATH_MODEL = "%%%%-%%%%-%%%%-%%%%-%%%%-%%%%-%%%%-%%%%";
static const char *FC_TEMP_FOLDER    = "flightcrew";

TempFolder::TempFolder()
    : m_PathToFolder( GetNewTempFolderPath() )
{
    fs::create_directories( m_PathToFolder );
}


TempFolder::~TempFolder()
{
    fs::remove_all( m_PathToFolder );
}


fs::path TempFolder::GetPath()
{
    return m_PathToFolder;
}


fs::path TempFolder::GetNewTempFolderPath()
{
#ifdef _WIN32
    // The specified "c:\\temp" path will only be used
    // when the TMP environment variable is undefined
    // http://msdn.microsoft.com/en-us/library/hs3e7355.aspx
    wchar_t *tmp_name = _wtempnam( L"c:\\temp", L"unused" );
    fs::path main_temp_folder = fs::path( tmp_name ).parent_path();
    free( tmp_name );
#else
    // GCC bitches and moans if we use tempnam(), so
    // we'll just use the P_tmpdir macro. We can't use
    // that on Win because it points to the drive root there
    // instead of the system temp folder
    // http://www.delorie.com/gnu/docs/glibc/libc_295.html
    fs::path main_temp_folder = fs::path( P_tmpdir );
#endif    

    return main_temp_folder / fs::path( FC_TEMP_FOLDER ) / fs::unique_path( UNIQUE_PATH_MODEL );
}


} // namespace FlightCrew
