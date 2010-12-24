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
#ifndef METADATAALLOWEDCHILDREN_H
#define METADATAALLOWEDCHILDREN_H

#include "../AllowedChildrenValidator.h"

namespace FlightCrew
{
    
/**
 * Checks that the <metadata> has only allowed children.
 */
class MetadataAllowedChildren : public AllowedChildrenValidator
{
public:

    virtual std::vector<Result> ValidateXml( 
        const xc::DOMDocument &document,
        const fs::path &filepath = fs::path() );

private:

    /**
     * Validates the metadata children when using dc- and x-metadata children.
     *
     * @param children The children of the metadata element.
     */
    std::vector<Result> ValidateDCXChildrenSubset( std::vector< xc::DOMElement* > children );

    /**
     * Validates the metadata children when using standard children.
     *
     * @param children The children of the metadata element.
     */
    std::vector<Result> ValidateStandardChildren( std::vector< xc::DOMElement* > children );       
};

} // namespace FlightCrew

#endif // METADATAALLOWEDCHILDREN_H
