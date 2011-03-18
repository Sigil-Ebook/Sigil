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
#ifndef DOMSCHEMAVALIDATOR_H
#define DOMSCHEMAVALIDATOR_H

#include <xercesc/framework/MemBufInputSource.hpp>
namespace XERCES_CPP_NAMESPACE { class MemBufInputSource; };
namespace xc = XERCES_CPP_NAMESPACE;
namespace XercesExt { class LocationAwareDOMParser; }
namespace xe = XercesExt;
#include "IValidator.h"

namespace FlightCrew
{

class DomSchemaValidator : public IValidator
{

protected:

    std::vector< Result > ValidateAgainstSchema( 
        const fs::path &filepath, 
        const std::string &external_schema_location,
        const std::vector< const xc::MemBufInputSource* > &schemas,
        const std::vector< const xc::MemBufInputSource* > &dtds );

private:

    void LoadSchemas( 
        xe::LocationAwareDOMParser &parser,
        const std::string &external_schema_location,
        const std::vector< const xc::MemBufInputSource* > &schemas,
        const std::vector< const xc::MemBufInputSource* > &dtds );
};

} // namespace FlightCrew

#endif // DOMSCHEMAVALIDATOR_H
