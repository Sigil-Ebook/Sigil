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
#ifndef ATTRIBUTESPRESENTVALIDATOR_H
#define ATTRIBUTESPRESENTVALIDATOR_H

#include "XmlValidator.h"
#include <QName.h>

namespace xe = XercesExt;

namespace FlightCrew
{

class AttributesPresentValidator : public XmlValidator
{

protected:
    
    /**
     * Checks that the attributes of all the elements matching the given name
     * are allowed.
     *
     * @param element_qname The name of the element whose attributes we want to check.
     * @param attribute_qnames A list of allowed attributes.
     * @param document The document being validated.
     * @return A list of validation results, empty if no problem were found.
     */
    std::vector< Result > HasOnlyAllowedAttributes( const xe::QName &element_qname,
                                                    const std::vector< xe::QName > &attribute_qnames,
                                                    const xc::DOMDocument &document );

    /**
     * Checks that all the elements matching the given name have all their
     * mandatory attributes.
     *
     * @param element_qname The name of the element whose attributes we want to check.
     * @param attribute_qnames A list of mandatory attributes.
     * @param document The document being validated.
     * @return A list of validation results, empty if no problem were found.
     */
    std::vector< Result > HasMandatoryAttributes( const xe::QName &element_qname,
                                                  const std::vector< xe::QName > &attribute_qnames,
                                                  const xc::DOMDocument &document );

private:

    bool IsAllowedAttribute( const xc::DOMAttr &attribute,
                             const std::vector< xe::QName > &allowed_attribute_qnames );
};

} // namespace FlightCrew

#endif // ATTRIBUTESPRESENTVALIDATOR_H