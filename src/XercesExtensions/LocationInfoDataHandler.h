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
#ifndef LOCATIONINFODATAHANDLER_H
#define LOCATIONINFODATAHANDLER_H

#include <xercesc/dom/DOMUserDataHandler.hpp>
namespace xc = XERCES_CPP_NAMESPACE;

namespace XercesExt
{

class LocationInfoDataHandler :
        public xc::DOMUserDataHandler
{
public:

    void handle( DOMOperationType operation,
                 const XMLCh *const key,
                 void *data,
                 const xc::DOMNode *src,
                 xc::DOMNode *dst );
};

}

#endif // LOCATIONINFODATAHANDLER_H
