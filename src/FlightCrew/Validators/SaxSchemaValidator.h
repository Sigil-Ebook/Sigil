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
#ifndef SAXSCHEMAVALIDATOR_H
#define SAXSCHEMAVALIDATOR_H

#include <xercesc/framework/MemBufInputSource.hpp>
namespace XERCES_CPP_NAMESPACE { class SAX2XMLReader; class MemBufInputSource; };
namespace xc = XERCES_CPP_NAMESPACE;
#include "IValidator.h"

namespace FlightCrew
{

class SaxSchemaValidator : public IValidator
{

protected:

    std::vector< Result > ValidateAgainstSchema( 
        const fs::path &filepath, 
        const std::string &external_schema_location,
        const std::vector< const xc::MemBufInputSource* > &schemas );

private:

    void LoadSchemas( xc::SAX2XMLReader &parser,
                      const std::string &external_schema_location,
                      const std::vector< const xc::MemBufInputSource* > &schemas );
};

} // namespace FlightCrew

#endif // SAXSCHEMAVALIDATOR_H
