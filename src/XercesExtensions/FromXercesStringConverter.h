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

#pragma once
#ifndef FROMXERCESSTRINGCONVERTER_H
#define FROMXERCESSTRINGCONVERTER_H

#include <xercesc/util/XMLString.hpp>
#include <string>

namespace xc = XERCES_CPP_NAMESPACE;

namespace XercesExt
{
    class FromXercesStringConverter
    {
    public:

        FromXercesStringConverter( const XMLCh* const xerces_string );

        ~FromXercesStringConverter();

        const char* Utf8String() const;

        std::string StandardString() const;

    private:

        char* m_Utf8String;
    };
}

#define fromX( str ) XercesExt::FromXercesStringConverter( (str) ).StandardString()

#endif // FROMXERCESSTRINGCONVERTER_H
