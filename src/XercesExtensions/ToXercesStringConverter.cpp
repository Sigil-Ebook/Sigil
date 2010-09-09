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

#include <xercesc/util/TransService.hpp>
#include "ToXercesStringConverter.h"

namespace XercesExt
{

ToXercesStringConverter::ToXercesStringConverter( const std::string &utf8_string )
{
    if ( utf8_string.length() > 0 )
    {
        xc::TranscodeFromStr transcoder( 
            (const XMLByte*) utf8_string.c_str(), utf8_string.length(), "UTF-8" );

        m_XercesString = transcoder.adopt();
    }

    else
    {
        m_XercesString = NULL;
    }
}


ToXercesStringConverter::ToXercesStringConverter( const char* const utf8_string )
{
    if ( utf8_string )
    {
        size_t string_length = strlen( utf8_string );

        if ( string_length > 0 )
        {
            xc::TranscodeFromStr transcoder( 
                (const XMLByte*) utf8_string, string_length, "UTF-8" );

            m_XercesString = transcoder.adopt();
        }

        else
        {
            m_XercesString = NULL;
        }
    }

    else
    {
        m_XercesString = NULL;
    }
}


ToXercesStringConverter::~ToXercesStringConverter()
{
    if ( m_XercesString )
    
        xc::XMLString::release( &m_XercesString );
}


const XMLCh* ToXercesStringConverter::XercesString() const
{
    return m_XercesString;
}

} // namespace XercesExt

