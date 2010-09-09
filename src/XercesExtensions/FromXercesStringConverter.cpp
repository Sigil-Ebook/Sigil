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
#include "FromXercesStringConverter.h"

namespace XercesExt
{

FromXercesStringConverter::FromXercesStringConverter( const XMLCh* const xerces_string )
{
    if ( xerces_string && 
         xc::XMLString::stringLen( xerces_string ) > 0 )
    {
        xc::TranscodeToStr transcoder( xerces_string, "UTF-8" );

        m_Utf8String = (char*) transcoder.adopt();
    }

    else
    {
        m_Utf8String = NULL;
    }
}


FromXercesStringConverter::~FromXercesStringConverter()
{
    if ( m_Utf8String )
        
        xc::XMLString::release( &m_Utf8String );
}


const char* FromXercesStringConverter::Utf8String() const
{
    return m_Utf8String;
}


std::string FromXercesStringConverter::StandardString() const
{
    return m_Utf8String ? std::string( m_Utf8String ) : std::string();
}

} // namespace XercesExt
