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
#ifndef XMLVALIDATOR_H
#define XMLVALIDATOR_H

#include <xercesc/dom/DOMDocument.hpp>
namespace xc = XERCES_CPP_NAMESPACE;
#include "IValidator.h"
#include "Result.h"

namespace FlightCrew
{

class XmlValidator : public IValidator
{
public:

    virtual std::vector<Result> ValidateFile( const fs::path &filepath );

    virtual std::vector<Result> ValidateXml( 
        const xc::DOMDocument &document,
        const fs::path &filepath = fs::path() ) = 0;

    virtual ~XmlValidator() {};

protected:

    Result ResultWithNodeLocation( ResultId error_id, 
                                   const xc::DOMNode &node );
};

} // namespace FlightCrew

#endif // XMLVALIDATOR_H
