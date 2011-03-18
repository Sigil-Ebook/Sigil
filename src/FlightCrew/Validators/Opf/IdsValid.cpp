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
#include "IdsValid.h"
#include <ToXercesStringConverter.h>
#include <FromXercesStringConverter.h>
#include <XmlUtils.h>

namespace FlightCrew
{

std::vector< Result > IdsValid::ValidateXml(
    const xc::DOMDocument &document,
    const fs::path& )
{
    std::vector< xc::DOMElement* > elements = xe::GetElementsByQName( 
        document, QName( "*", "*" ) );

    std::vector< Result > results;

    foreach( xc::DOMElement* element, elements )
    {
        if ( !element->hasAttribute( toX( "id" ) ) )

            continue;

        std::string id = fromX( element->getAttribute( toX( "id" ) ) );

        if ( !ValidId( id ) )
        {
            results.push_back( 
                ResultWithNodeLocation( ERROR_XML_BAD_ID_VALUE, *element )
                .AddMessageArgument( id )
                );
        }       
    }

    return results;
}

// The specification of what constitutes a valid ID name
// can be found here:
// http://www.w3.org/TR/REC-xml-names/#NT-NCName
bool IdsValid::ValidId( const std::string &id )
{
    if ( id.empty() )

        return false;

    std::string::const_iterator iter = id.begin();
    bool first_code_point = true;

    while ( iter != id.end() )
    {
        utf8::uint32_t code_point = utf8::next( iter, id.end() );

        if ( first_code_point )
        {
            if ( !ValidIdNameStartChar( code_point )  )

                return false;

            first_code_point = false;
        }

        else
        {
            if ( !ValidIdNameChar( code_point ) )

                return false;
        }
    }

    return true;
}


bool IdsValid::ValidIdNameStartChar( utf8::uint32_t code_point )
{
    return 
        code_point == '_'                                  ||
        ( 'A'     <= code_point && code_point <= 'Z'     ) ||
        ( 'a'     <= code_point && code_point <= 'z'     ) ||
        ( 0xC0    <= code_point && code_point <= 0xD6    ) || 
        ( 0xD8    <= code_point && code_point <= 0xF6    ) || 
        ( 0xF8    <= code_point && code_point <= 0x2FF   ) || 
        ( 0x370   <= code_point && code_point <= 0x37D   ) || 
        ( 0x37F   <= code_point && code_point <= 0x1FFF  ) || 
        ( 0x200C  <= code_point && code_point <= 0x200D  ) || 
        ( 0x2070  <= code_point && code_point <= 0x218F  ) || 
        ( 0x2C00  <= code_point && code_point <= 0x2FEF  ) || 
        ( 0x3001  <= code_point && code_point <= 0xD7FF  ) || 
        ( 0xF900  <= code_point && code_point <= 0xFDCF  ) || 
        ( 0xFDF0  <= code_point && code_point <= 0xFFFD  ) || 
        ( 0x10000 <= code_point && code_point <= 0xEFFFF );
}

bool IdsValid::ValidIdNameChar( utf8::uint32_t code_point )
{
    return 
        ValidIdNameStartChar( code_point )                 ||
        code_point == '-'                                  || 
        code_point == '.'                                  || 
        code_point == 0xB7                                 || 
        ( '0'     <= code_point && code_point <= '9'     ) ||
        ( 0x0300  <= code_point && code_point <= 0x036F  ) ||
        ( 0x203F  <= code_point && code_point <= 0x2040  );
}

} // namespace FlightCrew

