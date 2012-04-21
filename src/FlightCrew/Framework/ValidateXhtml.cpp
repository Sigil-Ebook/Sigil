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

#include <stdafx.h>
#include <vector>
#include <XercesInit.h>
#include "Result.h"
#include "Validators/Xml/WellFormedXml.h"
#include "Validators/Xml/UsesUnicode.h"
#include "Misc/Utilities.h"
#include "Validators/Xhtml/SatisfiesXhtmlSchema.h"
#include "Validators/Xhtml/UsesCorrectDtd.h"

namespace FlightCrew
{

std::vector< Result > ValidateXhtml( const fs::path &filepath )
{
    xe::XercesInit init;

    if ( !fs::exists( filepath ) )

        boost_throw( FileDoesNotExistEx() << ei_FilePath( Util::BoostPathToUtf8Path( filepath ) ) );

    std::vector< Result > results;
    Util::Extend( results, SatisfiesXhtmlSchema().ValidateFile( filepath ) );
    Util::Extend( results, UsesUnicode()         .ValidateFile( filepath ) );
    Util::Extend( results, UsesCorrectDtd()      .ValidateFile( filepath ) );
    return Util::SortedInPlace( results );
}

} // namespace FlightCrew