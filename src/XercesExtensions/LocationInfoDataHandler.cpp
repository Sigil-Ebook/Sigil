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

#include "LocationInfoDataHandler.h"
#include "NodeLocationInfo.h"

namespace XercesExt
{

void LocationInfoDataHandler::handle( DOMOperationType operation,
                                      const XMLCh *const,
                                      void *data,
                                      const xc::DOMNode*,
                                      xc::DOMNode* )
{
    NodeLocationInfo* location_info = static_cast< NodeLocationInfo* >( data );

    switch ( operation )
    {
        case NODE_DELETED:
            delete location_info;
            break;

        // Node deletion is the only thing we care about,
        // we won't be cloning nodes.
        default:
            break;
    }
}

}
