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
#ifndef ELEMENTCOUNTONEVALIDATOR_H
#define ELEMENTCOUNTONEVALIDATOR_H

#include "XmlValidator.h"
#include <QName.h>

namespace xe = XercesExt;

namespace FlightCrew
{

class ElementCountOneValidator : public XmlValidator
{

protected:
    
    /**
     * Verifies that there is only one element of the specified type in the document.
     * We try to report the location of the error as the location
     * of a parent element that is supposed to contain the required element.
     *
     * @param element_qname The name of the element whose correct count we want to check.
     * @param document The document being validated.
     * @return A list of validation results.
     */
    std::vector<Result> VerifyElementCountOne( const xe::QName &element_qname,
                                               const xe::QName &parent_qname,
                                               const xc::DOMDocument &document );
};

} // namespace FlightCrew

#endif // ELEMENTCOUNTONEVALIDATOR_H