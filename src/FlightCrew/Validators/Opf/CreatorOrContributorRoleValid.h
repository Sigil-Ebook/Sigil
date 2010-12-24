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
#ifndef CREATORORCONTRIBUTORROLEVALID_H
#define CREATORORCONTRIBUTORROLEVALID_H

#include "../XmlValidator.h"
#include <boost/unordered/unordered_set_fwd.hpp>

namespace FlightCrew
{

class CreatorOrContributorRoleValid : public XmlValidator
{
public:

    virtual std::vector<Result> ValidateXml( 
        const xc::DOMDocument &document,
        const fs::path &filepath = fs::path() );

private:

    boost::unordered_set< std::string > GetRelatorSet();
    
};

} // namespace FlightCrew

#endif // CREATORORCONTRIBUTORROLEVALID_H

