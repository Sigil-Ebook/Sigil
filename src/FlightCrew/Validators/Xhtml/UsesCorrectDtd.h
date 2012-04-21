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
#ifndef USESCORRECTDTD_H
#define USESCORRECTDTD_H

#include "../IValidator.h"

namespace FlightCrew
{

class UsesCorrectDtd : public IValidator
{
public:

    std::vector< Result > ValidateFile( const fs::path &filepath );

private:

    bool DtdCorrect( const std::string &inital_chars );

    int DtdStartLocation( const std::string &inital_chars );
};

} // namespace FlightCrew

#endif // USESCORRECTDTD_H
